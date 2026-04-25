#ifndef CLIENTCONSOLEHANDLER_H
#define CLIENTCONSOLEHANDLER_H

#include <mutex>
#include <string>
#include "Packet.hpp"

class ClientConsoleHandler {
public:
    ClientConsoleHandler(std::mutex& outputMut);

    // Lee input del usuario y construye un Packet
    Packet getUserRequest();

    // Muestra respuesta en consola
    void showResponse(const Packet& response) const;

private:
    std::mutex& coutMtx;

    void printMenu();
    void printResponseBody(const std::string& content) const;

    Packet buildGetFigureRequest() const;
};

#endif