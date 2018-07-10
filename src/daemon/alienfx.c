#include <./alienfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <libusb-1.0/libusb.h>
#include <string.h>
#include <stdbool.h>
#include "consts.h"
#include "protocol.h"

// to compile with debugging symbols:
// gcc alienfx.c -ggdb -enable-debug-log `pkg-config --libs --cflags libusb-1.0`
// to compile without debugging symbols
// gcc alienfx.c `pkg-config --libs --cflags libusb-1.0`

// compile with flag only if you want to write to path in order to integrate with slstatus
// TODO should probably write to tmp instead of random directory

// shouldn't need values but better safe than sorry...
// TODO fix this to READ in rgb every time because we have mutliple instances running right (slstatus, dwm, etc)
// TODO or have different modes (SLSTATUS MODE), (DWM MODE), b/c slstatus never really writes rgb ever, and dwm only writes.
static  uint r = 0;
static  uint g = 0;
static  uint b = 0;
static  bool paused = 0;



static uint t = COMMAND_SET_COLOR;
static uint16_t freq = 0;
static bool initialized = false;


void update_file(const char * filename, int val){
  FILE * the_file = fopen(filename, "w+");
  if(the_file == NULL){
    printf("PATH ERROR!");
    exit(0);
  }
  fprintf(the_file, "%x", val);
  fclose(the_file);
}

int read_file(const char * filename){
  FILE * the_file = fopen(filename, "r");
  if(the_file == NULL){
    printf("PATH ERROR!");
    exit(0);
  }
  int val;
  fscanf(the_file, "%x", &val);
  fclose(the_file);
  return val;
}

int get_type(){
  return read_file(TYPE_PATH);
}


////////////////////////////// BEGIN BOILER PLATE LOW LEVEL CODE ///////////////////
void initialize(libusb_context** context, libusb_device_handle** handle, unsigned short idVendor, unsigned short idProduct) {
  if (0 == libusb_init(context)) {
    // TODO add in if statement if we want debugging output
    libusb_set_debug(*context,3);
    if ((*handle = libusb_open_device_with_vid_pid(*context,idVendor,idProduct))) {
      fprintf(stderr,"device opened\r\n");
    } else {
      fprintf(stderr, "failed to open usb device; check idvendor and idproduct\r\n");
      exit(0);
    }
  } else {
    fprintf(stderr, "failed to initialize context\r\n");
    exit(0);
  }
}

void detach(libusb_device_handle* handle, int interface_number) {
  // alternative to claim_interface, I think?
	if (libusb_kernel_driver_active(handle, interface_number)){
		int success = libusb_detach_kernel_driver(handle, interface_number);
    if (success < 0){
      fprintf(stderr, "kernel driver active, and failed to detached with error %d\r\n", success);
      exit(0);
    }
  }
}

void attach(libusb_device_handle* handle, int interface_number) {
	int success = libusb_attach_kernel_driver(handle, interface_number);
  if(success < 0){
    fprintf(stderr, "attach_kernel_driver returned with %d\r\n", success);
    exit(0);
  }
}

/* int setdelay(libusb_device_handle* handle, unsigned int delay) */
/* { */
/* } */

void claim_interface(libusb_device_handle * handle, int interface_number){
  int success = libusb_claim_interface(handle, interface_number);
  if (success < 0){
    fprintf(stderr, "error claiming interface with code %d\r\n", success);
    exit(0);
  }
}

void release_interface(libusb_device_handle * handle, int interface_number){
  int success = libusb_release_interface(handle, interface_number);
  if (success < 0){
    fprintf(stderr, "error releasing interface with code %d\r\n", success);
    exit(0);
  }
}

void close_and_exit(libusb_device_handle * handle, libusb_context * context){
  libusb_close(handle);
  libusb_exit(context);
}


void single_write_to_fx(libusb_device_handle * handle, char * packet, int size){
  int unlimited_timeout = 0;
  int num_bytes_written = libusb_control_transfer(handle,
                                                  SEND_REQUEST_TYPE,
                                                  SEND_REQUEST,
                                                  SEND_VALUE,
                                                  SEND_INDEX,
                                                  packet,
                                                  size,
                                                  unlimited_timeout);
  if(num_bytes_written != size){
    // TODO do levels of debugging output
    // TODO should log
    fprintf(stderr, "Tried to write %d bytes but only wrote %d bytes \r\n", size, num_bytes_written);
  }

}

////////////////////////////// END BOILER PLATE LOW LEVEL CODE ///////////////////


