// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.
//
// Copyright (c) -2025 Universidad de Costa Rica

#include <arpa/inet.h>  // ntohs, htons
#include <fcntl.h>
#include <netdb.h>  // getaddrinfo, freeaddrinfo
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>  // close

#include <cstddef>
#include <cstdio>
#include <cstring>  // memset
#include <iostream>
#include <map>  //se supone que mi exeption puede usar
#include <regex>
#include <sstream>
#include <stdexcept>  // runtime_error

#include "copyright.h"
#include "syscall.h"
#include "system.h"
#include "vector"
// #include <system_error>

#define SC_NachOS 015177  // maldito Exeption con mayuscula
#define SEEMS 32          // 8*
#define LOCKS 32          // 8*
#define CONDS 32          // 8*
#define PROCSS 32
// esta cosa debe ser como un map? o si es un array o uno dinamico>>
Semaphore* semaphores[SEEMS] = {nullptr};
BitMap bitSems(SEEMS);
Lock* locks[LOCKS] = {nullptr};
BitMap bitLocks(LOCKS);
Condition* conditions[CONDS] = {nullptr};
BitMap bitConds(CONDS);

Thread* procs[PROCSS];
std::map<long, Semaphore*> mapaJoin;
extern void NachOS_PC() {
  int prev = machine->ReadRegister(PCReg);
  int pc = machine->ReadRegister(NextPCReg);
  machine->WriteRegister(PrevPCReg, prev);
  machine->WriteRegister(PCReg, pc);
  machine->WriteRegister(NextPCReg, pc + 4);
}

/*
 *  System call interface: Halt()
 */
void NachOS_Halt() {  // System call 0

  DEBUG('a', "Shutdown, initiated by user program.\n");
  interrupt->Halt();
}

/*
 *  System call interface: void Exit( int )
 */
void NachOS_Exit() {  // System call 1
  DEBUG('u', "Exit system call\n");
  int state = currentThread->taa->getUsage();
  if (mapaJoin.find((int)(long)currentThread) != mapaJoin.end() &&
      mapaJoin[(int)(long)currentThread] != nullptr) {
    mapaJoin[(int)(long)currentThread]
        ->V();  // Le avisamos al padre que ya morimos
  }
  currentThread->taa->delThread();  //
  if (state == 1) {                 // logica satanica
    currentThread->taa = nullptr;
  }
  currentThread->Finish();
}

// mi hijo esta llamando esta funcion
void NachosForkThread(void* p)  // el bueno guarda el anterior
{
  // std::cout<<"este pibe va forky thread"<<std::endl;
  AddrSpace* space;
  space = currentThread->space;  // ocupa guardar en memoria lo que esta

  space->InitRegisters();  // set the initial register values
  if (p) {
    currentThread->RestoreUserState();
  }

  space->RestoreState();  // esto no tengo claro que porque hace lo de tlb o lo
                          // de page table
                          //  i believe in my hearth
  machine->WriteRegister(RetAddrReg,
                         4);  // esta cosa solo debe hacerlo para un hilo, no
                              // para un exec, a menos que jm, y si tira -4
  machine->WriteRegister(PCReg, (long)p);
  machine->WriteRegister(NextPCReg, (long)p + 4);


  machine->Run();  // jump to the user progam
  ASSERT(false);
}


/*
 *  System call interface: SpaceId Exec( char * )
 */
void NachOS_Exec() {
  currentThread->SaveUserState();
  int filesito = machine->ReadRegister(4);
  std::vector<char> buffer;
  int car = 1;
  int p = 1;
  int i = 0;
  int spaceid = -1;

  while (p && car) {
    p = machine->ReadMem(filesito + i++, 1, &car);
    buffer.push_back((char)car);
  }

  if (p) {  // mapaJoin
    OpenFile* file = fileSystem->Open(buffer.data());
    if (file) {
      Thread* newT = new Thread("process");
      newT->space = new AddrSpace(file);
      newT->space->InitRegisters();
      newT->taa = new Taa(64);
      int quattre = 0;
      spaceid = (long)newT;
      mapaJoin[spaceid] = new Semaphore("joinsito", 0);
      newT->Fork(NachosForkThread, (void*)(long)quattre);

      delete file;
    }
  }
  currentThread->RestoreUserState();
  machine->WriteRegister(2, spaceid);
}
/*
 *  System call interface: int Join( SpaceId )
 */
