#ifndef CLIENTCONSOLEHANDLER_H
#define CLIENTCONSOLEHANDLER_H
#include <mutex>
#include <optional>
#include <string>
 
#include "Message.hpp"
/**
 * Manejo de logica de interaccion de cliente
 * permite registrar peticiones del cliente 
 * menu, entrada, valida request
 * formato hacia intermediario
 */
class ClientConsoleHandler{
private:
    std::mutex& coutMtx; //mutex viene desde afuera para coordinar

public:
    ClientConsoleHandler(std::mutex& );
    Message getUserRequest();
    void showResponse(const Message& response) const;
    void showError(const std::string& message) const;

    void printMenu();
    Message buildGetFigureRequest() const;
    void printResponseBody(const std::string& body) const;
};

#endif