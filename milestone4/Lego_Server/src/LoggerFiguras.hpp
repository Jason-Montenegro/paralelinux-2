#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <mutex>
#include <filesystem>
#include <sstream> 
namespace fs = std::filesystem;

//enums para que logger pueda usar
enum class Entidad {
    CLIENTE,
    INTERM,
    SERVIDOR
};
 
enum class TipoLog {
    SOLICITUD,    // recepción de solicitudes
    ENRUTAMIENTO, // enrutamiento de mensajes
    RESPUESTA,    // generación de respuestas
    ERROR         // ocurrencia de errores
};
//declaracion de metodos de enums
std::string EntidadtoString(Entidad e);
std::string TipotoString(TipoLog t);


class LoggerFiguras{
private:
    std::ofstream file;
    std::mutex mtx;
    std::string nombreArchivo;

public:
    std::string getFechaHora();
    LoggerFiguras();
    ~LoggerFiguras();
    void log(Entidad entidad, TipoLog tipo, const std::string& mensaje);
        std::string leerLog(); 
};

//instancia global, evita error con hilos
//extern Logger logger;

#endif