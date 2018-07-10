#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#define GET 0x0
#define SET 0x1
#define INCREMENT 0x2
#define DECREMENT 0x3
#define SUSPEND 0x4

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
