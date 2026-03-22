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

#include <stdexcept>

#include "Socket.h"
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
SSLSocket::SSLSocket(bool IPv6) {
  this->Init('s', IPv6);

  this->Context = nullptr;
  this->BIO = nullptr;

  this->InitSSL();  // Initializes to client context
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
SSLSocket::SSLSocket(char* certFileName, char* keyFileName, bool IPv6) {
  this->Init('s', IPv6);
  const SSL_METHOD* method = TLS_server_method();

  SSL_CTX* ctx = SSL_CTX_new(method);
  if (!ctx) {
    throw std::runtime_error(
        "SSLSocket::SSLSocket( char *, char *, bool ) 1 ctx");
  }
  this->Context = (void*)ctx;
  // dios demasiado texto este server
  // if (SSL_CTX_use_PrivateKey_file(this->sslContext, keyFileName,
  // SSL_FILETYPE_PEM) <= 0) {
  //      // Handle error: could not load key
  //  }
}

/**
 *  Class constructor
 *
 *  @param     int id: socket descriptor
 *
 **/
SSLSocket::SSLSocket(int id) { this->Init(id); }

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
                                 const char* keyFileName) {}

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
size_t SSLSocket::Read(void* buffer, size_t size) {  // donde demonios saco
  int st = -1;
  st = SSL_read((SSL*)this->BIO, buffer, size);
  if (-1 == st) {
    throw std::runtime_error("SSLSocket::Read( void *, size_t ) st");
  }

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
