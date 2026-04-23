#ifndef MESSAGE_H
#define MESSAGE_H
#include "MessageType.hpp"
#include <string>
#include <atomic>

//contador global de ids usando atomic
extern std::atomic<int> nextId;

struct Message{
    int id = 0;
    std::string body;
    
    //espacio msj solicitud/request
    RequestType reqType = RequestType::STOP;
    //espacio msj respuesta/response
    ResponseType respType = ResponseType::ERROR;


    static Message request(RequestType type, const std::string& body = "");
    static Message response(int id, ResponseType type, const std::string& body = "");
};

#endif