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
      src/bitwise-instructions.c \
      src/runner.c \
      src/cmp-branch-instructions.c

OBJ = $(SRC:src/%.c=obj/%.o)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj $(TARGET)

.PHONY: clean