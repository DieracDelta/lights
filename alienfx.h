#ifndef ALIENFX_H_
#define ALIENFX_H_

#include <stdbool.h>

/* #include <libusb-1.0/libusb.h> */

void poweroff_lights();

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

void low_power_mode(bool isOn);

#endif
