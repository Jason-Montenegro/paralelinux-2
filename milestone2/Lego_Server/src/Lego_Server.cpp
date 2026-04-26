
/// ivan desings
#include "Lego_Server.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "FileSystem.hpp"
#include "SSLSocket.h"
#include "Socket.h"
// #include "EzFileSystem.hpp"

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

int PORT4SSL = 8000;
int PORT4 = 8501;
int PORT6SSL = 9002;
int PORT6 = 9503;
// size_t BUFFER_SIZE = 8192;
int COLITA = 5;
uint32_t DEFAULT_DECISIONS_INODE = 1;

Lego_Server::Lego_Server(const char* host, const char* lego_ip,
                         int queue_capacity, ez::FileSystem& ezy_file,
                         std::string path)
    : ipVer(ipVer),
      host(host),
      queue_capacity(queue_capacity),
      ezy_file(ezy_file) {
  this->server_socket_4 = nullptr;
  this->server_socket_6 = nullptr;
  this->server_socket_4_ssl = nullptr;
  this->server_socket_6_ssl = nullptr;
  initializeFileSystem(path);
  std::cout << "tiene disco disco" << std::endl;

  if (queue_capacity < 1) {
    queue_capacity = COLITA;
  }
  std::cout << "termina de montar disco" << std::endl;
}

Lego_Server::~Lego_Server() {
  threads.clear(); 
}

void Lego_Server::handleSignal(int signal) {
  if (signal == SIGINT) {
    // Lego_Server::stopFlag = true;
    std::cout << " Cerrando Ivan Server" << std::endl;
  }
}

