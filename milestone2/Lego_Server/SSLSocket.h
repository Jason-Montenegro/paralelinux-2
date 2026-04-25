/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-i
  *  Grupos: 2 y 3
  *
  *   SSL Socket class interface
  *
  * (Fedora version)
  *
 **/

#ifndef SSLSocket_h
#define SSLSocket_h

#include "VSocket.h"


class SSLSocket : public VSocket {

   public:
      SSLSocket( bool IPv6 = false ,bool = false );				// Not possible to create with UDP, client constructor
      SSLSocket( const char *, const char *, bool = false ,bool = false);		// For server connections
      SSLSocket( int,bool=false);
      ~SSLSocket();
      int Connect( const char *, int );
      int Connect( const char *, const char * );
      size_t Write( const char * );
      size_t Write( const void *, size_t );
      ssize_t Read( void *, size_t );
      void ShowCerts();
      const char * GetCipher();
       //VSocket* AcceptConnection(){return nullptr;}
       void Copy(SSLSocket* original);
      void AcceptSSL();
   
    virtual VSocket* AcceptConnection() override;

   private:
      void InitSSL( bool = false );		// Defaults to create a client context, true if server context needed
      void InitContext( bool );
      void LoadCertificates( const char *, const char * );

// Instance variables      
      void * Context;				// SSL context
      void * BIO;				// SSL BIO (Basic Input/Output)

};

#endif

