#include "Intermediary.hpp"
#include <sstream>

Intermediary::Intermediary(std::string addr,ThreadQueue<Message>& broadcastOut,
                           ThreadQueue<Message>& forkToServer,
                           ThreadQueue<Message>& serverToFork,
                           ThreadQueue<Message>& clientToFork,
                           ThreadQueue<Message>& forkToClient):
                           broadcastOut(broadcastOut),myAdrr(addr), forkToServer(forkToServer), serverToFork(serverToFork),
                           clientToFork(clientToFork), forkToClient(forkToClient){}

void Intermediary::run( Control& serverReady, Control& clientReady){
    logger.log( Entidad::INTERM, TipoLog::INFO, 
    "Interm<"+ myAdrr+">: descubriendo servidores");

    discoverServers(serverReady);

    {
        std::lock_guard<std::mutex> lock(clientReady.mtx); //control
        clientReady.ready = true;
        clientReady.cv.notify_all();
    }
    logger.log(Entidad::INTERM, TipoLog::INFO,
    "Interm: listo para aceptar solicitudes");

    //hay un cliente, se lanza un fork en implementacion cada que llega client
    handleClient();

}
void Intermediary::discoverServers(Control& serverReady){
    //espera server
    {
        std::unique_lock<std::mutex> lock(serverReady.mtx);
        serverReady.cv.wait(lock, 
                [&serverReady]{return serverReady.ready; 
                });
    }

    //send addr
    Message broad = Message::makeMsg(Command::CONNECT, myAdrr);
    logger.log(Entidad::INTERM, TipoLog::INFO,
    "Interm: broadcast \""+ broad.serializeMsg()+"\"");
    
    broadcastOut.push(broad);
    //recibir addr de serv
    Message data = serverToFork.wait_and_pop();
    logger.log(Entidad::INTERM, TipoLog::INFO,
       "Interm: \""+data.serializeMsg()+"\"");
    
    if (data.com != Command::SERVER) {
        logger.log(Entidad::INTERM, TipoLog::ERROR,
            "Interm: esperaba P/S/, recibe " + CommandName(data.com));
        return;
    }
    std::string serverAddress = data.body;
    logger.log(Entidad::INTERM, TipoLog::INFO,
    "Inter<"+serverAddress+">: servidor registrado ");

    //Obtener cataogo, registra servermap
    fetchCatalogFrom(serverAddress);
}

void Intermediary::fetchCatalogFrom(const std::string& serverAddr){
    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Inter: Obteniendo catalogo de <"+serverAddr+">");

    Message resp = askServer(Message::makeMsg(Command::REQUEST, "dir"));
    closeServerSession();

    if (resp.com != Command::DATA){
        logger.log(Entidad::INTERM, TipoLog::ERROR,
        "Inter: respuesta incorrecta al solicitar catalogo:"+CommandName(resp.com));
        return;
    }

    std::unordered_set<std::string> figs;
    std::istringstream iss(resp.body);
    std::string token;
    while (std::getline(iss, token, '\n')) {
        // quitar prefijo "N. "
        size_t dot = token.find(". ");
        if (dot != std::string::npos)
            token = token.substr(dot + 2);
        // trim espacios
        size_t a = token.find_first_not_of(" \t\r");
        size_t b = token.find_last_not_of(" \t\r");
        if (a != std::string::npos)
            token = token.substr(a, b - a + 1);
        if (!token.empty())
            figs.insert(token);
    }

    serverMap.registerServer(serverAddr, figs);
    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Fork: serverMap: " + std::to_string(figs.size())
        + " figuras de <" + serverAddr + ">");
}

void Intermediary::handleClient(){
    while (true) {
        Message req = clientToFork.wait_and_pop();
        logger.log(Entidad::INTERM, TipoLog::INFO,
            "Interm: \"" + req.serializeMsg() + "\"");

        switch (req.com) {
            case Command::REQUEST:
                handleRequest(req);
                break;

            case Command::GET:
                handleGet(req);
                break;

            case Command::QUIT:
                // P/Q/ -> cliente cierra su conexión TCP con el Fork
                logger.log(Entidad::INTERM, TipoLog::INFO,
                    "Interm: P/Q/ del cliente, sesion cerrada.");
                return;

            default:
                logger.log(Entidad::INTERM, TipoLog::ERROR,
                    "Interm: comando no reconocido del cliente: "
                    + CommandName(req.com));
                {
                    Message err = Message::makeMsg(Command::DATA,
                        "404:comando no reconocido");
                    logger.log(Entidad::INTERM, TipoLog::INFO,
                        "Interm: \"" + err.serializeMsg() + "\"");
                    forkToClient.push(err);
                }
                break;
        }
    }
}

