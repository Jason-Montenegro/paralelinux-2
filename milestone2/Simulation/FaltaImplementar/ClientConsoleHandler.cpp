#include "ClientConsoleHandler.hpp"
#include <iostream>
#include <sstream>

#define ESC       "\x1B"
#define RESET     ESC "[0m"
#define BOLD      ESC "[1m"
#define RED       ESC "[38;5;196m"
#define ORANGE    ESC "[38;5;208m"
#define CYAN      ESC "[38;5;51m"
#define GREEN     ESC "[38;5;46m"
#define BG_DARK   ESC "[48;5;235m"

ClientConsoleHandler::ClientConsoleHandler(std::mutex& outputMut) 
                                            : coutMtx(outputMut){}

/**
 *  Muestra el menú, 
 * lee la opción del usuario y construye el Message
 */
Packet ClientConsoleHandler::getUserRequest() {
    std::lock_guard<std::mutex> lock(coutMtx);

    printMenu();

    std::string option;
    std::getline(std::cin, option);

    if (option == "0") {
        return Packet::request(PacketType::STOP);
    } 
    else if (option == "1") {
        return Packet::request(PacketType::LIST_FIGURES);
    } 
    else if (option == "2") {
        return buildGetFigureRequest();
    }

    std::cerr << RED << "[ERROR] Opcion invalida\n" << RESET;
    return Packet::request(PacketType::LIST_FIGURES);
}

void ClientConsoleHandler::showResponse(const Packet& response) const {
    std::lock_guard<std::mutex> lock(coutMtx);

    std::cout << "\n";

    switch (response.type) {
        case PacketType::FIGURES_LIST:
        case PacketType::RETURN_FIGURE:
        case PacketType::FIGURE_FOUND:
            std::cout << GREEN << packetTypeName(response.type) << RESET;
            break;

        case PacketType::FIGURE_NOT_FOUND:
        case PacketType::ERROR:
            std::cout << RED << packetTypeName(response.type) << RESET;
            break;

        default:
            std::cout << packetTypeName(response.type);
            break;
    }

    std::cout << "\n";

    printResponseBody(response.content);

    std::cout << std::endl;
}



// imprime el cuerpo de la respuesta
void ClientConsoleHandler::printResponseBody(const std::string& body) const {
    std::istringstream stream(body);
    std::string line;

    while (std::getline(stream, line)) {
        std::cout << "  " << line << "\n";
    }
}

// Imprime el menú principal
void ClientConsoleHandler::printMenu() {
    std::string text = "Menu servidor de Piezas";

    int width = 80; // ancho consola
    int boxWidth = text.size() + 4; // espacio interno del cuadro
    int padding = (width - boxWidth) / 2;

    // línea superior
    for (int i = 0; i < padding; i++) std::cout << " ";
    std::cout << "┌";
    for (int i = 0; i < boxWidth - 2; i++) std::cout << "─";
    std::cout << "┐\n";

    // línea del texto
    for (int i = 0; i < padding; i++) std::cout << " ";
    std::cout << "│ " << text << " │\n";

    // línea inferior
    for (int i = 0; i < padding; i++) std::cout << " ";
    std::cout << "└";
    for (int i = 0; i < boxWidth - 2; i++) std::cout << "─";
    std::cout << "┘\n";
    std::cout << "\n";
    std::cout << "  " ORANGE "\t\t\"1\"" RESET  "  ->  " RESET
              << CYAN "LIST_FIGURES" RESET
              << "   Consultar figuras disponibles\n";
    std::cout << "  " ORANGE "\t\t\"2\"" RESET  "  ->  " RESET
              << CYAN "GET_FIGURE " RESET
              << "    Consultar piezas de una figura\n";
    std::cout << "  " ORANGE "\t\t\"0\"" RESET  "  ->  " RESET
              << CYAN "STOP       " RESET
              << "    Terminar la simulación\n";
    std::cout << "\n"  "  Cliente: " RESET;
}



Packet ClientConsoleHandler::buildGetFigureRequest() const {
    std::string figureName;
    std::string segmentInput;

    std::cout << "\nNombre de la figura: ";
    std::getline(std::cin, figureName);

    std::cout << "Segmento (1 o 2): ";
    std::getline(std::cin, segmentInput);

    int segment = 1;

    try {
        segment = std::stoi(segmentInput);
        if (segment != 1 && segment != 2) {
            segment = 1;
        }
    } catch (...) {
        segment = 1;
    }

    return Packet::request(
        PacketType::GET_FIGURE,
        figureName,
        segment
    );
}
