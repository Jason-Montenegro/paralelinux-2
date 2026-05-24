#include <cstdlib>
#include <iostream>
#include <string>

#include "Lego_Client.hpp"

int main(int argc, char* argv[]) {
  int ipVer =4;  // 4 = IPv4, 6 = IPv6
  int ssl = 0;    // 0 = no SSL, 1 = SSL
  std::string host = "localhost";
  std::string lego_ip = "127.0.0.1";

  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argc >= 2) {
      if (argument.find("+ssl") !=
        std::string::npos) {  // figura numero o numero figura
        // std::string handle = "+ssl:";
        ssl = 1;
      } else if (argument.find("IPv6") != std::string::npos) {
        ipVer = 6;
      } else if (argument.find("host:") != std::string::npos) {
        host = argument.substr(5, argument.length() - 5);
        std::cout << "host: " << host << std::endl;
          /*std::string length = "host:";
          host =
              (argument.substr(length.size(), argument.length() - 1)).c_str();
          std::cout << "prueba del host correcto " << host << std::endl;*/
      } else if(argument.find("ip:") != std::string::npos) {
        lego_ip = argument.substr(3, argument.length() - 3);
        std::cout << "ip: " << lego_ip << std::endl;
             /*(argument.find("ip:") != std::string::npos) {
              std::string length = "ip:";
              lego_ip = (argument.substr(length.size(), argument.length() - 1))
                            .c_str();
              std::cout << "prueba del ip  correcto " << lego_ip << std::endl;*/

      }
    }
  }
  Lego_Client client(ipVer, ssl, host.c_str(), lego_ip.c_str());
  client.cicloCliente();
  return EXIT_SUCCESS;
}
