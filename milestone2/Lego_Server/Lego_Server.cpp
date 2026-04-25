
/// ivan desings
#include "Lego_Server.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <cstring>
#include <iostream>
#include <memory>
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
  initializeFileSystem(path);
  std::cout << "tiene disco disco" << std::endl;
  populateFigure("/duck",
                 "[1,brick 2x2 slope orange]"
                 "[2,brick 2x3 yellow][2,brick 2x2 yellow]",
                 "[2,brick 1x1 yellow eye]"
                 "[1,brick 2x1 yellow][1,brick 2x4 slope orange]");
  std::cout << "no llega aca" << std::endl;
  populateFigure("/pablo",
                 "[7,brick 6x6 slope red]"
                 "[2,brick 2x3 yellow][2,brick 2x2 yellow]",
                 "[2,brick 1x1 yellow eye]"
                 "[1,brick 2x1 yellow][1,brick 2x4 slope orange]");

  populateLogFile();
  if (queue_capacity < 1) {
    queue_capacity = COLITA;
  }
  std::cout << "termina de montar disco" << std::endl;
}

Lego_Server::~Lego_Server() {
  if (server_socket != nullptr) {
    server_socket->Close();
    delete server_socket;
  }
}

int Lego_Server::cicloServer() {
  VSocket* server_socket_4 = nullptr;
  VSocket* server_socket_6 = nullptr;
  VSocket* server_socket_4_ssl = nullptr;
  VSocket* server_socket_6_ssl = nullptr;
  // API* api = nullptr;
  std::vector<std::thread> worker_threads;

  try {
    server_socket_4 = new Socket('s', 0);
    server_socket_6 = new Socket('s', 1);
    server_socket_4_ssl =
        new SSLSocket((const char*)"cert0123.pem",  // cert0123.pem
                      (const char*)"key0123.pem", false,true)
    ;

    server_socket_6_ssl = new SSLSocket(
        (const char*)"cert0123.pem", (const char*)"key0123.pem", true, true);
    if (server_socket_4 == nullptr && server_socket_6 == nullptr &&
        server_socket_6_ssl == nullptr && server_socket_4_ssl == nullptr) {
      std::cerr << "Error: No se pudo crear socket del servidor" << std::endl;
      return 1;
    }

    // Bindea a la bicha
    BindPortAvailable(server_socket_4, PORT4);
    BindPortAvailable(server_socket_6, PORT6);
    BindPortAvailable(server_socket_4_ssl, PORT4SSL);
    BindPortAvailable(server_socket_6_ssl, PORT6SSL);

    // server_socket_4->Bind(PORT4);
    // server_socket_6->Bind(PORT6);
    // server_socket_4_ssl->Bind(PORT4SSL);
    // server_socket_6_ssl->Bind(PORT6SSL);

    server_socket_4->MarkPassive(queue_capacity);
    server_socket_6->MarkPassive(queue_capacity);
    server_socket_4_ssl->MarkPassive(queue_capacity);
    server_socket_6_ssl->MarkPassive(queue_capacity);
    std::vector<std::thread> threads;

    threads.emplace_back(&Lego_Server::acceptLoop, this, server_socket_4, 0);
    threads.emplace_back(&Lego_Server::acceptLoop, this, server_socket_6, 0);
    threads.emplace_back(&Lego_Server::acceptLoop, this, server_socket_4_ssl,
                         1);
    threads.emplace_back(&Lego_Server::acceptLoop, this, server_socket_6_ssl,
                         1);

    for (auto& thread : threads) {
      if (thread.joinable()) {
        thread.join();
      }
    }

  } catch (const std::exception& e) {
    std::cerr << "Muerte del server " << e.what() << std::endl;
    return 1;
  }

  return EXIT_SUCCESS;
}

