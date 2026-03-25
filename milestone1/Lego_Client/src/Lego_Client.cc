#include "Lego_Client.h"

#include <cstring>
#include <iostream>
#include <map>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>
#include <iomanip>
#include "SSLSocket.h"
#include "Socket.h"

// no se si se quejara jason que es mejor un define o alguna vaina loca que

static const char* SERVER_HOST = "os.ecci.ucr.ac.cr";
static const char* HTTPS_PORT = "443";
static const char* HTTP_PORT = "80";

// activa la conexion con el servidor
int Lego_Client::OpenConnection() {
  int st = -1;
  if (this->SSL) {
    // printf("Connecting SSL");
    if (4 == this->ipVer) {
      this->client = new SSLSocket();  // Create an IPv4 TCP SSL socket
      st = this->client->Connect(this->host, HTTPS_PORT);
    } else {
      this->client = new SSLSocket(true);  // Create an IPv6 TCP SSL socket
      st = this->client->Connect(this->lego_ip, HTTPS_PORT);
    }
  } else {
    if (4 == this->ipVer) {
      this->client = new Socket('s');  // Create an IPv4 TCP socket
      st = this->client->Connect(this->host, HTTP_PORT);
    } else {
      this->client = new Socket('s', true);  // Create an IPv6 TCP socket
      st = this->client->Connect(this->lego_ip, HTTP_PORT);
    }
  }
  return st;
}

std::string Lego_Client::buildHTTPRequest(const std::string& figura,
                                          int& mitad) {
  return "GET /lego/list.php?figure=" + figura +
         "&part=" + std::to_string(mitad) +
         " HTTP/1.1\r\n"
         "Host: os.ecci.ucr.ac.cr\r\n"
         "Accept: text/html\r\n"
         "Connection: close\r\n\r\n";
  // decidir si conexion es keep alive o close
}

std::string Lego_Client::buildListRequest() {
  return "GET /lego/index.php HTTP/1.1\r\n"
         "Host: os.ecci.ucr.ac.cr\r\n"
         "Accept: text/html\r\n"
         "Connection: close\r\n\r\n";
}

Lego_Client::Lego_Client(int ipVer, int SSL, const char* host,
                         const char* lego_ip)
    : client(nullptr), lego_ip(lego_ip), host(host), SSL(SSL), ipVer(ipVer) {}
Lego_Client::~Lego_Client() {}

