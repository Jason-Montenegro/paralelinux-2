#include "Lego_Client.h"

#include <cstring>
#include <iostream>
#include <map>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

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

void Lego_Client::cicloCliente() {
  std::string lista = solicitarLista();
  std::cout << "lista solicitada :" << std::endl << std::endl;
  // std::cout << lista << std::endl;
  auto figuras_lista = parseList(lista);
  for (const auto& f : figuras_lista) {
    std::cout << f << std::endl;
  }
  std::string figura;
  std::cin >> figura;
  solicitarMediaFigura(figura, 1);
  solicitarMediaFigura(figura, 2);
  std::cout << ": " << this->lego_figuras.size() << std::endl;
  for (const auto& par : lego_figuras) {
    std::cout << par.first << " cantidad: " << par.second << std::endl;
  }

  // close()
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