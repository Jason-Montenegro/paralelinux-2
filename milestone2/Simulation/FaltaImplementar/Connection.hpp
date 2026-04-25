#ifndef CONNECTION_H
#define CONNECTION_H

#include <memory>
#include "../ThreadQueue.hpp"
#include "Packet.hpp"

struct Connection {
    std::shared_ptr<ThreadQueue<Packet>> inbound;
    std::shared_ptr<ThreadQueue<Packet>> outbound;

    Connection() {
        inbound = std::make_shared<ThreadQueue<Packet>>();
        outbound = std::make_shared<ThreadQueue<Packet>>();
    }
    bool operator==(const Connection& o) const {
        return inbound.get()  == o.inbound.get() &&
               outbound.get() == o.outbound.get();
    }

    bool operator!=(const Connection& o) const { return !(*this == o); }

};
#endif