void Intermediary::handleRequest(const Message& req){
    if (req.body != "dir") {
        logger.log(Entidad::INTERM, TipoLog::ERROR,
            "Interm: REQUEST mal formado, body=\"" + req.body + "\"");

        Message warn = Message::makeMsg(Command::DATA,
            "Error: REQUEST solo acepta 'dir'");
        logger.log(Entidad::INTERM, TipoLog::INFO,
            "Interm: \"" + warn.serializeMsg() + "\"");
        forkToClient.push(warn);
        return;
    }
    if (!serverMap.hasServers()) {
        logger.log(Entidad::INTERM, TipoLog::INFO,
            "Interm: no hay servidores disponibles -> P/S/");
        Message noSrv = Message::makeMsg(Command::SERVER, "");
        logger.log(Entidad::INTERM, TipoLog::INFO,
            "Interm: \"" + noSrv.serializeMsg() + "\"");
        forkToClient.push(noSrv);
        return;
    }

    // Lista unificada con identificadores para duplicados (protocolo nota*)
    std::string allFigs = serverMap.allFigures();
    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Fork: catalogo unificado: \"" + allFigs + "\"");

    Message resp = Message::makeMsg(Command::DATA, allFigs);
    logger.log(Entidad::INTERM, TipoLog::INFO,
        "Fork -> Client: \"" + resp.serializeMsg() + "\"");
    forkToClient.push(resp);
}

void Intermediary::handleGet(const Message& get){
    std::string rawName = get.body;
    const size_t amp = rawName.find('&');
    if (amp != std::string::npos) rawName = rawName.substr(0, amp);

    // El cliente puede usar "duck@address" (identificador único de duplicados)
    std::string figureName = rawName;
    std::string targetAddr;
    const size_t at = rawName.find('@');
    if (at != std::string::npos) {
        figureName = rawName.substr(0, at);
        targetAddr = rawName.substr(at + 1);
    } else {
        targetAddr = serverMap.findServer(figureName);
    }

    // Evento: "Figura no existe" -> P/D/404:...
    if (targetAddr.empty()) {
        logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
            "Interm: \"" + figureName + "\" no en serverMap -> 404");

        Message notFound = Message::makeMsg(Command::DATA, "404:" + figureName);
        logger.log(Entidad::INTERM, TipoLog::INFO,
            "Interm: \"" + notFound.serializeMsg() + "\"");
        forkToClient.push(notFound);
        return;
    }

    // Construir body para el servidor: nombre limpio + segmento opcional
    std::string getBody = figureName;
    if (amp != std::string::npos)
        getBody += get.body.substr(get.body.find('&'));  // agrega "&seg"

    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Interm: reenviando P/G/" + getBody + " a [" + targetAddr + "]");

    
    Message serverResp = askServer(Message::makeMsg(Command::GET, getBody));
    closeServerSession();

    logger.log(Entidad::INTERM, TipoLog::INFO,
        "Interm: \"" + serverResp.serializeMsg() + "\"");

    // Evento: "Server cerrado durante conexion" -> P/D/ vacío
    if (serverResp.com == Command::UNKNOWN) {
        logger.log(Entidad::INTERM, TipoLog::ERROR,
            "Interm: servidor no responde -> P/D/ vacio");
        Message empty = Message::makeMsg(Command::DATA, "");
        logger.log(Entidad::INTERM, TipoLog::INFO,
            "Interm: \"" + empty.serializeMsg() + "\"");
        forkToClient.push(empty);
        return;
    }

    // Evento: "Figura borrada" -> server ya retornó P/D/404:...
    // Se reenvía directo al cliente sin modificar
    logger.log(Entidad::INTERM, TipoLog::INFO,
        "Interm: \"" + serverResp.serializeMsg() + "\"");
    forkToClient.push(serverResp);
}
    
Message Intermediary::askServer(const Message& msg) {
    std::lock_guard<std::mutex> lk(sessionMtx);
    logger.log(Entidad::INTERM, TipoLog::INFO,
        "Interm: \"" + msg.serializeMsg() + "\"");
    forkToServer.push(msg);
    return serverToFork.wait_and_pop();
}

void Intermediary::closeServerSession() {
    Message quit = Message::makeMsg(Command::QUIT);
    logger.log(Entidad::INTERM, TipoLog::INFO,
        "Interm: \"" + quit.serializeMsg() + "\" (cierre TCP)");
    forkToServer.push(quit);
}
