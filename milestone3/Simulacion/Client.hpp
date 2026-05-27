#ifndef CLIENT_H
#define CLIENT_H
#include <mutex>

#include "Control.hpp"
#include "Logger.hpp"
#include "Message.hpp"
#include "ThreadQueue.hpp"

/**
 * Espera a que interm este listo,
 * Recibe y muestra respuestas del Intermediario
 */
class Client {
private:
    ThreadQueue<Message>& clientToFork;
    ThreadQueue<Message>& forkToClient;

public:

    Client(ThreadQueue<Message>& clientToFork,
           ThreadQueue<Message>& forkToClient);
 
    // loop interactivo. Espera interControl antes de comenzar.
    void run(Control& clientReady, std::mutex& coutMtx);

    // bloquea hasta que fork permita
    void waitOnFork(Control& interControl) const;

    void printMenu(std::mutex& coutMtx);
    
    //muestra respuestas de simulacion segun el tipo de mensaje
    void showResponse(const Message& resp, std::mutex& coutMtx); 
};



#endif