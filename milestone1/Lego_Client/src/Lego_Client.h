#ifndef LEGO_CHIKI_CLIENTE
#define LEGO_CHIKI_CLIENTE

#include <map>
#include <string>
#include <vector>

#include "SSLSocket.h"
#include "Socket.h"

/**
 * @class Lego_Client
 * @brief Cliente HTTP/HTTPS para interactuar con el servidor de piezas Lego de
 * os.ecci.uc.ac.cr.
 *
 * Esta clase permite:
 * - Conectarse a un servidor (con o sin SSL)
 * - Solicitar la lista de figuras disponibles
 * - Solicitar piezas de una figura (completa o por mitades)
 * - Parsear respuestas HTML para extraer datos relevantes (sin mostrar HTML)
 * parsea la lista de figuras de legos
 * parsea la cantidad de figuras y las piezas especificas
 */
class Lego_Client {
 private:
  /**
  * @brief Mapa que almacena las piezas de una figura. actualmente, cuando un
  cliente pide las figuras, las almacena las muestra y vacia el map
  * key   nombre de la pieza (ej: brick-2x2-blue)
  * value -> cantidad total requerida de esa pieza en especifico

  */
  std::map<std::string, int> lego_figuras;  //
  /// @brief Socket polimorfico (puede ser Socket o SSLSocket).
  VSocket* client;
  /// @brief Direccion IP del servidor
  const char* lego_ip;
  ///@brief Indica si se usa SSL (1 = si, 0 = no).
  int SSL;  // usar SSL o no
  /// @brief Version de IP (4 o 6).
  int ipVer;  // 4 o 6
  ///@brief Nombre del host del servidor, en caso de que se haga la coneccion
  ///por nombre y no por ip.
  const char* host;

  /**
  * @brief Abre conexion con el servidor segun configuracion (IPv4/IPv6, SSL/no
  SSL). Dentro creara el objeto de Socket o SSLSocket
  *
  * @return int estado de la conexion
  */
  int OpenConnection();
  /**
   * @brief Lee la respuesta completa del servidor, en un ciclo con buffer en
   * caso de que la respuesta sea larga.
   * @return std::string respuesta HTTP completa
   */

  /**
   * @brief Lee la respuesta completa del servidor.
   *
   * @return std::string respuesta HTTP completa
   */
  std::string readResponse();

  /**
   * @brief Construye un request HTTP GET para solicitar una figura.
   *
   * @param figura Nombre de la figura
   * @param mitad Parte de la figura (1, 2 o completa)
   * @return std::string request HTTP
   */
  std::string buildHTTPRequest(const std::string& figura, int& mitad);

  /**
   * @brief Construye un request HTTP GET para obtener la lista de figuras.
   *
   * @return std::string request HTTP
   */
  std::string buildListRequest();

  /**
   * @brief Extrae el body de una respuesta HTTP.
   *
   * @param response Respuesta completa del servidor
   * @return std::string contenido del body
   */
  std::string parseBody(const std::string& response);

  /**
   * @brief Genera el menu del cliente.
   * @return std::string texto del menu
   */
  std::string mostrarMenu();

  /**
   * @brief Parsea la lista de figuras desde HTML.
   *
   * @param body HTML recibido
   * @return std::vector<std::string> lista de nombres de figuras
   */
  std::vector<std::string> parseList(const std::string& body);

  /**
   * @brief Parsea las piezas de una figura desde HTML.
   *
   * Extrae el nombre de las piezas y la cantidad de cada una y las guarda en un
   * map @see lego_figuras
   *
   * @param body HTML de la figura
   */
  void parseFigures(const std::string& body);

 public:
  /**
   * @brief Constructor del cliente Lego.
   *
   * @param ipVer Version IP (4 o 6)
   * @param SSL Activar SSL (1 si, 0 no)
   * @param host Host del servidor
   * @param lego_ip Direccion IP alternativa (default localhost)
   */
  Lego_Client(int ipVer = 4, int SSL = 0, const char* host = nullptr,
              const char* lego_ip = "127.0.0.1");

  /**
   * @brief Destructor del cliente.
   */
  ~Lego_Client();

  /**
   * @brief Solicita la lista de figuras al servidor.
   *
   * @return std::string body de la respuesta
   */
  std::string solicitarLista();

  /**
   * @brief Solicita una mitad o la totalidad de una figura, en caso de que
   * hayan piezas para esta figura o exista en primer lugar.
   *
   * @param figura Nombre de la figura
   * @param mitad Parte (1, 2 o completa)
   */
  void solicitarMediaFigura(const std::string& figura, int mitad);

  /**
   * @brief Ejecuta el ciclo interactivo del cliente (el menu y el proceso de
   * request al serividor de figuras)
   */
  void cicloCliente();
};

#endif