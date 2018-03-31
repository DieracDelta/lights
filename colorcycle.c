#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <stdbool.h>

#define	ALIENWARE_VENDORID		0x187c
#define	ALIENWARE_PRODUCTID	0x0530
#define INTERFACE_NUMBER 0

#define	OK			0
#define	LIBUSB_INIT_ERR		-1
#define	LIBUSB_OPEN_ERR		-2
#define	LIBUSB_SIZE_ERR		-3
#define	LIBUSB_WRITE_ERR	-4
#define	LIBUSB_READ_ERR		-5

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



int InitDevice(libusb_context** usbcontext,libusb_device_handle** usbhandle,unsigned short idVendor,unsigned short idProduct) {
if (0==libusb_init(usbcontext)) {
libusb_set_debug(*usbcontext,3);	
if ((*usbhandle=libusb_open_device_with_vid_pid(*usbcontext,idVendor,idProduct)))
  {
			fprintf(stderr,"device opened\n");	
} else {
exit(0);
return LIBUSB_OPEN_ERR;
}
} else {
return	LIBUSB_INIT_ERR;
}
return OK;
}
void usbdetach(libusb_device_handle* usbhandle)
{
	if (libusb_kernel_driver_active(usbhandle,INTERFACE_NUMBER))
		libusb_detach_kernel_driver(usbhandle,INTERFACE_NUMBER);

}
void usbattach(libusb_device_handle* usbhandle)
{
	libusb_attach_kernel_driver(usbhandle,0);
}
int usbsetdelay(libusb_device_handle* usbhandle,unsigned int delay)
{
	unsigned char data[9];
	unsigned int retval;
	int i;
	delay=(delay/STEP_SPEED)*STEP_SPEED;	// quantize to step multiple

	if (delay<MIN_SPEED) delay=MIN_SPEED;
	if (delay>MAX_SPEED) delay=MAX_SPEED;

	data[0]=0x02;
	data[1]=0x0e;	//COMMAND_SET_SPEED;
	data[2]=(delay>>8)&0xff;
	data[3]=(delay>>0)&0xff;
	data[4]=0;
	data[5]=0;
	data[6]=0;
	data[7]=0;
	data[8]=0;
	
	fprintf(stderr,"write> ");
	for (i=0;i<SEND_DATA_SIZE;i++) 
	{
		fprintf(stderr,"%02x ",0xff&((unsigned int)data[i]));
	}
	fprintf(stderr,"\n");
	retval=libusb_control_transfer(usbhandle,
					SEND_REQUEST_TYPE,
					SEND_REQUEST,
					SEND_VALUE,
					SEND_INDEX,
					data,
					SEND_DATA_SIZE,
					0);

	if (retval!=9)
		return LIBUSB_WRITE_ERR;

	return	OK;
}
int usbwrite(libusb_device_handle* usbhandle,unsigned char* data,unsigned short len)
{
	int retval;
	int i;
	if (len!=SEND_DATA_SIZE)
		return	LIBUSB_SIZE_ERR;


	fprintf(stderr,"write> ");
	for (i=0;i<SEND_DATA_SIZE;i++) 
	{
		fprintf(stderr,"%02x ",0xff&((unsigned int)data[i]));
	}
	fprintf(stderr,"\n");

	retval=libusb_control_transfer(usbhandle,
					SEND_REQUEST_TYPE,
					SEND_REQUEST,
					SEND_VALUE,
					SEND_INDEX,
					data,
					SEND_DATA_SIZE,
					0);
	if (retval!=SEND_DATA_SIZE)
		return	LIBUSB_WRITE_ERR;
	usleep(8000);	
	return OK;
}
int usbread(libusb_device_handle* usbhandle,char* data,unsigned int len)
{
	unsigned char buf[READ_DATA_SIZE];
	int readbytes;
	int i;

  // sanity check
	if (len!=READ_DATA_SIZE)
		return	LIBUSB_SIZE_ERR;

	readbytes = libusb_control_transfer(
                                      usbhandle,
				READ_REQUEST_TYPE,
				READ_REQUEST,
				READ_VALUE,
				READ_INDEX,
				buf,
				READ_DATA_SIZE,
				0);
	if (readbytes!=READ_DATA_SIZE){
    /* printf("LIBUSB_ERROR_TIMEOUT %d\r\n", LIBUSB_ERROR_TIMEOUT); */
    /* printf("LIBUSB_ERROR_PIPE %d\r\n", LIBUSB_ERROR_PIPE); */
    /* printf("LIBUSB_ERROR_NO_DEVICE %d\r\n", LIBUSB_ERROR_NO_DEVICE); */
    /* printf("readbytes ERROR was %d \r\n", readbytes); */
    // -8 is a LIBUSB_ERROR_OVERFLOW error
    exit(0);
		return LIBUSB_READ_ERR;
  }
	fprintf(stderr,"read>  ");
	for (i=0;i<READ_DATA_SIZE;i++) 
	{
		data[i]=buf[i];
		fprintf(stderr,"%02x ",0xff&((unsigned int)data[i]));
	}
	fprintf(stderr,"\n");
	return OK;
}
int main(void)
{
	unsigned char rst[]={0x02,0x07,0x04,0x00,0x00,0x00,0x00,0x00,0x00};
	unsigned char chk[]={0x02,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	unsigned char rply[8];

  /* FFFF9C8A4A836810h */
	unsigned char keys[6][9]={
		{0x02,0x07,0x04,0x00,0x00,0x00,0x00,0x00,0x00},	// RST
		{0x02,0x03,0x05,0x00,0x00,0x01,0x0f,0xf0,0x00},	// keyboard
		{0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // END
		{0x02,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // PGE
		{0x02,0x07,0x04,0x00,0x00,0x00,0x00,0x00,0x00},	// RST
		{0x02,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00}	// CHK
	};
	unsigned char front[6][9]={
		{0x02,0x07,0x04,0x00,0x00,0x00,0x00,0x00,0x00},	// RST
		{0x02,0x03,0x05,0x00,0x00,0x60,0x00,0xf0,0x00},	// front
		{0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // END
		{0x02,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // PGE
		{0x02,0x07,0x04,0x00,0x00,0x00,0x00,0x00,0x00},	// RST
		{0x02,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00}	// CHK
	};
	unsigned char prog1[17][9]={
		{0x02,0x07,0x04,0x00,0x00,0x00,0x00,0x00,0x00},	// RST
		{0x02,0x08,0x01,0x00,0x00,0x00,0x00,0x00,0x00},	// ADD 1
		{0x02,0x03,0x02,0x00,0x03,0x0f,0x0f,0x00,0x00}, // STA kl,kml,kmr,kr,tp,aw green
		{0x02,0x08,0x01,0x00,0x00,0x00,0x00,0x00,0x00},	// ADD 1
		{0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // END
		{0x02,0x08,0x01,0x00,0x00,0x00,0x00,0x00,0x00},	// ADD 1
		{0x02,0x03,0x03,0x00,0x00,0x60,0xf0,0x00,0x00}, // STA fl,fr red
		{0x02,0x08,0x01,0x00,0x00,0x00,0x00,0x00,0x00}, // ADD 1
		{0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // END
		{0x02,0x08,0x01,0x00,0x00,0x00,0x00,0x00,0x00}, // ADD 1
		{0x02,0x03,0x04,0x00,0x08,0x00,0x00,0xf0,0x00}, // STA sl blue
		{0x02,0x08,0x01,0x00,0x00,0x00,0x00,0x00,0x00}, // ADD 1
		{0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // END
		{0x02,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // SVE
		{0x02,0x07,0x04,0x00,0x00,0x00,0x00,0x00,0x00}, // RST
		{0x02,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // CHK
		{0x02,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00} // PGE
	};
	libusb_context*		usbcontext;
	libusb_device_handle*	usbhandle;
	int retval;
	int i,j,r,g,b;
	int ready;

	retval=InitDevice(&usbcontext,&usbhandle,ALIENWARE_VENDORID,ALIENWARE_PRODUCTID);
	usbdetach(usbhandle);

	if (retval==OK){
		int is_OK = usbread(usbhandle,rply,8);
    if (is_OK != 0){
      printf("FUCK13 %d\r\n", is_OK);
    }
  }else{
    printf("FUCK12 %d\r\n",retval);
    exit(0);
  }
	/* if (retval==OK) */
	/* 	retval=usbsetdelay(usbhandle,MIN_SPEED); */

	r=15;
	g=15;
	b=15;
	while (1)
	{
		for (r=0;r<16;r++)
		{
			keys[1][6]=(r<<4);
			keys[1][6]|=g;
			keys[1][7]=(b<<4);
			for (i=0;i<5;i++)
			{
				if (retval==OK)
				{
					int is_OK = usbwrite(usbhandle,keys[i],9);
          if (is_OK != 0){
            printf("FUCK15 %d\r\n", is_OK);
          }
				}
			}
			ready=0;
			while (!ready)
			{
				int is_OK = usbwrite(usbhandle,chk,9);
        if (is_OK != 0){
            printf("FUCK9 %d\r\n", is_OK);
          }
				usbread(usbhandle,rply,8);
				if (rply[0]==0x11) ready=1;
			}
		}
		for (r=15;r>=0;r--)
		{
			keys[1][6]=(r<<4);
			keys[1][6]|=g;
			keys[1][7]=(b<<4);
			for (i=0;i<5;i++)
			{
				if (retval==OK)
				{
					usbwrite(usbhandle,keys[i],9);
				}
			}
			ready=0;
			while (!ready)
			{
				usbwrite(usbhandle,chk,9);
				usbread(usbhandle,rply,8);
				if (rply[0]==0x11) ready=1;
			}
		}
	}
}
