#include "Logger.hpp"

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

Logger logger;

std::string EntidadtoString(Entidad e) {
  switch (e) {
    case Entidad::CLIENTE:  return "CLIENTE";
    case Entidad::INTERM:   return "INTERM";
    case Entidad::SERVIDOR: return "SERVIDOR";
    default:                break;
  }
  return "";
}

std::string TipotoString(TipoLog t) {
  switch (t) {
    case TipoLog::SOLICITUD:    return "SOLICITUD";
    case TipoLog::ENRUTAMIENTO: return "ENRUTAMIENTO";
    case TipoLog::RESPUESTA:    return "RESPUESTA";
    case TipoLog::ERROR:        return "ERROR";
    default:                    break;
  }
  return "";
}

std::string Logger::getFechaHora() {
  std::time_t now = std::time(nullptr);
  std::tm* local = std::localtime(&now);

  char buffer[20];
  std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M", local);
  return std::string(buffer);
}

std::string Logger::log(Entidad entidad, TipoLog tipo,
                        const std::string& mensaje) {
  std::string cadenaLog = "[" + ::EntidadtoString(entidad) + "] [" +
                          getFechaHora() + "] [" + TipotoString(tipo) + "] " +
                          mensaje;
  return cadenaLog;
}
