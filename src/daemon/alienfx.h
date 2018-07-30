#ifndef ALIENFX_H_
#define ALIENFX_H_

#include <stdbool.h>
#include <stdint.h>
#include "protocol.h"

static struct alienfx_profile * profiles;

void init_profiles();

void write_entire_profile(struct alienfx_profile * prof);

int get_handler(uint8_t profile_index, uint8_t * args, struct alienfx_response * resp);

int set_colors_handler(uint8_t profile_index, uint8_t * args, struct alienfx_response * resp);

int increment_colors_handler(uint8_t profile_index, uint8_t * args, struct alienfx_response * resp);

int decrement_colors_handler(uint8_t profile_index, uint8_t * args, struct alienfx_response * resp);

int toggle_pause_handler(uint8_t profile_index, uint8_t * args, struct alienfx_response * resp);

int increment_profile_handler(uint8_t profile_index, uint8_t * args, struct alienfx_response * resp);

int decrement_profile_handler(uint8_t profile_index, uint8_t * args, struct alienfx_response * resp);

int set_profile_handler(uint8_t profile_index, uint8_t * args, struct alienfx_response * resp);

/* int toggle_flash_handler(uint16_t region_flags, uint8_t *args, struct alienfx_response *resp); */

/* int decrement_freq_handler(uint16_t region_flags, uint8_t *args, struct alienfx_response *resp); */
/* int increment_freq_handler(uint16_t region_flags, uint8_t *args, struct alienfx_response *resp); */
/* int set_freq_handler(uint16_t region_flags, uint8_t *args, struct alienfx_response *resp); */


#endif
