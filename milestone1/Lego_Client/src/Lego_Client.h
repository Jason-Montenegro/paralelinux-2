#ifndef LEGO_CHIKI_CLIENTE
#define LEGO_CHIKI_CLIENTE

#include <map>
#include <string>
#include <vector>

#include "SSLSocket.h"
#include "Socket.h"

class Lego_Client {
 private:

  std::map<std::string, int> lego_figuras; // actualmente no almacena nada al final del ciclo
  VSocket* client;  // polimorfismo para Socket
  const char* lego_ip;
  int SSL;    // usar SSL o no
  int ipVer;  // 4 o 6
  const char* host;

  int OpenConnection();

  std::string readResponse();
  std::string buildHTTPRequest(const std::string& figura, int& mitad);
  std::string buildListRequest();
  std::string parseBody(const std::string& response);
  std::string mostrarMenu();
  std::vector<std::string> parseList(const std::string& body);
  void parseFigures(const std::string& body);

  /* Clase comprobacion mas adelante*/
  bool validateEntry(std::string);
 public:
  Lego_Client(int ipVer = 4, int SSL = 0, const char* host = nullptr,
              const char* lego_ip = "127.0.0.1");

  ~Lego_Client();
  std::string solicitarLista();
  void solicitarMediaFigura(const std::string& figura,
                                                int mitad);
  void cicloCliente();

  // fe80
  // static const char* legov6 = "fe80::8f5a:e2e1:7256:ffe3%enp0s31f6"; //ip6
  // static const char* legov4 = "127.0.0.1"; // ip4
};

#endif