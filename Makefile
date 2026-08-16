CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -Iinclude

TARGET = z33

SRC = src/main.c \
      src/cpu.c \
      src/memory.c \
      src/executor.c \
      src/parser.c \
	  src/exception.c \
	  src/arithmetic-instructions.c \
	  src/bitwise-instructions.c 

OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: clean