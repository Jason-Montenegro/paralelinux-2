#include "Intermediary.hpp"
#include <sstream>
Intermediary::Intermediary(ThreadQueue<Message>& c2I,ThreadQueue<Message>& i2C,
                 ThreadQueue<Message>& i2S,ThreadQueue<Message>& s2I):
                 clientToInter(c2I), interToClient(i2C), interToServer(i2S), serverToInter(s2I){}

void Intermediary::run(Control& interControl, Control& serverControl){
    this->waitOnServer(serverControl); //espera que server este listo
    logger.log(Entidad::INTERM, TipoLog::SOLICITUD, "Intermediario: servidor listo, iniciando tabla de rutas");

    //llena tabla de rutas
    this->initRouteTable();

    //notifica a cliente que inter esta listo para atender
    this->signalToClient(interControl);
    logger.log(Entidad::INTERM, TipoLog::SOLICITUD, "Intermediario: esperando solicitud de cliente");
    
    //ciclo de mensajes
    while (true){
        Message request1 =  this->clientToInter.wait_and_pop(); //primer mensaje de cola
        logger.log(Entidad::INTERM, TipoLog::SOLICITUD, "Intermediario: solicitud #" + 
            std::to_string(request1.id)+ " tipo=" + requestTypeName(request1.reqType)
            + " body=\"" + request1.body + "\"");

        if (request1.reqType == RequestType::STOP) {
            // Propagar STOP al servidor para que también termine su loop
            logger.log(Entidad::INTERM, TipoLog::SOLICITUD, "Intermediario: STOP recibido, propagando al servidor");
            this->interToServer.push(Message::request(RequestType::STOP));
            break;
        }
        //caso por tipo de mensaje
        switch (request1.reqType) {
            case RequestType::LIST_FIGURES:
                this->handleListFigures(request1);
                break;
            case RequestType::GET_FIGURE:
                this->handleGetFigure(request1);
                break;
            default:
                logger.log(Entidad::INTERM, TipoLog::ERROR,
                    "Intermediario: tipo de solicitud no reconocido: "
                    + requestTypeName(request1.reqType));
                this->interToClient.push(
                    Message::response(request1.id, ResponseType::ERROR,
                        "Solicitud no reconocida."));
                break;
        }
    }
}

void Intermediary::waitOnServer(Control& serverControl){
    // bloquea hasta que Server::signalToIntermediary ponga ready=true
    std::unique_lock<std::mutex> lock(serverControl.mtx);
    serverControl.cv.wait(lock, [&serverControl] {
        return serverControl.ready;
    });
}

void Intermediary::signalToClient(Control& interControl){
    std::lock_guard<std::mutex> lock(interControl.mtx);
    interControl.ready = true;
    interControl.cv.notify_one();
}

/**
 * intemediario obtiene lista completa de servidor
 * 
 */
void Intermediary::initRouteTable() {
    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Intermediario: enviando ASK_FIGURE ALL al servidor");
 
    Message resp = this->askServer(RequestType::ASK_FIGURE, "ALL", 0);
 
    if (resp.respType == ResponseType::FIGURES_LIST) {
        // Parsear la respuesta: cada línea tiene formato "NombreFigura (N segmentos)"
        std::unordered_set<std::string> figures;
        std::istringstream iss(resp.body);
        std::string line;
        while (std::getline(iss, line)) {
            auto paren = line.find(" (");
            if (paren != std::string::npos) {
                figures.insert(line.substr(0, paren));
            }
        }

        routeTable.AllocateFigures(figures);
        logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
            "Intermediario: tabla de rutas inicializada con "
            + std::to_string(figures.size()) + " figuras.");
    } else {
        logger.log(Entidad::INTERM, TipoLog::ERROR,
            "Intermediario: no se pudo inicializar la tabla de rutas");
    }
}
 
// maneja lista de figuras localmente,
// sin necesidad de consultar a servidores.
// también refresca la tabla de rutas en este momento.
void Intermediary::handleListFigures(const Message& req) {
    // Refrescar la tabla de rutas 
    this->initRouteTable();
 
    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Intermediario: LIST_FIGURES resolviendo localmente desde tabla de rutas");
 
    std::string list = routeTable.listAll();
    this->interToClient.push(
        Message::response(req.id, ResponseType::FIGURES_LIST, list));
 
    logger.log(Entidad::INTERM, TipoLog::RESPUESTA,
        "Intermediario: FIGURES_LIST enviada al cliente id= "
        + std::to_string(req.id));
}
 

