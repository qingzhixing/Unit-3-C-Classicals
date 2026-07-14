# Makefile for 55_rsa-crypto-demo

CC      = gcc
CFLAGS  = -O0 -Wall
TARGET  = rsa_crypto
SRC     = rsa_crypto.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all  clean
