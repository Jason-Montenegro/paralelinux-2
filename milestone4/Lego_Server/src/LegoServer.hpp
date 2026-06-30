#ifndef LEGO_SERVER_HPP
#define LEGO_SERVER_HPP

#include <algorithm>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "EzTypes.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"
#include "Socket.h"

using namespace ez;

extern int PORT4SSL;
extern int PORT4;
extern int PORT6SSL;
extern int PORT6;
extern int BACKLOG_QUEUE;
extern uint32_t DEFAULT_DECISIONS_INODE;

class LegoServer {
 public:
  /**
   * @brief Inicializa el sistema de archivos
   * @param key nombre del key para ssl
   * @param cert ip del key para ssl
   * @param queueCapacity tamano de backlog de conexiones
   * @param ezyFile sistema de archivos existente
   * @param path ruta donde montar/formatear FS
   */
  LegoServer(const char* key, const char* cert, int queueCapacity,
              ez::FileSystem& ezyFile, std::string path);

  ~LegoServer();

  /**
   * @brief Ejecuta el ciclo principal del servidor
   * @return EXIT_SUCCESS si no hubo errores, sino EXIT_FAILURE
   */
  int runServer();

 private:
  /**
   * @brief Handler de signal (SIGINT)
   * @param signal senal recibida recibida
   */
  static void handleSignal(int signal);

  /**
   * @brief Inicializa o monta el sistema de archivos
   * @param path ruta del sistema de archivos
   */
  void initializeFileSystem(std::string path);

  /**
   * @brief Pobla el sistema de archivos con datos iniciales
   */
  void populateFileSystem();

    /**
   * @brief Crea o sobrescribe una figura en el sistema de archivos
   * @param directory directorio de la figura
   * @param figuras1 contenido
   * @param figuras2 nombre/parte
   */
  void populateFigure(std::string directory, std::string figuras1,
                      std::string figuras2);

  /**
   * @brief Inicializa archivo de logs
   */
  void populateLogFile();

  /**
   * @brief Inicializa sockets del servidor
   */
  void initializeSockets();

  /**
   * @brief Asocia sockets del servidor con puertos correspondientes
   */
  void bindServer();

  /**
   * @brief Itera a partir de startPort en adelante hasta que encuentre un 
            puerto para enlazar
   * @param socket socket a bindear
   * @param startPort puerto inicial
   * @return puerto asignado
   */
  int bindAvailablePort(VSocket* socket, int startPort);

  /**
   * @brief Empieza a escuchar por conexiones (clientes)
   */
  void listen();

  /**
   * @brief Ciclo de espera y aceptacion de conexiones de clientes
   * @param stop_token token de parada
   * @param server socket escuchando
   */
  void acceptLoop(std::stop_token stop_token, VSocket* server);

  /**
   * @brief Atiende la comunicacion con un cliente
   * @param stop_token Token de parada
   * @param client Socket cliente
   */
  void handleClientConnection(std::stop_token stop_token, VSocket* client);

  /**
   * @brief Registra un evento en bitacora
   * @param e entidad
   * @param t tipo de bitacora
   * @param m mensaje
   */
  void registrarEvento(Entidad e, TipoLog t, const std::string& m);

  /**
   * @brief Lee encabezados HTTP desde el cliente
   * @param client socket cliente
   * @param datos stream acumulador
   * @param stringData encabezados
   * @param st token de paradas
   * @return valor > 0 si hubo exito, 0 si falla/conexion cerrada
   */
  int readHeaders(VSocket* client, std::stringstream& datos,
                  std::string& stringData, std::stop_token& st);

   /**
   * @brief Verifica version HTTP, para que al responder cierre conexion o no
   * @param stringData HTTP request
   * @param versionEsperada version esperada (ej: "1.1" o "1.0")
   * @return 1 si coincide, 0 si no
   */
  int HTTPver(const std::string& stringData, const std::string& versionEsperada);

  /**
   * @brief Lee el body HTTP basado en el encabezado HTTP Content-Length
   * @param body_s stream destino
   * @param stringData encabezados completos
   * @param client socket cliente
   * @param st token de parada
   * @return 1 si exito, 0 si falla
   */
  int readBody(std::stringstream& body_s, std::string& stringData,
               VSocket* client, std::stop_token& st);

  /**
   * @brief Parsea request HTTP para obtener figura y parte
   * @param stringData request completo
   * @return Par (figura, parte)
   */
  std::pair<std::string, int> parseLegoRequest(const std::string& stringData);

  /**
   * @brief Construye una respuesta HTTP
   * @param status codigo HTTP ("200 OK")
   * @param length longitud del body
   * @param type_conect tipo de conexion ("keep-alive"/"close")
   * @param contentType MIME type
   * @param content Body de la respuesta
   * @return respuesta HTTP completa
   */
  std::string buildHttpResponse(std::string status, uint32_t length,
                                 std::string type_conect,
                                 std::string contentType, std::string& content);
  /**
   * @brief Anuncia un nuevo servidor a la red para que algun intermediario
            existente lo conozca
   */
  void broadcastToNetwork();

  /**
   * @brief Escucha por mensajes UDP de intermediarios para descubrir al servidor
   * @param st token de parada
   */
  void serverDiscoveryListener(std::stop_token st);

  /**
   * @brief Responde con un mensaje al intermediario que descubre al servidor
   * @param discoverySocket socket escuchando por mensajes de intermediarios
   * @param sender direccion del intermediario
   */
  void answerToServerDiscovery(VSocket& discoverySocket, sockaddr_in sender);

  /**
   * @brief Cierra y libera sockets del servidor
   */
  void closeSockets();

  int puertoB{};
   ///@brief key del servidor para SSL
  const char* key{};
  /// @brief certificado del servidor para SSL
  const char* cert{};
  ///@brief sockets servidor para escuchar por clientes
  VSocket* server_socket_4{};
  VSocket* server_socket_6{};
  /// @brief las que lleven SSL ocupan certificado por cada ip
  VSocket* server_socket_4_ssl{};
  VSocket* server_socket_6_ssl{};
  ///@brief capacidad de backlog de conexiones
  int queueCapacity; 
  /// @brief mtx para acceso al sistema de archivos
  std::mutex ezyMtx;
  /// @brief interfaz de sistema de archivos
  ez::FileSystem& ezyFile;
  /// @brief hilos del servidor que aceptan conexiones entrantes
  std::vector<std::jthread> workerThreads;  
  /// respuesta HTML por defecto para 404
  std::string errorBody =
      "<h1>404 Not Found</h1><p>La figura o parte no existe en el sistema.</p>";
  /// respuesta HTML por defecto para request invalido
  std::string badReq =
      "<h1>400 Bad Request</h1><p>Este servidor requiere HTTP/1.1</p>";
};

#endif  // LEGO_SERVER_HPP
