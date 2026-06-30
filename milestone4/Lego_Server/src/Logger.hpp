#ifndef LOGGER_H
#define LOGGER_H

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

/// @brief Tipos de entidad para bitacoras
enum class Entidad { CLIENTE, INTERM, SERVIDOR };

/// @brief Tipos de bitacoras
enum class TipoLog {
  SOLICITUD,     // recep de solicitudes
  ENRUTAMIENTO,  // enrutamiento de mensajes
  RESPUESTA,     // generación de respuestas
  ERROR          // ocurrencia de errores
};

// Conversion de enums a string

std::string EntidadtoString(Entidad e);
std::string TipotoString(TipoLog t);

class Logger {
 public:
  std::string getFechaHora();
  /**
   * Logger crea una carpeta log
   * Abre el archivo y habilita el proceso de logs
  */
  Logger() = default;
  ~Logger() = default;
  std::string log(Entidad entidad, TipoLog tipo, const std::string& mensaje);
};

// instancia global, evita error con hilos
extern Logger logger;

#endif