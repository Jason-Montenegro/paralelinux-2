#include "Client.hpp"
#include <iostream>
#include <sstream>

// COLOR SALIDA
#define ESC    "\x1B"
#define RESET  ESC "[0m"
#define RED    ESC "[38;5;196m"
#define ORANGE ESC "[38;5;208m"
#define CYAN   ESC "[38;5;51m"
#define GREEN  ESC "[38;5;46m"

static std::string trimStr(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    return (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
}

Client::Client(ThreadQueue<Message>& ctFork,
           ThreadQueue<Message>& ftCLIENTE):
           clientToFork(ctFork), forkToClient(ftCLIENTE){}

void Client::run(Control& ClientReady, std::mutex& coutMtx){
    //bloquea antes de fork
    waitOnFork(ClientReady);
    logger.log(Entidad::CLIENTE, TipoLog::INFO,
        "Cliente en linea, iniciando sesion interactiva.");
    
    while (true){
        printMenu(coutMtx); 
        std::string op;
        getline(std::cin, op);

        if(op == "0"){
            //Message quit = Message::makeMsg(Command::QUIT, "Cliente envió QUIT, cerrando");

            logger.log(Entidad::CLIENTE, TipoLog::SOLICITUD, "CLIENTE envió QUIT, cerrando");
            clientToFork.push(Message::makeMsg(Command::QUIT));
        //{
            //std::lock_guard<std::mutex> lk(coutMtx);
            //std::cout << GREEN "\n  Conexion cerrada. Hasta luego.\n" RESET "\n";
        //}
            break;

        } else if (op == "1") {
            // P/R/dir
            Message req = Message::makeMsg(Command::REQUEST, "dir");
            logger.log(Entidad::CLIENTE, TipoLog::SOLICITUD,
                "Cliente:  option 1\"" + req.serializeMsg() + "\"");
            clientToFork.push(req);

            Message resp = forkToClient.wait_and_pop();
            logger.log(Entidad::CLIENTE, TipoLog::SOLICITUD,
                "Cliente: repuesta cliente\"" + resp.serializeMsg() + "\"");
            showResponse(resp, coutMtx);

        } else if (op == "2") {
            // P/G/<figura[&segmento]>
            std::string figureName, segStr;
            {
                std::lock_guard<std::mutex> lk(coutMtx);
                std::cout << "\n  Nombre de la figura: ";
            }
            std::getline(std::cin, figureName);
            figureName = trimStr(figureName);

            // Validar: nombre no vacío y no es un número puro
            bool esNumero = !figureName.empty() &&
                            figureName.find_first_not_of("0123456789") == std::string::npos;
            if (figureName.empty() || esNumero) {
                std::lock_guard<std::mutex> lk(coutMtx);
                std::cout << RED "  Nombre de figura invalido.\n" RESET;
                continue;
            }

            {
                std::lock_guard<std::mutex> lk(coutMtx);
                std::cout << "  Segmento (1 o 2): ";
            }
            std::getline(std::cin, segStr);

            int seg = 1;
            try {
                seg = std::stoi(segStr);
                if (seg < 1 || seg > 2) seg = 1;
            } catch (...) { seg = 1; }

            std::string body = figureName + "&" + std::to_string(seg);
            Message req = Message::makeMsg(Command::GET, body);
            logger.log(Entidad::CLIENTE, TipoLog::SOLICITUD,
                "CLIENTE -> Fork: \"" + req.serializeMsg() + "\"");
            clientToFork.push(req);
            Message resp = forkToClient.wait_and_pop();
            logger.log(Entidad::CLIENTE, TipoLog::SOLICITUD,
                "CLIENTE <- Fork: \"" + resp.serializeMsg() + "\"");
            showResponse(resp, coutMtx);

        } else {
            std::lock_guard<std::mutex> lk(coutMtx);
            std::cout << RED "  Opcion invalida.\n" RESET;
        }
    }
}
                       

void Client::waitOnFork(Control& ClientReady)const{
    std::unique_lock<std::mutex> lk(ClientReady.mtx);
    ClientReady.cv.wait(lk, [&ClientReady] 
        { return ClientReady.ready; });

}

void  Client::printMenu(std::mutex& coutMtx){
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

void Client::showResponse(const Message& resp, std::mutex& coutMtx) {
    std::lock_guard<std::mutex> lk(coutMtx);
    std::cout << "\n";

    if (resp.com == Command::DATA) {
        if (resp.body.empty()) {
            std::cout << ORANGE "  [P/D/] " RESET "Servidor no disponible.\n";

        } else if (resp.body.rfind("FIGURE_NOT_FOUND:", 0) == 0) {
            // repo devuelve: "FIGURE_NOT_FOUND: figura desconocida \"name\""
            std::cout << RED "FIGURE_NOT_FOUND\n" RESET
                      << "  " << resp.body << "\n";

        } else if (resp.body.rfind("404:", 0) == 0) {
            // intermediary devuelve 404 si no está en serverMap
            std::string name = trimStr(resp.body.substr(4));
            std::cout << RED "FIGURE_NOT_FOUND\n" RESET
                      << "  FIGURE_NOT_FOUND: \"" << name << "\" no registrada\n";

        } else if (resp.body.rfind("WARNING:", 0) == 0) {
            std::cout << ORANGE "  ADVERTENCIA " RESET
                      << resp.body.substr(8) << "\n";

        } else if (resp.body.find("Figura:") != std::string::npos) {
            // getFigure devuelve "Figura: X | Segmento: N\n  1. ...\n  2. ..."
            std::cout << GREEN "RETURN_FIGURE\n" RESET;
            std::istringstream iss(resp.body);
            std::string line;
            while (std::getline(iss, line)) {
                std::string t = trimStr(line);
                if (!t.empty())
                    std::cout << "  " << t << "\n";
            }

        } else {
            // getCatalog devuelve "1. Horse\n2. lion\n..."
            std::cout << GREEN "FIGURES_LIST\n" RESET;
            std::istringstream iss(resp.body);
            std::string line;
            while (std::getline(iss, line)) {
                std::string t = trimStr(line);
                if (!t.empty())
                    std::cout << "    " << t << "\n";
            }
        }

    } else if (resp.com == Command::SERVER && resp.body.empty()) {
        std::cout << ORANGE "  [P/S/] " RESET "No hay servidores disponibles aun.\n";
    } else {
        std::cout << CYAN "  [" << CommandName(resp.com) << "] " RESET
                  << resp.body << "\n";
    }
    std::cout << "\n";
}
