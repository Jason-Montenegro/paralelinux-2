#include "Lego_Client.hpp"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "SSLSocket.h"
#include "Socket.h"

static const char* HTTPS_PORT = "443";
static const char* HTTP_PORT = "80";

//definiciones de colores
#define ESC       "\x1B"
#define RESET     ESC "[0m"
#define BOLD      ESC "[1m"
#define RED       ESC "[38;5;196m"
#define ORANGE    ESC "[38;5;208m"
#define CYAN      ESC "[38;5;51m"
#define GREEN     ESC "[38;5;46m"
#define BG_DARK   ESC "[48;5;235m"

Lego_Client::Lego_Client(int ipVer, int SSL, const char* host,
                         const char* lego_ip)
    : client(nullptr), lego_ip(lego_ip), SSL(SSL), ipVer(ipVer), host(host) {}

Lego_Client::~Lego_Client() {}

// uso de colores "\033[0m" para blanco y "\033[31m" para verde en terminal.
void Lego_Client::cicloCliente() {
  int opcion = 1;
  std::string respuesta;
  //system("clear");  // limpiar terminal
  while (opcion) {
    try {
      do {
        mostrarMenu();
        std::getline(std::cin, respuesta);
      } while (!validateEntry(respuesta));
      opcion = stoi(respuesta);

      if (opcion == 1) {
        try {
          std::string lista = solicitarLista();
          std::cout << GREEN" \nlista solicitada: " << RESET << std::endl;

          int contador = 1;
          auto figuras_lista = parseList(lista);
          for (const auto& f : figuras_lista) {
            std::cout << contador << "|  " << std::left << std::setw(30) << f
                      << std::endl;
            contador++;
          }
          std::cout << std::endl;
        } catch (const std::exception& e) {
          //system("clear");
          std::cerr << RED"\nError al solicitar lista: " << e.what()
                    <<  RESET << std::endl;
        }
      } else if(opcion == 2) {
          try {
            procesarFigura();
          } catch (const std::exception& e) {
            //system("clear");
            std::cerr << RED"\nError al solicitar figura: " << e.what()
                      << RESET << std::endl;
          }
      }else if(opcion == 3){
        try {
          std::cout << solicitarLog() << std::endl;
        }catch (const std::exception& e) {
            std::cerr << RED
                      << "\nError al solicitar logs: "
                      << e.what()
                      << RESET << std::endl;
        }
       } else {
          if (opcion != 0) {
            //system("clear");
            std::cout
                << RED"\nOpcion no existente, intente de nuevo!"<<RESET
                << std::endl;
          }
      }
    } catch (const std::exception& e) {
      std::cerr << "Error general: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Error desconocido\n";
    }
  }
}

  // close();

void Lego_Client::mostrarMenu() {
  //std::string menu =
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
              << "   Ver lista de figuras. \n";
    std::cout << "  " ORANGE "\t\t\"2\"" RESET  "  ->  " RESET
              << CYAN "GET_FIGURE " RESET
              << "    Consultar piezas de una figura.\n";
    std::cout << "  " ORANGE "\t\t\"3\"" RESET  "  ->  " RESET
              << CYAN "GET_LOGS " RESET
              << "      Imprimir logs de intermediario.\n";
    std::cout << "  " ORANGE "\t\t\"0\"" RESET  "  ->  " RESET
              << CYAN "STOP       " RESET
              << "    Salir.\n";
    std::cout << ORANGE"\n\n"  "  Opcion Seleccionada: " RESET;
}

bool Lego_Client::validateEntry(std::string entry) {
  if (entry.empty()) {
    std::cin.clear();  // limpia error
    // std::cin.ignore(10000, '\n');
    std::cout << RED << "\nIngrese una respuesta NO vacia\n"
              << RESET<< std::endl;
    return false;
  }
  try {
    int respuesta = std::stoi(entry);
  } catch (const std::invalid_argument& e) {
    std::cout << RED << "\nIngrese un numero valido\n"
              << RESET << std::endl;
    return false;
  }

  return true;
}

