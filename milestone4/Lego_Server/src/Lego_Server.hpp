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
#include "FileSystem.hpp" //
#include "Logger.hpp"
#include "Socket.h"
using namespace ez;
extern int PORT4SSL;
extern int PORT4;
extern int PORT6SSL;
extern int PORT6;
extern size_t BUFFER_SIZE;
extern int BACKLOG_QUEUE;
extern uint32_t DEFAULT_DECISIONS_INODE;

class Lego_Server {
 public:
  /**
   * @brief Constructor del servidor
   * @param key nombre del key
   * @param cert ip del key
   * @param queue_capacity size de backlog de conexiones
   * @param fs Referencia al sistema de archivos
   * @param path Ruta donde montar/formatear FS
   */
  Lego_Server(const char* key, const char* cert, int queue_capacity,
              ez::FileSystem& fs, std::string path);

  /**
   * @brief Destructor
   */
  ~Lego_Server();

  /**
   * @brief Ejecuta el ciclo principal del servidor
   * @return EXIT_SUCCESS o EXIT_FAILURE
   */
  int cicloServer();

  /**
   * @brief Inicializa o monta el sistema de archivos
   * @param path Ruta del FS
   */
  void initializeFileSystem(std::string path);

  /**
   * @brief Pobla el sistema de archivos con datos iniciales
   */
  void populateFileSystem();

 private:
 
 int puertoB=0;
   ///@brief key del servidor
  const char * key;    
  ///
  const char * cert;
  ///@brief Sockets servidor
  VSocket* server_socket_4;
  VSocket* server_socket_6;
  /// @brief las que lleven ssl ocupan certificado (aparentemente por cada ip)
  VSocket* server_socket_4_ssl;
  VSocket* server_socket_6_ssl;

   ///@brief Backlog de conexiones
  int queue_capacity; 
  /**
   * @brief mutex para el acceso al file system, 
   * 
   */
  std::mutex ezy_mutex;     
  // referencia al propio fileSystem
  ez::FileSystem& ezy_file;
  /// @brief hilos del servidor que aceptan conexiones entrantes
  std::vector<std::jthread> threads;  

  /// Respuesta HTML por defecto para 404
  std::string errorBody =
      "<h1>404 Not Found</h1><p>La figura o parte no existe en el sistema.</p>";

  /// Respuesta HTML para request invalido
  std::string badReq =
      "<h1>400 Bad Request</h1><p>Este servidor requiere HTTP/1.1</p>";

  /**
   * @brief Inicializa sockets del servidor
   */
  void inicializar_Server();

  /**
   * @brief Realiza bind de sockets a puertos disponibles
   */
  void bindServer();

  /**
   * @brief Inicia loops de aceptación de conexiones
   */
  void listen();

  /**
   * @brief Cierra y libera sockets del servidor
   */
  void cierreServers();

  /**
   * @brief Handler de signal (SIGINT)
   * @param signal senal recibida recibida
   */
  static void handleSignal(int signal);

  /**
   * @brief Loop de wait de conexion y aceptacion de conexion de clientes
   * @param stop_token Token de parada
   * @param server Socket servidor
 
   */
  void acceptLoop(std::stop_token stop_token, VSocket* server);

  /**
   * @brief Maneja la comunicacion con un cliente
   * @param stop_token Token de parada
   * @param client Socket cliente
   */
  void connectionHandling(std::stop_token stop_token, VSocket* client);

  /**
   * @brief Lee headers HTTP desde el cliente
   * @param client Socket cliente
   * @param datos Stream acumulador
   * @param stringData String con headers
   * @param st Token de parada
   * @return >0 si exito, 0 si falla/(conexion cerrada)
   */
  int readHeaders(VSocket* client, std::stringstream& datos,
                  std::string& stringData, std::stop_token& st);

  /**
   * @brief Lee el body HTTP basado en Content-Length
   * @param body_s Stream destino
   * @param stringData Headers completos
   * @param client Socket cliente
   * @param st Token de parada
   * @return 1 si exito, 0 si falla
   */
  int readBody(std::stringstream& body_s, std::string& stringData,
               VSocket* client, std::stop_token& st);

  /**
   * @brief Construye una respuesta HTTP
   * @param status codesito HTTP ("200 OK")
   * @param length Longitud del body
   * @param type_conect Tipo de conexion ("keep-alive"/"close")
   * @param contentType MIME type
   * @param content Body de la respuesta
   * @return String HTTP completo
   */
  std::string build_HttpResponse(std::string status, uint32_t length,
                                 std::string type_conect,
                                 std::string contentType, std::string& content);

  /**
   * @brief Busca un puerto disponible y hace bind
   * @param socket Socket a bindear
   * @param startPort Puerto inicial
   * @return Puerto asignado
   */
  int BindPortAvailable(VSocket* socket, int startPort);

  /**
   * @brief Crea o sobrescribe una figura en el FS
   * @param directory Directorio de la figura
   * @param figuras1 Contenido
   * @param figuras2 Nombre/parte
   */
  void populateFigure(std::string directory, std::string figuras1,
                      std::string figuras2);

  /**
   * @brief Parsea request HTTP para obtener figura y parte
   * @param stringData Request completo
   * @return Par (figura, parte)
   */
  std::pair<std::string, int> parseLegoRequest(const std::string& stringData);

  /**
   * @brief Inicializa archivo de logs
   */
  void populateLogFile();

  /**
   * @brief Registra un evento en logs
   * @param e Entidad
   * @param t Tipo de log
   * @param m Mensaje
   */
  void registrarEvento(Entidad e, TipoLog t, const std::string& m);

  /**
   * @brief Verifica version HTTP, para que al responder cierre conexion o o no
   * @param stringData Request
   * @param versionEsperada version esperada (ej: "1.1")
   * @return 1 si coincide, 0 si no
   */
  int HTTPver(const std::string& stringData,
              const std::string& versionEsperada);

  /**
   * @brief Reemplaza placeholders en strings
   * @param target String objetivo
   * @param placeholder Texto a reemplazar
   * @param value Nuevo valor
   */
  void replace(std::string& target, const std::string& placeholder,
               const std::string& value);

  /**
   * @brief Construye pagina HTML de figura LEGO
   * @param figureName Nombre de figura
   * @param content Contenido parseado
   * @return HTML generado
   */
  std::string buildFigurePage(const std::string& figureName,
                              std::string content);



  void responderDiscovery(VSocket& discoverySocket, sockaddr_in sender);
  void discoveryListener(std::stop_token st);

    void broadCastToIntermediary();

};

#endif  // LEGO_SERVER_HPP
