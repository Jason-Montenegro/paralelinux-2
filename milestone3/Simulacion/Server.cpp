#include "Server.hpp"
#include <stdexcept>

Server::Server(const std::string& myAddr,ThreadQueue<Message>& broadcastIn,
               ThreadQueue<Message>& forkToServer,ThreadQueue<Message>& serverToFork):

    servAddr(myAddr),
    broadcast(broadcastIn),
    forkToServer(forkToServer),
    serverToFork(serverToFork){}

/**
 * Server se levanta antes que el Fork
 * wait_and_pop() en broadcast bloquea hasta que el Fork envíe P/C/
*/
void Server::run(Control& serverReady){
    {
        std::lock_guard<std::mutex> lk(serverReady.mtx);
        serverReady.ready = true;
        serverReady.cv.notify_one();
    }
    logger.log(Entidad::SERVIDOR, TipoLog::SOLICITUD,
        "Server: en linea <"+ this->servAddr +"> esperando, esperando solicitudes");
    
    Message broad = broadcast.wait_and_pop(); //espera el mensaje
    logger.log(Entidad::SERVIDOR, TipoLog::INFO,
       "Server broadcast: \"" + broad.serializeMsg() + "\"");
    //espera P/C/<fork_addr>
    if(broad.com != Command::CONNECT){
        logger.log(Entidad::SERVIDOR, TipoLog::ERROR,
       "Server: esperaba C, recibe \"" + CommandName(broad.com)+ "\"");

        return;
    }

    //responder a P/R/<addr>
    Message data = Message::makeMsg(Command::SERVER, servAddr);
        logger.log(Entidad::SERVIDOR, TipoLog::INFO,
        "Server: \"" + data.serializeMsg() + "\"");

    serverToFork.push(data);
    
    //Manejar la conexion con fork, este pide catalogo al inicio\
        P/R/DIR,  y responde con Server > P/D/fig1, fig2, fig3 > Fork
    logger.log(Entidad::SERVIDOR, TipoLog::INFO,
        "Server <" + servAddr + ">: listo para sesiones TCP");

    while (true){
        try{
            managerConnection();
        }
        catch(const std::runtime_error& e){
            logger.log(Entidad::SERVIDOR, TipoLog::INFO,
                "Server <" + servAddr + ">: " + e.what());
            break;
        } 
    }
}

/**
 * ManagerConnection(): maneja comandos de fork e interpreta lo que este desea
 * Procesa soli y responde
 * 
*/
void Server::managerConnection(){
    Message req = forkToServer.wait_and_pop();
    logger.log(Entidad::SERVIDOR, TipoLog::INFO,
        "Server: \"" + req.serializeMsg() + "\"");

    switch (req.com) {
        case Command::REQUEST: {
            Message resp = handleRequest(req);
            logger.log(Entidad::SERVIDOR, TipoLog::INFO,
                "Server: enviando respuesta \"" + resp.serializeMsg() + "\"");
            serverToFork.push(resp);
            Message quit = forkToServer.wait_and_pop();
            logger.log(Entidad::SERVIDOR, TipoLog::INFO,
                "Server: quit \"" + quit.serializeMsg() + "\" (sesion cerrada)");
            break;
        }
        case Command::GET: {
            Message resp = handleGet(req);
            logger.log(Entidad::SERVIDOR, TipoLog::INFO,
                "Server -> Fork: \"" + resp.serializeMsg() + "\"");
            serverToFork.push(resp);
            Message quit = forkToServer.wait_and_pop();
            logger.log(Entidad::SERVIDOR, TipoLog::INFO,
                "Server: quit \"" + quit.serializeMsg() + "\" (sesion cerrada)");
            break;
        }
        case Command::KILL:
            logger.log(Entidad::SERVIDOR, TipoLog::INFO,
                "Server: P/K/ recibido -> cerrando");
            serverToFork.push(Message::makeMsg(Command::ACK));
            throw std::runtime_error("shutdown por P/K/");

        case Command::QUIT:
            logger.log(Entidad::SERVIDOR, TipoLog::INFO,
                "Server: P/Q/ inesperado, cerrando sesion.");
            break;

        default:
            logger.log(Entidad::SERVIDOR, TipoLog::ERROR,
                "Server: comando no reconocido: " + CommandName(req.com));
            serverToFork.push(Message::makeMsg(Command::DATA, "404:comando no reconocido"));
            break;
    }
}

Message Server::handleRequest(Message req) {
    if (req.body != "dir") {
        logger.log(Entidad::SERVIDOR, TipoLog::ERROR,
            "Server: REQUEST mal usado, body=\"" + req.body + "\"");
        return Message::makeMsg(Command::DATA,
            "WARNING:REQUEST solo acepta 'dir'");
    }
    logger.log(Entidad::SERVIDOR, TipoLog::INFO,
        "Server: entregando catalogo");
    return Message::makeMsg(Command::DATA, repo.getCatalog());
}
 

/**
 * handleGet  ->  P/D/<piezas>  o  P/D/404:...
 * Evento del protocolo: Client solicita figura borrada: 
 * getFigure retorna "404:..." -> P/D/404:...
*/
Message Server::handleGet(Message req) {
    std::string figureName = req.body;
    size_t segment = 1;
 
    const size_t amp = req.body.find('&');
    if (amp != std::string::npos) {
        figureName = req.body.substr(0, amp);
        try { segment = static_cast<size_t>(std::stoi(req.body.substr(amp + 1))); }
        catch (...) { segment = 1; }
    }
 
    logger.log(Entidad::SERVIDOR, TipoLog::INFO,
        "Server: GET \"" + figureName + "\" seg=" + std::to_string(segment));
 
    // getFigure retorna "404:nombre" si no existe
    return Message::makeMsg(Command::DATA, repo.getFigure(figureName, segment));
}
