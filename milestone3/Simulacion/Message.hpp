#ifndef  MESSAGE_HPP
#define  MESSAGE_HPP

#include <string>
#include <atomic>
#include <regex>
#include <iostream>

/**
 * formato de prot: P/[Comando]/[mensaje]
 * ACK A Indica que el comando se hizo correctamente
 * Connect C Crear conexión inicial fork-server
 * Data D Respuesta del server (directorios, figuras)
 * Fork F Comunicación entre Forks
 * Get G Solicitud de figuras
 * Kill K Servidor anuncia su muerte
 * Patch P Actualización de servidores
 * Quit Q Cerrar conexión fork-server
 * Request R Solicitud de servidores o directorios
 * Server Data S Dirección IP y puerto del servidor
 */
enum class Command : char{
    ACK = 'A',
    CONNECT = 'C',
    DATA = 'D',
    FORK= 'F',
    GET = 'G',
    KILL = 'K',
    PATCH = 'P',
    QUIT = 'Q',
    REQUEST = 'R',
    SERVER = 'S',  
    UNKNOWN = '?'
};

inline std::string CommandName(Command c) {
    switch (c) {
        case Command::ACK: return "A";
        case Command::CONNECT: return "C";
        case Command::DATA: return "D";
        case Command::FORK:return "F";
        case Command::GET: return "G";
        case Command::KILL: return "K";
        case Command::PATCH:return "P";
        case Command::QUIT:return "Q";
        case Command::REQUEST:return "R";
        case Command::SERVER: return "S";
        default: return "UNKNOWN";
    }
}

/** 
 * Mensaje de protocolo, 
 * primeros caracteres P/ COMANDO/ CUERPO
 * 
*/
struct Message
{
    int id;
    Command com;
    std::string body;

    static Message makeMsg(Command co, std::string bod = ""){
        Message m;
        m.id = nextId.fetch_add(1);
        m.com = co;
        m.body = bod;
        return m;
    }

    std::string serializeMsg() const{
        std::string serial = std::string("P/") + 
                            static_cast<char> (com) + "/"+body;
        return serial;
    }

    static Message parseMsg(std::string  rawData){
        std::regex patronEncounter( "^P/([ACDFGKPQRS])/(.*)$");
        std::smatch match;
        if (std::regex_match(rawData, match, patronEncounter))
        {   
            Message m;
            m.com = static_cast<Command>(match[1].str()[0]);
            m.body = match[2].str();
            m.id = Message::nextId.fetch_add(1);
            return m;
        }
        //manejo de error
        Message error;
        error.com = Command::UNKNOWN;
        error.body = rawData;
        return error;
    }
    private:
    inline static std::atomic<int> nextId{0};


};
#endif