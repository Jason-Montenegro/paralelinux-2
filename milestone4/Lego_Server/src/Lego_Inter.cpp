
#include "Lego_Inter.hpp"

#include <ifaddrs.h>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/types.h>

#include <atomic>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <vector>

#include "SSLSocket.h"
#include "Socket.h"
std::atomic<bool> yaCerrando(false);
// #include "EzFileSystem.hpp"

int PUERTO_JOIN = 3030;
int PUERTO_PRUEBA_JOIN = 3030;
int PUERTO_PRUEBA_RESPONSE = 3031;
int PUERTO_RESPONSE_REQUEST = 3031;
int PUERTO_LEGO_SERVER = 5000;
int PUERTO_INTER = 5001;
int PORT4SSL = 8000;
int PORT4 = 8501;  // le meto puerto 80 http
int PORT6SSL = 9002;
int PORT6 = 9503;
// size_t BUFFER_SIZE = 8192;
int COLITA = 5;
uint32_t DEFAULT_DECISIONS_INODE = 1;

Lego_Inter::Lego_Inter(const char* key, const char* cert, int queue_capacity,
                       std::string path)
    : key(key), cert(cert), queue_capacity(queue_capacity) {
  this->server_socket_4 = nullptr;
  this->server_socket_6 = nullptr;
  this->server_socket_4_ssl = nullptr;
  this->server_socket_6_ssl = nullptr;

  this->loggerS = new LoggerFiguras();
  if (queue_capacity < 1) {
    queue_capacity = COLITA;
  }
  std::cout << "tnemesis" << std::endl;
}

Lego_Inter::~Lego_Inter() {
  threads.clear();
  if (this->discoverySocket) {
    delete this->discoverySocket;
  }
  if (loggerS) {
    delete loggerS;
  }
}

void Lego_Inter::handleSignal(int signal) {
  if (signal == SIGINT) {
    if (yaCerrando.exchange(true)) {
      return;
    }
    std::cout << "\nCierre de servidor" << std::endl;
  }
}

