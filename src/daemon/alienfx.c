#include "consts.h"
#include "protocol.h"
#include <./alienfx.h>
#include <libusb-1.0/libusb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

// TODO make args a bit stronger with method signatures like
// int someFunction(char arg[static 10])

static struct alienfx_profile *profiles;
static uint8_t current_profile_index = 0;

void init_profiles() {
  profiles = calloc(sizeof(struct alienfx_profile), NUM_PROFILES);
  for (int i = 0; i < NUM_PROFILES; i++) {
    profiles[i].regions = calloc(sizeof(struct region_state), NUM_REGIONS);
    for (int j = 0; j < NUM_REGIONS; j++) {
      // TODO initialize according to config file
      profiles[i].regions[j] = (struct region_state){
          .r = 0,
          .g = 69,
          .b = 0,
          .freq = 500,
          .type = COMMAND_SET_COLOR,
          .paused = false,
      };
    }
  }
  syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "ALIENFX! profile 0 %x \n",
         profiles[current_profile_index].regions[0].r);
}

void close_profiles() {
  for (int i = 0; i < NUM_PROFILES; i++) {
    free(profiles[i].regions);
  }
  free(profiles);
}

// FIXME this should change based on the flag value
int get_flag(int j) {
  if (j >= NOT_DEFINED_1_POS) {
    j++;
  }
  if (j >= NOT_DEFINED_2_POS) {
    j++;
  }
  return 1 << j;
}

// FIXME clean this up
int update_profile(uint8_t profile, uint16_t region_flags, uint8_t r, uint8_t g,
                   uint8_t b, uint16_t freq, uint8_t type, bool paused,
                   uint8_t use_flags, uint8_t op_flags) {
  // outta bounds
  if (profile >= NUM_PROFILES) {
    return -1;
  }

  for (int i = 0; i < NUM_REGIONS; i++) {
    // undefined
    int flag_val = get_flag(i);
    if (flag_val & region_flags) {
      if (use_flags & INTERNAL_USE_R) {
        if (op_flags & INTERNAL_SET) {
          profiles[profile].regions[i].r = r;
        } else if (op_flags & INTERNAL_INC) {
          profiles[profile].regions[i].r += r;
        } else if (op_flags & INTERNAL_DEC) {
          profiles[profile].regions[i].r -= r;
        }
      }
      if (use_flags & INTERNAL_USE_G) {
        if (op_flags & INTERNAL_SET) {
          profiles[profile].regions[i].g = g;
        } else if (op_flags & INTERNAL_INC) {
          profiles[profile].regions[i].g += g;
        } else if (op_flags & INTERNAL_DEC) {
          profiles[profile].regions[i].g -= g;
        }
      }
      if (use_flags & INTERNAL_USE_B) {
        if (op_flags & INTERNAL_SET) {
          profiles[profile].regions[i].b = b;
        } else if (op_flags & INTERNAL_INC) {
          profiles[profile].regions[i].b += b;
        } else if (op_flags & INTERNAL_DEC) {
          profiles[profile].regions[i].b -= b;
        }
      }
      if (use_flags & INTERNAL_USE_FREQ) {
        if (op_flags & INTERNAL_SET) {
          profiles[profile].regions[i].freq = freq;
        } else if (op_flags & INTERNAL_INC) {
          profiles[profile].regions[i].freq += freq;
        } else if (op_flags & INTERNAL_DEC) {
          profiles[profile].regions[i].freq -= freq;
        }
      }
      if (use_flags & INTERNAL_USE_TYPE) {
        if (op_flags & INTERNAL_SET) {
          profiles[profile].regions[i].type = type;
        }
      }
      if (use_flags & INTERNAL_USE_PAUSE) {
        if (op_flags & INTERNAL_SET) {
          profiles[profile].regions[i].paused = paused;
        }
      }
    }
  }
  return 0;
}

// TODO revamp for config
void update_file(const char *filename, int val) {
  FILE *the_file = fopen(filename, "w+");
  if (the_file == NULL) {
    printf("PATH ERROR!");
    exit(0);
  }
  fprintf(the_file, "%x", val);
  fclose(the_file);
}

////////////////////////////// BEGIN BOILER PLATE LOW LEVEL CODE
//////////////////////
void initialize(libusb_context **context, libusb_device_handle **handle,
                unsigned short idVendor, unsigned short idProduct) {
  if (0 == libusb_init(context)) {
    // TODO add in if statement if we want debugging output
    libusb_set_debug(*context, 3);
    if ((*handle =
             libusb_open_device_with_vid_pid(*context, idVendor, idProduct))) {
      fprintf(stderr, "device opened\r\n");
    } else {
      fprintf(stderr,
              "failed to open usb device; check idvendor and idproduct\r\n");
      exit(0);
    }
  } else {
    fprintf(stderr, "failed to initialize context\r\n");
    exit(0);
  }
}

