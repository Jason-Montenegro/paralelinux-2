#include "Logger.hpp"
//definicion de logger global
Logger logger;

//metodos de enums

//metodo para pasar la entidad de enum a string
std::string EntidadtoString(Entidad e){
    switch (e)
    {
    case Entidad::CLIENTE: 
        return "CLIENTE";
    case Entidad::INTERM: 
        return "INTERM";
    case Entidad::SERVIDOR:
        return "SERVIDOR";
    default:
        break;
    }
    return "";
}

std::string TipotoString(TipoLog t){
    switch (t){
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

    
//toma la fecha actual y hora
std::string Logger::getFechaHora(){
    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);

    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M", local);
    return std::string(buffer);
}

/**
 * Logger crea una carpeta log
 * Abre el archivo y habilita el proceso de logs
 * 
*/
Logger::Logger(){
    fs::create_directory("log");
    this->file.open("log/log.txt", std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Error abriendo archivo de log\n";
    }
}

Logger::~Logger() {
    if (file.is_open()) {
        file.close();
    }
}
/**
 * Proceso de loggeo, 
 * recibe entidad, tipo de log y cuerpo
 * agraga fecha y hora
 */
void Logger::log(Entidad entidad, TipoLog tipo, const std::string& mensaje){
    std::lock_guard<std::mutex> lock(mtx);
    std::string cadenaLog = "["+ ::EntidadtoString(entidad)+ "] ["
                            + getFechaHora() + "] [" +
                            TipotoString(tipo) + "] " + mensaje;
    file << cadenaLog << std::endl;
}