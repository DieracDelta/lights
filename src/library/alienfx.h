#ifndef ALIENFX_H_
#define ALIENFX_H_

#include "alienfx_protocol.h"


// TODO reimplement to read from files
/* uint8_t get_info(uint8_t profile_index, uint16_t region_flag, uint8_t get_flag); */

void set_colors(uint8_t profile_index, uint16_t region, uint8_t rgb_flags, uint8_t red, uint8_t blue, uint8_t green);

void increment_colors(uint8_t profile_index, uint16_t region, uint8_t rgb_flags, uint8_t red, uint8_t blue, uint8_t green);

void decrement_colors(uint8_t profile_index, uint16_t region, uint8_t rgb_flags, uint8_t red, uint8_t blue, uint8_t green);

void set_freq(uint8_t profile_index, uint16_t region, uint16_t freq);

void increment_freq(uint8_t profile_index, uint16_t region, uint16_t freq);

void decrement_freq(uint8_t profile_index, uint16_t region, uint16_t freq);

void toggle_pause(uint8_t profile_index, uint16_t region);

// TODO
void inc_profile();

void dec_profile();

void set_profile(uint8_t profile_index);

/* uint8_t get_cur_profile_index(); */

#endif
