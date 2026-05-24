#include "diningph.h"
// #include "H20.hpp"
DiningPh::DiningPh() {

    dp = new Lock( "dp" );

    for ( int i = 0; i < 5; i++ ) {
        self[ i ] = new Condition( "philo" );
        state[ i ] = Thinking;
    }

}


DiningPh::~DiningPh() {
}


void DiningPh::pickup( long who ) {

    dp->Acquire();

    state[ who ] = Hungry;
    test( who );
    if ( state[ who ] == Hungry )
        self[ who ]->Wait( dp );

    dp->Release();

}


void DiningPh::putdown( long who ) {

    dp->Acquire();

    state[ who ] = Thinking;
    test( (who + 4) % 5 );
    test( (who + 1) % 5 );

    dp->Release();

}


void DiningPh::test( long i ) {

    if ( ( state[ (i + 4) % 5 ] != Eating ) && 
         ( state[ i ] == Hungry ) && 
         ( state[ (i + 1) % 5] != Eating ) ) {
        state[ i ] = Eating;
        self[ i ]->Signal( dp );

    }

}

void DiningPh::print() {

    for ( int i = 0; i < 5; i++ ) {
        printf( "Philosopher %d is %s \n", i + 1, (state[i]==Hungry)?"Hungry":(state[i]==Thinking)?"Thinking":"Eating");

    }

}




H2O::H2O() {
    mutex = new Lock("H2O Mutex");
  
    oSem = new Semaphore("OSem", 0);
    hSem = new Semaphore("HSem", 0);
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
    hWaiting++;

    if (hWaiting >= 2 && oWaiting >= 1) {
        hWaiting -= 2; 
        --oWaiting ; 
        
        hSem->V(); 
        oSem->V(); 
        mutex->Release();
       
    } else {
        mutex->Release();
        hSem->P();
    }

      printf("Hidrogeno %ld hace enlace agua\n", id);
  
}

void H2O::Oxigeno(long id) {
    mutex->Acquire();
    oWaiting++;
   
    if (hWaiting >= 2) {
        hWaiting -= 2;
        --oWaiting;
        hSem->V(); 
        hSem->V();
        mutex->Release();
    } else {
        mutex->Release();
        oSem->P();
    }

     printf("Oxigeno %ld se une con moleculas h, se forma agua\n", id);
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