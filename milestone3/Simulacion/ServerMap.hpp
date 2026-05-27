#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

 

/**
 * Mapa de hilos del fork
 * relaciona con las figuras disponibles
 * Si dos servidores tienen la misma figura, allFigures() la lista como
 * "nombre@addr" para que el cliente pueda diferenciarlas

*/
class ServerMap{
private:

    mutable std::mutex mtx;

    std::unordered_map<std::string,
                       std::unordered_set<std::string>> map;
public:
    //registra o reemplaza lista de figuras
    void registerServer(const std::string& addr,
                        const std::unordered_set<std::string>& figures);

    //elimina server del mapa
    void removeServer(const std::string& add);

    //encuentra que server contiene la fig
    std::string findServer(const std::string& figureName) const;

    std::string allFigures() const; //lista de figuras 

    bool hasServers() const;
 
};

