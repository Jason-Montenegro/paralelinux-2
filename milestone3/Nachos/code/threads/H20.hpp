#ifndef H2O_H
#define H2O_H

#include "synch.h"
#include "system.h"
class H2O {
  public:
    H2O();         
    ~H2O();      
     
    void puente_Hidrogeno(long id);  
    void Oxigeno(long id);  

  private:
    Lock *mutex;        
    Semaphore *hSem;     
    Semaphore *oSem;    
    int hWaiting;       
    int oWaiting;      
};

#endif