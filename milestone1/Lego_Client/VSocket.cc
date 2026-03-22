/**
 *  Universidad de Costa Rica
 *  ECCI
 *  CI0123 Proyecto integrador de redes y sistemas operativos
 *  2026-i
 *  Grupos: 2 y 3
 **/



#include <arpa/inet.h>  // ntohs, htons
#include <netdb.h>      // getaddrinfo, freeaddrinfo
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>  // close

#include <cstddef>
#include <cstdio>
#include <cstring>    // memset
#include <stdexcept>  // runtime_error
#include "VSocket.h"
/**
 *  Class creator (constructor)
 *     use Unix socket system call
 *
 *  @param     char t: socket type to define
 *     's' for stream
 *     'd' for datagram
 *  @param     bool ipv6: if we need a IPv6 socket
 *
 **/
void VSocket::Init(char t, bool IPv6) {
  // asumo que esto crea el socket socket ya que tiene la version y t de
  // datagram o stream
  this->IPv6 = IPv6;
  this->type = t;
  int type = SOCK_STREAM;
  if (t != 'd' && t != 's') {
    throw std::runtime_error("VSocket::Init");
  }
  if (t == 'd') {
    type = SOCK_DGRAM;
  }
  if (IPv6) {
    this->idSocket = socket(AF_INET6, type, 0);
  } else {
    this->idSocket = socket(AF_INET, type, 0);
  }
  if (-1 == this->idSocket) {
    throw std::runtime_error("VSocket::Init");
  }
}

/**
 * Class destructor
 *
 **/
VSocket::~VSocket() { this->Close(); }

/**
 * Close method
 *    use Unix close system call (once opened a socket is managed like a file in
 * Unix)
 *
 **/
void VSocket::Close() {
  if (this->idSocket != -1) {
    close(this->idSocket);
    this->idSocket = -1;
  }
}

/**
 * TryToConnect method
 *   use "connect" Unix system call
 *
 * @param      char * host: host address in dot notation, example "10.84.166.62"
 * @param      int port: process address, example 80
 *
 **/

int VSocket::TryToConnect(const char* hostip, int port) {
  // Para IPv6
  int st = -1;
  if (this->IPv6) {
    struct sockaddr_in6 host6;
    struct sockaddr* ha;
    memset(&host6, 0, sizeof(host6));
    host6.sin6_family = AF_INET6;
    st = inet_pton(AF_INET6, hostip, &host6.sin6_addr);
    if (0 >= st) {  // 0 means invalid address, -1 means address error
      throw std::runtime_error(
          "Socket::Connect( const char *, int ) [inet_pton]");
    }
    host6.sin6_port = htons(port);
    ha = (struct sockaddr*)&host6;
    auto len = sizeof(host6);
    st = connect(this->idSocket, ha, len);

  } else {
    st = TryToConnectIPv4(hostip, port);
  }

  if (-1 == st) {
    throw std::runtime_error(
        "VSocket::TryToConnect( const char *, int ) [connect]");
  } else {
    this->port = port;
  }
  return st;
}

// int VSocket::TryToConnectIPv4(){
//   int st = -1;

//   return  st;
// }

int VSocket::TryToConnectIPv4(const char* hostip, int port) {
  // Como utilizar la estructura de conexión, consultar el manual
  // Para IPv4
  int st = -1;
  struct sockaddr_in host4;
  memset((char*)&host4, 0, sizeof(host4));
  host4.sin_family = AF_INET;
  st = inet_pton(AF_INET, hostip, &host4.sin_addr);
  if (-1 == st) {
    throw std::runtime_error("VSocket::TryToConnect, inet_pton");
  }
  host4.sin_port = htons(port);
  st = connect(idSocket, (sockaddr*)&host4, sizeof(host4));
  if (-1 == st) {
    throw std::runtime_error("VSocket::TryToConnect, connect");
  }
  return st;
}

/**
 * TryToConnect method
 *   use "connect" Unix system call
 *
 * @param      char * host: host address in dns notation, example
 * "os.ecci.ucr.ac.cr"
 * @param      char * service: process address, example "http"
 *
 **/
