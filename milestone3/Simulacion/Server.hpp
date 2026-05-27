#pragma once
#include <mutex>
#include <string>
#include "Control.hpp"
#include "Message.hpp"
#include "Logger.hpp"
#include "ThreadQueue.hpp"
#include "LegoRepository.hpp"

class Server{
private:
    std::string servAddr;
    ThreadQueue<Message>& broadcast;
    ThreadQueue<Message>& forkToServer;
    ThreadQueue<Message>& serverToFork;
    LegoRepository repo;


public:
    Server(const std::string& servAddr,ThreadQueue<Message>& broadcast,
            ThreadQueue<Message>& forkToServer,ThreadQueue<Message>& serverToFork);
    ~Server() = default;

    void run(Control& ServerReady);
    void managerConnection();

    Message handleRequest(Message REQ);

    Message handleGet(Message get);
};

