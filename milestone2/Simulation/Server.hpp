#ifndef SERVER_H
#define SERVER_H

#include "Control.hpp"
#include "LegoManager.hpp"
#include "Logger.hpp"
#include "Message.hpp"
#include "ThreadQueue.hpp"

class Server {
private:
    ThreadQueue<Message>& interToServer;
    ThreadQueue<Message>& serverToInter;
    LegoManager           manager;
 
public:
    Server(ThreadQueue<Message>& interToServer, ThreadQueue<Message>& serverToInter);
    void signalToIntermediary(Control& serverControl) const;
    void run(Control& serverControl);
};

#endif
