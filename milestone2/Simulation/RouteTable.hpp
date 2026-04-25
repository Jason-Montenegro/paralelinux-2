#ifndef ROUTETABLE_H
#define ROUTETABLE_H
#include <mutex>
#include <string>
#include <unordered_set>
/** 
 * Tabla de figuras globales del Intermediario
 * 1.responder LIST_FIGURES localmente sin consultar el servidor
 * 2. valida GET_FIGURE antes de reenviar al servidor, si la figura no está -> FIGURE_NOT_FOUND
 * 3. Eliminar una figura cuando el servidor confirma que no existe
 */
class RouteTable
{
private:
    std::mutex mtx;
    std::unordered_set<std::string> table;
public:
    //rellenar tabla con figuras conocidas
    void AllocateFigures(const std::unordered_set<std::string>& figures);

    // true si figura esta en tabla
    bool hasFigure(const std::string& name);

    //elimina figura de tabla global si esta no existe
    void removeFigure(const std::string& name);

    //devuelve lista formateada para FIGURE_LIST
    std::string listAll();

    std::unordered_set<std::string> TableCopy();
};


#endif