std::string Lego_Client::readResponse() {
  char buffer[5000];
  std::string response;
  try {
    // Leer respuesta
    int bytes;
    while ((bytes = client->Read(buffer, sizeof(buffer))) > 0) {
      response.append(buffer, bytes);
    }
  } catch (const std::exception& e) {
    std::cerr << "Error SSL: " << e.what() << std::endl;
  }

  return response;
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
  delete client;
  client = nullptr;
  return parseBody(response);
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

std::string Lego_Client::mostrarMenu(){
  std::string menu = "\033[32m Bienvenido a el servidor de piezas \033[32m\n"
          "1. \tVer lista de figuras\n"
          "2. \tPedir figura (nombre y si quiere solo una mitad especifica cual)\n"
          "0.\tSalir\n"
          "Seleccione una opcion \n";
  return menu;
}


// uso de colores "\033[0m" para blanco y "\033[31m" para verde en terminal.
void Lego_Client::cicloCliente() {
  int opcion = 1;
  system("clear");                           //limpiar terminal
  std::cout<<"\033[32m Bienvenido a el servidor de piezas\n"<<"\033[0m";
  while (opcion) {
    try {
      
      std::cout<<"\033[32m Seleccione accion a realizar:\n"<<"\033[0m"<<
          "1. Ver lista de figuras\n"
          "2. Pedir figura (digite el nombre y la mitad de la figura que desea)\n"
          "0. Salir\n"
          "Seleccione una opcion: \n";

      if (!(std::cin >> opcion)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "\033[31mEntrada invalida\n\033[31m"<<"\033[0m";
        continue;
      }
      // std::cout << lista << std::endl;
      if (opcion == 1) {
        try {
          std::string lista = solicitarLista();
          std::cout << "\033[32m\nlista solicitada :" << "\033[0m" << std::endl;

          int contador =1;
          auto figuras_lista = parseList(lista);
          for (const auto& f : figuras_lista) {
            std::cout <<contador <<"|  "<< std::left << std::setw(30) << f << std::endl;
            contador++;
          }
          std::cout << std::endl;
        } catch (const std::exception& e) {
          system("clear");
          std::cerr << "\033[31m\nError al solicitar lista: " << e.what() << "\033[31m"<<"\033[0m"<< std::endl;
        }
      } else {
        if (opcion == 2) {
          try {
            std::cout << "Figura: ";
            std::string figura;
            std::cin >> figura;
            int mitad;
            std::cout << "Ingrese la mitad (opcional): ";
            if (!(std::cin >> mitad)) {
              std::cin.clear();              // limpia error
              std::cin.ignore(10000, '\n');  // limpia buffer
              mitad = 3;                     // default = figura completa
              // continue; // es esto o while
            }
            // std::vector<std::string> piezas_figura;

            if (mitad == 1 || mitad == 2) {
              solicitarMediaFigura(figura, mitad);

            } else {
              for (int i = 1; i <= 2; ++i) {
                solicitarMediaFigura(figura, i);
              }
            }
            if (this->lego_figuras.size()) {
              std::cout << "\nCantidad de piezas totales";
              if (mitad == 1 || mitad == 2) {
                std::cout << "de la mitad" << mitad<< std::endl;
              }
              int i = 1;
              for (const auto& par : lego_figuras) {
                std::cout << i << "| " << std::left << std::setw(30) << par.first
                << " cantidad: " << par.second << std::endl;
                i++;
              }
              this->lego_figuras.clear();
            } else {
              system("clear");
              std::cout << "\033[31m\nEl servidor no posee las piezas de la figura indicada :( \033[31m"<<std::endl;
            }
            std::cout << std::endl;
          } catch (const std::exception& e) {
            system("clear");
            std::cerr << "\033[31m\nError al solicitar figura: " << e.what() << "\033[31m"<<std::endl;
          }
        } else {
          if(opcion != 0){
            system("clear");
            std::cout << "\033[31m\nOpcion no existente, intente de nuevo!\033[31m" << std::endl;
          }
         
        }

        // std::cout << "URL Imagenes" << std::endl;
        // las url de las imagenes que aun no ocupamos
        //  for (const auto& f : set_figuras1) {
        //    std::cout << f << std::endl;
        //  }
        //  for (const auto& f : set_figuras2) {
        //    std::cout << f << std::endl;
        //  }
      }

    } catch (const std::exception& e) {
      std::cerr << "Error general: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Error desconocido\n";
    }
  }
  // ocupa interfaz de cout y cin para ver la lista y que el usuario responda
  // close();
}

void Lego_Client::solicitarMediaFigura(const std::string& figura, int mitad) {
  OpenConnection();  // al parecer siempre abre conexion que pida algo, porque
                     // al recibir una respuesta se remuere

  std::string request = buildHTTPRequest(figura, mitad);
  this->client->Write(request.c_str());  //
  std::string mFigura = readResponse();
  std::string body = parseBody(mFigura);
  // std::cout << "figura" << std::endl;
  // std::cout << body << std::endl;
  // figuras.push_back(body);

  delete client;
  client = nullptr;
  parseFigures(body);
  // return parseImages(body);
}
std::string Lego_Client::parseBody(const std::string& response) {
  std::string body;
  try {
    // std::string response;
    //  para que solo parsee la lectura esstara en el send, no se que
    //  antiparadigma este
    //  char buffer[1024];
    //  int bytes;
    //  while ((bytes = this->client.Read(buffer, sizeof(buffer))) > 0) {
    //    response.append(buffer, bytes);
    //  }
    size_t pos = response.find("\r\n\r\n");
    if (pos != std::string::npos) {
      body = response.substr(pos + 4);
    }
  } catch (const std::exception& error) {
    // ocupo un throw creativo  NARCOCODE
  }
  return body;
}

// version super quemada
// idea de Brandon Palacios usar Regex
// Implementacion de ivan
std::vector<std::string> Lego_Client::parseList(const std::string& body) {
  std::regex regex_figuras(R"regex(<OPTION value="([^"]+)">)regex");
  std::vector<std::string> figuras;
  std::smatch match;
  std::string::const_iterator searchStart(body.cbegin());
  while (std::regex_search(searchStart, body.cend(), match, regex_figuras)) {
    std::string valor = match[1];

    if (valor != "None" && valor != "0" && valor != "1" && valor != "2") {
      figuras.push_back(valor);
    }

    searchStart = match.suffix().first;
  }

  return figuras;
}

void Lego_Client::parseFigures(const std::string& body) {
  std::regex img_regex(R"img(SRC="([^"]+)")img");
  std::smatch match;
  std::string::const_iterator searchStart(body.cbegin());
  while (std::regex_search(searchStart, body.cend(), match, img_regex)) {
    std::string ruta = match[1];

    if (ruta.find("/lego/images/") != std::string::npos) {
      size_t slash = ruta.find_last_of('/');
      std::string nombre = ruta.substr(slash + 1);
      size_t punto = nombre.find_last_of('.');
      if (punto != std::string::npos) {
        nombre = nombre.substr(0, punto);
        //
      }

      ++this->lego_figuras[nombre];
    }

    searchStart = match.suffix().first;
  }

  // return images;
}

/// std::regex img_regex(R"(SRC="([^"]+)")");