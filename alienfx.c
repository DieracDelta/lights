#include <./alienfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <libusb-1.0/libusb.h>
#include <string.h>

// the underlying protocol was heavily borrowed by https://github.com/erlkonig/alienfx

// to compile with debugging symbols:
// gcc alienfx.c -ggdb -enable-debug-log `pkg-config --libs --cflags libusb-1.0`
// to compile without debugging symbols
// gcc alienfx.c `pkg-config --libs --cflags libusb-1.0`



#define	ALIENWARE_VENDORID		0x187c
#define	ALIENWARE_PRODUCTID	0x0530
#define INTERFACE_NUMBER 0

#define SEND_REQUEST_TYPE 0x21
#define SEND_REQUEST      0x09
#define SEND_VALUE        0x202
#define SEND_INDEX        0x00
#define SEND_DATA_SIZE    9

#define	READ_REQUEST_TYPE	0xa1
#define	READ_REQUEST		0x01
#define	READ_VALUE		0x101
#define	READ_INDEX		0x0
#define	READ_DATA_SIZE		69

#define	MIN_SPEED		100
#define	MAX_SPEED		1000
#define	STEP_SPEED		100


// copied off alienfx project
#define STATE_BUSY               	  0x11
#define STATE_READY              	  0x10
#define STATE_UNKNOWN_COMMAND    	  0x12

#define SUPPORTED_COMMANDS       	    15
#define COMMAND_END_STORAGE      	  0x00 // End storage block
#define COMMAND_SET_MORPH_COLOR  	  0x01
#define COMMAND_SET_BLINK_COLOR  	  0x02
#define COMMAND_SET_COLOR        	  0x03
#define COMMAND_LOOP_BLOCK_END   	  0x04
#define COMMAND_TRANSMIT_EXECUTE 	  0x05 // End transmition and execute
#define COMMAND_GET_STATUS       	  0x06 // Get device status
#define COMMAND_RESET            	  0x07
#define COMMAND_SAVE_NEXT        	  0x08 // Save next inst' in storage block
#define COMMAND_SAVE             	  0x09 // Save storage data
#define COMMAND_BATTERY_STATE    	  0x0F // Set batery state
#define COMMAND_SET_SPEED        	  0x0E // Set display speed

#define RESET_TOUCH_CONTROLS     	  0x01
#define RESET_SLEEP_LIGHTS_ON    	  0x02
#define RESET_ALL_LIGHTS_OFF     	  0x03
#define RESET_ALL_LIGHTS_ON      	  0x04

#define DATA_LENGTH                      9

#define START_BYTE               	  0x02  // shows 0x00 for Area51; bug?
#define FILL_BYTE                	  0x00

#define BLOCK_LOAD_ON_BOOT       	  0x01
#define BLOCK_STANDBY            	  0x02
#define BLOCK_AC_POWER           	  0x05
#define BLOCK_CHARGING           	  0x06
#define BLOCK_BAT_POWER          	  0x08

// for a fact work
#define KB_FAR_RIGHT 0x1
#define KB_MID_RIGHT 0x2
#define KB_FAR_LEFT 0x8
#define KB_MID_LEFT 0x4
#define POWER_BUTTON 0x100
#define ALIEN_HEAD 0x20
#define ALIENWARE_NAME 0x40
#define BOT_RIGHT_BURNER 0x800
#define BOT_LEFT_BURNER 0x400
#define TOP_RIGHT_BURNER 0x2000
#define TOP_LEFT_BURNER 0x1000
#define KB_SPECIAL 0x4000
#define ALL_THE_THINGS 0xffff

// NOTE must be enabled in bios in order for this to be effective
#define TRACKPAD 0x80




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

int setdelay(libusb_device_handle* handle, unsigned int delay)
{
}

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
    fprintf(stderr, "Tried to write %d bytes but only wrote %d bytes \r\n", size, num_bytes_written);
  }

}


void complete_write_to_fx(libusb_device_handle * handle, int block, uint region, int r, int g, int b, int interface_number){

  unsigned char data1[] = { START_BYTE, COMMAND_RESET,
                            (unsigned char)RESET_ALL_LIGHTS_ON};

  single_write_to_fx(handle, data1, sizeof(data1));
  usleep(9000);

  char red = (char) (r & 0xff);
  char green = (char) (g & 0xff);
  char blue = (char) (b & 0xff);


  char reg1 = (char)((region >> 16) & 0xff);
  char reg2 = (char)((region >>  8) & 0xff);
  char reg3 = (char)((region >>  0) & 0xff);

  char data[] = { START_BYTE, COMMAND_SET_COLOR,
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


void perform_action(int region, int red, int green, int blue) {
	libusb_context*		context;
	libusb_device_handle*	handle;

  // init code
  // initialize a context and handle
	initialize(&context, &handle, ALIENWARE_VENDORID, ALIENWARE_PRODUCTID);
  // detach any current driver
	detach(handle, INTERFACE_NUMBER);
  // claim the interface
  claim_interface(handle, INTERFACE_NUMBER);



  /* complete_write_to_fx(handle, BLOCK_CHARGING, KB_FAR_LEFT, 0, 0, 0, INTERFACE_NUMBER); */
  complete_write_to_fx(handle, BLOCK_CHARGING, region, red, green, blue, INTERFACE_NUMBER);
  usleep(9001);


  // end code
  release_interface(handle, INTERFACE_NUMBER);
  attach(handle, INTERFACE_NUMBER);
  close_and_exit(handle, context);

  fprintf(stderr, "FINISHED successfully\r\n");
}

void poweroff_lights(){
  perform_action(ALL_THE_THINGS, 0, 0, 0);
}

void power_red_lights(){
  perform_action(ALL_THE_THINGS, 255, 0, 0);
}

static uint r = 0;
static uint g = 0;
static uint b = 0;
static bool initialized = false;

void up_it_red(){
  if(!initialized){
    perform_action(0xffffff, 0, 0, 0);
    r = 0, g = 0, b = 0;
    initialized = true;
  }
  r += 10;
  r &= 0xff;
  perform_action(0xffffff, r, g, b);

}

void up_it_green(){
  if(!initialized){
    perform_action(0xffffff, 0, 0, 0);
    r = 0, g = 0, b = 0;
    initialized = true;
  }
  g += 10;
  g &= 0xff;
  perform_action(0xffffff, r, g, b);
}

void up_it_blue(){
  if(!initialized){
    perform_action(0xffffff, 0, 0, 0);
    r = 0, g = 0, b = 0;
    initialized = true;
  }
  b += 10;
  b &= 0xff;
  perform_action(0xffffff, r, g, b);
}


void down_it_red(){
  if(!initialized){
    perform_action(0xffffff, 0, 0, 0);
    r = 0, g = 0, b = 0;
    initialized = true;
  }
  r -= 10;
  r &= 0xff;
  perform_action(0xffffff, r, g, b);

}

void down_it_green(){
  if(!initialized){
    perform_action(0xffffff, 0, 0, 0);
    r = 0, g = 0, b = 0;
    initialized = true;
  }
  g -= 10;
  g &= 0xff;
  perform_action(0xffffff, r, g, b);
}

void down_it_blue(){
  if(!initialized){
    perform_action(0xffffff, 0, 0, 0);
    r = 0, g = 0, b = 0;
    initialized = true;
  }
  b -= 10;
  b &= 0xff;
  perform_action(0xffffff, r, g, b);
}
