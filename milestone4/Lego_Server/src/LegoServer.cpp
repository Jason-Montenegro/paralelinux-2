#include "LegoServer.hpp"

#include <cstring>
#include <ifaddrs.h>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string>
#include <sys/types.h>
#include <thread>
#include <vector>

#include "EzTypes.hpp"
#include "FileSystem.hpp"
#include "SSLSocket.h"
#include "Socket.h"

/// Macro to simplify <expected> API with non-void expected types
/// creates a varName variable on success
/// returns std::unexpected(error)
#define ASSIGN_OR_RETURN(varName, expression)                         \
  auto&& resVarName##varName = (expression);                          \
  if (!resVarName##varName.has_value())                               \
    return std::unexpected<ez::EzError>(resVarName##varName.error()); \
  auto varName = std::move(resVarName##varName.value());

/// Macro to simplify <expected> API with non-void expected types
/// creates a varName variable on success
/// uses error handler
#define ASSIGN_OR_ELSE(varName, expression, onError) \
  auto&& resVarName##varName = (expression);         \
  if (!resVarName##varName.has_value())              \
    return onError(resVarName##varName.error());     \
  auto varName = std::move(resVarName##varName.value());

/// Macro to simplify <expected> API with void expected types
/// returns std::unexpected(error)
#define EXPECT_OR_RETURN(expression)                         \
  {                                                          \
    auto&& res##res = (expression);                          \
    if (!res##res.has_value())                               \
      return std::unexpected<ez::EzError>(res##res.error()); \
  }

/// Macro to simplify <expected> API with void expected types with error handler
#define EXPECT_OR_ELSE(expression, onError)                      \
  {                                                              \
    auto&& res##res = (expression);                              \
    if (!res##res.has_value()) return onError(res##res.error()); \
  }

int PORT4SSL = 9600;
int PORT4 = PORT4SSL + 11;
int PORT6SSL = PORT4 + 11;
int PORT6 = PORT6SSL + 11;
int BACKLOG_QUEUE = 5;
uint32_t DEFAULT_DECISIONS_INODE = 1;

LegoServer::LegoServer(const char* key, const char* cert, int queueCapacity,
                         ez::FileSystem& ezyFile, std::string path)
    : key(key), cert(cert), queueCapacity(queueCapacity), ezyFile(ezyFile) {
  this->server_socket_4 = nullptr;
  this->server_socket_6 = nullptr;
  this->server_socket_4_ssl = nullptr;
  this->server_socket_6_ssl = nullptr;
  initializeFileSystem(path);

  if (queueCapacity < 1) {
    queueCapacity = BACKLOG_QUEUE;
  }
  std::cout << "termina de montar disco" << std::endl;
}

void LegoServer::initializeFileSystem(std::string fsPath) {
  bool shouldPopulateFileSystem = false;
  if (!std::filesystem::exists(fsPath)) {
    shouldPopulateFileSystem = true;
    // format fs to ~95MB
    EXPECT_OR_ELSE(
        this->ezyFile.format(fsPath, 100'000'000),
        [&](const auto& err) { throw std::runtime_error(ez::errorToStr(err)); })
  }
  EXPECT_OR_ELSE(this->ezyFile.mount(fsPath), [&](const auto& err) {
    throw std::runtime_error(ez::errorToStr(err));
  })
  if (shouldPopulateFileSystem) {
    this->populateFileSystem();
  }
}

void LegoServer::populateFileSystem() {
  populateFigure("/duck",
                 "[1,brick 2x2 slope orange]"
                 "[2,brick 2x3 yellow][2,brick 2x2 yellow]",
                 "1");

  populateFigure("/duck",
                 "[2,brick 1x1 yellow eye]"
                 "[1,brick 2x1 yellow][1,brick 2x4 slope orange]",
                 "2");

  populateFigure("/horse",
                 "[5,brick 2x2 yellow]"
                 "[4,brick 2x4 white]"
                 "[2,brick 2x4 yellow]"
                 "[1,brick 2x6 yellow]",
                 "1");

  populateFigure("/horse",
                 "[1,brick 2x3 yellow]"
                 "[2,brick 1x1 yellow eye]"
                 "[2,brick 2x1 yellow]"
                 "[1,brick 2x2 white]",
                 "2");

  populateFigure("/lion",
                 "[1,brick 2x8 yellow]"
                 "[6,brick 2x2 yellow]"
                 "[3,brick 2x4 yellow]"
                 "[5,brick 1x4 yellow]"
                 "[6,brick 1x2 yellow]"
                 "[1,brick 2x4 black]",
                 "1");

  populateFigure("/lion",
                 "[1,plate 2x3 yellow]"
                 "[2,brick 1x2 black]"
                 "[2,brick 2x2 black]"
                 "[1,brick 2x2 red]"
                 "[1,brick 1x1 eyes dark grey]",
                 "2");

  populateFigure("/giraffe",
                 "[4,brick 2x2 black]"
                 "[4,brick 2x2 yellow]"
                 "[6,brick 2x1 yellow]"
                 "[6,brick 2x1 black]"
                 "[1,brick 2x6 yellow]",
                 "1");

  populateFigure("/giraffe",
                 "[2,brick 2x4 yellow]"
                 "[1,brick 2x3 yellow]"
                 "[1,brick 1x4 yellow]"
                 "[1,brick 2x3 red]"
                 "[2,brick 1x1 eye yellow]",
                 "2");

  populateFigure("/fish",
                 "[3,brick 2x4 yellow]"
                 "[1,brick 2x3 yellow]"
                 "[2,brick 1x1 yellow eye]",
                 "1");

  populateFigure("/fish",
                 "[2,brick 2x2 translucent blue]"
                 "[4,brick 2x2 translucent red]",
                 "2");

  populateFigure("/pablo",
                 "[7,brick 6x6 slope red]"
                 "[2,brick 2x3 yellow][2,brick 2x2 yellow]",
                 "1");

  populateFigure("/pablo",
                 "[2,brick 1x1 yellow eye]"
                 "[1,brick 2x1 yellow][1,brick 2x4 slope orange]",
                 "2");

  populateLogFile();
}

void LegoServer::populateFigure(std::string directory, std::string figuras,
                                 std::string mitad) {
  auto result = ezyFile.stat(directory);  // verifica que este el directorio o no

  if (result.error() == EzError::NOT_FOUND) {  // No existe
    auto res_dir = ezyFile.mkdir(directory);
  }
  std::string p1 = directory + "/" + mitad + ".txt";
  std::cout << "Intentando escribir: " << p1 << std::endl;
  auto res_w = this->ezyFile.writeToFile(p1, figuras);

  if (!res_w.has_value()) {
    std::cout << "Fallo fatal en: " << p1
              << " Error: " << ez::errorToStr(res_w.error()) << std::endl;
    throw std::runtime_error(
        ez::errorToStr(res_w.error()));  // no se hace catch de esto
  }
}

void LegoServer::populateLogFile() {  // el root tendra figuras
  EXPECT_OR_ELSE(
      this->ezyFile.writeToFile("/logs.txt", ""),
      [&](const auto& err) { throw std::runtime_error(ez::errorToStr(err)); })
}

LegoServer::~LegoServer() { workerThreads.clear(); }

void LegoServer::handleSignal(int signal) {
  if (signal == SIGINT) {
    std::cout << " Cerrando  Server" << std::endl;
  }
}

int LegoServer::runServer() {
  std::signal(SIGINT, LegoServer::handleSignal);
  std::vector<std::thread> worker_threads;

  try {
    initializeSockets();
    bindServer();
    listen();
    pause(); 

    for (auto& t : workerThreads) { 
      t.request_stop();
    }

    for (auto& t : workerThreads) {
      t.join();
    }
    closeSockets();
  } catch (const std::exception& e) {
    std::cerr << "Muerte del server " << e.what() << std::endl;
    registrarEvento(Entidad::SERVIDOR, TipoLog::ERROR,
                    "Fallo al iniciar servidor: " + std::string(e.what()));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void LegoServer::initializeSockets() {
  this->server_socket_4 = new Socket('s', 0);
  this->server_socket_6 = new Socket('s', 1);
  this->server_socket_4_ssl = new SSLSocket((const char*)cert,  // cert0123.pem
                                            (const char*)key, false, true);
  this->server_socket_6_ssl =
      new SSLSocket((const char*)cert, (const char*)key, true, true);

  if (server_socket_4 == nullptr && server_socket_6 == nullptr &&
      server_socket_6_ssl == nullptr && server_socket_4_ssl == nullptr) {
    throw std::runtime_error("No se pudo crear socket del servidor");
  }
}

void LegoServer::bindServer() {
  int p1 = bindAvailablePort(server_socket_4, PORT4);
  this->puertoB = p1;
  int p2 = bindAvailablePort(server_socket_6, PORT6);
  int p3 = bindAvailablePort(server_socket_4_ssl, PORT4SSL);
  int p4 = bindAvailablePort(server_socket_6_ssl, PORT6SSL);
  std::string puertos = "Servidor escuchando en puertos: ";
  puertos += std::to_string(p1) + ", " + std::to_string(p2) + ", " +
             std::to_string(p3) + ", " + std::to_string(p4);
  registrarEvento(Entidad::SERVIDOR, TipoLog::ENRUTAMIENTO, puertos);

  this->server_socket_4->MarkPassive(queueCapacity);
  this->server_socket_6->MarkPassive(queueCapacity);
  this->server_socket_4_ssl->MarkPassive(queueCapacity);
  this->server_socket_6_ssl->MarkPassive(queueCapacity);
  this->server_socket_4->SetTimeout(5);
  this->server_socket_6->SetTimeout(5);
  this->server_socket_4_ssl->SetTimeout(5);
  this->server_socket_6_ssl->SetTimeout(5);
}

int LegoServer::bindAvailablePort(VSocket* socket, int startPort) {
  int maxPort = startPort + 500;
  for (int port = startPort; port < maxPort; ++port) {
    try {
      socket->Bind(port);
      std::cout << "PUERTO " << port << " disponible" << std::endl;
      return port;
    } catch (const std::runtime_error& e) {
      // no hace nada
    }
  }
  throw std::runtime_error(
      "Sin puertos disponibles.");  // este error le hace catch runServer()
}

void LegoServer::listen() {
  workerThreads.emplace_back([this](std::stop_token st) {
    this->acceptLoop(st, this->server_socket_4);  // 80
  });

  workerThreads.emplace_back([this](std::stop_token st) {
    this->acceptLoop(st, this->server_socket_6);
  });

  workerThreads.emplace_back([this](std::stop_token st) {
    this->acceptLoop(st, this->server_socket_4_ssl);
  });

  workerThreads.emplace_back([this](std::stop_token st) {
    this->acceptLoop(st, this->server_socket_6_ssl);
  });
  // puede que exista puede que no
  // no espera por respuestas
  broadcastToNetwork();
  workerThreads.emplace_back([this](std::stop_token st) {
    this->serverDiscoveryListener(st);
  });
}

void LegoServer::acceptLoop(std::stop_token st, VSocket* server) {
  std::vector<std::jthread> workers;
  while (!st.stop_requested()) {
    VSocket* client = nullptr;
    try {
      client = server->AcceptConnection();
      if (client) {
        std::cout << "ya hay una nueva conexion" << std::endl;
        registrarEvento(Entidad::SERVIDOR, TipoLog::SOLICITUD,
                        "Nueva conexion TCP establecida");
        workers.emplace_back([this, client](std::stop_token st) {
          handleClientConnection(st, client);
        });
      }
    } catch (const std::exception& e) {
      if (client) {  // acepta pero hay una falla posterior
        delete client;
        client = nullptr;
      }
    }
  }
}

void LegoServer::handleClientConnection(std::stop_token st, VSocket* client) {
  SSLSocket* sslClient = dynamic_cast<SSLSocket*>(client);
  if (sslClient) {
    std::cout << "manejando conexion con SSL y verificando que el cliente "
                 "confia en el certificado, en medio handshake"
              << std::endl;
    try {
      sslClient->AcceptSSL();  // si falla hace
      registrarEvento(Entidad::INTERM, TipoLog::ENRUTAMIENTO,
                      "Handshake SSL completado con exito");
    } catch (const std::exception& e) {
      std::cerr << "Error de Handshake SSL" << e.what() << std::endl;
      registrarEvento(Entidad::INTERM, TipoLog::ERROR,
                      "Handshake SSL fallido: " + std::string(e.what()));
      client->Close();
      delete client;
      return;  // El hilo termina
    }
  }
  std::cout << "manejando conexiones" << std::endl;

  int keep_alive = 1;
  std::string connectionType;
  while (keep_alive && !st.stop_requested()) {
    std::stringstream datos;
    std::string stringData;

    // llega a 0
    if (!readHeaders(client, datos, stringData, st)) {
      std::cout << "muere conexion" << std::endl;
      client->Close();
      delete client;
      return;  // El hilo termina
    }

    registrarEvento(Entidad::CLIENTE, TipoLog::SOLICITUD,
                    "Datos HTTP recibidos (Header)");

    std::cout << stringData << std::endl;
    if (!HTTPver(stringData, "1.1")) {  // si es 1.0
      keep_alive = 0;
    }
    std::stringstream body_s;
    std::string body = "";

    readBody(body_s, stringData, client, st);
    body = body_s.str();
    connectionType = "keep-alive";
    if (stringData.find("Connection: close") != std::string::npos) {
      connectionType = "close";
      keep_alive = 0;
    }

    if (stringData.find("GET") != std::string::npos) {
      std::string figure = "";
      if (stringData.find("&lego/figure=") !=
          std::string::npos) {  /// lego/list.php?figure=
        auto request = parseLegoRequest(stringData);
        registrarEvento(Entidad::SERVIDOR, TipoLog::ENRUTAMIENTO,
                        "Procesando GET para figura: " + request.first);
        std::string fullPath =
            "/" + request.first + "/" + std::to_string(request.second) + ".txt";
        this->ezyMtx.lock();
        auto contenido = ezyFile.readFromFile(fullPath);
        ezyFile.removeFile(fullPath);  // las mitades se borran
        this->ezyMtx.unlock();
        if (!contenido) { // mitad de figura no existe en realidad
          std::string response =
              buildHttpResponse("404 Not Found", errorBody.size(), "text/html",
                                 connectionType, errorBody);
          client->Write(response.c_str());  // mismo cliente se pudre todo
          registrarEvento(
              Entidad::SERVIDOR, TipoLog::ERROR,
              "404 - Pieza no encontrada en FileSystem: " + fullPath);
        } else {
          registrarEvento(Entidad::SERVIDOR, TipoLog::RESPUESTA,
                          "Enviando pieza " + std::to_string(request.second) +
                              " de " + request.first);
          const auto& v = contenido.value();
          std::string str;
          str.reserve(v.size());
          // viejo lego server
          std::transform(
              v.cbegin(), v.cend(), std::back_inserter(str), [](std::byte b) {
                return static_cast<char>(std::to_integer<uint8_t>(b));
              });
          std::string response = buildHttpResponse(
              "200 OK", str.size(), "text/html", connectionType, str);
          client->Write(response.c_str());  // enviadito
        }
      } else {
        if (stringData.find("&logs=") != std::string::npos) {
          // modularizar con el de arriba
          this->ezyMtx.lock();
          auto contenido = ezyFile.readFromFile("/logs.txt");
          this->ezyMtx.unlock();
          const auto& v = contenido.value();
          std::string str;
          str.reserve(v.size());

          std::transform(
              v.cbegin(), v.cend(), std::back_inserter(str), [](std::byte b) {
                return static_cast<char>(std::to_integer<uint8_t>(b));
              });  //
          std::stringstream ss(str);
          std::string stringsito = ss.str();
          std::string response =
              buildHttpResponse("200 OK", stringsito.size(), "text/html",
                                 connectionType, stringsito);
          client->Write(response.c_str());
        } else {
          if (stringData.find("GET / ") != std::string::npos ||
              stringData.find("&list/figures") != std::string::npos) {
            stringData = datos.str();
            std::string response = "";
            this->ezyMtx.lock();
            auto res = ezyFile.listEntries("/");
            this->ezyMtx.unlock();

            if (res.has_value()) {
              std::string filasFiguras = "";
              for (const auto& entry : res.value()) {
                if (entry.type == InodeType::DIRECTORY) {
                  if (entry.name != "." && entry.name != "..") {
                    filasFiguras += entry.name + " ";
                  }
                }
              }
              client->Write(filasFiguras.c_str());
            }
          }
        }
      }
    } else {
      if (stringData.find("POST") != std::string::npos) {
        registrarEvento(Entidad::CLIENTE, TipoLog::SOLICITUD,
                        "Intento de POST para nueva figura");
        auto request = parseLegoRequest(stringData);
        std::string directory = "/" + request.first;
        // escribe la estructura de guardadito
        this->ezyMtx.lock();
        populateFigure(directory, body, std::to_string(request.second));
        this->ezyMtx.unlock();
        registrarEvento(Entidad::SERVIDOR, TipoLog::RESPUESTA,
                        "POST procesado: figura guardada en disco");
      }
    }
  }
  registrarEvento(Entidad::SERVIDOR, TipoLog::ENRUTAMIENTO,
                  "Cerrando conexion con el cliente");
  client->Close();
  delete client;
}

void LegoServer::registrarEvento(Entidad e, TipoLog t, const std::string& m) {
  this->ezyMtx.lock();
  std::string nuevaLinea = logger.log(e, t, m);
  std::string contenidoTotal = "";
  auto resultado = ezyFile.readFromFile("/logs.txt");
  if (resultado.has_value()) {
    const auto& bytes = resultado.value();

    contenidoTotal.assign(reinterpret_cast<const char*>(bytes.data()),
                          bytes.size());
  }
  // concatenar y persistir
  contenidoTotal += nuevaLinea + "\n";
  ezyFile.writeToFile("/logs.txt", contenidoTotal);
  this->ezyMtx.unlock();
}

int LegoServer::readHeaders(VSocket* client, std::stringstream& datos,
                             std::string& stringData, std::stop_token& st) {
  char bufferin;
  int bytesRead = 0;
  int finalizacion = 0;
  try {
    while (!finalizacion &&
           (bytesRead = client->Read(
                &bufferin, 1))) {  // si es igual a menos -1 que demonios hago
      if (bytesRead != -1) {
        datos << bufferin;
        if (bufferin == '\n') {
          stringData = datos.str();
          if (stringData.find("\r\n\r\n") != std::string::npos) {
            ++finalizacion;
          }
        }
      } else {  // -1 revisar
        // es un agregado en caso de timer al cliente para que cierre conexion
        // con el cliente y deje de hacer lo que estaba haciendo
        if (st.stop_requested()) {
          ++finalizacion;
          bytesRead = 0;
          registrarEvento(Entidad::CLIENTE, TipoLog::SOLICITUD,
                          "Cerrando conexion por condicion de parada  ");
        }
      }
    }
  } catch (...) {
    registrarEvento(Entidad::CLIENTE, TipoLog::ERROR,
                    "Error en lectura de datos de cleinte");
    bytesRead = 0;
  }
  return bytesRead;  // si termina en un numero positivo es que logro obtener ya
                     // todos los headers
}

int LegoServer::HTTPver(const std::string& stringData,
                         const std::string& versionEsperada) {
  std::size_t endOfFirstLine = stringData.find("\r\n");
  int retorno = 0;
  if (endOfFirstLine != std::string::npos) {
    std::string firstLine = stringData.substr(0, endOfFirstLine);

    std::size_t httpPos = firstLine.find("HTTP/");
    if (httpPos != std::string::npos) {
      std::string versionEncontrada = firstLine.substr(httpPos + 5);
      retorno = (versionEncontrada == versionEsperada);
    }
  }
  return retorno;
}

int LegoServer::readBody(std::stringstream& body_s, std::string& stringData,
                          VSocket* client, std::stop_token& st) {
  int retorno = 0;
  auto pos = stringData.find("Content-Length: ");
  if (pos != std::string::npos) {
    int content_length = 0;
    if (pos != std::string::npos) {
      uint32_t start = pos + 16;
      uint32_t end = stringData.find("\r\n", start);
      content_length = std::stoi(stringData.substr(start, end - start));
    }
    char bufferin = 0;
    int bytes_readed = 0;
    ssize_t result = 1;
    try {
      while (result && bytes_readed < content_length && !st.stop_requested()) {
        result = client->Read(&bufferin, 1);
        if (result > 0) {
          body_s << bufferin;
          ++bytes_readed;
        } else {
          if (st.stop_requested()) {
            bytes_readed = 0;
            registrarEvento(Entidad::CLIENTE, TipoLog::SOLICITUD,
                            "Cerrando conexion por condicion de parada  ");
          }
        }
      }
    } catch (const std::exception& e) {
      std::cerr << "Error fatal en readBody: " << e.what()
                << std::endl;  // retorno - igua
      registrarEvento(Entidad::CLIENTE, TipoLog::ERROR,
                      "no se logro procesar todo el length del body");
    }
    if (bytes_readed == content_length) {
      retorno = 1;
    }
  }
  return retorno;
}

std::pair<std::string, int> LegoServer::parseLegoRequest(
    const std::string& stringData) {
  std::string figure = "";
  int part = 1;
  size_t figPos = stringData.find("figure=");
  if (figPos != std::string::npos) {
    figPos += 7;
    size_t figEnd = stringData.find("&", figPos);
    if (figEnd == std::string::npos) {
      figEnd = stringData.find(" ", figPos);
    }
    if (figEnd != std::string::npos) {
      figure = stringData.substr(figPos, figEnd - figPos);
    }
  }
  std::cout << "figura solicitada " << figure << std::endl;

  size_t partPos = stringData.find("part=");
  if (partPos != std::string::npos) {
    try {
      part = std::stoi(stringData.substr(partPos + 5, 1));
    } catch (...) {
      part = 1;  //
    }
  }
  std::cout << "  parte soliciutada " << part << std::endl;
  return {figure, part};
}

std::string LegoServer::buildHttpResponse(
    const std::string statusCode,  // "200 OK", "404 Not Found", 100 ERROR.
    uint32_t length, std::string contentType, std::string type_conect,
    std::string& content) {
  return "HTTP/1.1 " + statusCode + "\r\n" + "Content-Type: " + contentType +
         "; charset=UTF-8\r\n" + "Content-Length: " + std::to_string(length) +
         "\r\n" + "Connection: " + type_conect + "\r\n" + "\r\n" + content;
}

void LegoServer::broadcastToNetwork() {
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) return;

  int enable = 1;
  setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(5001);  // Puertos
  addr.sin_addr.s_addr = inet_addr("192.168.1.255");//172.16.123.111");  // Red broadcast

  std::string msg = "SERVER " + std::to_string(this->puertoB);
  sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&addr, sizeof(addr));

  std::cout << "Broadcast enviado a intermediarios  existentes"
            << std::endl;
  close(sock);
}

void LegoServer::serverDiscoveryListener(std::stop_token st) {
  std::cout << "Escuchando para intermediario udp" << std::endl;
  VSocket * discoverySocket = new Socket('d', 0);
  int puertoUDP = 5000;
  discoverySocket->SocketD(puertoUDP);
  discoverySocket->SetTimeout(5);
  char buffer[1024];

  while (!st.stop_requested()) {
    try {
      sockaddr_in sender{};
      int bytes = discoverySocket->recvFrom(
        reinterpret_cast<uint8_t*>(buffer), sizeof(buffer) - 1, (sockaddr*)&sender);
      if (bytes > 0) {
        buffer[bytes] = '\0';
        std::string msg(buffer);

        if (msg == "FIGURAS_IVAN") {
          std::cout<<"descubrimiento"<<std::endl;
          answerToServerDiscovery(*discoverySocket, sender); 
        }
      }
    } catch (const std::runtime_error& e) {
      if (st.stop_requested()) {
        break;
      }
    } catch (...) {
      std::cerr << "[LegoServer] Error inesperado en discovery" << std::endl;
      if (st.stop_requested()) break;
    }
  }
  delete discoverySocket;
  

  std::cout << "Cerrando serverDiscoveryListener de Figuras limpiamente." << std::endl;
  // Al salir del scope, el destructor de VSocket debería encargarse de
  // hacer el close(sock) automáticamente por RAII
}

void LegoServer::answerToServerDiscovery(VSocket& discoverySocket,
    sockaddr_in sender) {
  std::string response = "SERVER " + std::to_string(this->puertoB);
  discoverySocket.sendTo(
      (response.c_str()), 
      response.size(), 
      (sockaddr*)&sender
  );
  std::cout << "Respondiendo a " << inet_ntoa(sender.sin_addr) << ":"
            << ntohs(sender.sin_port) << " con el mensaje: [" << response << "]"
            << std::endl;
}

void LegoServer::closeSockets() {
  if (server_socket_4) {
    this->server_socket_4->Close();
    delete this->server_socket_4;
  }

  if (server_socket_6) {
    this->server_socket_6->Close();
    delete this->server_socket_6;
  }
  if (server_socket_4_ssl) {  //
    this->server_socket_4_ssl->Close();
    delete this->server_socket_4_ssl;
  }
  if (server_socket_6_ssl) {
    this->server_socket_6_ssl->Close();
    delete this->server_socket_6_ssl;
  }
}