/** handleGetFigure
 * Consulta tabla de rutas -> si no está: FIGURE_NOT_FOUND inmediato
 * Si el servidor no la tiene: multicast a otros intermediarios
 * Si nadie la tiene: eliminar de tabla + FIGURE_NOT_FOUND
 * Construye RETURN_FIGURE para el cliente.*/

void Intermediary::handleGetFigure(const Message& req) {
    // body esperado: "nombre&segmento"
    const size_t sep = req.body.find('&');
    std::string figureName;
    std::string segStr = "1";
 
    if (sep != std::string::npos) {
        figureName = req.body.substr(0, sep);
        segStr     = req.body.substr(sep + 1);
    } else {
        // Sin separador: body entero es el nombre, segmento default 1
        figureName = req.body;
    }
 
    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Intermediario: GET_FIGURE \"" + figureName
        + "\" segmento=" + segStr);
 
    //validar contra tabla de rutas 
    if (!routeTable.hasFigure(figureName)) {
        logger.log(Entidad::INTERM, TipoLog::RESPUESTA,
            "Intermediario: \"" + figureName
            + "\" no esta en tabla de rutas -> FIGURE_NOT_FOUND");
        this->interToClient.push(
            Message::response(req.id, ResponseType::FIGURE_NOT_FOUND,
                "FIGURE_NOT_FOUND: \"" + figureName + "\" no registrada"));
        return;
    }
 
    //reenviar al servidor como ASK_FIGURE
    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Intermediario: reenviando ASK_FIGURE body=\"" + req.body + "\"");
 
    Message serverResp = this->askServer(RequestType::ASK_FIGURE,
        req.body, req.id);
 
    //si el servidor no la tiene, intentar multicast
    if (serverResp.respType == ResponseType::FIGURE_NOT_FOUND) {
        size_t seg = 1;
        try { seg = static_cast<size_t>(std::stoi(segStr)); } catch (...) {}
 
        std::string fromPeer = this->multicastToIntermediaries(figureName, seg);
 
        if (!fromPeer.empty()) {
            // Un intermediario par tenía la figura
            logger.log(Entidad::INTERM, TipoLog::RESPUESTA,
                "Intermediario: figura obtenida de intermediario par.");
            this->interToClient.push(
                Message::response(req.id, ResponseType::RETURN_FIGURE, fromPeer));
            return;
        }
 
        // ningún intermediario tiene la figura -> eliminar de tabla de figuras globales
        logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
            "Intermediario: \"" + figureName
            + "\" no encontrada en ningun servidor, eliminando de tabla");
        routeTable.removeFigure(figureName);
 
        this->interToClient.push(
            Message::response(req.id, ResponseType::FIGURE_NOT_FOUND,
                "FIGURE_NOT_FOUND: \"" + figureName + "\" no disponible."));
        return;
    }
 
    // construir RETURN_FIGURE para el cliente 
    logger.log(Entidad::INTERM, TipoLog::RESPUESTA,
        "Intermediario: RETURN_FIGURE \"" + figureName + "\" a cliente");
    this->interToClient.push(
        Message::response(req.id, ResponseType::RETURN_FIGURE,
            serverResp.body));
}
 

//   Envía un Message al servidor y espera su respuesta de forma síncrona.
//   Si originalId != 0 lo conserva para mantener la correlación.
Message Intermediary::askServer(RequestType type,const std::string& body,int originalId) {
    Message req = Message::request(type, body);
    if (originalId != 0) {
        req.id = originalId;
    }
    this->interToServer.push(req);
    return this->serverToInter.wait_and_pop();
}
 

/**
 * En la simulación no hay otros intermediarios, retorna vacío
 *  Implementación real pendiente:
 *     - otros Intermediarios
 *     - Enviar INTERMEDIARY_REQUEST (tipo, mitad, socket_cliente, nombre)
 *     - Esperar en cada canal; tomar el primero en responder FIGURE_FOUND
 *     - Si todos responden FIGURE_NOT_FOUND -> retornar ""
*/ 
std::string Intermediary::multicastToIntermediaries(
        const std::string& figureName, size_t segment) {
    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Intermediary: STUB multicast para \"" + figureName
        + "\" seg=" + std::to_string(segment)
        + " (sin pares conectados en simulacion).");
    // TODO: implementar con VSocket + hilos por canal (puertos 2000-3000)
    return "";
}