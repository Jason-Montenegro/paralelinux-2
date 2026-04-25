#include <mutex>
#include <thread>

#include "Client.hpp"
#include "Control.hpp"
#include "Intermediary.hpp"
#include "Logger.hpp"
#include "Message.hpp"
#include "Server.hpp"
#include "ThreadQueue.hpp"

/**
 * Punto de entrada. Conecta todos los componentes del protocolo
 * mediante ThreadQueue (canales de comunicación).
 *
 * Orden de arranque (garantizado por Control + mutex + cv):
 *  Server arranca -> notifica serverControl
 *  Intermediary espera serverControl -> pobla tabla de rutas -> notifica interControl
 *  Client espera interControl -> inicia menú interactivo
 */
int main() {
    //Colas de comunicación 
    ThreadQueue<Message> clientToInter;
    ThreadQueue<Message> interToClient;
    ThreadQueue<Message> interToServer;
    ThreadQueue<Message> serverToInter;

   
    Client client(clientToInter, interToClient);
    Intermediary intermediary(clientToInter, interToClient,interToServer, serverToInter);
    Server server(interToServer, serverToInter);

    Control serverControl; //sincronizacion
    Control interControl;

    // Mutex compartido
    std::mutex coutMtx;

    logger.log(Entidad::CLIENTE, TipoLog::SOLICITUD,
        "main: iniciando sistema");


    std::thread serverThread([&server, &serverControl] {
        server.run(serverControl);});

    std::thread interThread([&intermediary, &interControl, &serverControl] {
        intermediary.run(interControl, serverControl);});

    std::thread clientThread([&client, &interControl, &coutMtx] {
        client.run(interControl, coutMtx);});

    // Join inverso
    clientThread.join();
    interThread.join();
    serverThread.join();

    logger.log(Entidad::CLIENTE, TipoLog::SOLICITUD,
        "main: sistema cerrado correctamente");
    return 0;
}