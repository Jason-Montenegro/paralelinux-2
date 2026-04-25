#include "LegoManager1.hpp"
#include <string>

Packet LegoManager1::handle(const Packet& request) {

    // ASK_FIGURE con figureName="ALL" -> catálogo completo
    if (request.type == PacketType::ASK_FIGURE && request.figureName == "ALL") {
        logger.log(Entidad::SERVIDOR, TipoLog::SOLICITUD,
            "LegoManager1: ASK_FIGURE ALL -> devolviendo catalogo.");
        return Packet::response(request.id, PacketType::FIGURES_LIST, repo.getCatalog());
    }

    // ASK_FIGURE normal
    if (request.type == PacketType::ASK_FIGURE) {
        const std::string& name = request.figureName;
        int segment = request.segment;

        logger.log(Entidad::SERVIDOR, TipoLog::SOLICITUD,
            "LegoManager1: ASK_FIGURE \"" + name
            + "\" segmento=" + std::to_string(segment));

        if (!repo.hasFigure(name)) {
            logger.log(Entidad::SERVIDOR, TipoLog::RESPUESTA,
                "LegoManager1: FIGURE_NOT_FOUND para \"" + name + "\"");
            return Packet::response(request.id, PacketType::FIGURE_NOT_FOUND,
                "FIGURE_NOT_FOUND: \"" + name + "\"");
        }

        std::string data = repo.getFigure(name, segment);
        logger.log(Entidad::SERVIDOR, TipoLog::RESPUESTA,
            "LegoManager1: FIGURE_FOUND para \"" + name + "\"");
        return Packet::response(request.id, PacketType::FIGURE_FOUND, data);
    }

    // Tipo inesperado
    logger.log(Entidad::SERVIDOR, TipoLog::ERROR,
        "LegoManager1: tipo inesperado: " + packetTypeName(request.type));
    return Packet::response(request.id, PacketType::ERROR,
        "Solicitud no reconocida por el servidor.");
}