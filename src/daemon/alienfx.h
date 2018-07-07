#ifndef ALIENFX_H_
#define ALIENFX_H_

#include <stdbool.h>


/* #include <libusb-1.0/libusb.h> */

void power_off_lights();

void power_on_lights();

void power_red_lights();

void up_it_red();

void up_it_green();

void up_it_blue();

void down_it_red();

void down_it_green();

void down_it_blue();

int get_red();

int get_green();

int get_blue();

int get_type();

void low_power_mode(bool isOn);

void make_blinky();

void make_solid();

void up_blinky_speed();

void down_blinky_speed();

#endif