void NachOS_Join() {  // System call 3
  long id_hijo = (long)machine->ReadRegister(4);
  int resultado = -1;
  if (id_hijo != 0 && mapaJoin.find(id_hijo) != mapaJoin.end()) {
    Thread* hijo = (Thread*)id_hijo;  // cast miedo
    // hijo->semJoin->P();
    mapaJoin[id_hijo]->P();
    delete mapaJoin[id_hijo]; 
    mapaJoin.erase(id_hijo);
    resultado = 1;
  } else {
    DEBUG('u', "Join invalido \n");
  }
  machine->WriteRegister(2, resultado);
}

/*
 *  System call interface: void Create( char * )
 */
void NachOS_Create() {
  // std::cout<<"entra a create"<<std::endl;
  // printf("entra a NachOS_Create\n");
  int fileId = machine->ReadRegister(4);
  // char *buffer = new char[size];
  std::vector<char> buffer;
  int car = 1;
  int p = 1;
  int i = 0;
  while (p && car) {
    p = machine->ReadMem(fileId + i++, 1, &car);
    buffer.push_back((char)car);
  }
  int st = -1;
  if (p) {
    fileSystem->Create(buffer.data(), 0)
     
    }
  // machine->WriteRegister(2, st);
}

/*
 *  System call interface: OpenFileId Open( char * )
 */
void NachOS_Open() {  // System call 5
  // printf("entra a open \n");
  int fileId = machine->ReadRegister(4);
  // char *buffer = new char[size];
  std::vector<char> buffer;
  int car = 1;
  int p = 1;
  int i = 0;
  while (p && car) {
    p = machine->ReadMem(fileId + i++, 1, &car);
    buffer.push_back((char)car);
  }
  int st = -1;
  if (p) {
    int fd = ::open(buffer.data(), 0666);  // no se si
    // std::cout<<"el fd es " << fd <<" estoy en open "<<std::endl;
    if (fd != -1) {
      st = currentThread->taa->guardar(fd);
    }
    if (st == -1) {  // toco hacerle close
      ::close(fd);
    }
  }
  // std::cout<<"por algun motivo en open "<<st<<std::endl;
  machine->WriteRegister(2, st);  // st devuelve pos
}


/*
 *  System call interface: OpenFileId Write( char *, int, OpenFileId )
 */
void NachOS_Write() {  // System call 6
  // printf("el usa esta cosa?\n");
  int addr = machine->ReadRegister(4);
  int size = machine->ReadRegister(5);
  int file = machine->ReadRegister(6);
  int car;
  if (file == 1) {
    DEBUG('u', "Write system call\n");
    // printf("el size es %d\n", size);// ocupo que sea 8
    for (int i = 0; i < size; ++i) {
      if (!machine->ReadMem(addr + i, 1, &car)) {
        machine->WriteRegister(2, -1);
        return; 
      }
      printf("%c", car);
    }
    machine->WriteRegister(2, file);
  } else {
    // // ocupo quemar las posiciones 0 1 2
    if (file != 0) {
      if (file == ConsoleError) {
        printf("%d\n", machine->ReadRegister(4));
        machine->WriteRegister(2, 2);
      } else {
        int fd = currentThread->taa->getFd(file);
        if (-1 != fd) {
          char* buffer = new char[size + 1];
          buffer[size] = 0;

          for (int i = 0; i < size; ++i) {
            if (!machine->ReadMem(addr + i, 1, &car)) {
              break;
            }
            buffer[i] = car;
          }
          write(fd, buffer, size);
          delete[] buffer;
        }
        machine->WriteRegister(2, fd);
      }

    } else {
      machine->WriteRegister(2, -1);
      // error;
    }
  }
}

// AddrSpace::InitRegisters", "AddrSpace::RestoreState" y "AddrSpace::SaveState"
/*
 *  System call interface: OpenFileId Read( char *, int, OpenFileId )
 */