void detach(libusb_device_handle *handle, int interface_number) {
  if (libusb_kernel_driver_active(handle, interface_number)) {
    int success = libusb_detach_kernel_driver(handle, interface_number);
    if (success < 0) {
      fprintf(stderr,
              "kernel driver active, and failed to detached with error %d\r\n",
              success);
      exit(0);
    }
  }
}

void attach(libusb_device_handle *handle, int interface_number) {
  int success = libusb_attach_kernel_driver(handle, interface_number);
  if (success < 0) {
    fprintf(stderr, "attach_kernel_driver returned with %d\r\n", success);
    exit(0);
  }
}

void claim_interface(libusb_device_handle *handle, int interface_number) {
  int success = libusb_claim_interface(handle, interface_number);
  if (success < 0) {
    fprintf(stderr, "error claiming interface with code %d\r\n", success);
    exit(0);
  }
}

void release_interface(libusb_device_handle *handle, int interface_number) {
  int success = libusb_release_interface(handle, interface_number);
  if (success < 0) {
    fprintf(stderr, "error releasing interface with code %d\r\n", success);
    exit(0);
  }
}

void close_and_exit(libusb_device_handle *handle, libusb_context *context) {
  libusb_close(handle);
  libusb_exit(context);
}

void single_write_to_fx(libusb_device_handle *handle, char *packet, int size) {
  int unlimited_timeout = 0;
  int num_bytes_written = libusb_control_transfer(
      handle, SEND_REQUEST_TYPE, SEND_REQUEST, SEND_VALUE, SEND_INDEX,
      (unsigned char *)packet, size, unlimited_timeout);
  if (num_bytes_written != size) {
    // TODO do levels of debugging output
    // TODO should log
    fprintf(stderr, "Tried to write %d bytes but only wrote %d bytes \r\n",
            size, num_bytes_written);
  }
}

////////////////////////////// END BOILER PLATE LOW LEVEL CODE
//////////////////////

// this is where most of the work goes down
void complete_write_to_fx(libusb_device_handle *handle, unsigned char data[],
                          int data_size) {

  /* unsigned char data1[] = { START_BYTE, COMMAND_RESET, */
  /*                           (unsigned char)RESET_ALL_LIGHTS_ON}; */
  unsigned char data1[] = {START_BYTE, COMMAND_RESET,
                           (unsigned char)RESET_ALL_LIGHTS_ON};

  single_write_to_fx(handle, (char *)data1, sizeof(data1));
  usleep(9000);

  single_write_to_fx(handle, (char *)data, data_size);
  /* char c1[] = */
  /*   { */
  /*     (char) START_BYTE, */
  /*     (char) COMMAND_SET_MORPH_COLOR, */
  /*     // block */
  /*     0x01, */
  /*     0x01, // 0xff for *all* regions */
  /*     0x00, // is upper half of region */
  /*     0x20, // is lower half of region */
  /*     0, */
  /*     0x80, */
  /*     0, */
  /*     0x50, */
  /*     0, */
  /*     0, */
  /*   }; */
  /* char c2[] = */
  /*   { */
  /*     (char) START_BYTE, */
  /*     (char) COMMAND_SET_MORPH_COLOR, */
  /*     // block */
  /*     0x01, */
  /*     0x02, // 0xff for *all* regions */
  /*     0x00, // is upper half of region */
  /*     0x80, // is lower half of region */
  /*     0, */
  /*     0x80, */
  /*     0, */
  /*     0x50, */
  /*     0, */
  /*     0, */
  /*   }; */
  /* char c1[] = */
  /*   { */
  /*     (char) START_BYTE, */
  /*     (char) COMMAND_SET_BLINK_COLOR, */
  /*     // block */
  /*     0x01, */
  /*     0x01, // 0xff for *all* regions */
  /*     0x00, // is upper half of region */
  /*     0x20, // is lower half of region */
  /*     0x50, */
  /*     0, */
  /*     0, */
  /*   }; */
  /* char c2[] = */
  /*   { */
  /*     (char) START_BYTE, */
  /*     (char) COMMAND_SET_BLINK_COLOR, */
  /*     // block */
  /*     0x01, */
  /*     (char) 0xff, // 0xff for *all* regions */
  /*     (char) 0x00, // is upper half of region */
  /*     (char) 0x80, // is lower half of region */
  /*     0, */
  /*     (char) 0x80, */
  /*     0, */
  /*   }; */
  /* single_write_to_fx(handle, c1, sizeof(c1)); */
  /* single_write_to_fx(handle, c2, sizeof(c2)); */

  char data2[] = {START_BYTE, COMMAND_LOOP_BLOCK_END};
  single_write_to_fx(handle, data2, sizeof(data2));

  char data3[] = {START_BYTE, COMMAND_TRANSMIT_EXECUTE};
  single_write_to_fx(handle, data3, sizeof(data3));
}

