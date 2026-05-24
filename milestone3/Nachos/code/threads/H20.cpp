#include "H2O.hpp"


H2O::H2O() {
    mutex = new Lock("H2O Mutex");
  
    oSem = new Condition("OSem");
    hSem = new Condition("HSem");

    hWaiting = 0;
    oWaiting = 0;
}

H2O::~H2O() {
    delete mutex;
    delete hSem;
    delete oSem;
}

void H2O::puente_Hidrogeno(  long id) {
    mutex->Acquire();
   

    if (hWaiting >= 1 && oWaiting >= 1) {
       --hWaiting ; 
        --oWaiting; 
        
        hSem->Signal(mutex); 
        oSem->Signal(mutex); 
      
       
    } else {
         ++hWaiting;
          printf("Hidrogeno %ld espera\n", id);
         hSem->Wait(mutex);
          printf("Hidrogeno %ld es liberado\n", id);
      //  hSem->P();
    }  
  

    printf("Hidrogeno %ld hace enlace agua\n", id);
    mutex->Release();
  
}

void H2O::Oxigeno(long id) {
    mutex->Acquire();
   
    if (hWaiting >= 2) {
        hWaiting -= 2;
        --oWaiting;
        hSem->Signal(mutex); 
        hSem->Signal(mutex);
        // hSem->V(); 
        // hSem->V();
       
    } else {
         ++oWaiting; 
        // mutex->Release();
      //  oSem->P();
      osem->Wait(mutex);
    }

     printf("Oxigeno %ld se une con moleculas h, se forma agua\n", id);
      mutex->Release();
}




// void H2O::ThreadTest() {
//     Thread *t;
//     monitorH2O = new H2O();

//     DEBUG('t', "Iniciando prueba de H2O");

   
//     for (long k = 0; k < 4; k++) {
//         t = new Thread("hidrogeno");
//         t->Fork(Hidrogeno, (void *) k);
//     }


//     for (long k = 0; k < 2; k++) {
//         t = new Thread("oxigeno");
//         t->Fork(Oxigeno, (void *) k);
//     } 
// }