void NachOS_Read() {  // System call 7

  int addr =
      machine->ReadRegister(4);  // ubicacion donde quiero que escriban asumo
  int size = machine->ReadRegister(5);
  int file = machine->ReadRegister(6);
  std::cout << file << std::endl;
  int car;
  if (size > 0) {
    char* buffer = new char[size + 1];
    int bytesRead = 0;
    int basura = (int)'\n';
    int megab = (int)'\r';
    if (file == 0) {
      DEBUG('u', "Read from stdin\n");
      int i;
      for (i = 0; i < (size - 1) && (car = getchar()) != EOF && car != basura &&
                  car != megab;
           ++bytesRead) {
        buffer[bytesRead] = (char)car;
        ++i;
      }
      buffer[bytesRead] = 0;
    } else {
      int fd = -1;
      if (file > 2 && (fd = currentThread->taa->getFd(file)) != -1) {  // 0  1 2
        bytesRead = read(fd, buffer, size);
      } else {
        bytesRead = -1;
      }
    }
    for (int i = 0; i < bytesRead; ++i) {
      if (!machine->WriteMem(addr + i, 1, buffer[i])) {
        bytesRead = -1;
      }
    }
    delete[] buffer;
    machine->WriteRegister(2, bytesRead);
  }

  //  NachOS_PC();
}

/*
 *  System call interface: void Close( OpenFileId )
 */
static void NachOS_Close() {  // System call 8
  int fileID = machine->ReadRegister(4);
  currentThread->taa->close(fileID);  // no retorno nada aunque de error?? que
  machine->WriteRegister(2, fileID);
}

/*
 *  System call interface: void Fork( void (*func)() )
 */

void NachOS_Fork() {
  // NachOS_PC();
  DEBUG('u', "Entering Fork System call\n");
  currentThread->SaveUserState();  // so you save here

  Thread* newT = new Thread("child");
  newT->space = new AddrSpace(currentThread->space);
  newT->taa = currentThread->taa;
  newT->taa->addThread();
  DEBUG('u', "sigue todo normal chavales\n");
  currentThread->RestoreUserState();
  newT->SaveUserState();  // respuesta al pasado, si copia los registros y cpu
                          // del tata so later it will call restore state user
  newT->Fork(NachosForkThread, (void*)(long)machine->ReadRegister(4));

  // machine->WriteRegister(2, (long)newT);

  DEBUG('u', "Exiting Fork System call\n");
  // printf("despues de aca nada avanza bien, sera nachos poc?\n");
}
/*
 *  System call interface: void Yield()
 */

/* Yield the CPU to another runnable thread, whether in this address space
 * or not.
 */

void NachOS_Yield() {  // System call 10
                      
  DEBUG('u', "SWITCH (YIELD)\n");
  currentThread->Yield();
  DEBUG('u', " gFinish yield\n");
}

/*
 *  System call interface: Sem_t SemCreate( int )
 */
void NachOS_SemCreate() {  // System call 11
  int initialV = machine->ReadRegister(4);
  int pos = bitSems.Find();
  if (pos != -1) {
    semaphores[pos] = new Semaphore("antikernel sem", initialV);
  }
  machine->WriteRegister(2, pos);
}

/*
 *  System call interface: int SemDestroy( Sem_t )
 */
void NachOS_SemDestroy() {  // System call 12
  int semPos = machine->ReadRegister(4);
  bool exist = bitSems.Test(semPos);
  int st = -1;
  if (exist) {
    st = 1;
    bitSems.Clear(semPos);
    delete semaphores[semPos];
    semaphores[semPos] = nullptr;
  }
  machine->WriteRegister(2, st);
}

/*
 *  System call interface: int SemSignal( Sem_t )
 */
void NachOS_SemSignal() {  // System call 13
  int semPos = machine->ReadRegister(4);
  bool exist = bitSems.Test(semPos);
  int st = -1;
  if (exist) {
    semaphores[semPos]->V();  // se supone que esto es signal??
    st = 1;
  }
  machine->WriteRegister(2, st);
}

/*
 *  System call interface: int SemWait( Sem_t )
 */
void NachOS_SemWait() {  // System call 14
  int semPos = machine->ReadRegister(4);
  bool exist = bitSems.Test(semPos);
  int st = -1;

  if (exist) {
    semaphores[semPos]->P();  // P() es Wait
    st = 1;
  }
  machine->WriteRegister(2, st);
}

/*
 *  System call interface: Lock_t LockCreate( int )
 */