// this is where most of the work goes down
void complete_write_to_fx(libusb_device_handle * handle, int block, uint region, int r, int g, int b, int interface_number, int type){

  unsigned char data1[] = { START_BYTE, COMMAND_RESET,
                            (unsigned char)RESET_ALL_LIGHTS_ON};

  single_write_to_fx(handle, data1, sizeof(data1));
  usleep(9000);

  char reg1 = (char)((region >> 16) & 0xff);
  char reg2 = (char)((region >>  8) & 0xff);
  char reg3 = (char)((region >>  0) & 0xff);

  char data[] = { START_BYTE, type,
                  block,
                  reg1, // 0xff for *all* regions
                  reg2, // 0xff for *all* regions
                  reg3, // 0xff for *all* regions
                  r,
                  g,
                  b};
  /* interface_number}; */
  single_write_to_fx(handle, data, sizeof(data));

  char data2[] = { START_BYTE, COMMAND_LOOP_BLOCK_END };
  single_write_to_fx(handle, data2, sizeof(data2));

  char data3[] = { START_BYTE, COMMAND_TRANSMIT_EXECUTE};
  single_write_to_fx(handle, data3, sizeof(data3));

}

// boiler plate code wrapper to complete_write_to_fx
void perform_action(int region, int red, int green, int blue, int type, int block_type){
	libusb_context*		context;
	libusb_device_handle*	handle;

  // boiler plate init code
  // initialize a context and handle
	initialize(&context, &handle, ALIENWARE_VENDORID, ALIENWARE_PRODUCTID);
  // detach any current driver
	detach(handle, INTERFACE_NUMBER);
  // claim the interface
  claim_interface(handle, INTERFACE_NUMBER);

  /* complete_write_to_fx(handle, BLOCK_CHARGING, KB_FAR_LEFT, 0, 0, 0, INTERFACE_NUMBER); */
  complete_write_to_fx(handle, block_type, region, red, green, blue, INTERFACE_NUMBER, type);
  // gotta sleep for some time, but 9001 is enough and pretty arbitrary
  usleep(9001);

  // end code
  release_interface(handle, INTERFACE_NUMBER);
  attach(handle, INTERFACE_NUMBER);
  close_and_exit(handle, context);
}

