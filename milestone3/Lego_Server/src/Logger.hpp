#ifndef LOGGER_H
#define LOGGER_H

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

// enums para que logger pueda usar
enum class Entidad { CLIENTE, INTERM, SERVIDOR };

enum class TipoLog {
  SOLICITUD,     // recep de solicitudes
  ENRUTAMIENTO,  // enrutamiento de mensajes
  RESPUESTA,     // generación de respuestas
  ERROR          // ocurrencia de errores
};
// declaracion de metodos de enums
std::string EntidadtoString(Entidad e);
std::string TipotoString(TipoLog t);

class Logger {
 public:
  std::string getFechaHora();
  Logger();
  ~Logger();
  std::string log(Entidad entidad, TipoLog tipo, const std::string& mensaje);
};

// instancia global, evita error con hilos
extern Logger logger;

#endif