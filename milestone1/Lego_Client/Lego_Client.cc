#include "Lego_Client.h"

#include <cstring>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include <vector>
#include <cstring>
#include <string.h>
#include <stdlib.h>

#include "SSLSocket.h"
#include "Socket.h"

// no se si se quejara jason que es mejor un define o alguna vaina loca que

static const char* SERVER_HOST = "os.ecci.ucr.ac.cr";
static const char* HTTPS_PORT = "443";
static const char* HTTP_PORT = "80";
static const char* legov6 = "fe80::8f5a:e2e1:7256:ffe3%enp0s31f6";
static const char* legov4 = "127.0.0.1";

// activa la conexion con el chiky server
int Lego_Client::OpenConnection(const char* hostname, const char* port) {
  int st = -1;
  if (this->SSL) {
    // printf("Connecting SSL");
    if (4 == this->ipVer) {
      this->client = new SSLSocket();  // Create an IPv4 TCP SSL socket
      st = this->client->Connect(SERVER_HOST, HTTPS_PORT);
      // printf(" IPv4\n");
    } else {
      this->client = new SSLSocket(true);  // Create an IPv6 TCP SSL socket
      st = this->client->Connect(legov6, HTTPS_PORT);
      // printf(" IPv6\n");
    }
  } else {
    // printf("Connecting non-SSL");
    if (4 == this->ipVer) {
      this->client = new Socket('s');  // Create an IPv4 TCP socket
      st = this->client->Connect(legov4, HTTP_PORT);
      // printf(" IPv4\n");
    } else {
      this->client = new Socket('s', true);  // Create an IPv6 TCP socket
      st = this->client->Connect(legov6, HTTP_PORT);
      // printf(" IPv6\n");
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

Lego_Client::Lego_Client(bool SSL, int puerto, const char* host)
    : client(nullptr), SSL(SSL) {
  this->ipVer = 4;
  this->SSL = true;
  // if (mitad != 1 && mitad != 2) {
  //   throw LegoClientException("LegoClient(const std::string& , int , bool
  //   )");
  // }
}

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
  auto set_figuras1 = solicitarMediaFigura(figura, 1);
  auto set_figuras2 = solicitarMediaFigura(figura, 2);

  for (const auto& f : set_figuras1) {
    std::cout << f << std::endl;
  }
  for (const auto& f : set_figuras2) {
    std::cout << f << std::endl;
  }

  // ocupa interfaz de cout y cin para ver la lista y que el usuario responda
  // close();
}

std::vector<std::string> Lego_Client::solicitarMediaFigura(
    const std::string& figura, int mitad) {
  OpenConnection();  // al parecer siempre abre conexion que pida algo, porque
                     // al recibir una respuesta se remuere

  std::string request = buildHTTPRequest(figura, mitad);
  this->client->Write(request.c_str());
  std::string mFigura = readResponse();
  std::string body = parseBody(mFigura);
  // std::cout << "figura" << std::endl;
  // std::cout << body << std::endl;
  // figuras.push_back(body);
  //  TODO IVAN no tengo claro aun claro si quieren que tenga un
  //  vector con string de cada pequena pieza de la figura
  delete client;
  client = nullptr;
  return parseImages(body);
}

//strstr> search for first appear of chain from another chain. ret a pointer to first or Null.
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
    size_t pos = response.find("\r\n\r\n"); //cuerpo despues de cabeceras
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

std::vector<std::string> Lego_Client::parseImages(const std::string& body) {
  std::regex img_regex(R"img(SRC="([^"]+)")img");

  std::vector<std::string> images;
  std::smatch match;
  std::string::const_iterator searchStart(body.cbegin());

  while (std::regex_search(searchStart, body.cend(), match, img_regex)) {
    std::string ruta = match[1];

    // opcional: filtrar solo imágenes de piezas
    if (ruta.find("/lego/images/") != std::string::npos) {
      images.push_back("https://os.ecci.ucr.ac.cr" + ruta);
    }

    searchStart = match.suffix().first;
  }

  return images;
}
/// std::regex img_regex(R"(SRC="([^"]+)")");


/* 
*  Implementation of a parser of whole body HTML,
*  takes messages and transforms to object LegoPiece, 
*  takes num of piece and description
*/
std::vector<LegoPiece> Lego_Client::parseResponse(const std::string& response){
  std::vector<LegoPiece> vectLegos;

  //Locate http body
  const char* body = strstr(response.c_str(), "\r\n\r\n");
  if (body == nullptr){
    throw std::runtime_error("Lego_Client::parseBody( const string) headers not found");
  }
  body +=4; //jump \r\n\r\n

  // Locate table
  const char* table =  strstr(body, "<TABLE");
  if (!table){
    throw std::runtime_error("Lego_Client::parseBody( const string)  Table not found");
  }

  // skip headers text <TR>
  const char* ini = strstr(table, "<TR>");
  if (!ini) return vectLegos;

  const char* end = strstr(table, "</TR>");
  if (!end)return vectLegos;

  const char* pos = end + 5; // after </TR>

  const char* row;
  while ((row=strstr(pos, "<TR>"))!= nullptr){  
    
    // find number of piece, locate TD
    const char* tdQty = strstr(row, "<TD");
    if (!tdQty) break; 

    const char* startNum = strchr(tdQty, '>'); 
    startNum++; //space
    const char* endNum = strstr(startNum, "</TD>");

    // extract quantity
    std::string num(startNum, endNum - startNum);
    size_t notSpace = num.find_first_not_of(" \t\n\r");
    if (notSpace!= std::string::npos){
      num = num.substr(notSpace); // extract the string of number
    }
    int aux = 0;
    try{ // try to stoi
      aux = std::stoi(num);
    }
    catch(const std::exception& e){
      pos = end;
      continue;
    }
    
    //Locate <TD>, for description of piece
    const char* tdDesc = strstr(endNum, "<TD");
    if (!tdDesc) break;
    
    const char* startDesc = strchr(tdDesc, '>');
    if (!startDesc) break;
    startDesc++;

    const char* endDesc = strstr(startDesc, "</TD>");
    if (!endDesc) break;

    // Extract description
    std::string description(startDesc, endDesc - startDesc);
    size_t firstChar = description.find_first_not_of(" \t\n\r");
    size_t lastChar = description.find_last_not_of(" \t\n\r");
    if (firstChar != std::string::npos && lastChar != std::string::npos) {
      description = description.substr(firstChar, lastChar - firstChar + 1);
    }
    vectLegos.emplace_back(aux, description);
    pos = endDesc; // move to the end of this row for next iteration
  }
  return vectLegos;
}
/**/