#ifndef CONSTS_H
#define CONSTS_H
#define WRITEPATH 1

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

#define DATA_LENGTH                 9

#define START_BYTE               	  0x02
#define FILL_BYTE                	  0x00

#define BLOCK_LOAD_ON_BOOT       	  0x01
#define BLOCK_STANDBY            	  0x02
#define BLOCK_AC_POWER           	  0x05
#define BLOCK_CHARGING           	  0x06
#define BLOCK_BAT_POWER          	  0x08

#define NUM_PROFILES 4


// for specifying which internal handler
#define INTERNAL_SET 0x1
#define INTERNAL_INC 0x2
#define INTERNAL_DEC 0x4

#define INTERNAL_USE_R 0x1
#define INTERNAL_USE_G 0x2
#define INTERNAL_USE_B 0x4
#define INTERNAL_USE_FREQ 0x8
#define INTERNAL_USE_TYPE 0x10
#define INTERNAL_USE_PAUSE 0x20
#define INTERNAL_ALL_FLAGS 0xff

#define RED_PATH "/home/dieraca/.config/slstatus/.r"
#define GREEN_PATH "/home/dieraca/.config/slstatus/.g"
#define BLUE_PATH "/home/dieraca/.config/slstatus/.b"
#define TYPE_PATH "/home/dieraca/.config/slstatus/.type"
#define BLINKY_PATH "/home/dieraca/.config/slstatus/.blink_speed"
#define LOW_POWER_PATH "/home/dieraca/.config/slstatus/.low_bat"

#define BLINKY_STEP_SIZE 16 // amount to increment blinky speed by
#endif