int Lego_Server::cicloServer() {
  std::signal(SIGINT, Lego_Server::handleSignal);
  // API* api = nullptr;
  std::vector<std::thread> worker_threads;

  try {
    inicializar_Server();
    bindServer();
    listen();
    pause();  // espera que manden ctrl c

    for (auto& t : threads) {  // esparce la condicion de parada
      t.request_stop();
    }
   
    for (auto& t : threads) {
      t.join();
    }
     cierreServers();
  } catch (const std::exception& e) {
    std::cerr << "Muerte del server " << e.what() << std::endl;
    registrarEvento(Entidad::SERVIDOR, TipoLog::ERROR,
                    "Fallo al iniciar servidor: " + std::string(e.what()));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void Lego_Server::inicializar_Server() {
  this->server_socket_4 = new Socket('s', 0);
  this->server_socket_6 = new Socket('s', 1);
  this->server_socket_4_ssl =
      new SSLSocket((const char*)"cert0123.pem",  // cert0123.pem
                    (const char*)"key0123.pem", false, true);

  this->server_socket_6_ssl = new SSLSocket((const char*)"cert0123.pem",
                                      (const char*)"key0123.pem", true, true);
  if (server_socket_4 == nullptr && server_socket_6 == nullptr &&
      server_socket_6_ssl == nullptr && server_socket_4_ssl == nullptr) {
    //  std::cerr << "Error: No se pudo crear socket del servidor" << std::endl;
    throw std::runtime_error("No se pudo crear socket del servidor");
  }
}

void Lego_Server::bindServer() {
  int p1 = BindPortAvailable(server_socket_4, PORT4);
  int p2 = BindPortAvailable(server_socket_6, PORT6);
  int p3 = BindPortAvailable(server_socket_4_ssl, PORT4SSL);
  int p4 = BindPortAvailable(server_socket_6_ssl, PORT6SSL);
  std::string puertos = "Servidor escuchando en puertos: ";  // aja
  puertos += std::to_string(p1) + ", " + std::to_string(p2) + ", " +
             std::to_string(p3) + ", " + std::to_string(p4);
  registrarEvento(Entidad::SERVIDOR, TipoLog::ENRUTAMIENTO, puertos);

  this->server_socket_4->MarkPassive(queue_capacity);
  this->server_socket_6->MarkPassive(queue_capacity);
  this->server_socket_4_ssl->MarkPassive(queue_capacity);
  this->server_socket_6_ssl->MarkPassive(queue_capacity);

  // tipo de solucion, no se si sea mejor hacer una conexion falsa con los
  // server_socket
  this->server_socket_4->SetTimeout(30);
  this->server_socket_6->SetTimeout(30);
  this->server_socket_4_ssl->SetTimeout(30);
  this->server_socket_6_ssl->SetTimeout(30);
}

int Lego_Server::BindPortAvailable(VSocket* socket, int startPort) {
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
      "Sin puertos disponibles.");  // este error lo hace catch ciclo server
}

void Lego_Server::listen() {
  threads.emplace_back([this](std::stop_token st) {
    this->acceptLoop(st, this->server_socket_4, 0);
  });

  threads.emplace_back([this](std::stop_token st) {
    this->acceptLoop(st, this->server_socket_6, 0);
  });

  threads.emplace_back([this](std::stop_token st) {
    this->acceptLoop(st, this->server_socket_4_ssl, 1);
  });

  threads.emplace_back([this](std::stop_token st) {
    this->acceptLoop(st, this->server_socket_6_ssl, 1);
  });
}

void  Lego_Server::cierreServers() {
  if(server_socket_4){
    this->server_socket_4->Close();
     delete this->server_socket_4;
  }
  
   if(server_socket_6){
      this->server_socket_6->Close();
       delete this->server_socket_6;
  }
   if(server_socket_4_ssl){ //
    this->server_socket_4_ssl->Close();
    delete this->server_socket_4_ssl;
  }
  if(server_socket_6_ssl){
     this->server_socket_6_ssl->Close();
     delete this->server_socket_6_ssl;
  }  
}
void Lego_Server::acceptLoop(std::stop_token st, VSocket* server, int ssl) {
  std::vector<std::jthread> workers;
  std::thread* worker;
  // VSocket* client = nullptr;
  while (!st.stop_requested()) {
    VSocket* client = nullptr;
    try {
      client = server->AcceptConnection();
      if (client) {
        std::cout << "ya hay una nueva conexion" << std::endl;

        registrarEvento(Entidad::SERVIDOR, TipoLog::SOLICITUD,
                        "Nueva conexion TCP establecida");
        if (ssl) {
          SSLSocket* sslClient = dynamic_cast<SSLSocket*>(client);
          SSLSocket* sslServer = dynamic_cast<SSLSocket*>(server);
          if (sslClient && sslServer) {
            sslClient->Copy(sslServer);
          }
        }
        workers.emplace_back([this, client](std::stop_token st) {
          connectionHandling(st, client);
        });
      }
    } catch (const std::exception& e) {
      if (client) {  // acepta pero hay una falla posterio
        delete client;
        client = nullptr;
      }
    }
  }
}

void Lego_Server::connectionHandling(std::stop_token st, VSocket* client) {
  SSLSocket* sslClient = dynamic_cast<SSLSocket*>(client);
  if (sslClient) {
    // esta en
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
  // char buffer[1024]{};

  int bytesRead = 0;
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

    std::cout << "prueba" << std::endl;
    std::cout << stringData << std::endl;

    if (!HTTPver(stringData, "1.1")) {
      keep_alive = 0;
    }
    std::stringstream body_s;
    std::string body = "";

    readBody(body_s, stringData,client,st);
    body = body_s.str();
    connectionType = "keep-alive";
    if (stringData.find("Connection: close") != std::string::npos) {
      connectionType = "close";
      keep_alive = 0;
    }
    if (stringData.find("GET") != std::string::npos) {
      std::string figure = "";
      int mitad = 0;
      if (stringData.find("&lego/figure=") !=
          std::string::npos) {  /// lego/list.php?figure=
        auto request = parseLegoRequest(stringData);
        registrarEvento(Entidad::SERVIDOR, TipoLog::ENRUTAMIENTO,
                        "Procesando GET para figura: " + request.first);
        std::string fullPath =
            "/" + request.first + "/" + std::to_string(request.second) + ".txt";
        this->ezy_mutex.lock();
        auto contenido = ezy_file.readFromFile(fullPath);
        this->ezy_mutex.unlock();
        if (!contenido) {
          std::cout << "figura no existente" << std::endl;
          std::string response =
              build_HttpResponse("404 Not Found", errorBody.size(), "text/html",
                                 connectionType, errorBody);
          client->Write(response.c_str());  // mismo cliente se pudre todo
          registrarEvento(
              Entidad::SERVIDOR, TipoLog::ERROR,
              "404 - Pieza no encontrada en FileSystem: " + fullPath);
          // return;
          //  manejar error
          //    return contenido.error();
        } else {
          registrarEvento(Entidad::SERVIDOR, TipoLog::RESPUESTA,
                          "Enviando pieza " + std::to_string(request.second) +
                              " de " + request.first);
          std::cout << "llega al casteo" << std::endl;
          const auto& v = contenido.value();
          std::string str;
          str.reserve(v.size());

          std::transform(
              v.cbegin(), v.cend(), std::back_inserter(str), [](std::byte b) {
                return static_cast<char>(std::to_integer<uint8_t>(b));
              });  //
          std::cout << "casteocorrecto" << std::endl;
          std::string body_html = buildFigurePage(request.first, str);
          std::string response =
              build_HttpResponse("200 OK", body_html.size(), "text/html",
                                 connectionType, body_html);
          //  std::cout << "envio respuesta" << response << std::endl;
          client->Write(response.c_str());  // enviadito
        }
      } else {
        if (stringData.find("&logs=") != std::string::npos) {
          // modularizar con el de arriba
          this->ezy_mutex.lock();
          auto contenido = ezy_file.readFromFile("/logs.txt");
          this->ezy_mutex.unlock();
          const auto& v = contenido.value();
          std::string str;
          str.reserve(v.size());

          std::transform(
              v.cbegin(), v.cend(), std::back_inserter(str), [](std::byte b) {
                return static_cast<char>(std::to_integer<uint8_t>(b));
              });  //
          std::cout << "todos los benditos logs" << str;
          std::stringstream ss(str);
          std::string line;
          std::string filasHtml = "";
          while (std::getline(ss, line)) {
            if (!line.empty()) {
              filasHtml += "<tr><td>INFO</td><td>" + line + "</td></tr>";
            }
          }
          std::string htmlFinal = LOG_TEMPLATE;  // Copiamos la plantilla
          std::string placeholder = "{{logContent}}";
          size_t pos = htmlFinal.find(placeholder);
          if (pos != std::string::npos) {
            htmlFinal.replace(pos, placeholder.length(), filasHtml);
          }
          std::string response =
              build_HttpResponse("200 OK", htmlFinal.size(), "text/html",
                                 connectionType, htmlFinal);
          client->Write(response.c_str());

        } else {
          if (stringData.find("GET / ") != std::string::npos ||
              stringData.find("&list/figures") != std::string::npos) {
            stringData = datos.str();
            std::string response = "";
            std::cout << "entro a una pagina y quiere respuesta" << std::endl;
            this->ezy_mutex.lock();
            auto res = ezy_file.listEntries("/");
            this->ezy_mutex.unlock();

            if (res.has_value()) {
              std::string filasFiguras = "";
              for (const auto& entry : res.value()) {
                if (entry.type == InodeType::DIRECTORY) {
                  if (entry.name != "." && entry.name != "..") {
                    filasFiguras += "<tr>";
                    filasFiguras += "  <td>" + entry.name + "</td>";
                    // filasFiguras +=
                    //     "  <td>Disponible</td>";
                    filasFiguras += "</tr><br>";
                  }
                }
              }
              response =
                  build_HttpResponse("200 OK", filasFiguras.size(), "text/html",
                                     connectionType, filasFiguras);
              client->Write(response.c_str());
            }
            // std::string html = "aun no se que poner :)";
          }
        }
      }
    } else {
      if (stringData.find("POST") != std::string::npos) {
        // modularizar esta parte
        registrarEvento(Entidad::CLIENTE, TipoLog::SOLICITUD,
                        "Intento de POST para nueva figura");
        auto request = parseLegoRequest(stringData);

        // std::string fullPath = "/" + request.first + "/" +
        // std::to_string(request.second) + ".txt";
        std::string directory = "/" + request.first;
        // escribe la estructura de guardadito
        this->ezy_mutex.lock();
        populateFigure(directory, body, std::to_string(request.second));
        this->ezy_mutex.unlock();
        // TODO (ivan)  hacer que el populate le retorne si lo logro  poner,
        // aunque probablemente es de try por el throw
        registrarEvento(Entidad::SERVIDOR, TipoLog::RESPUESTA,
                        "POST procesado: figura guardada en disco");
      }

      // memset(buffer, 0, sizeof(buffer));
    }
  }
  registrarEvento(Entidad::SERVIDOR, TipoLog::ENRUTAMIENTO,
                  "Cerrando conexion con el cliente");
  client->Close();
  delete client;
}

int Lego_Server::readHeaders(VSocket* client, std::stringstream& datos,
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
        // con el cliente y deje de hacer lo que estaba gaciendo
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

int Lego_Server::readBody(std::stringstream& body_s, 
                          std::string& stringData,VSocket * client,std::stop_token& st) {
  int retorno = 0;
  int pos = 0;
  if (pos = stringData.find("Content-Length: ") != std::string::npos) {
    int bodySize = 0;
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
     // body = body_s.str();
    }
  }
  return retorno;
}

std::string Lego_Server::buildFigurePage(const std::string& figureName,
                                         std::string content) {
  std::cout << "llega al build" << std::endl;
  std::string liElements = "<ul>\n";
  int totalPieces = 0;

  size_t pos = 0;
  size_t endPos = 0;
  while ((pos = content.find('[', pos)) != std::string::npos &&
         (endPos = content.find(']', pos)) != std::string::npos) {
    std::string entry = content.substr(pos + 1, endPos - pos - 1);
    size_t commaPos = entry.find(',');
    if (commaPos != std::string::npos) {
      std::string cantStr = entry.substr(0, commaPos);
      std::string nameStr = entry.substr(commaPos + 1);

      totalPieces += std::stoi(cantStr);
      liElements +=
          "    <li><strong>" + cantStr + "x</strong> " + nameStr + "</li>\n";
    }
    pos = endPos + 1;
  }
  liElements += "  </ul>";

  std::string html = FIGURES_TEMPLATE;

  replace(html, "{{figureName}}", figureName);
  replace(html, "{{figurePartList}}", liElements);
  replace(html, "{{totalPieceQuantity}}",
          "Cantidad Total: " + std::to_string(totalPieces));

  return html;
}
void Lego_Server::replace(std::string& target, const std::string& placeholder,
                          const std::string& value) {
  size_t pos = 0;
  while ((pos = target.find(placeholder, pos)) != std::string::npos) {
    target.replace(pos, placeholder.length(), value);
    pos += value.length();
  }
}

std::pair<std::string, int> Lego_Server::parseLegoRequest(
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
  std::cout << "  parte soliciutada" << part << std::endl;
  return {figure, part};
}

void Lego_Server::registrarEvento(Entidad e, TipoLog t, const std::string& m) {
  this->ezy_mutex.lock();
  std::string nuevaLinea = logger.log(e, t, m);
  std::string contenidoTotal = "";
  auto resultado = ezy_file.readFromFile("/logs.txt");
  if (resultado.has_value()) {
    const auto& bytes = resultado.value();

    contenidoTotal.assign(reinterpret_cast<const char*>(bytes.data()),
                          bytes.size());
  }
  // 3. Concatenar y persistir
  contenidoTotal += nuevaLinea + "\n";
  ezy_file.writeToFile("/logs.txt", contenidoTotal);
  this->ezy_mutex.unlock();
}

std::string Lego_Server::build_HttpResponse(
    const std::string statusCode,  // "200 OK", "404 Not Found", etc.
    uint32_t length, std::string contentType, std::string type_conect,
    std::string& content) {
  return "HTTP/1.1 " + statusCode + "\r\n" + "Content-Type: " + contentType +
         "; charset=UTF-8\r\n" + "Content-Length: " + std::to_string(length) +
         "\r\n" + "Connection: " + type_conect + "\r\n" + "\r\n" + content;
}

int Lego_Server::HTTPver(const std::string& stringData,
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

void Lego_Server::initializeFileSystem(std::string fsPath) {
  bool shouldPopulateFileSystem = false;
  if (!std::filesystem::exists(fsPath)) {
    shouldPopulateFileSystem = true;
    // format fs to ~95MB
    EXPECT_OR_ELSE(
        this->ezy_file.format(fsPath, 100'000'000),
        [&](const auto& err) { throw std::runtime_error(ez::errorToStr(err)); })
  }
  EXPECT_OR_ELSE(this->ezy_file.mount(fsPath), [&](const auto& err) {
    throw std::runtime_error(ez::errorToStr(err));
  })
  if (shouldPopulateFileSystem) {
    this->populateFileSystem();
  }
}

void Lego_Server::populateFileSystem() {
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
  /*
    the chosen format is arbitrarily chosen for parsing the content
    preview of fs:

    logs.txt
    amogus/
      1.txt:
        "[4,brick 1x2 red][3,brick 4x6 red][4,brick 2x4 red]"
      2.txt:
        "[6,brick 2x3 light blue]"
    duck/
      1.txt:
        "[1,brick 2x2 slope orange][2,brick 2x3 yellow][2,brick 2x2 yellow]"
      2.txt:
        "[2,brick 1x1 yellow eye][1,brick 2x1 yellow][1,brick 2x4 slope
    orange]"
    */
}

void Lego_Server::populateFigure(std::string directory, std::string figuras,
                                 std::string mitad) {
  auto result =
      ezy_file.stat(directory);  // verifica que este el directorio o no

  if (result.error() == EzError::NOT_FOUND) {  // No existe
    auto res_dir = ezy_file.mkdir(directory);
  }
  std::string p1 = directory + "/" + mitad + ".txt";
  std::cout << "Intentando escribir: " << p1 << std::endl;
  auto res_w = this->ezy_file.writeToFile(p1, figuras);

  if (!res_w.has_value()) {
    std::cout << "Fallo fatal en: " << p1
              << " Error: " << ez::errorToStr(res_w.error()) << std::endl;
    throw std::runtime_error(
        ez::errorToStr(res_w.error()));  // una pereza hacer catch de esto
  }
}

// void Lego_Server::populateDuckFigure() {
//   EXPECT_OR_ELSE(this->ezy_file.mkdir("duck"), [&](const auto& err) {
//     throw std::runtime_error(ez::errorToStr(err));
//   })

//   EXPECT_OR_ELSE(
//       this->ezy_file.writeToFile("1.txt",
//                                  "[1,brick 2x2 slope orange]"
//                                  "[2,brick 2x3 yellow][2,brick 2x2 yellow]"),
//       [&](const auto& err) { throw std::runtime_error(ez::errorToStr(err));
//       })

//   EXPECT_OR_ELSE(
//       this->ezy_file.writeToFile(
//           "2.txt",
//           "[2,brick 1x1 yellow eye]"
//           "[1,brick 2x1 yellow][1,brick 2x4 slope orange]"),
//       [&](const auto& err) { throw std::runtime_error(ez::errorToStr(err));
//       })
// }

void Lego_Server::populateLogFile() {  // el root tendra figuras
  EXPECT_OR_ELSE(
      this->ezy_file.writeToFile("/logs.txt", ""),
      [&](const auto& err) { throw std::runtime_error(ez::errorToStr(err)); })
}

// bool SendResponse(VSocket* socket, const std::string& response) {
//   try {
//     socket->Write(response.c_str(), response.length());
//     return true;
//   } catch (const std::exception& e) {
//     std::cerr << "Error enviando respuesta: " << e.what() << std::endl;
//     return false;
//   }
// }

/**
 * Handles individual client connection
 *
 * @param client Pointer to client socket
 * @param api Pointer to FileSystem API
 */