void Lego_Client::procesarFigura() {
  std::cout << ORANGE"\n\tFigura: "<<RESET;
  std::string figura;
  std::cin >> figura;
  std::cin.ignore(10000, '\n');

  std::string respuesta;
  int mitad;

  do {
    std::cout << ORANGE "\tIngrese la mitad (opcional): "<< RESET;
    std::getline(std::cin, respuesta);
  } while (!validateEntry(respuesta));

  mitad = std::stoi(respuesta);

  if (mitad == 1 || mitad == 2) {
    solicitarMediaFigura(figura, mitad);
  } else {
    for (int i = 1; i <= 2; ++i) {
      solicitarMediaFigura(figura, i);
    }
  }

  if (!this->lego_figuras.empty()) {
    std::cout << CYAN"\n\tPiezas totales "<<RESET;

    if (mitad == 1 || mitad == 2) {
      std::cout << CYAN"de la mitad: " <<RESET<< mitad << std::endl;

    }

    int i = 1;
    int conteo = 0;

    for (const auto& par : lego_figuras) {
      std::cout << i << "| " << std::left << std::setw(30) << par.first
                << " cantidad: " << par.second << std::endl;
      conteo += par.second;
      i++;
    }
    std::cout << CYAN"\n\tTotal de piezas de la estructura: " << RESET<< conteo << std::endl;
    this->lego_figuras.clear();

  } else {
    //system("clear");
    std::cout << RED"\nEl servidor no posee las piezas de la figura "
                 "indicada :( \n\n" <<RESET;
  }

  std::cout << std::endl;
}

// activa la conexion con el servidor
int Lego_Client::OpenConnection() {
  int st = -1;
  //puertos locales
  const char* port_ssl  = "8000";  // PORT4SSL
  const char* port_plain = "8501"; // PORT4
  if (this->SSL) {
    // printf("Connecting SSL");
    if (4 == this->ipVer) {
      this->client = new SSLSocket();  // Create an IPv4 TCP SSL socket
      st = this->client->Connect(this->host, port_ssl);
    } else {
      this->client = new SSLSocket(true);  // Create an IPv6 TCP SSL socket
      st = this->client->Connect(this->lego_ip, "9002");
    }
  } else {
    if (4 == this->ipVer) {
      this->client = new Socket('s');  // Create an IPv4 TCP socket
      st = this->client->Connect(this->host, port_plain);
    } else {
      this->client = new Socket('s', true);  // Create an IPv6 TCP socket
      st = this->client->Connect(this->lego_ip, "9503");
    }
  }
  return st;
}

void Lego_Client::solicitarMediaFigura(const std::string& figura, int mitad) {
  OpenConnection();  // al parecer siempre abre conexion que pida algo, porque
                     // al recibir una respuesta se remuere

  std::string request = buildHTTPRequest(figura, mitad);
  this->client->Write(request.c_str());  //
  std::string mFigura = readResponse();
  std::string body = parseBody(mFigura);

  delete client;
  client = nullptr;
  parseFigures(body);
  // return parseImages(body);
}
std::string Lego_Client::buildHTTPRequest(const std::string& figura,
                                          int& mitad) {
  return "GET /&lego/figure=" + figura + //se cambia solo pide figura
         "&part=" + std::to_string(mitad) +
         " HTTP/1.1\r\n"
         "Host: localhost\r\n"
         "Accept: text/html\r\n"
         "Connection: close\r\n\r\n";
  // decidir si conexion es keep alive o close
}

std::string Lego_Client::readResponse() {
  char buffer[5000];
  std::string response;
  try {
    // Leer respuesta
    ssize_t bytes;
    while ((bytes = static_cast<ssize_t>(
                client->Read(buffer, sizeof(buffer)))) > 0) {  //
      response.append(buffer, bytes);
    }
  } catch (const std::exception& e) {
    std::cerr << "Error SSL: " << e.what() << std::endl;
  }

  return response;
}

std::string Lego_Client::parseBody(const std::string& response) {
  std::string body;
  try {
    size_t pos = response.find("\r\n\r\n");
    if (pos != std::string::npos) {
      body = response.substr(pos + 4);
    }
  } catch (const std::exception& error) {
    std::cerr << "Error parse body: " << error.what() << std::endl;
  }
  return body;
}

// Los clientes serán capaces de obtener el listado de piezas de los servidores
// de piezas, en primera instancia y posteriormente de los servidores
// intermedios, deberán interactuar con ellos por medio del protocolo HTTP en un
// puerto TCP determinado (TCP/80 o TCP/443).
std::string Lego_Client::solicitarLista() {
  OpenConnection();
  std::string request = buildListRequest();
  client->Write(request.c_str());
  std::string response = readResponse();
  //Comprobacion Jafet
  //std::cout << "RESPUESTA COMPLETA!!!!!!!!!!!!!!!1\n";
 // std::cout << response << std::endl;

  //std::string body = parseBody(response);

  //std::cout << "BODY\n";
  //std::cout << body << std::endl;
  //fin 

  delete client;
  client = nullptr;
  return parseBody(response);
}

