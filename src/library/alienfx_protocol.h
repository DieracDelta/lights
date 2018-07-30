#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#define NUM_HANDLERS 5

// OPS
#define GET 0x0
#define SET_COLOR 0x1
#define INCREMENT_COLOR 0x2
#define DECREMENT_COLOR 0x3
#define PAUSE 0x4
#define SET_FREQ 0x5
#define INCREMENT_FREQ 0x6
#define DECREMENT_FREQ 0x7
#define SET_PROFILE 0x8
#define INCREMENT_PROFILE 0x9
#define DECREMENT_PROFILE 0xa


// REGION FLAGS
// for a fact work in alienware 15 R3;
// need to change for other models
// note that all values <= 2 bytes
// NOTE trackpad must be enabled in bios in order for this to be effective
// 0x10, 0x200 and 0x8000 are the only ones missing
#define NUM_REGIONS 13
#define KB_FAR_RIGHT 0x1
#define KB_MID_RIGHT 0x2
#define KB_MID_LEFT 0x4
#define KB_FAR_LEFT 0x8
#define ALIEN_HEAD 0x20
#define ALIENWARE_NAME 0x40
#define TRACKPAD 0x80
#define POWER_BUTTON 0x100
#define BOT_LEFT_BURNER 0x400
#define BOT_RIGHT_BURNER 0x800
#define TOP_LEFT_BURNER 0x1000
#define TOP_RIGHT_BURNER 0x2000
#define KB_SPECIAL 0x4000
#define ALL_THE_THINGS 0xffff
#define NUM_UNDEFINED 2
#define NOT_DEFINED_1 0x10
#define NOT_DEFINED_1_POS 4
#define NOT_DEFINED_2 0x200
#define NOT_DEFINED_2_POS 9

// ACTION FLAGS
#define RED_FLAG 0x0
#define GREEN_FLAG 0x2
#define BLUE_FLAG 0x4
#define FREQ_FLAG 0x8
#define PROF_FLAG 0x10

// TODO pack it boi
struct alienfx_msg{
  uint8_t OP;
  uint8_t profile_index;
  uint8_t args[6];
};

struct alienfx_response{
  uint64_t r_val;
  bool error;
};

struct region_state{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint16_t freq;
  uint8_t type;
  bool paused;
};

struct alienfx_profile{
  struct region_state *regions;
};

#endif
