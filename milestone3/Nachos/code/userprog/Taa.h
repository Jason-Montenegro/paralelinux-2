#ifndef TAA_H
#define TAA_H

#include "bitmap.h"
#include <vector>


class Taa {
  private:
    int size;
    BitMap isOpen{64};          
    std::vector<int> archives;   
    //int usage;	
    // antes de ir a exeption esta en el star.s pasa swtich al translate, sube a modo kernel y no usuario
    // y de ahi en exeption. el taa siempre lo va a a manejar solo un hilo
    int usage; 
  public:
    Taa(Taa * father) ;
    Taa(int size);
    ~Taa() = default;                      
    
    void close(int pos);
    int spaceAvailable();         
    int getFd(int pos);
    int guardar(int fd);
    void addThread();							// If a user thread is using this table, add it
    void delThread();							// If a user thread is using this table, delete it
    int getUsage(){return this->usage;} //sale mas fino que del thread retorne un int y dependiendo de eso 
                                        // llame el destructor de addrs space o el destructor del stack
};

#endif