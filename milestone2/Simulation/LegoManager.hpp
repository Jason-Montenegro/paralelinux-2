#ifndef LEGOMANAGER_H
#define LEGOMANAGER_H

#include "LegoRepository.hpp"
#include "Logger.hpp"
#include "Message.hpp"

/**
 * Recibe un Message ASK_FIGURE y produce la respuesta consultando el LegoRepository
 *
 * Casos:
 *   ASK_FIGURE  body="ALL"             -> FIGURES_LIST (catálogo completo)
 *   ASK_FIGURE  body="nombre&segmento" -> FIGURE_FOUND | FIGURE_NOT_FOUND
 */
class LegoManager {
private:
    LegoRepository repo;

public:
    Message handle(const Message& request);
};

#endif