
/**
 * Main server function
 * Initializes FileSystem API and accepts connections
 */

#include "Lego_Inter.hpp"

int main(int argc, char** argv) {
  std::cout<<"Inicio de servidor "<<std::endl;
  std::string key = "key.pem";  // el nombre propio
  std::string cert = "cert.pem";
  std::string container = "b.bin";
  uint32_t queue_capacity = 20;
  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argc >= 2) {
      if (argument.find("key:") != std::string::npos) {
        std::string length = "key:";
        key = (argument.substr(length.size(), argument.length() - 1));
        std::cout << "prueba del key correcto " << key << std::endl;
      } else {
        if (argument.find("cert:") != std::string::npos) {
          std::string length = "cert:";
          cert = (argument.substr(length.size(), argument.length() - 1));
          std::cout << "prueba del cert  correcto " << cert << std::endl;
        } else { 
            if (argument.find("queueCapacity:") != std::string::npos) {
              std::string length = "queueCapacity:";
              queue_capacity = std::stoi(
                  (argument.substr(length.size(), argument.length() - 1)));
            }
          
        }
      }
    }
  }
  Lego_Inter server(key.c_str(), cert.c_str(), queue_capacity,container);
  server.cicloServer();
  
  return EXIT_SUCCESS;
  }

