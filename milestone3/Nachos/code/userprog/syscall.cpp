

#include "syscalls.h"

#include <unistd.h>
#include <fcntl.h>

void Write(char *buffer, int size, OpenFileId id){ //write actualmente para archivos reales
  
    write(id,buffer,size);// es una llamada unix
} // creo que esto es solo llamada del unix , ya si es escritura dentro del bendito nachos es en exeption

/* Read "size" bytes from the open file into "buffer".  
 * Return the number of bytes actually read -- if the open file isn't
 * long enough, or if it is an I/O device, and there aren't enough 
 * characters to read, return whatever is available (for I/O devices, 
 * you should always wait until you can return at least one character).
 */
int Read(char *buffer, int size, OpenFileId id){ 
  ssize_t retorno  = -1;
  if(id>0 && (unsigned long long)id<SIZE_MAX ){
    retorno =  read(id, buffer,  size);
  }
  // no se si el nachos tiene try catch o que 
  return retorno;
}
// ssize_t read(int fd, void *buf, size_t count);

/* Close the file, we're done reading and writing to it. */
void Close(OpenFileId id){
  ///
  close(fd);
  ////
}