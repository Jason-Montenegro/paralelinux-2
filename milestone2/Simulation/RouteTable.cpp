#include "RouteTable.hpp"
#include <sstream>

void RouteTable::AllocateFigures(const std::unordered_set<std::string>& figures){
    std::lock_guard<std::mutex> lock(mtx);
    this->table = figures;
}

bool RouteTable::hasFigure(const std::string& name){
    std::lock_guard<std::mutex> lock(mtx);
    int counter =  this->table.count(name);
    if(counter>0){
        return true;
    }
    return false;
}

void RouteTable::removeFigure(const std::string& name){
    std::lock_guard<std::mutex> lock(mtx);
    this->table.erase(name);

}

std::string RouteTable::listAll(){
    std::lock_guard<std::mutex> lock(mtx);
    std::ostringstream os; //manipulacion de cadena
    int i =1;
    for (const auto& name : this->table) {
        os << "  " << i++ << ". " << name << "\n";
    }
    return os.str().empty() ? "  (sin figuras registradas)\n" : os.str();
}

/**
 * Evita que el mutex quede bloqueado por siempre
 * permite hacer una copia
 */
std::unordered_set<std::string> RouteTable::TableCopy(){
    std::lock_guard<std::mutex> lock(mtx);
    return this->table;
}


