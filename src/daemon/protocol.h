#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#define NUM_HANDLERS 5

#define GET 0x0
#define SET_COLOR 0x1
#define INCREMENT_COLOR 0x2
#define DECREMENT_COLOR 0x3
#define PAUSE 0x4


#define RED_FLAG 0x0
#define GREEN_FLAG 0x2
#define BLUE_FLAG 0x4
#define FREQ_FLAG 0x8

// TODO pack it boi
struct alienfx_msg{
  uint8_t OP;
  uint8_t args[6];
};

struct alienfx_response{
  uint64_t r_val;
  bool error;
};

#endif
