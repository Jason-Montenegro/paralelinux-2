#ifndef SWITCH_H
#define SWITCH_H

#include <mutex>
#include <string>
#include <vector>
#include "Connection.hpp"
#include "../ThreadQueue.hpp"
#include "Packet.hpp"

class Switch {
private:
    std::vector<Connection> connections;
    std::mutex mtx;
    std::string switchIp; 
public:
    Switch(std::string ip);
    void connect(Connection connection);
    void broadcast(const Connection& sender, const Packet& joinPacket);
    const std::string& getIp() const;
};
#endif