int Lego_Inter::cicloServer() {
  std::signal(SIGINT, Lego_Inter::handleSignal);
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

void Lego_Inter::inicializar_Server() {
  this->server_socket_4 = new Socket('s', 0);
  this->server_socket_6 = new Socket('s', 1);
  this->server_socket_4_ssl = new SSLSocket((const char*)cert,  // cert0123.pem
                                            (const char*)key, false, true);

  this->server_socket_6_ssl =
      new SSLSocket((const char*)cert, (const char*)key, true, true);
  this->discoverySocket = new Socket('d', 0);

  this->interSocket = new Socket('d', 0);
  // serverC
  this->serverC = new Socket('s', 0);
  //
  if (server_socket_4 == nullptr && server_socket_6 == nullptr &&
      server_socket_6_ssl == nullptr && server_socket_4_ssl == nullptr) {
    //  std::cerr << "Error: No se pudo crear socket del servidor" << std::endl;
    throw std::runtime_error("No se pudo crear socket del servidor");
  }
}

void Lego_Inter::bindServer() {
  int p1 = BindPortAvailable(server_socket_4, PORT4);
  int p2 = BindPortAvailable(server_socket_6, PORT6);
  int p3 = BindPortAvailable(server_socket_4_ssl, PORT4SSL);
  int p4 = BindPortAvailable(server_socket_6_ssl, PORT6SSL);
  this->serverC->Bind(PUERTO_RESPONSE_REQUEST);
  std::string puertos = "Servidor escuchando en puertos: ";  // aja
  puertos += std::to_string(p1) + ", " + std::to_string(p2) + ", " +
             std::to_string(p3) + ", " + std::to_string(p4);
  registrarEvento(Entidad::SERVIDOR, TipoLog::ENRUTAMIENTO, puertos);
  this->serverC->MarkPassive(queue_capacity);
  this->server_socket_4->MarkPassive(queue_capacity);
  this->server_socket_6->MarkPassive(queue_capacity);
  this->server_socket_4_ssl->MarkPassive(queue_capacity);
  this->server_socket_6_ssl->MarkPassive(queue_capacity);

  // tipo de solucion, no se si sea mejor hacer una conexion falsa con los
  // server_socket
  this->server_socket_4->SetTimeout(5);
  this->server_socket_6->SetTimeout(5);
  this->server_socket_4_ssl->SetTimeout(5);
  this->server_socket_6_ssl->SetTimeout(5);
  this->discoverySocket->SetTimeout(5);
  this->serverC->SetTimeout(5);
  this->interSocket->SetTimeout(5);
}

int Lego_Inter::BindPortAvailable(VSocket* socket, int startPort) {
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

void Lego_Inter::listen() {
  threads.emplace_back([this](std::stop_token st) {
    this->acceptLoop(st, this->server_socket_4, 0);
  });

  threads.emplace_back([this](std::stop_token st) {
    this->acceptLoop(st, this->server_socket_6, 0);
  });

  threads.emplace_back([this](std::stop_token st) {
    this->acceptLoop(st, this->server_socket_4_ssl, 0);
  });

  threads.emplace_back([this](std::stop_token st) {
    this->acceptLoop(st, this->server_socket_6_ssl, 0);
  });

  threads.emplace_back(
      [this](std::stop_token st) { this->acceptLoop(st, this->serverC, 1); });

  // threads.emplace_back([this](std::stop_token st) { discoveryListener(st);
  // });
  threads.emplace_back([this](std::stop_token st) {
    discoverIntermediary(st, *this->discoverySocket, PUERTO_INTER);
  });
  threads.emplace_back([this](std::stop_token st) {
    discoverIntermediary(st, *this->interSocket, PUERTO_JOIN);
  });

  std::string ivan = "FIGURAS_IVAN";

  JafetPrime::Join joinsito;
  this->interSocket->UpdateLocalIP();

  std::string ip_str = this->interSocket->getLocalIP();
  if (inet_pton(AF_INET, ip_str.c_str(), &joinsito.sourceIp) <= 0) {
    std::cerr << "Error convirtiendo IP de origen" << std::endl;
  }
  sem_inter.acquire();  // que lo incremetne 2
  sem_inter.acquire();

  auto listin = joinsito.serialize();
  // this->discoverySocket->BroadCast(
  //     "172.16.123.95", ivan.c_str(), ivan.size(),
  //     PUERTO_LEGO_SERVER);  //   void Broadcast(std::string ip.const void*
  //                           //   buffer, size_t size, puerto);

  for (int i = 0; i < BROADCASTS.size(); ++i) {
    this->interSocket->BroadCast(BROADCASTS[i].c_str(), listin.data(),
                                 listin.size(), PUERTO_JOIN);
  }
  

  // casos de
  this->discoverySocket->BroadCast("192.168.0.255", ivan.c_str(), ivan.size(),
                                   PUERTO_LEGO_SERVER);

  //while(true){
    this->interSocket->BroadCast("192.168.0.255", listin.data(), listin.size(), \
                               PUERTO_PRUEBA_JOIN);//PUERTO_PRUEBA_JOIN
  //   sleep(5);
  // }
  //});

  // threads.emplace_back(
  //   [this](std::stop_token st) { discoverIntermediary(st, socket); });

  // broadcastDiscovery(int puerto, const char* ip, const T& msg,
  //                      int& discoverySocket);  //

  // broadcastDiscovery();// ver si creo hilo
}

void Lego_Inter::cierreServers() {
  if (server_socket_4) {
    this->server_socket_4->Close();
    delete this->server_socket_4;
    this->server_socket_4 = nullptr;
  }
  if (server_socket_6) {
    this->server_socket_6->Close();
    delete this->server_socket_6;
    this->server_socket_6 = nullptr;
  }
  if (server_socket_4_ssl) {
    this->server_socket_4_ssl->Close();
    delete this->server_socket_4_ssl;
    this->server_socket_4_ssl = nullptr;
  }
  if (server_socket_6_ssl) {
    this->server_socket_6_ssl->Close();
    delete this->server_socket_6_ssl;
    this->server_socket_6_ssl = nullptr;
  }
  if (serverC) {
    this->serverC->Close();
    delete this->serverC;
    this->serverC = nullptr;
  }
  if (this->interSocket) {
    delete this->interSocket;
    this->interSocket = nullptr;
  }
  if (this->discoverySocket) {
    delete this->discoverySocket;
    this->discoverySocket = nullptr;
  }
}

void Lego_Inter::acceptLoop(std::stop_token st, VSocket* server,
                            int SuperSatan) {
  std::vector<std::jthread> workers;
  while (!st.stop_requested()) {
    VSocket* client = nullptr;
    try {
      client = server->AcceptConnection();
      if (client) {
        if (!SuperSatan) {
          registrarEvento(Entidad::SERVIDOR, TipoLog::SOLICITUD,
                          "Nueva conexion TCP establecida");
          workers.emplace_back([this, client](std::stop_token st) {
            connectionHandling(st, client);
          });
        } else {
          std::cout << "entra a loop inter de accept" << std::endl;
          workers.emplace_back([this, client](std::stop_token st) {
            SatanHandling(st, client);
          });
        }
      }

    } catch (const std::exception& e) {
      //
      if (client) {  // acepta pero hay una falla posterio
        delete client;
        client = nullptr;
      }
    }
  }
}

void Lego_Inter::SatanHandling(std::stop_token st, VSocket* client) {
  uint8_t tipo = 0;
  while (!st.stop_requested() ) {
    if (client->Read(&tipo, 1) <= 0) break;
    std::cout<<"llega nueva solicitud tipo ";
    if (tipo == JafetPrime::INTERMEDIARY_HANDSHAKE) {
      std::cout<<"INTERMEDIARY_HANDSHAKE"<<std::endl;
      responderHandShake(client);
    } else {
      if (tipo == JafetPrime::INTERMEDIARY_REQUEST) {
      std::cout<<"INTERMEDIARY_REQUEST"<<std::endl;
        responderIntermediario(client);
      }  // request lo hara el otro bastardo
    }
  }
  delete client;
  client = nullptr;
}

// SatanHandling
// responderHandShake
// responderIntermediario
// discoverIntermediary
//
void Lego_Inter::responderHandShake(VSocket* client) {
  uint32_t incomingLength = 0;
  if (client->Read(&incomingLength, sizeof(incomingLength)) <= 0) return;
  std::string incomingContent(incomingLength, '\0');
  if (incomingLength > 0) {
    client->Read(&incomingContent[0], incomingLength);
  }
  if (!incomingContent.empty()) {
    std::stringstream ss(incomingContent);
    std::string figuraRecibida;
    mtx_global.lock();
    std::string ipRemota = client->getRemoteIPV4();  // getRemoteIPV4()
    std::cout << "la ip remota es" << ipRemota << std::endl;
    while (std::getline(ss, figuraRecibida, ',')) {
      if (!figuraRecibida.empty()) {
        Lego_Inter::RutaFigura nuevaRuta;
        nuevaRuta.ip = ipRemota;
        nuevaRuta.mitadFaltante = 0;  // no sabe como eta con la cosa
        nuevaRuta.puerto = PUERTO_PRUEBA_RESPONSE;
        this->tablaGlobal[figuraRecibida].push_back(nuevaRuta);
        std::cout<<"figuraRecibida: "<<figuraRecibida<<std::endl;
      }
    }
    mtx_global.unlock();
  }
  std::string listaFiguras = "";
  mtx_inter.lock();
  for (auto& [nombreFigura, ruta] : this->tablaPropia) {
    if (!listaFiguras.empty()) {
      listaFiguras += ",";
    }
    listaFiguras += nombreFigura;
  }
  mtx_inter.unlock();
  JafetPrime::Handshake miHandshake;
  miHandshake.content = listaFiguras;
  miHandshake.contentLength = miHandshake.content.size();
  std::vector<uint8_t> responseBytes = miHandshake.serialize();
  try {
    client->Write(responseBytes.data(), responseBytes.size());  // todo fino
  } catch (...) {
  }
}

void Lego_Inter::responderIntermediario(VSocket* client) {
  uint8_t mitad = 0;
  uint8_t nameLength = 0;

  if (client->Read(&mitad, 1) > 0 && client->Read(&nameLength, 1) > 0) {
    std::string figureName(nameLength, '\0');
    if (nameLength > 0) {
      client->Read(&figureName[0], nameLength);
      std::string str = solicitarFiguraServer(figureName, mitad);
      if (str != "") {
        JafetPrime::Response successResponse;
        successResponse.mid = mitad;  // datos que ahora son innecesarios
        successResponse.figureName = figureName;
        successResponse.figureNameLength = figureName.size();
        successResponse.content = str;
        successResponse.contentLength = str.size();
        std::vector<uint8_t> buffer = successResponse.serialize();
        client->Write(buffer.data(), buffer.size());
      } else {
        uint8_t errorByte = JafetPrime::FIGURE_NOT_FOUND;
        client->Write(&errorByte, 1);
      }
    }
  }
  // client.Read(&tipo, 1) if ()
}

void Lego_Inter::connectionHandling(std::stop_token st, VSocket* client) {
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

    // std::cout << "prueba" << std::endl;
    std::cout << stringData << std::endl;
    //
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

        // request.first + "/" + std::to_string(request.second)
        std::string str = solicitarFiguraServer(request.first, request.second);
        if (str == "") {
          str = solicitarFiguraGlobal(
              request.first, request.second);  // no me encontro el disco duro
        }
        if (str == "") {
          // ocupa solicii

          //
          std::cout << "figura no existente" << std::endl;
          std::string response =
              build_HttpResponse("404 Not Found", errorBody.size(), "text/html",
                                 connectionType, errorBody);
          client->Write(response.c_str());  // mismo cliente se pudre todo
          registrarEvento(Entidad::SERVIDOR, TipoLog::ERROR,
                          "Server Intermediario en la papa ");
          // return;
          //  manejar error
          //    return contenido.error();
        } else {  //
          registrarEvento(Entidad::SERVIDOR, TipoLog::RESPUESTA,
                          "Enviando pieza " + std::to_string(request.second) +
                              " de " + request.first);
          // std::cout << "llega al casteo" << std::endl;

          // std::cout << "casteocorrecto" << std::endl;
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
          std::string str = this->loggerS->leerLog();
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
            //   stringData = datos.str();
            std::string response = "";
            std::string filasFiguras = "";
            std::unordered_set<std::string> figurasProcesadas;
            mtx_inter.lock();  // cambiarlo a global si uso tabla Global

            for (auto& entry : tablaPropia) {
              filasFiguras += "<tr>";
              filasFiguras += "  <td>" + entry.first + "</td>";
              filasFiguras +=
                  "  <td><span class='badge local'>Local</span></td>";
              filasFiguras += "</tr>";
              figurasProcesadas.insert(entry.first);
            }
            mtx_inter.unlock();
            //  std::cout << " figuras  locales armadas" << std::endl;
            mtx_global.lock();
            if (!tablaGlobal.empty()) {
              for (auto& entry : tablaGlobal) {
                // ocupa no existir en la tabla local, pero no se si eso es un
                // caso existente
                if (figurasProcesadas.find(entry.first) ==
                    figurasProcesadas.end()) {
                  filasFiguras += "<tr>";
                  filasFiguras += "  <td>" + entry.first + "</td>";
                  filasFiguras +=
                      "  <td><span class='badge global'>Remota</span></td>";  // no es tan necesario ver esto
                  filasFiguras += "</tr>";
                  // figurasProcesadas.insert(entry.first);
                }
              }
            }
            mtx_global.unlock();
            //    std::cout << " figuras globales armadas" << std::endl;
            //  std::cout << filasFiguras << std::endl;
            response =
                build_HttpResponse("200 OK", filasFiguras.size(), "text/html",
                                   connectionType, filasFiguras);

            try {
              if (client != nullptr) {
                client->Write(response.c_str());
              }
            } catch (const std::runtime_error& e) {
              std::cerr << "[HTTP] No se pudo escribir al cliente: " << e.what()
                        << std::endl;

            } catch (...) {
              std::cerr << "[HTTP] Error desconocido al escribir en el socket"
                        << std::endl;
            }
            //}
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

int Lego_Inter::readHeaders(VSocket* client, std::stringstream& datos,
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
//
int Lego_Inter::readBody(std::stringstream& body_s, std::string& stringData,
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
      // body = body_s.str();
    }
  }
  return retorno;
}

// a esto lo tengo que cambiar bien ppiedrero
//  TODO ver lo de nombres con / o no
std::string Lego_Inter::buildFigurePage(const std::string& figureName,
                                        std::string content) {
  // std::cout << "llega al build" << std::endl;
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
void Lego_Inter::replace(std::string& target, const std::string& placeholder,
                         const std::string& value) {
  size_t pos = 0;
  while ((pos = target.find(placeholder, pos)) != std::string::npos) {
    target.replace(pos, placeholder.length(), value);
    pos += value.length();
  }
}

std::pair<std::string, int> Lego_Inter::parseLegoRequest(
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

void Lego_Inter::registrarEvento(Entidad e, TipoLog t, const std::string& m) {
  loggerS->log(e, t, m);
}

std::string Lego_Inter::build_HttpResponse(
    const std::string statusCode,  // "200 OK", "404 Not Found", 100 ERROR.
    uint32_t length, std::string contentType, std::string type_conect,
    std::string& content) {
  return "HTTP/1.1 " + statusCode + "\r\n" + "Content-Type: " + contentType +
         "; charset=UTF-8\r\n" + "Content-Length: " + std::to_string(length) +
         "\r\n" + "Connection: " + type_conect + "\r\n" + "\r\n" +
         content;  ////\r\n\r\n
}

int Lego_Inter::HTTPver(const std::string& stringData,
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

std::string Lego_Inter::solicitarFiguraServer(const std::string& figura,
                                              int mitad) {
  std::string retorno = "";
  RutaFigura ruta;
  bool figuraExiste = false;
  mtx_inter.lock();
  if (tablaPropia.find(figura) != tablaPropia.end()) {
    ruta = tablaPropia[figura];
    mtx_inter.unlock();
    VSocket* socketServer = nullptr;
    bool borrarFigura = false;
    bool actualizarMitad = false;
    int st = OpenConnection(ruta.ip, ruta.puerto, socketServer, 10);
    if (st != -1 && socketServer != nullptr) {
      socketServer->SetTimeout(10);
      std::string request = buildHTTPRequest(figura, mitad);
      socketServer->Write(request.c_str());
      std::string respuesta;
      char buffer;
      int bytes = 0;
      while ((bytes = socketServer->Read(&buffer, 1)) > 0) {
        respuesta += buffer;
      }
      std::cout << "lecturea" << std::endl;
      delete socketServer;
      socketServer = nullptr;
      if (respuesta.find("404 Not Found") == std::string::npos) {
        size_t bodyPos = respuesta.find("\r\n\r\n");
        if (bodyPos != std::string::npos) {
          retorno = respuesta.substr(bodyPos + 4);
        }
      } else {
        mtx_inter.lock();
        if (ruta.mitadFaltante && ruta.mitadFaltante != mitad) {
          tablaPropia.erase(figura);  // si ya fue borrada no hay problema
        } else {
          ruta.mitadFaltante = mitad;
          // verificar si aun existe
          if (tablaPropia.find(figura) != tablaPropia.end()) {
            tablaPropia[figura].mitadFaltante = mitad;
          }
        }
        mtx_inter.unlock();
      }
    } else {
      if (socketServer) {
        delete socketServer;
        socketServer = nullptr;
      }
      mtx_inter.lock();
      tablaPropia.erase(figura);
      mtx_inter.unlock();
    }
  } else {
    mtx_inter.unlock();
  }

  return retorno;
}

//  std::map<std::string, std::list< RutaFigura > >
//  ip , puerto
std::string Lego_Inter::solicitarFiguraGlobal(const std::string& figura,
                                              int mitad) {
  std::string partes = "";
  std::list<RutaFigura> copiaLista;

  mtx_global.lock();
  if (tablaGlobal.find(figura) != tablaGlobal.end()) {
    copiaLista = tablaGlobal[figura];
    mtx_global.unlock();

    std::list<RutaFigura> rutasAExcluir;
    // Para guardar a quien le cambio la mitad
    // si usaba la lista original no habia problema,
    // //pero no seria optimo tener el mutex por todo el tiempo
    std::list<RutaFigura> rutasAActualizar;
    VSocket* figurin = nullptr;

    // Recorremos la copia local de forma segura
    for (const RutaFigura& ruta : copiaLista) {
      JafetPrime::Request request;
      request.tipo = JafetPrime::INTERMEDIARY_REQUEST;
      request.mitad = mitad;
      request.contentLength = figura.size();
      request.content = figura;
      std::vector<uint8_t> msg = request.serialize();
      int st = OpenConnection(ruta.ip, ruta.puerto, figurin, 1);
      if (st != -1 && figurin != nullptr) {
        figurin->SetTimeout(20);
        figurin->Write(msg.data(), msg.size());
        partes = recvInter(*figurin);
        delete figurin;
        figurin = nullptr;

        if (partes != "") {
          std::cout<<"partes recividas de brandon"<<partes<<std::endl;
          break;  // solucion super horrible
                  // pero creo que no puedo ponerle condicional al for que tenog
        } else {
          if (ruta.mitadFaltante && ruta.mitadFaltante != mitad) {
            rutasAExcluir.push_back(ruta);
          } else {
            RutaFigura rutaModificada = ruta;
            rutaModificada.mitadFaltante = mitad;
            rutasAActualizar.push_back(rutaModificada);
          }
        }
      } else {
        if (figurin) {
          delete figurin;
          figurin = nullptr;
        }
        rutasAExcluir.push_back(ruta);
      }
    }
    mtx_global.lock();
    auto& listaReal = tablaGlobal[figura];
    if (!rutasAExcluir.empty()) {
      for (const auto& invalida : rutasAExcluir) {
        auto it = listaReal.begin();
        while (it != listaReal.end()) {
          if (it->ip == invalida.ip && it->puerto == invalida.puerto) {
            it = listaReal.erase(it);
          } else {
            ++it;
          }
        }
      }
    }
    if (!rutasAActualizar.empty()) {
      for (const auto& valor_actualizado : rutasAActualizar) {
        for (auto& real : listaReal) {
          if (real.ip == valor_actualizado.ip &&
              real.puerto == valor_actualizado.puerto) {
            real.mitadFaltante = valor_actualizado.mitadFaltante;
          }
        }
      }
    }
    if (listaReal.empty()) {
      tablaGlobal.erase(figura);
    }
    mtx_global.unlock();
  } else {
    mtx_global.unlock();
  }
  return partes;
}
//
// uint8_t tipo = INTERMEDIARY_RESPONSE;
// uint8_t mid = 1; // 1: primera, 2: segunda, 3: todas
// uint8_t figureNameLength = 0;
// std::string figureName;
// uint32_t contentLength = 0;
// std::string content; // Estructura: [cant,nombre][cant,nombre]
// ssize_t Socket::Read(void* buffer, size_t size) {
std::string Lego_Inter::recvInter(VSocket& socket) {
  std::string rt = "";
  JafetPrime::Response response;  // se fue cantando alegremente cuack cuacks
  uint8_t buffer = 0;
  uint8_t mitad = 0;
  uint32_t content_length;
  socket.Read(&buffer, 1);  // lee tipo
  if (buffer == JafetPrime::INTERMEDIARY_RESPONSE) {
    socket.Read(&mitad, 1);   // lee mitad, se asume que se recive bien peor no
                              // se si ocupa mecanismos
    socket.Read(&buffer, 1);  // length de figura name
    // char name_figura[buffer];  // posiblemente ahora sea url
    std::vector<char> name_figura(
        buffer);  // por el momento no hacemos nada con esto
    socket.Read(
        name_figura.data(),
        buffer);  // como no hacemos nada seria mejor quitarlo del protocolo
    socket.Read(&content_length, sizeof(content_length));
    // char partes[content_length];
    // std::vector<char> partes(content_length, ' ');
    if (content_length > 0) {
      std::string partes(content_length, ' ');
      // try cacheo en el coso
      socket.Read(
          &partes[0],
          content_length);  // usando string como buffer para lo que quiero
      return partes;        // antipardigma ivan
    }
  }
  return rt;  // ocucapara algo para devolver el objeto del contenido
}

void Lego_Inter::realName(std::string& nameFaka) {
  std::string realN;
  int sizeS = nameFaka.size();
  for (int j = 0; j < sizeS - 1; ++j) {  // p,/a p/,//a
    if (nameFaka[j] == '/') {
      realN += nameFaka[j + 1];
      ++j;  // avanza para avanzar 2
    } else {
      realN += nameFaka[j];
    }
  }
  if (sizeS > 0) {
    char fCharacter = nameFaka[sizeS - 1];
    if (fCharacter != '/') {
      realN += fCharacter;
    }
  }
}

//   ,perrito  [carro] [2,perrito]
//  std::list<std::pair<std::string, int>> Lego_Inter::parseBichin(std::string
//  entrada){
//   std::list<std::pair<std::string, int>>  parseo;
//   std::regex rgx(R"(\[(\d+),((?:/\.|[^\]])*)\])");// esta basura
//   std::smatch match;
//   auto begin = std::sregex_iterator(entrada.begin(), entrada.end(), rgx);//
//   mas rapido con string auto end = std::sregex_iterator(); for (auto i =
//   begin; i != end; ++i) {
//     std::string name= std::stoi(match[1]);
//     realName(name);
//     parseo.push_back(std::make_pair(name, int));
//     std::smatch match = *i;
//     //parseo(match[2])+= std::stoi(match[1]);
//     std::string nombre;
//   }
//     return 0;
// }

// cambiar esto
// ocupo que  pruebe con 2  variables de rango de puertos
// tambien ocupo que reciba socket u no este quemadin
int Lego_Inter::OpenConnection(std::string ip, int puerto, VSocket*& server,
                               int rango) {
  int st = -1;
  // puertos locales
  int port = 0;
  server = new Socket('s');  // lo crea y todo
  server->SetTimeout(1);
  try {
    st = server->Connect(ip.c_str(), puerto);
  } catch (...) {
    st = -1;
  }
  if (st != -1) {
    return st;
  }
  // st = this->serverC->Connect(this->host, port_plain);
  int ipR = rango + puerto;
  for (port = puerto; port <= ipR && st == -1;
       ++port) {  // puedo meterle atributo de rango para el open
    try {
      // std::cout << "prueba " << port << std::endl;
      st = server->Connect(ip.c_str(), port);
    } catch (...) {
      st = -1;
    }
  }

  return st;
}

std::string Lego_Inter::buildHTTPRequest(const std::string& figura,
                                         int& mitad) {
  return "GET /&lego/figure=" + figura +  // se cambia solo pide figura
         "&part=" + std::to_string(mitad) +
         " HTTP/1.1\r\n"
         "Host: localhost\r\n"
         "Accept: text/html\r\n"
         "Connection: close\r\n\r\n";
  // decidir si conexion es keep alive o close
}

// utilizado tanto para el server de piezas como los intermediarios

// agregar AcceptConnection
void Lego_Inter::discoverIntermediary(std::stop_token st, VSocket& socket,
                                      int& puertito) {
  int opt = 1;
  // SocketD(socket );
  socket.SocketD(puertito);
  uint8_t buffer[1024];
  sem_inter.release();
  while (!st.stop_requested()) {
    try {
      sockaddr_in sender{};
      socklen_t len = sizeof(sender);
      int bytes = 0;
      if (bytes = socket.recvFrom(buffer, sizeof(buffer), (sockaddr*)&sender)) {
        buffer[bytes] = 0;
        uint8_t tipoMensaje = buffer[0];
        std::cout << "el buffer tiene " << buffer[0] << std::endl;
        if (puertito == PUERTO_JOIN &&
            tipoMensaje == JafetPrime::INTERMEDIARY_JOIN) {
          JafetPrime::Join joinRecibido;
          // ocupo dejar igual el join
          if (JafetPrime::Join::deserialize(buffer, bytes, joinRecibido) > 0) {
            std::string ipRealIntermediario = inet_ntoa(joinRecibido.sourceIp);
            std::string ipIntermediario = inet_ntoa(sender.sin_addr);
            socket.UpdateLocalIP();
            if (ipIntermediario != socket.getLocalIP() &&ipIntermediario!=std::string("192.168.0.176") ) {
              VSocket* handSocket;
              OpenConnection(ipIntermediario, PUERTO_PRUEBA_RESPONSE,
                             handSocket,
                             10);  // no recuerod si era el 3010 jajja
              handSocket->SetTimeout(5);
              threads.emplace_back([this, ipIntermediario,
                                    handSocket](std::stop_token st) mutable {
                this->handShakeReceive(st, (ipIntermediario), *handSocket);
              });
              std::cout << "IP del  remitente: " << ipIntermediario
                        << std::endl;
              std::cout << "ip real: " << ipRealIntermediario << std::endl;
            }
          } else {
            std::cout << "[ERROR] Paquete JOIN malformado. ROMPIO EL DOCEAVO "
                         "MANDAMIENTO"
                      << std::endl;
          }
        } else {
          std::string msg(reinterpret_cast<char*>(buffer));
          std::cout << "Mensaje UDP recibido: " << msg << std::endl;
          if (msg.find("SERVER") != std::string::npos) {  // ma little boy
            std::stringstream ss(msg);
            std::string tipo;
            int puerto;
            ss >> tipo;
            std::string ipReal = inet_ntoa(sender.sin_addr);
            ss >> puerto;

            std::cout << "Descubierto " << ipReal << ": " << puerto
                      << std::endl;
            solicitarListaFiguras(ipReal, puerto);
            VSocket* vsocket;
            // //post join handshake se manda
            // //

            std::unordered_set<std::string> ipsRemotasUnicas;

            mtx_global.lock();

            // TODO (ivan),  esto sale mas optimo si guardo un set de las ips y
            // no uso la  la misma estructura de las figuras para las ips
            for (const auto& [figura, listaRutas] : this->tablaGlobal) {
              for (const auto& ruta : listaRutas) {
                if (!ruta.ip.empty() && ruta.ip != "0.0.0.0") {
                  ipsRemotasUnicas.insert(ruta.ip);
                }
              }
            }
            mtx_global.unlock();
            for (const std::string& ipDestino : ipsRemotasUnicas) {
              VSocket* vsocket = nullptr;
              try {
                OpenConnection(ipDestino, PUERTO_RESPONSE_REQUEST, vsocket, 5);
                if (vsocket != nullptr) {
                  vsocket->SetTimeout(5);
                  threads.emplace_back([this, vsocket](std::stop_token st) {
                    this->solicitarListaFiguras(*vsocket);
                    delete vsocket;
                  });
                }

              } catch (const std::exception& e) {
                std::cerr << "[INTERMEDIARIO] No se pudo conectar TCP con "
                          << ipDestino << ": " << e.what() << std::endl;
                if (vsocket != nullptr) {
                  delete vsocket;
                }
              }
            }
          }
        }
      }
    } catch (const std::runtime_error& e) {
      if (st.stop_requested()) {
        break;
      }

    } catch (...) {
      std::cerr << "Error inesperado" << std::endl;
      if (st.stop_requested()) break;
    }
  }
  std::cout << "final en llegada" << std::endl;
  // close(socket);
}

void Lego_Inter::solicitarListaFiguras(const std::string& ip, int puerto) {
  // std::cout << "llega a solicitud" << std::endl;
  VSocket* listin;
  int st = OpenConnection(ip, puerto, listin,
                          500);  // conectarse al lego server
  if (st != -1) {
    listin->SetTimeout(5);
    std::string request =
        "GET /&list/figures HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Accept: text/html\r\n"
        "Connection: close\r\n\r\n";
    listin->Write(request.c_str());

    char buffer[5000];
    std::string response;
    try {
      // Leer respuesta
      ssize_t bytes;
      while ((bytes = static_cast<ssize_t>(
                  listin->Read(buffer, sizeof(buffer)))) > 0) {  //
        response.append(buffer, bytes);
      }  // esto solo es la solicitud de la lsita a lo close

      std::stringstream ss(response);
      std::string figura;
      // todo el uso de tablas tanto global como local ocupa mutex
      // es mejor un modelo vista controlador pero me da pereza crear el struct
      mtx_inter.lock();
      while (ss >> figura) {
        tablaPropia[figura] = {ip, puerto};
        std::cout << figura << " - " << ip << ":" << puerto << std::endl;
      }
      mtx_inter.unlock();
    } catch (const std::exception& e) {
      std::cerr << "Error SSL: " << e.what() << std::endl;
    }
    std::cout << response << std::endl;
    delete listin;
    listin = nullptr;
  }
}

void Lego_Inter::solicitarListaFiguras(  // inter inter.
    VSocket& socket) {  // para el inter inter esto es handshake

  std::cout << "llega a solicitud con intermediario" << std::endl;
  // Conectarse al otro intermediario
  // if (socket != -1) {

  std::string listaPropia = "";
  mtx_inter.lock();
  for (auto const& [figura, info] : tablaPropia) {  // armando el que envia
    if (!listaPropia.empty()) listaPropia += ",";
    listaPropia += figura;
  }
  mtx_inter.unlock();

  JafetPrime::Handshake miHandshake;
  miHandshake.tipo = JafetPrime::INTERMEDIARY_HANDSHAKE;
  miHandshake.content = listaPropia;

  std::vector<uint8_t> handshakeBytes = miHandshake.serialize();
  // write(socket, handshakeBytes.data(), handshakeBytes.size());
  socket.Write(handshakeBytes.data(), handshakeBytes.size());
  std::cout << "termina la solicitud" << std::endl;

  // close(st);
  // }
}

// debe solicitar y luego debe de recibr
void Lego_Inter::handShakeReceive(std::stop_token st,
                                  std::string& ipIntermediario,
                                  VSocket& socket) {
  solicitarListaFiguras(socket);
  recibirListaInter(ipIntermediario, socket);
}

void Lego_Inter::recibirListaInter(std::string& ipIntermediario,
                                   VSocket& socket) {
  try {
    uint8_t tipo = 0;
    uint32_t contentLength = 0;
    if (socket.Read(&contentLength, sizeof(contentLength)) > 0) {
      std::string content(contentLength, '\0');
      if (contentLength > 0) {
        socket.Read(&content[0], contentLength);
        std::stringstream ss(content);
        std::string figura;
        mtx_global.lock();
        while (std::getline(ss, figura, ',')) {
          RutaFigura nuevaRuta;
          nuevaRuta.ip = ipIntermediario;
          nuevaRuta.puerto =
              PUERTO_PRUEBA_RESPONSE;  // bien quemado por el momento pero meh
          tablaGlobal[figura].push_back(nuevaRuta);

          std::cout << "   [Descubierta] " << figura << " - " << ipIntermediario
                    << ":" << PUERTO_PRUEBA_RESPONSE << std::endl;
        }
        mtx_global.unlock();
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error en lectura de Handshake de forma elegante: " << e.what()
              << std::endl;
  }
}
