// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create several threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include <unistd.h>
#include "copyright.h"
#include "system.h"
#include "diningph.h"
// #include "H2O.hpp"
H2O* monitorH2O;
DiningPh * dp;

void Philo( void * p ) {

    int eats, thinks;
    long who = (long) p;

    currentThread->Yield();

    for ( int i = 0; i < 10; i++ ) {

        printf(" Philosopher %ld will try to pickup sticks\n", who + 1);

        dp->pickup( who );
        dp->print();
        eats = Random() % 6;

        currentThread->Yield();
        sleep( eats );

        dp->putdown( who );

        thinks = Random() % 6;
        currentThread->Yield();
        sleep( thinks );
    }

}


//----------------------------------------------------------------------
// SimpleThread
// 	Loop 10 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"name" points to a string with a thread name, just for
//      debugging purposes.
//----------------------------------------------------------------------

void
SimpleThread(void* name)
{
    // Reinterpret arg "name" as a string
    char* threadName = (char*)name;
    
    // If the lines dealing with interrupts are commented,
    // the code will behave incorrectly, because
    // printf execution may cause race conditions.
    for (int num = 0; num < 10; num++) {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
	printf("*** thread %s looped %d times\n", threadName, num);
	//interrupt->SetLevel(oldLevel);
        //currentThread->Yield();
    }
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> Thread %s has finished\n", threadName);
    //interrupt->SetLevel(oldLevel);
}



//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between several threads, by launching
//	ten threads which call SimpleThread, and finally calling 
//	SimpleThread ourselves.
//----------------------------------------------------------------------


void Hidrogeno(void* arg) {
    long id = (long)arg;
    printf("Hidrogeno %ld llegando\n", id);
    monitorH2O->puente_Hidrogeno(id);
}

void Oxigeno(void* arg) {
    long id = (long)arg;
    printf("Oxigenito %ld llegando\n", id);
    monitorH2O->Oxigeno(id);
}



void
ThreadTest()
{
    Thread * Ph;

    DEBUG('t', "Entering SimpleTest");

    
    // monitorH2O = new H2O();

    // Thread* t;

    // for (long k = 0; k < 2; k++) {
    //     t = new Thread("O");
    //     t->Fork(Oxigeno, (void*)k);
    // }
    // for (long k = 0; k < 4; k++) {
    //     t = new Thread("H");
    //     t->Fork(Hidrogeno, (void*)k);
    // }


   // return;

    dp = new DiningPh();

    for ( long k = 0; k < 5; k++ ) {
        Ph = new Thread( "dp" );
        Ph->Fork( Philo, (void *) k );
    }

    return;

    for ( int k=1; k<5; k++) {
      char* threadname = new char[100];
      sprintf(threadname, "Hilo %d", k);
      Thread* newThread = new Thread (threadname);
      newThread->Fork (SimpleThread, (void*)threadname);
    }
    
    SimpleThread( (void*)"Hilo 0");
}