// los clientes son capaces de solicitar los logs al intermediario
// el intermediario resuelve la solicitud de logs y los envia al cliente
std::string Lego_Client::solicitarLog() {
  OpenConnection();
  std::string request = buildLogRequest();
  client->Write(request.c_str());
  std::string response = readResponse();
  delete client;
  client = nullptr;
  return parseLog(parseBody(response));
}
// creador del request de log
std::string Lego_Client::buildLogRequest(){
  return "GET /&logs=1 HTTP/1.1\r\n"
         "Host: localhost\r\n"
         "Accept: text/html\r\n"
         "Connection: close\r\n\r\n";
}
std::string Lego_Client::buildListRequest() {
  return "GET /&list/figures HTTP/1.1\r\n"
         "Host: localhost\r\n"
         "Accept: text/html\r\n"
         "Connection: close\r\n\r\n";
}
// Cada equipo de trabajo debe obtener las piezas necesarias para construir una
// figura sencilla de Lego®
// , el servidor de piezas detalla el inventario de piezas para poder
// armarla, pero este servidor sólo entregará la primera o segunda parte de las
// piezas, de acuerdo con la solicitud. El cliente deberá desplegar el detalle
// completo de las piezas necesarias para construir la figura solicitada (ambas
// mitades), utilizando la cantidad y los nombres de las piezas provistos por el
// servidor, no así las imágenes descriptivas. El detalle de las piezas para
// armar la figura no debe presentar elementos HTML en su presentación al
// usuario.


// idea de Brandon Palacios usar Regex
// Implementacion de ivan (version harcodeada)
// std::vector<std::string> Lego_Client::parseList(const std::string& body) {
//   std::cout<<body<<std::endl;
//   std::regex regex_figuras(R"regex(<OPTION value="([^"]+)">)regex");
//   std::vector<std::string> figuras;
//   std::smatch match;
//   std::string::const_iterator searchStart(body.cbegin());
//   while (std::regex_search(searchStart, body.cend(), match, regex_figuras)) {
//     std::string valor = match[1];

//     if (valor != "None" && valor != "0" && valor != "1" && valor != "2") {
//       figuras.push_back(valor);
//     }

//     searchStart = match.suffix().first;
//   }

//   return figuras;
//}
  // (version mas descente)
  std::vector<std::string> Lego_Client::parseList(const std::string& body) {
    std::vector<std::string> figuras;
    std::regex row_regex(R"(<tr>\s*<td>([^<]+)</td>\s*<td)"); //salida de list cambiada
    std::smatch m;
    auto it = body.cbegin();
    while (std::regex_search(it, body.cend(), m, row_regex)){
      figuras.push_back(m[1]);
      it = m.suffix().first;
    }
    /* Implementacion sin find
    size_t inicio = body.find("<SELECT");
    size_t fin = body.find("</SELECT>"); // hardcodeado aun pero en menor medida
    if (inicio != std::string::npos && fin != std::string::npos) {
      std::regex regex_figuras(R"regex(<OPTION value="([^"]+)">)regex");
    std::smatch m;
    std::string bloque = body.substr(inicio, fin - inicio);

    for (auto it = bloque.cbegin();
         std::regex_search(it, bloque.cend(), m, regex_figuras);
         it = m.suffix().first) {
      if (m[1] != "None") figuras.push_back(m[1]);
    }
  }*/

  return figuras;
}

void Lego_Client::parseFigures(const std::string& body) {
  std::regex piece_regex(R"(<li><strong>(\d+)x</strong>\s*([^<]+)</li>)");
  std::smatch match;
  auto it = body.cbegin();
  while (std::regex_search(it, body.cend(), match, piece_regex)) {
        int cantidad = std::stoi(match[1]);
        std::string nombre = match[2];
        // trim espacios al final
        nombre.erase(nombre.find_last_not_of(" \t\r\n") + 1);
        this->lego_figuras[nombre] += cantidad;
        it = match.suffix().first;
  }
}

std::string Lego_Client::parseLog(const std::string& body){
  std::regex regexLog(R"(<tr><td>INFO</td><td>(.*?)</td></tr>)");

    std::smatch match;
    std::string resultado;
    std::string::const_iterator inicio = body.cbegin();

    while (std::regex_search(inicio, body.cend(), match, regexLog)) {
        resultado += match[1].str();
        resultado += '\n';
        inicio = match.suffix().first;
    }

    return resultado;
}
/// std::regex img_regex(R"(SRC="([^"]+)")");
