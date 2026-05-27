#include <thread>
#include <mutex>
#include <iostream>

#include "Message.hpp"
#include "ThreadQueue.hpp"
#include "Control.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "Intermediary.hpp"
#include "Client.hpp"

int main() {

    ThreadQueue<Message> broadcastChan;  // UDP: Fork -> Server
    ThreadQueue<Message> forkToServer;   // TCP: Fork -> Server
    ThreadQueue<Message> serverToFork;   // TCP: Server -> Fork
    ThreadQueue<Message> clientToFork;   // TCP: Client -> Fork
    ThreadQueue<Message> forkToClient;   // TCP: Fork -> Client

    //Sincronización de arranque
    Control    serverReady;  // Server en línea (escucha broadcast)
    Control    clientReady;  // Fork en línea (acepta clientes)
    std::mutex coutMtx;      // serializa salida en consola

    Server server(
        "172.16.123.71:8080",  // dirección simulada del servidor
        broadcastChan,         // UDP broadcast entrada
        forkToServer,          // TCP Fork->Server
        serverToFork           // TCP Server->Fork
        
    );

    Intermediary fork(
        "172.16.123.70:8080",   // dirección simulada del Fork
        broadcastChan,          // UDP broadcast salida
        forkToServer,           // TCP Fork->Server
        serverToFork,           // TCP Server->Fork
        clientToFork,           // TCP Client->Fork
        forkToClient            // TCP Fork->Client
    );

    Client client(
        clientToFork,           // TCP Client->Fork
        forkToClient            // TCP Fork->Client
    );

    //Hilos
    std::thread tServer([&]() {
        server.run(serverReady);
    });

    std::thread tFork([&]() {
        fork.run(serverReady, clientReady);
    });

    std::thread tClient([&]() {
        client.run(clientReady, coutMtx);
    });

    // Esperar finalización
    tClient.join();   // el cliente termina al enviar P/Q/
    tFork.join();     // el Fork termina cuando el cliente envía P/Q/
    // Notificar al servidor que cierre limpiamente
    forkToServer.push(Message::makeMsg(Command::KILL,""));
    serverToFork.wait_and_pop();
    tServer.detach(); // el servidor vive hasta que el proceso termina

    std::cout<<"\n __________________________________________ \n"
              << "|            Simulacion finalizada         |\n"
              << "|__________________________________________|\n";
    return 0;
}