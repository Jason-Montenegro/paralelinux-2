#include "synch.h"

class DiningPh {

public:
    DiningPh();
    ~DiningPh();
    void pickup( long who );
    void putdown( long who );
    void print();

private:
    void test( long who );
    enum { Thinking, Hungry, Eating } state[ 5 ];
    Lock * dp;			// Locks for each method in class
    Condition * self[ 5 ];

};

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