bool is_valid_region(uint16_t region){
  switch(region){
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

// arg 0 specifies what to get (rgb, freq, or color)
// remainder of args are not relevant
int get_handler(char * args, struct alienfx_response *resp){
  if(sizeof(args) != 6 * sizeof(char)) return -1;

  switch(args[0]){
  case RED_FLAG:
    resp->r_val = r;
    break;
  case GREEN_FLAG:
    resp->r_val = g;
    break;
  case BLUE_FLAG:
    resp->r_val = b;
    break;
  case FREQ_FLAG:
    resp->r_val = freq;
    break;
  default:
    resp->error = true;
    return -1;
  }
  resp->error = false;
  return 0;
}

int colors_handler(uint8_t * args, struct alienfx_response * resp, uint8_t flags){
  uint16_t region = (args[0] << 8) | args[1];
  if(!is_valid_region(region)){
    return -1;
  }

  if(RED_FLAG & args[2]){
    if(flags & COLOR_SET){
      r = args[3];
    }
    else if (flags & COLOR_INC){
      r += args[3];
    }
    else if (flags & COLOR_DEC){
      r -= args[3];
    }
    r &= 0xff;
    paused = false;
  }
  if(GREEN_FLAG & args[2]){
    if(flags & COLOR_SET){
      g = args[4];
    }
    else if (flags & COLOR_INC){
      g += args[4];
    }
    else if (flags & COLOR_DEC){
      g -= args[4];
    }
    g &= 0xff;
    paused = false;
  }
  if(BLUE_FLAG & args[2]){
    if(flags & COLOR_SET){
      b = args[5];
    }
    else if (flags & COLOR_INC){
      b += args[5];
    }
    else if (flags & COLOR_DEC){
      b -= args[5];
    }
    b &= 0xff;
    paused = false;
  }
  perform_action(region, r, g, b, t, BLOCK_CHARGING);
  return 0;
}

// arg[0] specifies upper half of region
// arg[1] specifies lower half of region
// arg[2] specifies which of rgb to set
// arg[3] specifies by how much if red
// arg[4] specifies by how much if green
// arg[5] specifies by how much if blue
int set_colors_handler(uint8_t *args, struct alienfx_response *resp){
  return colors_handler(args, resp, COLOR_SET);
}
int increment_colors_handler(uint8_t *args, struct alienfx_response *resp){
  return colors_handler(args, resp, COLOR_INC);
}
int decrement_colors_handler(uint8_t *args, struct alienfx_response *resp){
  return colors_handler(args, resp, COLOR_DEC);
}

// args don't matter
int toggle_pause_handler(uint8_t *args, struct alienfx_response *resp){
  if(paused){
    perform_action(ALL_THE_THINGS, 0, 0, 0, t, BLOCK_CHARGING);
  }else{
    perform_action(ALL_THE_THINGS, r, g, b, t, BLOCK_CHARGING);
  }
  paused = !paused;
  return 0;
}




void check_and_initialize_lights(){
  if(!initialized){
    //r = get_red(), g = get_green(), b = get_blue(), t = get_type(), freq = get_freq();
    perform_action(0xffffff, r, g, b, t, BLOCK_CHARGING);
    initialized = true;
  }
}


void low_power_mode(bool turnOn){
  check_and_initialize_lights();
  if(turnOn){
    if(read_file(LOW_POWER_PATH) == 1){
      return;
    }
    libusb_context*		context;
    libusb_device_handle*	handle;
    //enable
    initialize(&context, &handle, ALIENWARE_VENDORID, ALIENWARE_PRODUCTID);
    // detach any current driver
    detach(handle, INTERFACE_NUMBER);
    // claim the interface
    claim_interface(handle, INTERFACE_NUMBER);

    /* complete_write_to_fx(handle, BLOCK_CHARGING, KB_FAR_LEFT, 0, 0, 0, INTERFACE_NUMBER); */
    unsigned char data1[] = { START_BYTE, COMMAND_RESET,
                              (unsigned char)RESET_ALL_LIGHTS_ON};

    single_write_to_fx(handle, data1, sizeof(data1));
    usleep(9001);

    char data[] = { START_BYTE, COMMAND_SET_BLINK_COLOR,
                    0x01,
                    0xff, // 0xff for *all* regions
                    0xff, // 0xff for *all* regions
                    0xff, // 0xff for *all* regions
                    0xff,
                    0,
                    0};
    single_write_to_fx(handle, data, sizeof(data));
    usleep(9001);
    char data2[] = { START_BYTE, COMMAND_LOOP_BLOCK_END };
    single_write_to_fx(handle, data2, sizeof(data2));
    char data3[] = { START_BYTE, COMMAND_TRANSMIT_EXECUTE};
    single_write_to_fx(handle, data3, sizeof(data3));
    release_interface(handle, INTERFACE_NUMBER);
    attach(handle, INTERFACE_NUMBER);
    close_and_exit(handle, context);
    update_file(LOW_POWER_PATH, 1);
  } else{
    // disable
    if(read_file(LOW_POWER_PATH) != 0){
      perform_action(0xffffff, r, g, b, t, BLOCK_CHARGING);
      update_file(LOW_POWER_PATH, 0);
    }
  }
}

void make_blinky(){
  check_and_initialize_lights();
  t = COMMAND_SET_BLINK_COLOR;
  perform_action(0xffffff, r, g, b, t, BLOCK_CHARGING);
#ifdef WRITEPATH
  update_file(TYPE_PATH, t);
#endif
}

void make_solid(){
  check_and_initialize_lights();
  t = COMMAND_SET_COLOR;
  perform_action(0xffffff, r, g, b, t, BLOCK_CHARGING);
#ifdef WRITEPATH
  update_file(TYPE_PATH, t);
#endif
}

// TODO figure this out
void set_freq(bool increase_speed){

  freq = read_file(BLINKY_PATH);

  freq +=  BLINKY_STEP_SIZE * ((increase_speed) ? 1 : -1);

  printf("SPEED: %d\r\n", freq);

  if(freq > MAX_SPEED)
    freq = MAX_SPEED;
  if(freq < MIN_SPEED)
    freq = MIN_SPEED;

  printf("SPEED2: %d\r\n", freq);


  libusb_context*		context;
  libusb_device_handle*	handle;
  //enable
  initialize(&context, &handle, ALIENWARE_VENDORID, ALIENWARE_PRODUCTID);
  // detach any current driver
  detach(handle, INTERFACE_NUMBER);
  // claim the interface
  claim_interface(handle, INTERFACE_NUMBER);

  /* complete_write_to_fx(handle, BLOCK_CHARGING, KB_FAR_LEFT, 0, 0, 0, INTERFACE_NUMBER); */
  unsigned char data1[] = { START_BYTE, COMMAND_RESET,
                            (unsigned char)RESET_ALL_LIGHTS_ON};

  single_write_to_fx(handle, data1, sizeof(data1));
  usleep(9001);

  // TODO assertion about top and bottom halves being mutliple of 100
  char top_half = (freq >> 8) & 0xff;
  char bottom_half = freq & 0xff;

  char data[] = { START_BYTE, COMMAND_SET_SPEED, top_half, bottom_half};

  single_write_to_fx(handle, data, sizeof(data));
  usleep(9001);
  char data2[] = { START_BYTE, COMMAND_LOOP_BLOCK_END };
  single_write_to_fx(handle, data2, sizeof(data2));
  char data3[] = { START_BYTE, COMMAND_TRANSMIT_EXECUTE};
  single_write_to_fx(handle, data3, sizeof(data3));
  release_interface(handle, INTERFACE_NUMBER);
  attach(handle, INTERFACE_NUMBER);
  close_and_exit(handle, context);
  update_file(BLINKY_PATH, freq);
}

void up_freq(){
  set_freq(true);
}

void down_freq(){
  set_freq(false);
}