void NachOS_LockCreate() {                // System call 15
  int semPos = machine->ReadRegister(4);  // que hago con el int????
  int pos = bitLocks.Find();              // Busca un slot libre y lo marca
  if (pos != -1) {
    locks[pos] = new Lock("UserLock");
  }
  machine->WriteRegister(2, pos);  //
}

/*
 *  System call interface: int LockDestroy( Lock_t )
 */
void NachOS_LockDestroy() {  // System call 16

  int lockPos = machine->ReadRegister(4);
  bool exist = bitLocks.Test(lockPos);
  int st = -1;
  if (exist) {
    st = 1;
    bitLocks.Clear(lockPos);
    delete locks[lockPos];
    locks[lockPos] = nullptr;
  }
  machine->WriteRegister(2, st);
}

/*
 *  System call interface: int LockAcquire( Lock_t )
 */
void NachOS_LockAcquire() {  // System call 17
  int lockPos = machine->ReadRegister(4);
  bool exist = bitLocks.Test(lockPos);
  int st = -1;
  if (exist) {
    // Bloquea si otro hilo lo tiene
    locks[lockPos]->Acquire();
    st = 1;
  }
  machine->WriteRegister(2, st);
}

/*
 *  System call interface: int LockRelease( Lock_t )
 */
void NachOS_LockRelease() {  // System call 18
  int lockPos = machine->ReadRegister(4);
  bool exist = bitLocks.Test(lockPos);
  int st = -1;
  if (exist) {
    locks[lockPos]->Release();
    st = 1;
  }
  machine->WriteRegister(2, st);
}

/*
 *  System call interface: Cond_t LockCreate( int )
 */
void NachOS_CondCreate() {  // System call 19
  int pos = bitConds.Find();
  if (pos != -1) {
    conditions[pos] = new Condition("UserCond");
  }
  machine->WriteRegister(2, pos);
}

/*
 *  System call interface: int CondDestroy( Cond_t )
 */
void NachOS_CondDestroy() {  // System call 20
  int condPos = machine->ReadRegister(4);
  bool exist = bitConds.Test(condPos);
  int st = -1;
  if (exist) {
    st = 1;
    bitConds.Clear(condPos);
    delete conditions[condPos];
    conditions[condPos] = nullptr;
  }
  machine->WriteRegister(2, st);
}

/*
 *  System call interface: int CondSignal( Cond_t )
 */
void NachOS_CondSignal() {  // System call 21
  int condPos = machine->ReadRegister(4);
  int lockPos = machine->ReadRegister(5);

  bool condExist = bitConds.Test(condPos);
  bool lockExist = bitLocks.Test(lockPos);
  int st = -1;

  if (condExist && lockExist) {
    conditions[condPos]->Signal(locks[lockPos]);
    st = 1;
  }
  machine->WriteRegister(2, st);
}

/*
 *  System call interface: int CondWait( Cond_t )
 */
void NachOS_CondWait() {  // System call 22
  int condPos = machine->ReadRegister(4);
  int lockPos = machine->ReadRegister(5);

  bool condExist = bitConds.Test(condPos);
  bool lockExist = bitLocks.Test(lockPos);
  int st = -1;

  if (condExist && lockExist) {
    conditions[condPos]->Wait(locks[lockPos]);
    st = 1;
  }
  machine->WriteRegister(2, st);
}

/*
 *  System call interface: int CondBroadcast( Cond_t )
 */
void NachOS_CondBroadcast() {  // System call 23

  int condPos = machine->ReadRegister(4);
  int lockPos = machine->ReadRegister(5);
  bool condExist = bitConds.Test(condPos);
  bool lockExist = bitLocks.Test(lockPos);
  int st = -1;
  if (condExist && lockExist) {
    conditions[condPos]->Broadcast(locks[lockPos]);
    st = 1;
  }
  machine->WriteRegister(2, st);
}

/*
 *  System call interface: Socket_t Socket( int, int )
 */
