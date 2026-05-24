#include <cstdlib>
#include <iostream>
#include <string>

#include "Lego_Client.h"

int main(int argc, char* argv[]) {
  int ipVer = 6;  // 4 = IPv4, 6 = IPv6
  int ssl = 0;    // 0 = no SSL, 1 = SSL
  std::string host = "os.ecci.ucr.ac.cr";
  std::string container = "disk.bin";
  std::string lego_ip = "";
  
  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argc >= 2) {// see if i have to pass the freaking service 
      if (argument.find("+ssl") !=
          std::string::npos) {  // figura numero o numero figura
        // std::string handle = "+ssl:";
        ssl = 1;
      } else {
          if (argument.find("host:") != std::string::npos) {
            std::string length = "host:";
            host =
                (argument.substr(length.size(), argument.length() - 1)).c_str();
            std::cout << "prueba del host correcto " << host << std::endl;
          } else {
            if (argument.find("ip:") != std::string::npos) {
              std::string length = "ip:";
              lego_ip = (argument.substr(length.size(), argument.length() - 1))
                            .c_str();
              std::cout << "prueba del ip  correcto " << lego_ip << std::endl;
            } else {
              if(argument.find("container:")!= std::string::npos){
                container = (argument.substr(length.size(), argument.length() - 1))
                            .c_str();
              }
            }
          }
        
      }
    }
  }
  Lego_Client client(ipVer, ssl, host.c_str(), lego_ip.c_str());
  client.cicloCliente();
  return EXIT_SUCCESS;
}
