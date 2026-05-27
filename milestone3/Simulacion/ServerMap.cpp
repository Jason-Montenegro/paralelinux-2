#include "ServerMap.hpp"
#include <sstream>
 
void ServerMap::registerServer(const std::string& addr,const std::unordered_set<std::string>& figures) { 

    std::lock_guard<std::mutex> lk(mtx);
    map[addr] = figures;

}
 
void ServerMap::removeServer(const std::string& addr) {
    std::lock_guard<std::mutex> lk(mtx);
    map.erase(addr);
}
 
std::string ServerMap::findServer(const std::string& figureName) const {

    std::lock_guard<std::mutex> lk(mtx);
    for (const auto& [addr, figs] : map)
        if (figs.count(figureName))
            return addr;

    return "";
}
 
/*std::string ServerMap::allFigures() const {
    std::lock_guard<std::mutex> lk(mtx);
 
    //cuenta cuantos tienen cada figura (para detectar duplicados)
    std::unordered_map<std::string, int> count;
    for (const auto& [addr, figs] : map){
        for (const auto& f : figs){
            count[f]++;
        }
    }
    //std::ostringstream oss;
    std::string result;
    bool first = true;
    for (const auto& [addr, figs]: map) {
        for (const auto& fig : figs){
            if (!first){
                result += ",";
            }
            // duplicados con @addr como identificador
            //oss << (count[fig] > 1 ? fig + "@" + addr : fig);
            //first = false;
            if (count[fig] > 1) {
                result += fig + "@" + addr;
            } else {
                result += fig;
            }

            first = false;
        }
    }
    return result;
}  */

std::string ServerMap::allFigures() const {
    std::lock_guard<std::mutex> lk(mtx);

    std::unordered_map<std::string, int> count;
    for (const auto& [addr, figs] : map)
        for (const auto& f : figs)
            count[f]++;

    std::string result;
    int n = 1;
    for (const auto& [addr, figs] : map) {
        for (const auto& fig : figs) {
            result += std::to_string(n++) + ". ";
            result += (count[fig] > 1) ? fig + "@" + addr : fig;
            result += "\n";
        }
    }
    return result;
}
 
bool ServerMap::hasServers() const {
    std::lock_guard<std::mutex> lk(mtx);
    return !map.empty();
}
