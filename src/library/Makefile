CC = gcc
OBJ = alienfx.o
STATIC = libalienfx.a
SOURCES  := $(wildcard *.c)
INCLUDES := $(wildcard *.h)
OBJECTS  := $(SOURCES:.c=*.o)
PREFIX = /usr/lib
LIBS = -libusb-1.0

ifeq ($(DEBUG), 1)
CFLAGS += -ggdb
endif

$(STATIC): $(OBJECTS)
	@echo "[Link (Static)]"
	@ar rcs $@ $^

#$@ is the name of the file being generated, and $< the first prerequisite (usually the source file), and $^ refers to all files being evaluated
.c.o:
	@echo [Compile] $<
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY: install
install: $(STATIC)
	@install -m 0755 $< ${DESTDIR}${PREFIX}
	cp alienfx.h /usr/include
clean:
	rm *.a *.o
