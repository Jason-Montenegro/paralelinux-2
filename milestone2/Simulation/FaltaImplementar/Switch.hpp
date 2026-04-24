#ifndef SWITCH_H
#define SWITCH_H

#include <vector>
#include <memory>
#include "Connection.hpp"

class Switch {
private:
    std::vector<std::shared_ptr<Connection>> intermediaries;

public:
    void registerIntermediary(std::shared_ptr<Connection> conn);
    void broadcastJoin(const Message& msg);
};
#endif