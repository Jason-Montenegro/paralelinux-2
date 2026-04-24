#include "Switch.hpp"

void Switch::registerIntermediary(std::shared_ptr<Connection> conn) {
    intermediaries.push_back(conn);
}

void Switch::broadcastJoin(const Message& msg) {
    for (auto& conn : intermediaries) {
        conn->inbound->push(msg);
    }
}
