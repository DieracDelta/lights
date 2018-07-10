#ifndef PROTOCOL_H
#include <stdbool.h>
#include <stdint.h>
#define PROTOCOL_H

#define GET 0
#define SET 1
#define INCREMENT 2
#define DECREMENT 3
#define SUSPEND 4

#define RED_FLAG 0
#define GREEN_FLAG 2
#define BLUE_FLAG 4
#define FREQ_FLAG 6

// TODO pack it boi
struct alienfx_msg{
  uint8_t OP;
  uint8_t arg1;
  uint8_t arg2;
  uint8_t arg3;
};

#endif
