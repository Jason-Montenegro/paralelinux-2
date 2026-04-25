#include "Switch.hpp"
#include <utility>

Switch::Switch(std::string ip): switchIp(ip){}

/**
 * Registra conexion con switch
 * 
 */
void Switch::connect(Connection connection){
    std::scoped_lock lock(this->mtx);
    this->connections.emplace_back(connection);
}

 /**
  * reenvia paquete a todos
  * 
  */
void Switch::broadcast(const Connection& sender, const Packet& joinPacket){
    std::scoped_lock lock(this->mtx);
    for(auto& conn : this->connections){
        if (conn != sender){
            conn.inbound->push(joinPacket);
            
        }
        
    }
}

const std::string& Switch::getIp() const { return this->switchIp; }
