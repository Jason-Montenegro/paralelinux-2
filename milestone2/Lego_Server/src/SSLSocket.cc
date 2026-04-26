/**
 *  Universidad de Costa Rica
 *  ECCI
 *  CI0123 Proyecto integrador de redes y sistemas operativos
 *  2026-i
 *  Grupos: 2 y 3
 *
 *  SSL Socket class implementation
 *
 * (Fedora version)
 *
 **/

// SSL includes

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <iostream>
#include <stdexcept>

// #include "Socket.h"
#include "SSLSocket.h"

/**
 *  Class constructor
 *     use base class
 *
 *  @param     char t: socket type to define
 *     's' for stream
 *     'd' for datagram
 *  @param     bool ipv6: if we need a IPv6 socket
 *
 **/

/**
 *  Class constructor
 *
 *  @param     int id: socket descriptor
 *
 **/
SSLSocket::SSLSocket(int id, bool IPv6) : VSocket(id, IPv6) {
  this->Context = nullptr;
  this->BIO = nullptr;
}

SSLSocket::SSLSocket(bool IPv6, bool contx) {
  this->Init('s', IPv6);

  this->Context = nullptr;
  this->BIO = nullptr;

  this->InitSSL(contx);  // Initializes to client context
}

/**
 *  Class constructor
 *     use base class
 *
 *  @param     char t: socket type to define
 *     's' for stream
 *     'd' for datagram
 *  @param     bool IPv6: if we need a IPv6 socket
 *
 **/
SSLSocket::SSLSocket(const char* certFileName, const char* keyFileName,
                     bool IPv6, bool contx)
    : SSLSocket(IPv6, contx) {
  this->LoadCertificates(certFileName, keyFileName);
}

/**
 * Class destructor
 *
 **/
SSLSocket::~SSLSocket() {
  // SSL destroy
  if (nullptr != this->Context) {
    SSL_CTX_free(reinterpret_cast<SSL_CTX*>(this->Context));
  }
  if (nullptr != this->BIO) {
    SSL_free(reinterpret_cast<SSL*>(this->BIO));
  }

  this->Close();
}

/**
 *  InitSSL
 *     use SSL_new with a defined context
 *
 *  Create a SSL object
 *
 **/
void SSLSocket::InitSSL(bool serverContext) {
  this->InitContext(serverContext);
  SSL* ssl = nullptr;
  ssl = SSL_new((SSL_CTX*)this->Context);
  if (ssl == nullptr) {
    throw std::runtime_error("SSLSocket::InitSSl( bool ) 1 ssl");
  }
  this->BIO = (void*)ssl;
}

/**
 *  InitContext
 *     use SSL_library_init, OpenSSL_add_all_algorithms, SSL_load_error_strings,
 * TLS_server_method, SSL_CTX_new
 *
 *  Creates a new SSL server context to start encrypted comunications, this
 * context is stored in class instance
 *
 **/
void SSLSocket::InitContext(bool serverContext) {
  const SSL_METHOD* method = TLS_client_method();
  ;

  if (serverContext) {
    method = TLS_server_method();
  } else {
    method = TLS_client_method();
  }
  if (nullptr == method) {
    throw std::runtime_error("SSLSocket::InitContext( bool ) 1 method");
  }
  SSL_CTX* context = SSL_CTX_new(method);

  if (context == nullptr) {
    throw std::runtime_error("SSLSocket::InitContext( bool ) 2 context");
  }
  this->Context = (void*)context;
}

/**
 *  Load certificates
 *    verify and load certificates
 *
 *  @param	const char * certFileName, file containing certificate
 *  @param	const char * keyFileName, file containing keys
 *
 **/
void SSLSocket::LoadCertificates(const char* certFileName,
                                 const char* keyFileName) {
  SSL_CTX* ctx = static_cast<SSL_CTX*>(this->Context);

  if (!ctx) {
    throw std::runtime_error("LoadCertificates: SSL_CTX is null");
  }

  if (SSL_CTX_use_certificate_file(ctx, certFileName, SSL_FILETYPE_PEM) <= 0) {
    unsigned long err = ERR_get_error();
    char buf[256];
    ERR_error_string_n(err, buf, sizeof(buf));
    std::cerr << "t:" << buf << std::endl;
    throw std::runtime_error("LoadCertificates: error loading certificate");
  }
  if (SSL_CTX_use_PrivateKey_file(ctx, keyFileName, SSL_FILETYPE_PEM) <= 0) {
    throw std::runtime_error("LoadCertificates: error loading private key");
  }

  if (!SSL_CTX_check_private_key(ctx)) {
    throw std::runtime_error(
        "LoadCertificates: private key does not match certificate");
  }
}

/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	int port, service number
 *
 **/
