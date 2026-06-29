#include "LoggerFiguras.hpp"
// definicion de logger global
LoggerFiguras logger;

// metodos de enums

// metodo para pasar la entidad de enum a string
std::string EntidadtoString(Entidad e) {
  switch (e) {
    case Entidad::CLIENTE:
      return "CLIENTE";
    case Entidad::INTERM: // aun no pequenna bebe
      return "INTERM";
    case Entidad::SERVIDOR:
      return "SERVIDOR";
    default:
      break;
  }
  return "";
}

std::string TipotoString(TipoLog t) {
  switch (t) {
    case TipoLog::SOLICITUD:
      return "SOLICITUD";
    case TipoLog::ENRUTAMIENTO:
      return "ENRUTAMIENTO";
    case TipoLog::RESPUESTA:
      return "RESPUESTA";
    case TipoLog::ERROR:
      return "ERROR";
    default:
      break;
  }
  return "";
}

std::string LoggerFiguras::getFechaHora() {
  std::time_t now = std::time(nullptr);
  std::tm* local = std::localtime(&now);

  char buffer[20];
  std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M", local);
  return std::string(buffer);
}

/**
 * LoggerFiguras crea una carpeta log
 * Abre el archivo y habilita el proceso de logs
 *
 */
LoggerFiguras::LoggerFiguras(){
    fs::create_directory("logInter");
    this->file.open("logInter/logInter.txt", std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Error abriendo archivo de log\n";
    }
}

LoggerFiguras::~LoggerFiguras() {
    if (file.is_open()) {
        file.close();
    }
}

void LoggerFiguras::log(Entidad entidad, TipoLog tipo, const std::string& mensaje){
    std::lock_guard<std::mutex> lock(mtx);
    std::string cadenaLog = "["+ ::EntidadtoString(entidad)+ "] ["
                            + getFechaHora() + "] [" +
                            TipotoString(tipo) + "] " + mensaje;
    file << cadenaLog << std::endl;
}

std::string LoggerFiguras::leerLog() {
    std::lock_guard<std::mutex> lock(this->mtx);
    std::ifstream archivoLectura("log/logInter.txt"); 
    std::stringstream buffer;
    buffer << archivoLectura.rdbuf();
    return buffer.str();
}