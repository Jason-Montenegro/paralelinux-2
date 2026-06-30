#ifndef LOGGER_FIGURAS_H
#define LOGGER_FIGURAS_H

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

/// @brief Tipos de entidad para bitacoras
enum class Entidad { CLIENTE, INTERM, SERVIDOR };

/// @brief Tipos de bitacoras
enum class TipoLog {
    SOLICITUD,    // recepción de solicitudes
    ENRUTAMIENTO, // enrutamiento de mensajes
    RESPUESTA,    // generación de respuestas
    ERROR         // ocurrencia de errores
};

// Conversion de enums a string

std::string EntidadtoString(Entidad e);
std::string TipotoString(TipoLog t);

class LoggerFiguras {
 public:
  std::string getFechaHora();
  /**
   * LoggerFiguras crea una carpeta log
   * Abre el archivo y habilita el proceso de logs
  */
  LoggerFiguras();
  ~LoggerFiguras();
  void log(Entidad entidad, TipoLog tipo, const std::string& mensaje);
      std::string leerLog(); 
 private:
  std::ofstream file;
  std::mutex mtx;
  std::string nombreArchivo;
};

#endif
