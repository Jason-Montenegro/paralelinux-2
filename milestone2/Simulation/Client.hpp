#ifndef CLIENT_H
#define CLIENT_H
#include <mutex>
 
#include "ClientConsoleHandler.hpp"
#include "Control.hpp"
#include "Logger.hpp"
#include "Message.hpp"
#include "ThreadQueue.hpp"

/**
 * Espera a que interm este listo,
 * Muestra menu por medio de ClientConsoleHandler
 * Enviar LIST_FIGURES | GET_FIGURE | STOP al Intermediario
 * Recibe y muestra respuestas del Intermediario
 */
class Client {
private:
    ThreadQueue<Message>& clientToInter;
    ThreadQueue<Message>& interToClient;

public:

    Client(ThreadQueue<Message>& clientToInter,
           ThreadQueue<Message>& interToClient);
 
    // loop interactivo. Espera interControl antes de comenzar.
    void run(Control& interControl, std::mutex& coutMtx);
    // bloquea hasta que Intermediary::signalToClient ponga ready=true
    void waitOnIntermediary(Control& interControl) const;
};



#endif