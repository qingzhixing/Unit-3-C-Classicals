# Makefile for 53_raytracer-from-scratch

CC      = gcc
CFLAGS  = -O0
TARGET  = raytracer
SRC     = raytracer.c
LDLIBS  = -lm

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: all  clean
