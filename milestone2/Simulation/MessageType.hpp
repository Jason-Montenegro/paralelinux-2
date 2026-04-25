#ifndef MESSAGETYPE_H
#define MESSAGETYPE_H
#include <string>
#include <cstdint>
/* SOLICITUD: Quien envia  */
enum RequestType : uint8_t{
    //cliente -> inter
    LIST_FIGURES, // solicita lista de figuras disponibles
    GET_FIGURE,   // solicita figura especifica (body = "nombre&segmento")

    // inter -> server
    ASK_FIGURE,   //solicita figura especifica

    //control
    STOP
    //JOIN,
    //HANDSHAKE

};

enum ResponseType : uint8_t{
    // server -> interm
    FIGURE_FOUND,       // servidor encuentra pieza
    FIGURE_NOT_FOUND,   // Figura no encontrada / no existe

    // interm -> cliente
    FIGURES_LIST,       // lista de figuras disponibles
    RETURN_FIGURE,      // piezas de la figura solicitada
 
    // Errores
    ERROR

};

/**
 *  metodos de MESSAGETYPE
 *  permite convertir a nombre en protocolo, 
 *  pasa a string 
*/
std::string requestTypeName(RequestType t);
std::string responseTypeName(ResponseType t);

#endif