void Lego_Server::acceptLoop(VSocket* server, int ssl) {
  std::vector<std::thread*> workers;
  std::thread* worker;
  VSocket* client;
  for (;;) {
    try {
      
      client = server->AcceptConnection();
      std::cout<<"ya hay una nueva conexion"<<std::endl;
      if (!client) {
        std::cout<<"no lo logra>??"<<std::endl;
        continue; // nullptr 
      }
    } catch (const std::exception& e) {
      std::cerr << "Fallo en procesar el tipo de socket, no es ni tcp ni udp  " << e.what()
                << std::endl;
      // Si el error es por falta de recursos, dormimos un poco para no saturar
      // el CPU
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    if (ssl) {
      SSLSocket* sslClient = dynamic_cast<SSLSocket*>(client);
      SSLSocket* sslServer = dynamic_cast<SSLSocket*>(server);
      if (sslClient && sslServer) {
        sslClient->Copy(sslServer);
      }
    }
    worker = new std::thread(&Lego_Server::connectionHandling, this, client);
    workers.push_back(worker);
    // std::thread(task, client);
  }
  // for (auto& thread : threads) {
  // if (thread && thread->joinable()) {
  //   thread->join();
  // }}
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
  throw std::runtime_error("Sin puertos disponibles.");
}

void Lego_Server::connectionHandling(VSocket* client) {
  SSLSocket* sslClient = dynamic_cast<SSLSocket*>(client);
  if (sslClient) {
    // esta en
    std::cout << "manejando conexion con SSL y verificando que el cliente "
                 "confia en el certificado, en medio handshake"
              << std::endl;
    try {
      sslClient
          ->AcceptSSL();  // si falla hace thow de que el cliente es un llorno
    } catch (const std::exception& e) {
      std::cerr << "Error de Handshake SSL" << e.what() << std::endl;
      client->Close();
      delete client;
      return;  // El hilo termina
    }
  }
  std::cout << "manejando conexiones" << std::endl;
  // char buffer[1024]{};
  char bufferin;
  std::stringstream datos;
  std::string stringData;
  int bytesRead = 0;
  while (true) {
    while (bytesRead = client->Read(&bufferin, 1)) {
      datos << bufferin;
      if (bufferin == '\n') {
        stringData = datos.str();
        if (stringData.find("\r\n\r\n") != std::string::npos) {
          // foundSeparator = true;
          break;
        }
      }
    }
    if (!bytesRead) {
      std::cout << "muere conexion" << std::endl;
      break;
    }
    std::cout << "prueba" << std::endl;
    std::cout << stringData << std::endl;
    std::stringstream body_s;
    std::string body = "";

    int bodySize = 0;
    int pos = stringData.find("Content-Length: ");
    if (pos != std::string::npos) {
      uint32_t start = pos + 16;
      uint32_t end = stringData.find("\r\n", start);
      bodySize = std::stoi(stringData.substr(start, end - start));
    }

    if (pos = stringData.find("Content-Length: ") != std::string::npos) {
      int bodySize = 0;
      int content_length = 0;
      if (pos != std::string::npos) {
        uint32_t start = pos + 16;
        uint32_t end = stringData.find("\r\n", start);
        content_length = std::stoi(stringData.substr(start, end - start));
      }

      int bytes_readed = 0;
      for (; bytes_readed < content_length && client->Read(&bufferin, 1);
           ++bytes_readed) {
        // lectura segura para no bloquearse, jamas confiar en el usuario
        body_s << bufferin;
      }
      if (bytes_readed != content_length) {
        // manejo de que realmente no leyo todo lo qeu indicaba content length
      } else {
        body = body_s.str();
      }
    }

    //   return "GET /lego/list.php?figure=" + figura +
    //  "&part=" + std::to_string(mitad) +
    //  " HTTP/1.1\r\n"
    //  "Host: os.ecci.ucr.ac.cr\r\n"
    //  "Accept: text/html\r\n"
    //  "Connection: close\r\n\r\n";

    // decidir si conexion es keep alive o close
    if (stringData.find("GET") != std::string::npos) {
      std::string figure = "";
      int mitad = 0;
      if (stringData.find("&lego/figure=") !=
          std::string::npos) {  /// lego/list.php?figure=
        size_t figPos = stringData.find("figure=") + 7;
        size_t figEnd = stringData.find("&", figPos);
        if (figEnd == std::string::npos) figEnd = stringData.find(" ", figPos);
        figure = stringData.substr(figPos, figEnd - figPos);

        std::cout << "figura solicitada " << figure << std::endl;
        std::string part = "1";
        size_t partPos = stringData.find("part=");
        if (partPos != std::string::npos) {
          mitad = std::stoi(stringData.substr(partPos + 5, 1));
        }
        std::cout << "mitad solicitada " << mitad << std::endl;

        std::string fullPath = "/" + figure + "/" + part + ".txt";
        // figure += "/" + relative_path;
        auto contenido = ezy_file.readFromFile(fullPath);
        if (!contenido) {
          // manejar error
          //   return contenido.error();
        }
        const auto& v = contenido.value();
        std::string str;
        str.reserve(v.size());

        std::transform(v.cbegin(), v.cend(), std::back_inserter(str),
                       [](std::byte b) {
                         return static_cast<char>(std::to_integer<uint8_t>(b));
                       });  //
        std::string response = build_HttpResponse(str.size(), "close", str);
        client->Write(response.c_str());  // enviadito
      } else {
        if (stringData.find("&logs=") != std::string::npos) {
          // modularizar con el de arriba
        } else {
          if (stringData.find("GET / ") != std::string::npos) {
            stringData = datos.str();
            std::string response = "";
            std::cout << "entro a una pagina y quiere respuesta" << std::endl;
            std::string html =
                "aun no se que poner :)"<<std::endl;
            response = build_HttpResponse(html.size(), "close", html);
            client->Write(response.c_str());
          }
        }
      }
    } else {
      if (stringData.find("POST") != std::string::npos) {
        // modularizar esta parte
        if (stringData.find("&lego/figure=") !=
            std::string::npos) {  /// lego/list.php?figure=
          std::string length = "&lego/figure=";
          std::string figure;
          figure =
              (stringData.substr(length.size(), stringData.length() - 1));  //
          int mitad = 0;
          if (stringData.find("&part=") != std::string::npos) {
            std::string length = "&part=";
            mitad = std::stoi(
                stringData.substr(length.size(), stringData.length() - 1));
          }
          std::string relative_path;
          if (mitad == 1) {
            relative_path = "1";
          } else {
            if (mitad == 2) {  // sino manda toda
              relative_path = "2";
            }
          }
          std::string fullPath = "/" + figure + "/" + relative_path + ".txt";
          std::string directory = "/" + figure;
          auto result =
              ezy_file.stat(directory);  // verifica que este el directorio o no

          // if (result.has_value()) { // Si existe (puede ser archivo o
          // directorio)

          // } else {
          if (result.error() == EzError::NOT_FOUND) {  // No existe
            ezy_file.mkdir(directory);
          }
          // figure += "/" + relative_path;
          // }
          ezy_file.writeToFile(fullPath,
                               body);  // escribe la estructura de guardadito
        }
      }

      // memset(buffer, 0, sizeof(buffer));
    }
  }
  client->Close();
  delete client;
}

std::string Lego_Server::build_HttpResponse(uint32_t length,
                                            std::string type_conect,
                                            std::string& content) {
  return "HTTP/1.1 200 OK\r\n"
         "Content-Type: text/plain\r\n"
         "Content-Length: " +
         std::to_string(length) +
         "\r\n"
         "Connection: " +
         type_conect + "\r\n\r\n" + content;
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
  // if (shouldPopulateFileSystem) {
  //   this->populateFileSystem();
  // }
}

// void Lego_Server::populateFileSystem() {
//   this->populateAmogusFigure();
//   this->populateDuckFigure();
//   this->populateLogFile();
//   /*
//     the chosen format is arbitrarily chosen for parsing the content
//     preview of fs:

//     logs.txt
//     amogus/
//       1.txt:
//         "[4,brick 1x2 red][3,brick 4x6 red][4,brick 2x4 red]"
//       2.txt:
//         "[6,brick 2x3 light blue]"
//     duck/
//       1.txt:
//         "[1,brick 2x2 slope orange][2,brick 2x3 yellow][2,brick 2x2 yellow]"
//       2.txt:
//         "[2,brick 1x1 yellow eye][1,brick 2x1 yellow][1,brick 2x4 slope
//     orange]"
//     */
//}

void Lego_Server::populateFigure(std::string directory, std::string figuras1,
                                 std::string figuras2) {
  std::cout << "Intentando crear directorio: " << directory << std::endl;
  auto res_dir = this->ezy_file.mkdir(directory);
  if (!res_dir.has_value()) {
    std::cout << "Aviso mkdir: " << ez::errorToStr(res_dir.error())
              << std::endl;
    // Si el error es ALREADY_EXISTS, podemos seguir
  }

  std::string p1 = directory + "/1.txt";
  std::cout << "Intentando escribir: " << p1 << std::endl;
  auto res_w = this->ezy_file.writeToFile(p1, figuras1);

  if (!res_w.has_value()) {
    // AQUÍ ES DONDE ESTÁ DANDO EL NOT_FOUND
    std::cout << "Fallo fatal en: " << p1
              << " Error: " << ez::errorToStr(res_w.error()) << std::endl;
    throw std::runtime_error(ez::errorToStr(res_w.error()));
  }
}

void Lego_Server::populateDuckFigure() {
  EXPECT_OR_ELSE(this->ezy_file.mkdir("duck"), [&](const auto& err) {
    throw std::runtime_error(ez::errorToStr(err));
  })

  EXPECT_OR_ELSE(
      this->ezy_file.writeToFile("1.txt",
                                 "[1,brick 2x2 slope orange]"
                                 "[2,brick 2x3 yellow][2,brick 2x2 yellow]"),
      [&](const auto& err) { throw std::runtime_error(ez::errorToStr(err)); })

  EXPECT_OR_ELSE(
      this->ezy_file.writeToFile(
          "2.txt",
          "[2,brick 1x1 yellow eye]"
          "[1,brick 2x1 yellow][1,brick 2x4 slope orange]"),
      [&](const auto& err) { throw std::runtime_error(ez::errorToStr(err)); })
}

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
