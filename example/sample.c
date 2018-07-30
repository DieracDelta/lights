#include <alienfx.h>
#include <stdio.h>
#include <alienfx_protocol.h>

int main(){

  /* decrement_colors(0, ALL_THE_THINGS, RED_FLAG, 50, 0, 0); */
  set_colors(0, ALL_THE_THINGS, RED_FLAG | GREEN_FLAG | BLUE_FLAG, 50, 0, 0);
}