int SSLSocket::Connect(const char* hostName, int port) {
  int st;
  // Connect( host, port );
  //  Call SSL_set_fd
  //  Call SSL_connect()
  //  check for errors

  st = this->TryToConnect(hostName,
                          port);  // Establish a non ssl connection first
  SSL_set_fd((SSL*)this->BIO, this->idSocket);
  st = SSL_connect((SSL*)this->BIO);
  if (st <= 0) {
    throw std::runtime_error("SSLSocket::Connect( const char *, int ) 1 st");
  }
  return st;
}

/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	char * service, service name
 *
 **/
int SSLSocket::Connect(const char* host, const char* service) {
  int st;

  st = this->TryToConnect(host, service);
  SSL_set_fd((SSL*)this->BIO, this->idSocket);
  st = SSL_connect((SSL*)this->BIO);
  if (st <= 0) {
    throw std::runtime_error("SSLSocket::Connect( const char *, int ) 1 st");
  }

  return st;
}

/**
 *  Read
 *     use SSL_read to read data from an encrypted channel
 *
 *  @param	void * buffer to store data read
 *  @param	size_t size, buffer's capacity
 *
 *  @return	size_t byte quantity read
 *
 *  Reads data from secure channel
 *
 **/
ssize_t SSLSocket::Read(void* buffer, size_t size) {  // donde demonios saco
  ssize_t st = -1;
  st = SSL_read((SSL*)this->BIO, buffer, size);
  return st;
}

/**
 *  Write
 *     use SSL_write to write data to an encrypted channel
 *
 *  @param	void * buffer to store data read
 *  @param	size_t size, buffer's capacity
 *
 *  @return	size_t byte quantity written
 *
 *  Writes data to a secure channel
 *
 **/
size_t SSLSocket::Write(const char* string) {
  int st = -1;
  st = this->Write(string, strlen(string));
  // if ( -1 == st ) {
  //    throw std::runtime_error( "SSLSocket::Write( const char * ) 1 sttt" );
  // }

  return st;
}

/**
 *  Write
 *     use SSL_write to write data to an encrypted channel
 *
 *  @param	const void * buffer to store data to write
 *  @param	size_t size, buffer's capacity
 *
 *  @return	size_t byte quantity written
 *
 *  Reads data from secure channel
 *
 **/
size_t SSLSocket::Write(const void* buffer, size_t size) {
  int st = -1;
  st = SSL_write((SSL*)this->BIO, buffer, size);
  if (-1 == st) {
    throw std::runtime_error("SSLSocket::Write( void *, size_t )");
  }

  return st;
}

VSocket* SSLSocket::AcceptConnection() {
  int new_id = -1;
  try {
    new_id = this->WaitForConnection();  // O WaitForConnection()
  } catch (const std::exception& e) {
    return nullptr;
  }

  if (new_id < 0) return nullptr;  // no deberia de pasar
                                   // SOCK_STREAM
  SSLSocket* peer;
  try {
    peer = new SSLSocket(new_id, this->IPv6);

    // peer->type = this->type;
  } catch (const std::exception& e) {
    throw std::runtime_error("id invalida");
  }
  //linea agregada de lo que en teoria hace el copy
  peer->Context = this->Context;
  SSL_CTX_up_ref((SSL_CTX*)this->Context);

  SSL* ssl_obj = SSL_new((SSL_CTX*)this->Context);
  SSL_set_fd(ssl_obj, new_id);
  peer->BIO = (void*)ssl_obj;

  if (SSL_accept(ssl_obj) <= 0) {
    delete peer;
    return nullptr;
  }

  return peer;
}
/**
 *   Show SSL certificates
 *
 **/
void SSLSocket::ShowCerts() {
  X509* cert;
  char* line;

  cert = SSL_get_peer_certificate((SSL*)this->BIO);
  if (nullptr != cert) {
    printf("Server certificates:\n");
    line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    printf("Subject: %s\n", line);
    free(line);
    line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    printf("Issuer: %s\n", line);
    free(line);
    X509_free(cert);
  } else {
    printf("No certificates.\n");
  }
}

/**
 *   Return the name of the currently used cipher
 *
 **/
const char* SSLSocket::GetCipher() {
  return SSL_get_cipher(reinterpret_cast<SSL*>(this->BIO));
}

void SSLSocket::Copy(SSLSocket* original) {
  if (original) {
    auto* ctx = reinterpret_cast<SSL_CTX*>(original->Context);
    if (ctx){
    SSL_CTX_up_ref(ctx);
    this->Context = ctx;
    }
  }
}

/**
 *   SSLSocket::Accept
 *
 *  waits for a TLS/SSL client to initiate the TLS/SSL handshake
 *
 **/
void SSLSocket::AcceptSSL() {
  if (this->BIO) {
    int st = SSL_accept(static_cast<SSL*>(this->BIO));
    if (st <= 0) {
      throw std::runtime_error("SSLSocket::AcceptSSL: Error en el handshake");
    }
  }
}

/**
 *   Get SSL ciphers
 *
 **/
