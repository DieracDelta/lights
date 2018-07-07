SHELL = /bin/sh
CC = gcc
SOURCES  := $(wildcard *.c)
OBJ = alienfx.o main.o
SOURCES  := $(wildcard *.c)
INCLUDES := $(wildcard *.h)
OBJECTS  := $(SOURCES:.c=*.o)
LIBS = -libusb-1.0
OUTFILE = alienfxd

ifeq ($(DEBUG), 1)
CFLAGS += -ggdb
endif

$(OUTFILE): main main.o

.c.o:
	@echo [Compile] $<
	@$(CC) -c $(CFLAGS) $< -o $@

alienfx.o: alienfx.h consts.h


clean:
	rm *.o

.PHONY: clean install
