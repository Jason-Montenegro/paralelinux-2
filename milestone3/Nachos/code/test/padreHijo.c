#include "syscall.h"

void hijo(int);

int main() {
  Fork(hijo);

  Write("padre", 5, 1);
  Exit(0);
}

void hijo(int dummy) {
  Write(" hijo", 5, 1);
  Exit(0);
}