// boiler plate code wrapper to complete_write_to_fx
void perform_action(unsigned char data[], int data_size) {
  libusb_context *context;
  libusb_device_handle *handle;

  // boiler plate init code
  // initialize a context and handle
  initialize(&context, &handle, ALIENWARE_VENDORID, ALIENWARE_PRODUCTID);
  // detach any current driver
  detach(handle, INTERFACE_NUMBER);
  // claim the interface
  claim_interface(handle, INTERFACE_NUMBER);

  /* complete_write_to_fx(handle, BLOCK_CHARGING, KB_FAR_LEFT, 0, 0, 0,
   * INTERFACE_NUMBER); */
  complete_write_to_fx(handle, data, data_size);
  // gotta sleep for some time, but 9001 is enough and pretty arbitrary
  usleep(9001);

  // end code
  release_interface(handle, INTERFACE_NUMBER);
  attach(handle, INTERFACE_NUMBER);
  close_and_exit(handle, context);
}

void alienfx_write(uint region, int r, int g, int b, uint16_t freq,
                   uint8_t type) {
  // for some reason this isn't needed...maybe legacy?
  /* char reg1 = (char)((region >> 16) & 0xff); */
  char reg1 = (unsigned char)((region >> 8) & 0xff);
  char reg2 = (unsigned char)((region >> 0) & 0xff);
  printf("writing to region %x", region);
  if (type == COMMAND_SET_COLOR) {
    unsigned char data[] = {
        (char)START_BYTE,
        (char)COMMAND_SET_COLOR,
        // block
        0xff, // this needs to be 0xff (or at least 0x10) for some reason
        /* 0xff, */
        0xff, // this also needs to be 0xff for some reason
        // SWAP this out with below for extra features
        /* 0xff, */
        /* 0xff, */
        reg1, // 0xff for *all* regions
        reg2, // 0xff for *all* regions
        r,
        g,
        b,
    };
    syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "yeet3\n");
    perform_action(data, sizeof(data));
    syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "yeet4\n");
  } else if (type == COMMAND_SET_BLINK_COLOR) {
    char data1[] = {
        (char)START_BYTE,
        (char)COMMAND_SET_MORPH_COLOR,
        // block
        (char)0x01,
        (char)0x01, // 0xff for *all* regions
        (char)0x00, // is upper half of region
        (char)0x20, // is lower half of region
        0,
        (char)0x80,
        b,
        (char)0x50,
        0,
        0,
    };
    /* char data1[] = */
    /*   { */
    /*     (char) START_BYTE, */
    /*     (char) COMMAND_SET_COLOR, */
    /*     // block */
    /*     0x10, // this needs to be 0xff (or at least 0x10) for some reason */
    /*     0xff, // this also needs to be 0xff for some reason */
    /*     reg1, // 0xff for *all* regions */
    /*     reg2, // 0xff for *all* regions */
    /*     r, */
    /*     g, */
    /*     b, */
    /*   }; */
    syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "yeet\n");
    perform_action((unsigned char *)data1, sizeof(data1));
    uint8_t upper_half = (freq >> 8) & 0xff;
    uint8_t lower_half = freq & 0xff;
    char data2[] = {
        COMMAND_SET_SPEED,
        upper_half,
        lower_half,
    };
    perform_action((unsigned char *)data2, sizeof(data2));
    syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "yeet2\n");
  } else {
    return;
  }
}

void freq_write(uint16_t freq) {
  char top_half = (freq >> 8) & 0xff;
  char bottom_half = freq & 0xff;

  syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "WEE\n");
  unsigned char data[] = {START_BYTE, COMMAND_SET_SPEED, top_half, bottom_half};
  perform_action(data, sizeof(data));
}

void write_entire_profile(struct alienfx_profile *profile) {
  // not current profile
  if (profile != NULL && profile != profiles + current_profile_index) {
    return;
  }
  // the default
  struct alienfx_profile *prof =
      (profile == NULL) ? profiles + current_profile_index : profile;

  /* syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "ALIENFX! point 70 %p %p",
   * profiles); */
  /* struct region_state bruh = prof->regions[0]; */
  syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "ALIENFX! point 7000 ");
  for (struct region_state *cur_region = prof->regions;
       cur_region < prof->regions + NUM_REGIONS; cur_region++) {
    /* if(!cur_region->paused){ */

    syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "setting red value as %x\n",
           cur_region->r);
    syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "setting blue value as %x\n",
           cur_region->b);
    syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "setting green value as %x\n",
           cur_region->g);
    syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "the fucking region as %x\n",
           get_flag(cur_region - prof->regions));
    alienfx_write(get_flag(cur_region - prof->regions), cur_region->r,
                  cur_region->g, cur_region->b, cur_region->freq,
                  cur_region->type);
    /* } else{ */
    // if it's paused, write zeroes
    /* alienfx_write(get_flag(cur_region - prof->regions), 0, 0, 0, 0,
     * COMMAND_SET_COLOR); */
    /* } */
    /* break; */
  }
}

