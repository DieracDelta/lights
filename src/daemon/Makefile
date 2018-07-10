SHELL = /bin/sh
CC = gcc
CFLAGS += -MD -MP -Wall
DEPS = $(shell find . -name "*.d")
SOURCES  := $(wildcard *.c)
OBJ = alienfx.o protocol.o consts.o main.o
SOURCES  := $(wildcard *.c)
INCLUDES := $(wildcard *.h)
OBJECTS  := $(SOURCES:.c=*.o)
LIBS = -lusb-1.0
LDFLAGS = $(LIBS) $(CFLAGS)
OUTFILE = alienfxd

ifeq ($(DEBUG), 1)
CFLAGS += -ggdb
endif

main: main.o alienfx.o
-include $(DEPS)


#.c.o:
	#@echo [Compile] $<
	#@$(CC) -c $(CFLAGS) $< -o $@

install: uninstall
	@echo [Install] $<
	@cp -f lightdaemon /etc/init.d/lightdaemon
	@chmod +755 /etc/init.d/lightdaemon
	@cp -f alienfxd /usr/sbin/alienfxd

uninstall:
	@rm /etc/init.d/lightdaemon
	@rm /usr/sbin/alienfxd

clean:
	@echo [clean] $<
	@rm *.o
	@rm *.d

.PHONY: clean install uninstall
