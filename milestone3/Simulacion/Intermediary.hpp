#pragma once 
#include <string>
#include "Control.hpp"
#include "Message.hpp"
#include "Logger.hpp"
#include "ServerMap.hpp"
#include "ThreadQueue.hpp"
#include <unordered_set>

#include <mutex>

class Intermediary
{
private:
    std::string& myAdrr;
    ThreadQueue<Message>& broadcastOut;
    ThreadQueue<Message>& forkToServer;
    ThreadQueue<Message>& serverToFork;
    ThreadQueue<Message>& clientToFork;
    ThreadQueue<Message>& forkToClient;

public:
    std::mutex sessionMtx;
    ServerMap serverMap;
    Intermediary(std::string myAddr,ThreadQueue<Message>& broadcastOut,ThreadQueue<Message>& forkToServer,
                ThreadQueue<Message>& serverToFork, ThreadQueue<Message>& clientToFork,ThreadQueue<Message>& forkToClient);
    void run(Control& serverReady, Control& clientReady);
    void discoverServers(Control& serverReady);
    void fetchCatalogFrom(const std::string& serverAddr);
    void handleClient();
    void handleRequest(const Message& req);
    void handleGet(const Message& req);
    Message askServer(const Message& msg);
    void closeServerSession();
       
};

