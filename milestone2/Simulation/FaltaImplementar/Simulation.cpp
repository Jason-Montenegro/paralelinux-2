#include "Simulation.hpp"
#include <iostream>
#include <thread>
#include "Switch.hpp"
#include "Client1.hpp"
#include "Intermediary1.hpp"
#include "Server1.hpp"

Simulation::Simulation(std::string switchIp,
                       std::string clientIp,
                       std::string intermediaryIp)
    : switchIp(switchIp),
      clientIp(clientIp),
      intermediaryIp(intermediaryIp) {}

void Simulation::run() {
    /*
     * Se necesitan dos conexiones con los inbound/outbound cruzados:
     *
     *   clientSide.outbound  == interSide.inbound   (cliente escribe, inter lee)
     *   clientSide.inbound   == interSide.outbound  (inter escribe, cliente lee)
     *
     * Lo mismo para inter <-> server.
     */

    // --- Conexión cliente <-> intermediario ---
    Connection clientSide;   // lo recibe Client1
    Connection interClientSide; // lo recibe Intermediary1 (lado cliente)

    // Cruzar los punteros
    interClientSide.inbound  = clientSide.outbound;  // inter lee lo que cliente escribe
    interClientSide.outbound = clientSide.inbound;   // inter escribe lo que cliente lee

    // --- Conexión intermediario <-> servidor ---
    Connection interServerSide; // lo recibe Intermediary1 (lado servidor)
    Connection serverSide;      // lo recibe Server1

    interServerSide.outbound = serverSide.inbound;   // inter escribe, server lee
    interServerSide.inbound  = serverSide.outbound;  // server escribe, inter lee

    // --- Actores ---
    Client1       client(clientSide);
    Intermediary1 inter(interClientSide, interServerSide);
    Server1       server(serverSide);

    // --- Sincronización ---
    Control interControl;   // cliente espera a que inter esté listo
    Control serverControl;  // inter espera a que server esté listo
    std::mutex coutMtx;

    // --- Hilos ---
    std::thread tServer([&]{ server.run(serverControl); });
    std::thread tInter ([&]{ inter.run(interControl, serverControl); });
    std::thread tClient([&]{ client.run(interControl, coutMtx); });

    tClient.join();
    tInter.join();
    tServer.join();
}