#include <alienfx.h>
#include <stdio.h>
#include <alienfx_protocol.h>

int main(){
  printf("hi");
  set_colors(0, ALL_THE_THINGS, RED_FLAG | GREEN_FLAG | BLUE_FLAG, 0, 0, 0xff);
  printf("hi");
  /* set_freq(0, ALL_THE_THINGS, 500); */
}
