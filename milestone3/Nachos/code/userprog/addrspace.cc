// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include <iostream>

using namespace std;
//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

// AddrSpace:AddrSpace(AddrSpace *parent)
// {

// }

AddrSpace::AddrSpace(AddrSpace *parent)
{
    int stackPages = divRoundUp(UserStackSize, PageSize);
    int sharedPages = parent->numPages - stackPages;

    // nuevas páginas para stack
    numPages =parent->numPages;

    pageTable = new TranslationEntry[numPages];

    for (int i = 0; i < sharedPages; ++i) {
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = parent->pageTable[i].physicalPage; 
        pageTable[i].valid = parent->pageTable[i].valid;
        pageTable[i].use = parent->pageTable[i].use;
        pageTable[i].dirty = parent->pageTable[i].dirty;
        pageTable[i].readOnly = parent->pageTable[i].readOnly;
    }

    // NUEVO stack para el hijo
        //cout<<"lo que quiere mi hijo "<< numPages-sharedPages<<endl;
    //cout<<machine->freeFrames->NumClear()<<endl;// un print miedo
    for (int i = sharedPages; i < numPages; i++) {/// desde 
        pageTable[i].virtualPage = i;
        int frame = machine->freeFrames->Find();
        ASSERT(frame != -1);
        pageTable[i].physicalPage = frame;

        pageTable[i].valid = true;
        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;

        int physAddr = frame * PageSize;

        bzero(&(machine->mainMemory[physAddr]), PageSize);
    }
   // printf("salio del ciclo\n");
}

static Lock * memoryLock;
AddrSpace::AddrSpace(OpenFile *executable)
{
   //   memoryLock = new Lock("memory lock");
    NoffHeader noffH;
    unsigned int i, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);
    printf("[ADDRSPACE] Magic in file: 0x%x\n", noffH.noffMagic);
printf("[ADDRSPACE] NOFFMAGIC constant: 0x%x\n", NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;// page size por el page size

    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 
    this->pageTable = new TranslationEntry[numPages];
    //cout<<"lo que quiero "<< numPages<<endl;
     //cout<<machine->freeFrames->NumClear()<<endl;// un print miedo
    for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	//pageTable[i].physicalPage = i;
    
    //memoryLock->Acquire();
    pageTable[i].physicalPage = machine->freeFrames->Find();
    ASSERT(pageTable[i].physicalPage  != -1);
    //    memoryLock->Release();
	pageTable[i].valid = true;
	pageTable[i].use = false;
	pageTable[i].dirty = false;
	pageTable[i].readOnly = false;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    }
    
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
//bzero(&(machine->mainMemory[pageTable[i].physicalPage * PageSize]), PageSize);
   // bzero(machine->mainMemory, size);
    // Limpiar ÚNICAMENTE el pedazo de memoria física asignado a este proceso
   // bzero(&(machine->mainMemory[pageTable[0].physicalPage * PageSize]), size);
    for ( int i = 0; i < numPages; i++) {
        int physicalAddr = pageTable[i].physicalPage * PageSize;
        bzero(&(machine->mainMemory[physicalAddr]), PageSize);// fd
    }
// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
        contiguos(executable, noffH.code);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
       contiguos(executable, noffH.initData);
    }

}


// typedef struct noffHeader {
//    int noffMagic;		/* should be NOFFMAGIC */
//    Segment code;		/* executable code segment */ 
//    Segment initData;		/* initialized data segment */
//    Segment uninitData;		/* uninitialized data segment --
// 				 * should be zero'ed before use 
// 				 */
// } NoffHeader;
//
void AddrSpace::contiguos(OpenFile *executable, Segment segment){
     char *buffer = new char[segment.size];
    executable->ReadAt(buffer,
                       segment.size,
                       segment.inFileAddr);
    for (int i = 0; i < segment.size; i++) {
        int virtualAddr = segment.virtualAddr + i;
        int vpn = virtualAddr / PageSize;
        int offset = virtualAddr % PageSize;
        int physicalPage = pageTable[vpn].physicalPage;
        int physicalAddr = physicalPage * PageSize + offset;
        //BYTES  FOR PAGE , logic not there
        machine->mainMemory[physicalAddr] = buffer[i];// donde va cada inicio de direccion
    }// dd

    delete[] buffer;
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    if (currentThread->taa != nullptr) {
       //
       //  printf("el primer hilo entra aca\n");
        int stackPages = divRoundUp(UserStackSize, PageSize);
        int fatherPages = numPages - stackPages; 
        for (int i = fatherPages; i < numPages; ++i) {
            if (pageTable[i].valid) {
                machine->freeFrames->Clear(pageTable[i].physicalPage);
            }
        }
    } else {  // padre o chabmeador  
        cout<<"destrcutor entra"<<endl;
        for ( int i = 0; i < numPages; ++i) {
            if (pageTable[i].valid) {//     
                machine->freeFrames->Clear(pageTable[i].physicalPage);
            }
    }
    }
    delete [] this->pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{ //NumPhysPages
    int i;
    //numPages
    //    for ( int i = 0; i < numPages; ++i) {
    //      machine->freeFrames->Clear(i);// bitmap libresito
    
    // }
    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16); // el bendito coso de inicio que realmente esta al final
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------
// especifico a a cada addrs espace, guarda la posicion entonces

// void AddrSpace::SaveState() //aaaa Sve del thread hace esto oko oko
// {
//   //  machine->pageTable = ;
//     //this->pageTable = machine->pageTable;
// }

void AddrSpace::SaveState()
{
#ifndef USE_TLB

#else
     cout<<"estamos usando SaveState"<<endl;
    int vpn = 0;
    for (int i = 0; i < TLBSize; ++i) {
        if (machine->tlb[i].valid == true) {
            vpn = machine->tlb[i].virtualPage;
            
           // paginas virtuales REALES
            ASSERT(vpn >= 0 && vpn < numPages);

            // algo como que copia del tlb y pasa al lado virtuallll
            pageTable[vpn].dirty = machine->tlb[i].dirty;
            pageTable[vpn].use   = machine->tlb[i].use;
        }
    }
#endif
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    if(machine->tlb == NULL){
        machine->pageTable = pageTable;// el pagetable  que este manejando
        machine->pageTableSize = numPages;
    } else{
        cout<<"estamos usando tlb"<<endl;
        // ./machine/machine.h:const int TLBSize = 4;
        for (int i = 0; i < TLBSize; i++) {// esta cosa solo tiene 8 entradas
            machine->tlb[i].valid = false;
        } 
    }
}
    
// void
// Thread::SaveUserState()
// {
//     for (int i = 0; i < NumTotalRegs; ++i )
// 	userRegisters[i] = machine->ReadRegister(i);
// }



// void

// Thread::RestoreUserState()
// {
//     for (int i = 0; i < NumTotalRegs; ++i)
// 	machine->WriteRegister(i, userRegisters[i]);
// }