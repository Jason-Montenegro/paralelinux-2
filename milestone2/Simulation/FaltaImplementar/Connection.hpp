#ifndef CONNECTION_H
#define CONNECTION_H

#include <memory>
#include "ThreadQueue.hpp"
#include "Message.hpp"

struct Connection {
    std::shared_ptr<ThreadQueue<Message>> inbound;
    std::shared_ptr<ThreadQueue<Message>> outbound;

    Connection() {
        inbound = std::make_shared<ThreadQueue<Message>>();
        outbound = std::make_shared<ThreadQueue<Message>>();
    }
};
#endif