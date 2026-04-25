#pragma once
#include <string>
#include <cstdint>
#include <atomic>

enum class PacketType : uint8_t {
    // Cliente -> Intermediario
    LIST_FIGURES,
    GET_FIGURE,
    // Intermediario -> Cliente
    FIGURES_LIST,
    RETURN_FIGURE,
    FIGURE_NOT_FOUND,
    // Intermediario <-> Servidor
    ASK_FIGURE,
    FIGURE_FOUND,
    // Red de intermediarios
    INTERMEDIARY_JOIN,
    INTERMEDIARY_HANDSHAKE,
    INTERMEDIARY_REQUEST,
    INTERMEDIARY_RESPONSE,
    ERROR,
    STOP
};

inline std::string packetTypeName(PacketType t) {
    switch (t) {
        case PacketType::LIST_FIGURES:          return "LIST_FIGURES";
        case PacketType::GET_FIGURE:            return "GET_FIGURE";
        case PacketType::FIGURES_LIST:          return "FIGURES_LIST";
        case PacketType::RETURN_FIGURE:         return "RETURN_FIGURE";
        case PacketType::FIGURE_NOT_FOUND:      return "FIGURE_NOT_FOUND";
        case PacketType::ASK_FIGURE:            return "ASK_FIGURE";
        case PacketType::FIGURE_FOUND:          return "FIGURE_FOUND";
        case PacketType::INTERMEDIARY_JOIN:     return "INTERMEDIARY_JOIN";
        case PacketType::INTERMEDIARY_HANDSHAKE:return "INTERMEDIARY_HANDSHAKE";
        case PacketType::INTERMEDIARY_REQUEST:  return "INTERMEDIARY_REQUEST";
        case PacketType::INTERMEDIARY_RESPONSE: return "INTERMEDIARY_RESPONSE";
        case PacketType::ERROR:                 return "ERROR";
        case PacketType::STOP:                  return "STOP";
    }
    return "UNKNOWN";
}

struct Packet {
    int         id         = 0;
    PacketType  type       = PacketType::STOP;
    std::string sourceIp;       // usado en JOIN/HANDSHAKE
    std::string figureName;     // nombre de figura
    int         segment    = 1; // 1, 2 o 3 (toda)
    std::string content;        // lista de figuras, piezas, error msg, etc.

    static std::atomic<int>& counter() {
        static std::atomic<int> c{0};
        return c;
    }

    // Construye un paquete de solicitud con id único
    static Packet request(PacketType t,
                                std::string fig     = "",
                                int         seg     = 1,
                                std::string src     = "",
                                std::string body    = "") {
        Packet p;
        p.id         = counter().fetch_add(1);
        p.type       = t;
        p.figureName = std::move(fig);
        p.segment    = seg;
        p.sourceIp   = std::move(src);
        p.content    = std::move(body);
        return p;
    }

    // Construye una respuesta vinculada al id original
    static Packet response(int origId, PacketType t,
                                 std::string body = "") {
        Packet p;
        p.id      = origId;
        p.type    = t;
        p.content = std::move(body);
        return p;
    }
};