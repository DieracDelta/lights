CC = gcc
CFLAGS =`pkg-config --libs --cflags libusb-1.0`
DEPS = alienfx.h
OBJ = alienfx.o

ifeq ($(DEBUG), 1)
	CFLAGS += -ggdb
endif

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
alienfx: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)
clean:
	rm alienfx alienfx.o