//  id = Socket( AF_INET_NachOS, SOCK_STREAM_NachOS );
void NachOS_Socket() {  // System call 30
  // 4 y 5 , retorno en registro 2 , se mete en tabla de archivos
  // tabla nachil, return from syscall mover registros del pc register
  int type = machine->ReadRegister(4);
  int stream = machine->ReadRegister(5);  // vSOCK_STREAM
  int realStream = SOCK_DGRAM;

  if (stream == SOCK_STREAM_NachOS) {
    realStream = SOCK_STREAM;
  }
  // en syscall typedef int Socket_t;
  Socket_t idSocket = 0;
  if (type == AF_INET6_NachOS) {  // en syscall se define eseta bicha
    idSocket = socket(AF_INET6, realStream, 0);
  } else {
    idSocket = socket(AF_INET, realStream, 0);
  }
  int pos = currentThread->taa->guardar(idSocket);
  machine->WriteRegister(2, pos);
  // currentThread->taa->   machine->WriteRegister( 2, pos );// que el que la
  // llame maneje el -1
}

/*
 *  System call interface: Socket_t Connect( char *, int )
 */

// Connect( id, "163.178.104.62", 80 );
void NachOS_Connect() {  // System call 31
                         // DEBUG("entra a connect ");
  Socket_t faka = (Socket_t)machine->ReadRegister(
      4);  // sigue siendo int en esta instancia
           // printf("El ID del socket (faka) es: %d\n", faka);
  int socket = currentThread->taa->getFd(faka);  // esto deberia retornar int
  if (socket ==
      -1) {  // caso que se este intentando usar un file que realmente no existe
    machine->WriteRegister(2, socket);
    return;  // me da pereza orientar mejor el codigo
  }
  int logical_host = machine->ReadRegister(5);  // o ip
  int port = machine->ReadRegister(6);
  char hostip[256];
  int car = 1;
  int p = 1;
  for (int i = 0; i < 256 && p && car; ++i) {
    p = machine->ReadMem(logical_host + i, 1, &car);
    hostip[i] = (char)(car);
    // p = car;// tecnicamente se rompe si es 0
  }
  if (!p) {
    machine->WriteRegister(2, -1);
    return;
  }
  struct sockaddr_storage addr;
  socklen_t len = sizeof(addr);
  int st = -1;
  int domain = 0;
  socklen_t domain_len = sizeof(domain);
  if (getsockopt(socket, SOL_SOCKET, SO_DOMAIN, &domain, &domain_len) < 0) {
    machine->WriteRegister(2, -1);
    return;
  }

  if (domain == AF_INET) {
    struct sockaddr_in host4;
    memset((char*)&host4, 0, sizeof(host4));
    host4.sin_family = AF_INET;
    host4.sin_port = htons(port);
    st = inet_pton(AF_INET, hostip, &host4.sin_addr);
    if (st > 0) {
      st = connect(socket, (sockaddr*)&host4, sizeof(host4));
    } else {
      st = -1;
    }
  } else {  // AF_INET6
    struct sockaddr_in6 host6;
    struct sockaddr* ha;
    memset(&host6, 0, sizeof(host6));
    host6.sin6_family = AF_INET6;
    host6.sin6_port = htons(port);
    // const char *puntero =  reinterpret_cast<const char*>(hostip);
    st = inet_pton(AF_INET6, hostip, &host6.sin6_addr);
    if (0 < st) {  // 0 means invalid address, -1 means address error

      ha = (struct sockaddr*)&host6;
      auto len = sizeof(host6);
      st = connect(socket, ha, len);
    } else {
      st = -1;
    }
  }
  //}
  machine->WriteRegister(2, st);
}

/*
 *  System call interface: int Bind( Socket_t, int )
 */
void NachOS_Bind() {  // System call 32
  Socket_t faka = (Socket_t)machine->ReadRegister(4);
  int socket = currentThread->taa->getFd(faka);
  if (socket == -1) {
    machine->WriteRegister(2, socket);
    return;
  }
  int port = machine->ReadRegister(5);  // o ip
  struct sockaddr_storage addr;
  socklen_t len = sizeof(addr);
  int st = -1;
  if (getsockname(socket, (struct sockaddr*)&addr, &len) >= 0) {
    if (addr.ss_family != AF_INET) {
      struct sockaddr_in6 host6;
      memset(&host6, '\0', sizeof(host6));
      host6.sin6_family = AF_INET6;
      host6.sin6_addr = in6addr_any;
      host6.sin6_port = htons(port);
      st = bind(socket, (struct sockaddr*)&host6, sizeof(host6));

    } else {
      struct sockaddr_in host4;
      host4.sin_family = AF_INET;
      host4.sin_addr.s_addr = htonl(INADDR_ANY);
      host4.sin_port = htons(port);
      memset(host4.sin_zero, '\0', sizeof(host4.sin_zero));
      st = bind(socket, (struct sockaddr*)&host4, sizeof(host4));
    }
  }
  machine->WriteRegister(2, st);
}

