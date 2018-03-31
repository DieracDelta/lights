#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <stdbool.h>

// compile with:
// gcc alienfx.c -ggdb -enable-debug-log `pkg-config --libs --cflags libusb-1.0`


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
#define	READ_DATA_SIZE		8

#define	MIN_SPEED		100
#define	MAX_SPEED		1000
#define	STEP_SPEED		100



void initialize(libusb_context** context, libusb_device_handle** handle, unsigned short idVendor, unsigned short idProduct) {
  if (0 == libusb_init(context)) {
    // TODO add in if statement if we want debugging output
    libusb_set_debug(*context,3);
    if ((*handle = libusb_open_device_with_vid_pid(*context,idVendor,idProduct))) {
        fprintf(stderr,"device opened\n");
    } else {
      fprintf(stderr, "failed to open usb device; check idvendor and idproduct");
      exit(0);
    }
  } else {
    fprintf(stderr, "failed to initialize context");
    exit(0);
  }
}

void detach(libusb_device_handle* handle, int interface_number) {
  // alternative to claim_interface, I think?
	if (libusb_kernel_driver_active(handle, interface_number)){
		int success = libusb_detach_kernel_driver(handle, interface_number);
    if (success < 0){
      fprintf(stderr, "kernel driver active, and failed to detached with error %d", success);
        exit(0);
      }
  }
}

void attach(libusb_device_handle* handle, int interface_number) {
	int success = libusb_attach_kernel_driver(handle, interface_number);
  if(success < 0){
    fprintf(stderr, "attach_kernel_driver returned with %d", success);
    exit(0);
  }
}

int setdelay(libusb_device_handle* handle, unsigned int delay)
{
}
/* int write(libusb_device_handle* handle, unsigned char* data, unsigned short len) { */
	/* int retval; */
	/* int i; */
	/* if (len!=SEND_DATA_SIZE) */
	/* 	return	LIBUSB_SIZE_ERR; */


	/* fprintf(stderr,"write> "); */
	/* for (i=0;i<SEND_DATA_SIZE;i++)  */
  /*   { */
  /*     fprintf(stderr,"%02x ",0xff&((unsigned int)data[i])); */
  /*   } */
	/* fprintf(stderr,"\n"); */

	/* retval=libusb_control_transfer(handle, */
  /*                                SEND_REQUEST_TYPE, */
  /*                                SEND_REQUEST, */
  /*                                SEND_VALUE, */
  /*                                SEND_INDEX, */
  /*                                data, */
  /*                                SEND_DATA_SIZE, */
  /*                                0); */
	/* if (retval!=SEND_DATA_SIZE) */
	/* 	return	LIBUSB_WRITE_ERR; */
	/* usleep(8000);	 */
	/* return OK; */
/* } */

/* int read(libusb_device_handle* handle, char* data, unsigned int len) { */
	/* unsigned char buf[READ_DATA_SIZE]; */
	/* int readbytes; */
	/* int i; */

  /* // sanity check */
	/* if (len!=READ_DATA_SIZE) */
	/* 	return	LIBUSB_SIZE_ERR; */

	/* readbytes = libusb_control_transfer( */
  /*                                     handle, */
  /*                                     READ_REQUEST_TYPE, */
  /*                                     READ_REQUEST, */
  /*                                     READ_VALUE, */
  /*                                     READ_INDEX, */
  /*                                     buf, */
  /*                                     READ_DATA_SIZE, */
  /*                                     0); */
	/* if (readbytes!=READ_DATA_SIZE){ */
  /*   // -8 is a LIBUSB_ERROR_OVERFLOW error */
  /*   exit(0); */
	/* 	return LIBUSB_READ_ERR; */
  /* } */
	/* fprintf(stderr,"read>  "); */
	/* for (i=0;i<READ_DATA_SIZE;i++)  */
  /*   { */
  /*     data[i]=buf[i]; */
  /*     fprintf(stderr,"%02x ",0xff&((unsigned int)data[i])); */
  /*   } */
	/* fprintf(stderr,"\n"); */
/*   return 0; */
/* } */

void claim_interface(libusb_device_handle * handle, int interface_number){
  int success = libusb_claim_interface(handle, interface_number);
  if (success < 0){
    fprintf(stderr, "error claiming interface with code %d", success);
    exit(0);
  }
}

void release_interface(libusb_device_handle * handle, int interface_number){
  int success = libusb_release_interface(handle, interface_number);
  if (success < 0){
    fprintf(stderr, "error releasing interface with code %d", success);
    exit(0);
  }
}

void close_and_exit(libusb_device_handle * handle, libusb_context * context){
  libusb_close(handle);
  libusb_exit(context);
}

int main(void)
{
	libusb_context*		context;
	libusb_device_handle*	handle;

  // initialize a context and handle
	initialize(&context, &handle, ALIENWARE_VENDORID, ALIENWARE_PRODUCTID);
  // detach any current driver
	detach(handle, INTERFACE_NUMBER);
  // claim the interface
  claim_interface(handle, INTERFACE_NUMBER);




  // end code
  release_interface(handle, INTERFACE_NUMBER);
  attach(handle, INTERFACE_NUMBER);
  close_and_exit(handle, context);

  printf("FINISHED");
  /* example packet FFFF9C8A4A836810h */
}
