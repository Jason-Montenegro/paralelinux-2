#ifndef INTERMEDIARY_H
#define INTERMEDIARY_H

#include "Control.hpp"
#include "Logger.hpp"
#include "Message.hpp"
#include "RouteTable.hpp"
#include "ThreadQueue.hpp"

/**
 * Se encarga de coordinar comunicacion, usa colas
 * uso:
 * se encarga de interpretar el tipo de solicitud,
 * consulta la la tabla de rutas (tabla de figuras globales),
 * reenvia la solicitud al servidor si aplica,
 * construiye y envia respuesta final al cliente
 */

class Intermediary{
private:
    ThreadQueue<Message>& clientToInter;
    ThreadQueue<Message>& interToClient;
    ThreadQueue<Message>& interToServer;
    ThreadQueue<Message>& serverToInter;
 
    RouteTable routeTable;
    
public:
    Intermediary(ThreadQueue<Message>& client2Inter,ThreadQueue<Message>& inter2Client,
                 ThreadQueue<Message>& inter2Server,ThreadQueue<Message>& server2Inter);

    // Loop principal. Espera serverControl, luego notifica interControl.
    void run(Control& interControl, Control& serverControl);

    void waitOnServer(Control& serverControl) ;
 
    // Desbloquea al Cliente notificando que el intermediario está listo
    void signalToClient(Control& interControl) ;
 
    // rellena la RouteTable
    // Se llama una vez al iniciar y cuando el cliente envía LIST_FIGURES
    void initRouteTable();
 
    // LIST_FIGURES -> FIGURES_LIST
    void handleListFigures(const Message& req);
 
    // GET_FIGURE -> ASK_FIGURE -> RETURN_FIGURE | FIGURE_NOT_FOUND 
    void handleGetFigure(const Message& req);
 
    // Envía una solicitud al servidor y espera su respuesta
    Message askServer(RequestType type, const std::string& body, int originalId);
 
    // comunicacion interm-interm
    std::string multicastToIntermediaries(const std::string& figureName, size_t segment);

    //void sendJoin();
    //void handleJoin(Message& msg);
   // void handleHandshake(Message& msg);
 

};


#endif