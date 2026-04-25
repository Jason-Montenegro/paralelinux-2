#ifndef LEGO_SERVER_HPP
#define LEGO_SERVER_HPP

#include <string>
#include <vector>
#include <thread>
#include <memory>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "FileSystem.hpp"
#include "EzTypes.hpp"
#include "Socket.h"
using namespace ez;
// Constantes Globales (Extern para evitar redifiniciones si se incluye en varios archivos)
extern int PORT4SSL;
extern int PORT4;
extern int PORT6SSL;
extern int PORT6;
extern size_t BUFFER_SIZE;
extern int BACKLOG_QUEUE;
extern uint32_t DEFAULT_DECISIONS_INODE;

class Lego_Server {
public:
    // Constructor y Destructor
    Lego_Server(const char* host, const char* lego_ip,int queue_capacity, ez::FileSystem &,std::string);
    ~Lego_Server();


    int cicloServer();

    void initializeFileSystem(std::string);
    //void populateFileSystem();

private:

    VSocket* server_socket = nullptr;
    std::string host;
    int ipVer;
    int queue_capacity;
    ez::FileSystem& ezy_file;
    //FileSystem ezy_file; 
    static inline std::string fsPath = "lego_storage.bin"; // Ruta del FS

    void acceptLoop(VSocket* server,int)  ;
    void connectionHandling(VSocket* client);
    std::string build_HttpResponse(uint32_t length, std::string type_conect, std::string& content);
   int BindPortAvailable(VSocket* socket, int startPort);
    void populateFigure(std::string directory, std::string figuras1, std::string figuras2);
    void populateDuckFigure();
    void populateAmogusFigure(); // Implementar en .cpp
    void populateLogFile();
};




#endif // LEGO_SERVER_HPP