bool is_valid_region(uint16_t region) {
  switch (region) {
  case KB_FAR_RIGHT:
  case KB_MID_RIGHT:
  case KB_FAR_LEFT:
  case KB_MID_LEFT:
  case POWER_BUTTON:
  case ALIEN_HEAD:
  case ALIENWARE_NAME:
  case BOT_RIGHT_BURNER:
  case BOT_LEFT_BURNER:
  case TOP_RIGHT_BURNER:
  case TOP_LEFT_BURNER:
  case KB_SPECIAL:
  case ALL_THE_THINGS:
    return true;
  }
  return false;
}

int colors_handler(uint8_t profile_index, uint8_t *args, uint8_t color_flags) {
  uint16_t region = (args[0] << 8) | args[1];
  syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "OUTER THING GETS CALLED");

  if (RED_FLAG & args[2]) {
    syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "INNER THING GETS CALLED");
    syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "421\n");
    update_profile(profile_index, region, args[3], 0, 0, 0, 0, 0,
                   INTERNAL_USE_R, color_flags);
  }
  if (GREEN_FLAG & args[2]) {
    update_profile(profile_index, region, 0, args[4], 0, 0, 0, 0,
                   INTERNAL_USE_G, color_flags);
  }
  if (BLUE_FLAG & args[2]) {
    update_profile(profile_index, region, 0, 0, args[5], 0, 0, 0,
                   INTERNAL_USE_B, color_flags);
  }
  if (profile_index == current_profile_index) {
    write_entire_profile(profiles + profile_index);
    syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "Color handler was called! \n");
  }
  return 0;
}

// arg[0] specifies upper half of region
// arg[1] specifies lower half of region
// arg[2] specifies which of rgb to set
// arg[3] specifies by how much if red
// arg[4] specifies by how much if green
// arg[5] specifies by how much if blue
int set_colors_handler(uint8_t profile_index, uint8_t *args) {
  return colors_handler(profile_index, args, INTERNAL_SET);
}
int increment_colors_handler(uint8_t profile_index, uint8_t *args) {
  return colors_handler(profile_index, args, INTERNAL_INC);
}
int decrement_colors_handler(uint8_t profile_index, uint8_t *args) {
  syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_INFO), "ALIENFX is doing the job\n");
  return colors_handler(profile_index, args, INTERNAL_DEC);
}

// arg[0] specifies upper half of region
// arg[1] specifies lower half of region
// arg[2] represents upper half of new freq
// arg[3] represents lower half of new freq
// TODO may not work depending on whether or not I *have* to do increments of
// 100
// TODO implement with the profile write thingy
int freq_handler(uint8_t profile_index, uint8_t *args, uint8_t flags) {
  uint16_t region = (args[0] << 8) | args[1];
  int16_t new_freq = (args[2] << 8) | args[3];
  update_profile(profile_index, region, 0, 0, 0, new_freq, 0, 0,
                 INTERNAL_USE_FREQ, flags);
  write_entire_profile(profiles + current_profile_index);
  return 0;
}

int set_freq_handler(uint8_t profile_index, uint8_t *args) {
  return freq_handler(profile_index, args, INTERNAL_SET);
}
int increment_freq_handler(uint8_t profile_index, uint8_t *args) {
  return freq_handler(profile_index, args, INTERNAL_INC);
}
int decrement_freq_handler(uint8_t profile_index, uint8_t *args) {
  return freq_handler(profile_index, args, INTERNAL_DEC);
}

// arg[0] specifies upper half of region
// arg[1] specifies lower half of region
int toggle_pause_handler(uint8_t profile_index, uint8_t *args) {
  uint16_t region = (args[0] << 8) | args[1];
  for (int i = 0; i < NUM_REGIONS; i++) {
    if (get_flag(i) & region)
      update_profile(profile_index, region, 0, 0, 0, 0, 0,
                     !profiles[profile_index].regions[i].paused,
                     INTERNAL_USE_PAUSE, INTERNAL_SET);
  }
  return 0;
}

// TODO I'm assuming this literally just sets the profile handler
int set_profile_handler(uint8_t a) {
  /* int set_profile_handler(){ */
  if (a < NUM_PROFILES) {
    current_profile_index = a;
    write_entire_profile(profiles + current_profile_index);
  }
  return 0;
}
