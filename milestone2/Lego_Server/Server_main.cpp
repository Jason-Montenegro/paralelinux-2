
/**
 * Main server function
 * Initializes FileSystem API and accepts connections
 */

#include "FileSystem.hpp"
#include "Lego_Server.hpp"
int main(int argc, char** argv) {
  // Lego_Server server(); //tengo que pasarle el file system
  // API* api = nullptr;
      std::cout<<">>"<<std::endl;
  std::string host = "ivan.com";  // el nombre propio
  std::string lego_ip = "";
  std::string container = "disk.bin";
  uint32_t queue_capacity = 20;
  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argc >= 2) {
      if (argument.find("host:") != std::string::npos) {
        std::string length = "host:";
        host = (argument.substr(length.size(), argument.length() - 1));
        std::cout << "prueba del host correcto " << host << std::endl;
      } else {
        if (argument.find("ip:") != std::string::npos) {
          std::string length = "ip:";
          lego_ip = (argument.substr(length.size(), argument.length() - 1));
          std::cout << "prueba del ip  correcto " << lego_ip << std::endl;
        } else {
          if (argument.find("container:") != std::string::npos) {
            std::string length = "container:";
            container = (argument.substr(length.size(), argument.length() - 1));
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
  }
     std::cout<<"pasa atributos"<<std::endl;
    ez::FileSystem ez_file;

    // if (!container.empty()) {
    //   auto mount_res = ez_file.mount(container);

    //   if (!mount_res.has_value()) {
    //     std::cout << "Fallo al montar '" << container <<  " formateando"
    //               << std::endl;

    //     auto format_res = ez_file.format(container, 256 * 40000);

    //     if (!format_res.has_value()) {
    //       std::cout << " no se pudo formatear el disco, se usara temp"
    //                 << std::endl;
    //       ez_file.format("temp.bin", 256 * 40000);
    //     }
    //   } else {
    //     std::cout << "Contenedor '" << container << "' montado correctamente."
    //               << std::endl;
    //   }
    // } else {
    //   ez_file.format("temp.bin", 256 * 40000);
    // }

    Lego_Server server(host.c_str(), lego_ip.c_str(), queue_capacity, ez_file,container);
    server.cicloServer();
    std::cout<<"termina"<<std::endl;
    return EXIT_SUCCESS;
  }