/*
 *  System call interface: int Listen( Socket_t, int )
 */
void NachOS_Listen() {  // System call 33
  int faka = machine->ReadRegister(4);
  Socket_t socket = (Socket_t)currentThread->taa->getFd(faka);
  if (socket == -1) {
    machine->WriteRegister(2, socket);
    return;
  }
  int backlog = machine->ReadRegister(5);  // o ip
  int st = -1;
  st = listen(socket, backlog);
  machine->WriteRegister(2, st);
}

/*
 *  System call interface: int Accept( Socket_t )
 */
// el wait for conection de clases pasadas
void NachOS_Accept() {                         // System call 34
  if (currentThread->taa->spaceAvailable()) {  // solo ocupa 1
    int faka = machine->ReadRegister(4);
    Socket_t socket = (Socket_t)currentThread->taa->getFd(faka);
    if (socket == -1) {
      machine->WriteRegister(2, socket);
      return;
    }
    struct sockaddr_storage addr;  // contenedor generico, si no a puro new
    socklen_t addrlen = sizeof(addr);
    int st = -1;
    st = accept(socket, (struct sockaddr*)&addr, &addrlen);
    int pos = currentThread->taa->guardar(st);
    machine->WriteRegister(2, pos);
    if (pos == -1 && st > 0) {  // existe el socket pero no hay espacio
      ::close(st);
    }
  }
}

/*
 *  System call interface: int Shutdown( Socket_t, int )
 */
void NachOS_Shutdown() {                // System call 25 realmente es el 35
  int file = machine->ReadRegister(4);  //
  Socket_t socket = (Socket_t)currentThread->taa->getFd(file);
  int st = -1;
  if (socket != -1) {
    int mode = machine->ReadRegister(5);  // o ip
    st = shutdown(
        socket,
        mode);  // ocupo manejar el currentThread->taa->si da error el file???
  }
  machine->WriteRegister(2, st);
}

// System call interface: void ParseLego( Socket_t, int )

