  #include "Taa.h"
  #include <unistd.h>




  Taa::Taa(Taa * father) { 
    usage = 1;
    archives.resize(father->size,-1);
    this->size = size; // invencion del papel
    int temp = 0;
    for(int i =0;i<size;++i){
      //temp = father->archives[i];
      temp = father->getFd(i);
      this->archives[i] = temp;
      if(temp!=-1){
      this->isOpen.Mark(i);// marca la posicion
      }
    }
  }
  // ~Taa::Taa (){// default esta bien
  //   if(!usage){

  //   }
  // }
  Taa::Taa(int size) : size(size){
      this->usage = 1;
      //isOpen = new BitMap(size);
      archives.resize(64,-1);// no estoy seguro si quemar el size de cada hilo o eso es variable
      isOpen.Mark(0);// solucion basura por salida estandar, pero tendria que empezar desde -3 
      isOpen.Mark(1);
        isOpen.Mark(2);
  }

  // void
  // BitMap::Mark(int which) 
  // { 
  //     ASSERT(which >= 0 && which < numBits);
  //     map[which / BitsInWord] |= 1 << (which % BitsInWord);
  // }

  int Taa::guardar(int fd){ // esto se hace para open 
    int pos =-1;
    if(fd>=0){
      pos = isOpen.Find();
      if(pos!=-1 ){
        archives[pos] = fd;
      // isOpen.Mark(pos);
      }
    } 
  return pos;// osea si aun hay espacio devuelve pos sino -1
  }

  int Taa::getFd(int pos){
    //printf("por algun motivo\n");
    if(pos<size && pos>=0){
    return archives[pos]; //esta predeterminado en -1
    } else{
      return -1;
    }
  }

  void Taa::close(int pos){
    if(pos<size && pos>=0){
      int archivo = archives[pos];
      if(archivo != -1){
        archives[pos] = -1;
        ::close(archivo);
        isOpen.Clear(pos);
      }
    }
  }

  int Taa::spaceAvailable(){
    return isOpen.NumClear();
  }


  void Taa::addThread(){//usage lo tengo atomic porque no se si el usuario se encarga de la proteccion o que pex
    ++this->usage;// atomic
    }
                
  void Taa::delThread(){
  //printf("ok entro a del\n");
    --this->usage;
    // printf("El valor de c es: %d\n", this->usage.load());
    if(!this->usage){// ya no esta ni el papa ni el hijo
    for (int i = 0; i < size; i++) {// no se si es
        this->close(i); 
      } // mas elegante un int externo para el delet
        delete this;// no se si delete es solo como que cierra o directamente libera el espacio de taa
    }
  }