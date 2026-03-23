#ifndef LEGO_CHIKI_CLIENTE
#define LEGO_CHIKI_CLIENTE

#include <string>
#include <vector>

#include "SSLSocket.h"
#include "Socket.h"

struct LegoPiece {
    int cantidad;
    std::string descripcion;
    LegoPiece(int c, std::string d) : cantidad(c), descripcion(d) {}
};

class Lego_Client {
 private:
  std::vector<std::string> figuras;

  VSocket* client;  // polimorfismo para Socket
  bool SSL;         // usar SSL o no
  int ipVer;        // 4 o 6

  int OpenConnection(const char* hostname = nullptr,
                     const char* port = nullptr);

  std::string readResponse();
  std::string buildHTTPRequest(const std::string& figura, int& mitad);
  std::string buildListRequest();
  std::string parseBody(const std::string& response);
  std::vector<std::string> parseList(const std::string& body);
  std::vector<std::string> parseImages(const std::string& body);

  //Parser Jafet
  std::vector<LegoPiece> parseResponse(const std::string& body);

 public:
  Lego_Client(bool SSL = false, int puerto = 80, const char* host = nullptr);

  ~Lego_Client();
  std::string solicitarLista();
  std::vector<std::string> solicitarMediaFigura(const std::string& figura,
                                                int mitad);
  void cicloCliente();
};

#endif