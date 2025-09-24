#include "kernel/types.h"
#include "user/user.h"

int main(void){

  printf("up ");
  int ticks = uptime();
  printf("%d clock ticks\n", ticks);
  exit(0);
}
