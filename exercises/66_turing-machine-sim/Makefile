# Makefile for 66_turing-machine-sim
#
# TODO: Fill in all fields marked with ??? and implement the build rules.
#       完成后删除本 TODO 行。

CC      = gcc
CFLAGS  = -O0
TARGET  = turing_machine
SRC     = turing_machine.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET)

.PHONY: all  clean
