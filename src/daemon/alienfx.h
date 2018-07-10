#ifndef ALIENFX_H_
#define ALIENFX_H_

#include <stdbool.h>
#include <stdint.h>
#include "protocol.h"

void power_off_lights();

void power_on_lights();

int get_type();

void low_power_mode(bool isOn);

void make_blinky();

void make_solid();

void up_blinky_speed();

void down_blinky_speed();

int get_handler(uint8_t * args, struct alienfx_response * resp);

int set_colors_handler(uint8_t * args, struct alienfx_response * resp);

int increment_handler(uint8_t * args, struct alienfx_response * resp);

int decrement_handler(uint8_t * args, struct alienfx_response * resp);

int pause_handler(uint8_t arg1, uint8_t arg2, uint8_t arg3);

#endif
