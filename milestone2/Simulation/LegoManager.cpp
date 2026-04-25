#include "LegoManager.hpp"
#include <string>

Message LegoManager::handle(const Message& request) {

    // ASK_FIGURE con body="ALL" -> catálogo completo
    if (request.reqType == RequestType::ASK_FIGURE && request.body == "ALL") {
        logger.log(Entidad::SERVIDOR, TipoLog::SOLICITUD,
            "LegoManager: ASK_FIGURE ALL -> devolviendo catalogo.");
        return Message::response(request.id,
            ResponseType::FIGURES_LIST, repo.getCatalog());
    }

    // ASK_FIGURE con body="nombre&segmento"
    if (request.reqType == RequestType::ASK_FIGURE) {
        const size_t sep = request.body.find('&');

        if (sep == std::string::npos) {
            logger.log(Entidad::SERVIDOR, TipoLog::ERROR,
                "LegoManager: formato de body invalido: \"" + request.body + "\"");
            return Message::response(request.id, ResponseType::ERROR,
                "Formato invalido. Use: nombre&segmento");
        }

        const std::string name    = request.body.substr(0, sep);
        const std::string partStr = request.body.substr(sep + 1);

        size_t segment = 1;
        try {
            segment = static_cast<size_t>(std::stoi(partStr));
        } catch (...) {
            // Protocolo §XI: segmento inválido -> usar 1 por defecto
            logger.log(Entidad::SERVIDOR, TipoLog::ERROR,
                "LegoManager: segmento invalido \""
                + partStr + "\", usando segmento 1.");
            segment = 1;
        }

        logger.log(Entidad::SERVIDOR, TipoLog::SOLICITUD,
            "LegoManager: ASK_FIGURE \"" + name
            + "\" segmento=" + std::to_string(segment));

        std::string content = repo.getFigure(name, segment);

        if (content.find("FIGURE_NOT_FOUND") != std::string::npos) {
            logger.log(Entidad::SERVIDOR, TipoLog::RESPUESTA,
                "LegoManager: FIGURE_NOT_FOUND para \"" + name + "\"");
            return Message::response(request.id,
                ResponseType::FIGURE_NOT_FOUND, content);
        }

        logger.log(Entidad::SERVIDOR, TipoLog::RESPUESTA,
            "LegoManager: FIGURE_FOUND para \"" + name
            + "\" segmento=" + std::to_string(segment));
        return Message::response(request.id, ResponseType::FIGURE_FOUND, content);
    }

    // Tipo inesperado en el servidor
    logger.log(Entidad::SERVIDOR, TipoLog::ERROR,
        "LegoManager: tipo inesperado: " + requestTypeName(request.reqType));
    return Message::response(request.id, ResponseType::ERROR,
        "Solicitud no reconocida por el servidor.");
}