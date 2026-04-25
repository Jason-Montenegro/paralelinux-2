#ifndef LEGOREPOSITORY_H
#define LEGOREPOSITORY_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using Pieza   = std::pair<size_t, std::string>;   // (cantidad, descripcion)
using Segmento = std::vector<Pieza>;
using Partes   = std::vector<Segmento>;            // índice 0 = segmento 1

/**
 * Almacén de figuras. Cada figura tiene 2 segmentos 
 *  "House", "Orchid", "Fish", "Giraffe".
 *
 * TODO (etapa filesystem): reemplazar tabla estatica por lectura
 * desde el sistema de archivos propio (inodos + bitmap).
 */
class LegoRepository {
private:
    std::unordered_map<std::string,std::vector<
                    std::vector<std::pair<size_t, std::string>>>> figures;
public:
    LegoRepository();

    std::string getCatalog();
    std::string getFigure( std::string name, size_t segment);
    bool hasFigure( std::string name);
    std::unordered_set<std::string> availableFigures();


};

#endif