#include "Intermediary1.hpp"
#include <sstream>

Intermediary1::Intermediary1(Connection clientConn, Connection serverConn)
    : clientConn(std::move(clientConn)),
      serverConn(std::move(serverConn)) {}

void Intermediary1::addPeer(Connection peerConn) {
    peers.emplace_back(std::move(peerConn));
}

void Intermediary1::run(Control& interControl, Control& serverControl) {
    this->waitOnServer(serverControl);
    logger.log(Entidad::INTERM, TipoLog::SOLICITUD,
        "Intermediario: servidor listo, iniciando tabla de rutas");

    this->initRouteTable();
    this->signalToClient(interControl);
    logger.log(Entidad::INTERM, TipoLog::SOLICITUD,
        "Intermediario: esperando solicitud de cliente");

    while (true) {
        Packet req = clientConn.inbound->wait_and_pop();

        logger.log(Entidad::INTERM, TipoLog::SOLICITUD,
            "Intermediario: solicitud #" + std::to_string(req.id)
            + " tipo=" + packetTypeName(req.type)
            + " figura=\"" + req.figureName + "\"");

        if (req.type == PacketType::STOP) {
            logger.log(Entidad::INTERM, TipoLog::SOLICITUD,
                "Intermediario: STOP recibido, propagando al servidor");
            serverConn.outbound->push(Packet::request(PacketType::STOP));
            break;
        }

        switch (req.type) {
            case PacketType::LIST_FIGURES: handleListFigures(req); break;
            case PacketType::GET_FIGURE:   handleGetFigure(req);   break;
            default:
                logger.log(Entidad::INTERM, TipoLog::ERROR,
                    "Intermediario: tipo no reconocido: " + packetTypeName(req.type));
                clientConn.outbound->push(
                    Packet::response(req.id, PacketType::ERROR,
                        "Solicitud no reconocida."));
                break;
        }
    }
}

void Intermediary1::waitOnServer(Control& serverControl) {
    std::unique_lock<std::mutex> lock(serverControl.mtx);
    serverControl.cv.wait(lock, [&serverControl] {
        return serverControl.ready;
    });
}

void Intermediary1::signalToClient(Control& interControl) {
    std::lock_guard<std::mutex> lock(interControl.mtx);
    interControl.ready = true;
    interControl.cv.notify_one();
}

void Intermediary1::initRouteTable() {
    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Intermediario: enviando ASK_FIGURE ALL al servidor");

    // "ALL" se codifica como figureName vacío y segment=0
    Packet resp = this->askServer(PacketType::ASK_FIGURE, "ALL", 0);

    if (resp.type == PacketType::FIGURES_LIST) {
        std::unordered_set<std::string> figures;
        std::istringstream iss(resp.content);
        std::string line;
        while (std::getline(iss, line)) {
            // formato: "NombreFigura (N segmentos)"
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

void Intermediary1::handleListFigures(const Packet& req) {
    this->initRouteTable();
    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Intermediario: LIST_FIGURES resolviendo localmente");

    std::string list = routeTable.listAll();
    clientConn.outbound->push(
        Packet::response(req.id, PacketType::FIGURES_LIST, list));

    logger.log(Entidad::INTERM, TipoLog::RESPUESTA,
        "Intermediario: FIGURES_LIST enviada al cliente id="
        + std::to_string(req.id));
}

void Intermediary1::handleGetFigure(const Packet& req) {
    const std::string& figureName = req.figureName;
    int segment = req.segment;

    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Intermediario: GET_FIGURE \"" + figureName
        + "\" segmento=" + std::to_string(segment));

    if (!routeTable.hasFigure(figureName)) {
        logger.log(Entidad::INTERM, TipoLog::RESPUESTA,
            "Intermediario: \"" + figureName
            + "\" no esta en tabla de rutas -> FIGURE_NOT_FOUND");
        clientConn.outbound->push(
            Packet::response(req.id, PacketType::FIGURE_NOT_FOUND,
                "FIGURE_NOT_FOUND: \"" + figureName + "\" no registrada"));
        return;
    }

    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Intermediario: reenviando ASK_FIGURE \"" + figureName + "\"");

    Packet serverResp = this->askServer(PacketType::ASK_FIGURE,
        figureName, segment, req.id);

    if (serverResp.type == PacketType::FIGURE_NOT_FOUND) {
        std::string fromPeer = this->multicastToIntermediaries(figureName, segment);

        if (!fromPeer.empty()) {
            logger.log(Entidad::INTERM, TipoLog::RESPUESTA,
                "Intermediario: figura obtenida de intermediario par.");
            clientConn.outbound->push(
                Packet::response(req.id, PacketType::RETURN_FIGURE, fromPeer));
            return;
        }

        logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
            "Intermediario: \"" + figureName
            + "\" no encontrada en ningun servidor, eliminando de tabla");
        routeTable.removeFigure(figureName);

        clientConn.outbound->push(
            Packet::response(req.id, PacketType::FIGURE_NOT_FOUND,
                "FIGURE_NOT_FOUND: \"" + figureName + "\" no disponible."));
        return;
    }

    logger.log(Entidad::INTERM, TipoLog::RESPUESTA,
        "Intermediario: RETURN_FIGURE \"" + figureName + "\" a cliente");
    clientConn.outbound->push(
        Packet::response(req.id, PacketType::RETURN_FIGURE,
            serverResp.content));
}

Packet Intermediary1::askServer(PacketType type, std::string figureName,
                                     int segment, int origId) {
    Packet req = Packet::request(type, std::move(figureName), segment);
    if (origId != 0) req.id = origId;
    serverConn.outbound->push(req);
    return serverConn.inbound->wait_and_pop();
}

std::string Intermediary1::multicastToIntermediaries(
        const std::string& figureName, int segment) {
    logger.log(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
        "Intermediario: multicast para \"" + figureName
        + "\" seg=" + std::to_string(segment));

    for (auto& peer : peers) {
        peer.outbound->push(Packet::request(
            PacketType::INTERMEDIARY_REQUEST, figureName, segment));

        Packet resp = peer.inbound->wait_and_pop();
        if (resp.type == PacketType::INTERMEDIARY_RESPONSE && !resp.content.empty()) {
            return resp.content;
        }
    }
    return "";  // nadie lo tenía
}