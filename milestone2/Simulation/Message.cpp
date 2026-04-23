#include "Message.hpp"
/**
 * Declaracion de contador de ids
 * cada request con un response
 * 
 */
std::atomic<int> nextId;

/**
 * Construccion de mensaje de request
 * (genera id único)
 * Se crea instancia Mensaje, utiliza atomic para el id único
 * usa fetch_add(1), post incremento del contador
*/
Message Message::request(RequestType tipo, const std::string& body){
    Message msj;
    msj.id = nextId.fetch_add(1);
    msj.reqType = tipo;
    msj.body = body;
    return msj;
}

/**
 * Construccion de mensaje de response
 * vinculada a un request (conserva el id)
 * Se crea instancia Mensaje, utiliza atomic para el id único
 * usa fetch_add(1), post incremento del contador
*/
Message Message::response(int idAsignado, ResponseType tipo, const std::string& body){
    Message msj;
    msj.id =  idAsignado;
    msj.respType = tipo;
    msj.body = body;
    return msj;
}   