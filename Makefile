.POSIX:
.PHONY: all clean

SOURCE = write_gpt.c
TARGET = write_gpt

CC = gcc 
CFLAGS = \
	 -std=c17 \
	 -Wall \
	 -Wextra \
	 -Wpedantic \
	 -O2

all: $(TARGET)
clean:
	rm -f $(TARGET).exe
