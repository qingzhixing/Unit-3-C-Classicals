# Makefile for 49_dining-philosophers-sync
#
# 多线程程序工程化要点：编译期和链接期都要告诉工具链"这是线程程序"。
#   - 编译期 -pthread：定义 _REENTRANT，让某些 libc 函数走线程安全版本
#   - 链接期 -pthread：链接 pthread 库（现代 glibc 已合入 libc，但仍需此标志）

CC      = gcc
CFLAGS  = -pthread
TARGET  = dining_philosophers
SRC     = dining_philosophers.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all test clean
