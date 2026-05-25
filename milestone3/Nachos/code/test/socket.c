#include "syscall.h"


char * strcat(char *s, const char *append) {
   char *save = s;
   for (; *s; ++s);
   while ((*s++ = *append++) != '\0');
   return(save);
}

int strlen( char *str) {
   const char *s;
   for (s = str; *s; ++s);
   return (s - str);
}


char * strcpy(char *to, const char *from) {
    char *save = to;
    while ((*to++ = *from++) != '\0');
    return save;
}

int main() {
	int id;
  char s[100]; // Un buffer temporal razonable para la entrada de la consola
  // 1. Strings base
  // el chiky coso:  GET /lego/list.php?figure=fish&part=1
  char *endpoint = "GET &lego/figure=duck&part=1";
  char *protocol = " HTTP/1.0\r\nUser-Agent: nachos\r\n\r\n\r\n";
   int req_size = strlen(endpoint) + strlen(s) + strlen(protocol) + 1;


  char dynamic_req[req_size];
   //char * dynamic_req = new char[req_size];
 //  int* dynamic_req = (int*) calloc(10, sizeof(int));
   int i =0;
   for (i = 0; i < req_size; ++i) { dynamic_req[i] = 0; }

   strcpy(dynamic_req, endpoint);
   strcat(dynamic_req, s);
   strcat(dynamic_req, protocol);

   Write("\nRequest construido: ", 21, 1);
   Write(dynamic_req, strlen(dynamic_req), 1);
   Write("\n", 1, 1);
   id = Socket(AF_INET_NachOS, SOCK_STREAM_NachOS); 
   int puerto = 8501;
   int chikyPuerto  = 80;
   //163.178.104.62
   if (Connect(id, "127.0.0.1", puerto) < 0) {
       Write("Error: No se pudo conectar al host\n", 35, 1);
		  //
   } else {
    Write(dynamic_req, strlen(dynamic_req), id);   
    ParseLego(id,0);
    Close(id);
    Exit(0);
    }
    Exit(-1);
}