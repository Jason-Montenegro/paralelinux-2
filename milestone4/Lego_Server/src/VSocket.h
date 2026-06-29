/**
 *  Universidad de Costa Rica
 *  ECCI
 *  CI0123 Proyecto integrador de redes y sistemas operativos
 *  2026-i
 *  Grupos: 2 y 3
 *
 ****** VSocket base class interface
 *
 * (Fedora version)
 *
 **/

#ifndef VSocket_h
#define VSocket_h
#include <arpa/inet.h>  // ntohs, htons
#include <netdb.h>      // getaddrinfo, freeaddrinfo
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>  // close
#include <string>
#include <cstddef>
#include <cstdio>
#include <cstring>    // memset
#include <stdexcept>  // runtime_error
#include <system_error>
#include <ifaddrs.h>
class VSocket {
 public:
  VSocket();
  VSocket(int, bool);
  void Init(char, bool = false);
  virtual ~VSocket();

  void Close();
  int TryToConnect(const char*, int);
  int TryToConnect(const char*, const char*);
  int TryToConnectIPv4(const char* hostip, int port);

  virtual int Connect(const char*, int) = 0;
  virtual int Connect(const char*, const char*) = 0;

  virtual ssize_t Read(void*, size_t) = 0;
  virtual size_t Write(const void*, size_t) = 0;
  virtual size_t Write(const char*) = 0;
  virtual VSocket* AcceptConnection() = 0;

  int Bind(int);
  int BindIPv4(int);
  int MarkPassive(int backlog);
  int WaitForConnection();
  int Shutdown(int mode);
  void SetTimeout(int seconds, int useconds = 0);
  // UDP methods
  size_t sendTo(const void*, size_t, void*);
  size_t recvFrom(void*, size_t, void*);

  void SocketD(int& puerto);
  void BroadCast(const char* ip,const void* buffer, size_t size, int puerto);
  void UpdateLocalIP();
  std::string getLocalIP() ;
  std::string getRemoteIPV4();
 protected:
  int idSocket;  // Socket identifier
  bool IPv6;     // Is IPv6 socket?
  int port;      // Socket associated port
  char type;     // Socket type (datagram, stream, etc.)
  std::string localIP;  // ip del socket
};

#endif  // VSocket_h