void NachOS_ParseLego() {               // System call 36
  int file = machine->ReadRegister(4);  //
  int chiky = machine->ReadRegister(5);
  Socket_t socket = (Socket_t)currentThread->taa->getFd(file);
  int st = -1;
  std::stringstream sstream;
  char buffer;

  if (socket != -1) {  // nos estamos montando en
                       // char bufferin[256];
    int bytesRead = 1;
    while (bytesRead > 0) {
      bytesRead = read(socket, &buffer, 1);
      if (bytesRead > 0) {
        // Write(bufferin, bytesRead, 1);
        sstream << buffer;
      }
    }
    std::string response = sstream.str();
    std::map<std::string, int> lego_figuras;
    int cantidadGlobal = 0;
    // std::regex row_regex(
    // R"ROW(<TR><TD ALIGN=center>\s*(\d+)\s*</TD>[\s\S]*?SRC="([^"]+)")ROW");
    std::regex row_regex;
    if (chiky) {
      row_regex.assign(
          R"ROW(<TR><TD ALIGN=center>\s*(\d+)\s*</TD>\s*<TD ALIGN=center>\s*([\w\s\-]+?)\s*</TD>\s*<TD ALIGN=center>\s*<IMG SRC="([^"]+)")ROW");
    } else {
      row_regex.assign(
          R"ROW(<li><strong>(\d+)x</strong>\s*([\w\s\-]+?)\s*</li>)ROW");
    }
    std::smatch match;
    auto it = response.cbegin();
    while (std::regex_search(it, response.cend(), match, row_regex)) {
      int cantidad = std::stoi(match[1].str());
      std::string nombre_pieza = match[2].str();
      std::string ruta_imagen = "";
      if (chiky) {
        ruta_imagen = match[3].str();
      }
      cantidadGlobal += cantidad;
      std::string ruta = match[2];
      lego_figuras[nombre_pieza] += cantidad;
      it = match[0].second;

      //
    }
    printf("Cantidad total de piezas:%d\n", cantidadGlobal);

    for (auto& it : lego_figuras) {
      const char* str = it.first.data();  // string
      printf("%s", str);
      printf(": %d\n", it.second);
    }
  }
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

void ExceptionHandler(ExceptionType which) {
  int type = machine->ReadRegister(2) - SC_Base;
  // printf("syscall = %d\n", type);
  switch (which) {
    case SyscallException:
      switch (type) {
        case SC_Halt:  // System call # 0
          NachOS_Halt();
          break;
        case SC_Exit:  // System call # 1
          NachOS_Exit();
          return;
        case SC_Exec:  // System call # 2
          NachOS_Exec();
          break;
        case SC_Join:  // System call # 3
          NachOS_Join();
          break;
        case SC_Create:  // System call # 4
          NachOS_Create();
          break;
        case SC_Open:  // System call # 5
          NachOS_Open();
          break;
        case SC_Read:  // System call # 6
          NachOS_Read();
          break;
        case SC_Write:  // System call # 7
          NachOS_Write();
          break;
        case SC_Close:  // System call # 8
          NachOS_Close();
          break;

        case SC_Fork:  // System call # 9
          NachOS_Fork();
          break;
        case SC_Yield:  // System call # 10
          NachOS_Yield();
          break;

        case SC_SemCreate:  // System call # 11
          NachOS_SemCreate();
          break;
        case SC_SemDestroy:  // System call # 12
          NachOS_SemDestroy();
          break;
        case SC_SemSignal:  // System call # 13
          NachOS_SemSignal();
          break;
        case SC_SemWait:  // System call # 14
          NachOS_SemWait();
          break;

        case SC_LckCreate:  // System call # 15
          NachOS_LockCreate();
          break;
        case SC_LckDestroy:  // System call # 16
          NachOS_LockDestroy();
          break;
        case SC_LckAcquire:  // System call # 17
          NachOS_LockAcquire();
          break;
        case SC_LckRelease:  // System call # 18
          NachOS_LockRelease();
          break;

        case SC_CondCreate:  // System call # 19
          NachOS_CondCreate();
          break;
        case SC_CondDestroy:  // System call # 20
          NachOS_CondDestroy();
          break;
        case SC_CondSignal:  // System call # 21
          NachOS_CondSignal();
          break;
        case SC_CondWait:  // System call # 22
          NachOS_CondWait();
          break;
        case SC_CondBroadcast:  // System call # 23
          NachOS_CondBroadcast();
          break;

        case SC_Socket:  // System call # 30
          NachOS_Socket();
          break;
        case SC_Connect:  // System call # 31
          NachOS_Connect();
          break;
        case SC_Bind:  // System call # 32
          NachOS_Bind();
          break;
        case SC_Listen:  // System call # 33
          NachOS_Listen();
          break;
        case SC_Accept:  // System call # 32
          NachOS_Accept();
          break;
        case SC_Shutdown:  // System call # 33
          NachOS_Shutdown();
          break;
        case SC_ParseLego:  // System call # 34 que me invente yo
          NachOS_ParseLego();
          break;

        default:
          printf("NachOS version: %d-%d\n", (SC_Base + SC_NachOS) / 10,
                 (SC_Base + SC_NachOS) % 10);
          printf("Unexpected syscall exception %d\n", type);
          ASSERT(false);
          break;
      }
      NachOS_PC();  // ver si es general
      break;

    case PageFaultException: {
      break;
    }

    case ReadOnlyException:
      printf("Read Only exception (%d)\n", which);
      ASSERT(false);
      break;

    case BusErrorException:
      printf("Bus error exception (%d)\n", which);
      ASSERT(false);
      break;

    case AddressErrorException:
      printf("Address error exception (%d)\n", which);
      ASSERT(false);
      break;

    case OverflowException:
      printf("Overflow exception (%d)\n", which);
      ASSERT(false);
      break;

    case IllegalInstrException:
      printf("Ilegal instruction exception (%d)\n", which);
      ASSERT(false);
      break;

    default:
      printf("Unexpected exception %d\n", which);
      ASSERT(false);
      break;
  }
}
