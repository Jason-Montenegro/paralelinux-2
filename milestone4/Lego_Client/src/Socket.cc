/**
 *  Universidad de Costa Rica
 *  ECCI
 *  CI0123 Proyecto integrador de redes y sistemas operativos
 *  2026-i
 *  Grupos: 2 y 3
 *
 *  ******   Socket class implementation
 *
 * (Fedora version)
 *
 **/

#include "Socket.h"  // Derived class

#include <arpa/inet.h>   // ntohs d
#include <stdio.h>       // printf
#include <sys/socket.h>  // sockaddr_in
#include <unistd.h>      // write, read

#include <cstring>
#include <stdexcept>

/**
 *  Class constructor
 *     use Unix socket system call
 *
 *  @param     char t: socket type to define
 *     's' for stream
 *     'd' for datagram
 *  @param     bool ipv6: if we need a IPv6 socket
 *
 **/
Socket::Socket(char t, bool IPv6) {
  this->Init(t, IPv6);  // Call base class constructor
}

Socket::Socket(int idSocket, bool IPv6): VSocket(idSocket,IPv6) {
    if (idSocket < 0) {
        throw std::runtime_error("Invalid socket id");
    }
 
}

/**
 *  Class destructor
 *
 *  @param     int id: socket descriptor
 *
 **/
Socket::~Socket() {}

/**
 * Connect method
 *   use "TryToConnect" in base class
 *
 * @param      char * host: host address in dot notation, example "10.1.166.62"
 * @param      int port: process address, example 80.
 *
 **/
int Socket::Connect(const char* hostip, int port) {
  return this->TryToConnect(hostip, port);
}

/**
 * Connect method
 *   use "TryToConnect" in base class
 *
 * @param      char * host: host address in dns notation, example
 * "os.ecci.ucr.ac.cr"
 * @param      char * service: process address, example "http"
 *
 **/
int Socket::Connect(const char* host, const char* service) {
  return this->TryToConnect(host, service);
}

/**
 * Read method
 *   use "read" Unix system call (man 3 read)
 *
 * @param      void * buffer: buffer to store data read from socket
 * @param      int size: buffer capacity, read will stop if buffer is full
 *
 **/
ssize_t Socket::Read(void* buffer, size_t size) {
  ssize_t st = read(this->idSocket, buffer, size);
  
  if (-1 == st) {
    if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
     throw std::runtime_error(std::string("Socket::Read(void,size_t): paso algo ") + strerror(errno));
    }
    
  }

  return st;
}

/**
 * Write method
 *   use "write" Unix system call (man 3 write)
 *
 * @param      void * buffer: buffer to store data write to socket
 * @param      size_t size: buffer capacity, number of bytes to write
 *
 **/
size_t Socket::Write(const void* buffer, size_t size) {
  // ocupo el sockfd del file descriptor
  // ssize_t write(int fd, const void buf[.count], size_t count);
  // no se que es ssize t pero wsl medio loquita con mi int sszite_t
  ssize_t st = write(this->idSocket, buffer, size);
  if (-1 == st) {
    throw std::runtime_error(std::string("Socket::Write( void *, size_t ) ") +
                             strerror(errno));
  }
  return static_cast<size_t>(st);
  ;
}

/**
 * Write method
 *   use "write" Unix system call (man 3 write)
 *
 * @param      char * text: text to write to socket
 *
 **/
size_t Socket::Write(const char* text) {
  ssize_t st = write(this->idSocket, text, strlen(text));

  if (-1 == st) {
    throw std::runtime_error(std::string("Socket::Write( char * ) ") +
                             strerror(errno));
  }

  return st;
}

/**
 * AcceptiConnection method
 *    use base class to accept connections
 *
 *  @returns   a new class instance
 *
 *  Waits for a new connection to service (TCP mode: stream)
 *
 **/
VSocket* Socket::AcceptConnection() {
    int new_id = -1;
  try {
   new_id = this->WaitForConnection();  // O WaitForConnection()
  } catch (const std::exception& e)  {
    return nullptr;
  }

  if (new_id < 0) return nullptr;// no deberia de pasar

  VSocket* peer = new Socket(new_id, this->IPv6);


  return peer;
}