int VSocket::TryToConnect(const char* host, const char* service) {

  struct addrinfo hints, *result, *rp;
  memset(&hints, 0, sizeof(struct addrinfo));
  // hints.ai_family =  AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_family =
      this->IPv6 ? AF_INET6 : AF_INET;  // el bendito operador ternario de jason
  hints.ai_socktype = SOCK_STREAM;      /* Stream socket */
  hints.ai_flags = 0;                   // AI_PASSIVE
  hints.ai_protocol = 0;                /* Any protocol */
  int st = -1;
  st = getaddrinfo(host, service, &hints, &result);
  if (0 != st) {  // segun manual devuelve algun numero postivo si no funca
    throw std::runtime_error("VSocket::TryToConnect : 1");
  }
  for (rp = result; rp; rp = rp->ai_next) {
    this->idSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (this->idSocket == -1) continue;
    st = connect(idSocket, rp->ai_addr, rp->ai_addrlen);
    if (0 == st) break; /* Success */
    close(this->idSocket);
  }
  freeaddrinfo(result);

  if (-1 == st) {
    throw std::runtime_error("VSocket::TryToConnect :2");
  }
  // throw std::runtime_error( "VSocket::TryToConnect" );
  return st;
}

/**
 * Bind method
 *    use "bind" Unix system call (man 3 bind) (server mode)
 *
 * @param      int port: bind a unamed socket to a port defined in sockaddr
 * structure
 *
 *  Links the calling process to a service at port
 *
 **/
int VSocket::Bind(int port) {
  int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
  int st = -1;
  if (IPv6) {
    struct sockaddr_in6 host6;
    memset(&host6, '\0', sizeof(host6));
    host6.sin6_family = AF_INET6;
    host6.sin6_addr = in6addr_any;
    host6.sin6_port = htons(port);

    st = bind(this->idSocket, (struct sockaddr*)&host6, sizeof(host6));
    if (st == -1) {
      throw std::runtime_error("VSocket::Bind");
    }
  } else {
    st = BindIPv4(port);
  }

  return st;
}
int VSocket::BindIPv4(int port) {
  int st = -1;
  struct sockaddr_in host4;

  host4.sin_family = AF_INET;
  host4.sin_addr.s_addr = htonl(INADDR_ANY);
  host4.sin_port = htons(port);
  memset(host4.sin_zero, '\0', sizeof(host4.sin_zero));
  st = bind(this->idSocket, (struct sockaddr*)&host4, sizeof(host4));
  if (st == -1) {
    throw std::runtime_error("VSocket::Bind");
  }
  return st;
}

/**
 *  sendTo method
 *
 *  @param	const void * buffer: data to send
 *  @param	size_t size data size to send
 *  @param	void * addr address to send data
 *
 *  Send data to another network point (addr) without connection (Datagram)
 *
 **/
size_t VSocket::sendTo(const void* buffer, size_t size, void* addr) {
  int st = -1;
  socklen_t dest_addr;
  if (this->IPv6) {
    dest_addr = sizeof(struct sockaddr_in6);
  } else {
    dest_addr = sizeof(struct sockaddr_in);
  }
  st = sendto(this->idSocket, buffer, size, 0, (struct sockaddr*)addr,
              dest_addr);
  return st;
}

/**
 *  recvFrom method
 *
 *  @param	const void * buffer: data to send
 *  @param	size_t size data size to send
 *  @param	void * addr address to receive from data
 *
 *  @return	size_t bytes received
 *
 *  Receive data from another network point (addr) without connection (Datagram)
 *
 **/
size_t VSocket::recvFrom(void* buffer, size_t size, void* addr) {
  int st = -1;
  socklen_t addrlen;
  if (this->IPv6) {
    addrlen = sizeof(struct sockaddr_in6);
  } else {
    addrlen = sizeof(struct sockaddr_in);
  }
  st = recvfrom(this->idSocket, buffer, size, 0, (struct sockaddr*)addr,
                &addrlen);
  if (st == -1) {
    throw std::runtime_error("VSocket::recvFrom");
  }
  return st;
}
