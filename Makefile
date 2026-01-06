CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
TARGET = switch

SRC = src/main.c src/frame.c src/record.c src/tools.c src/command.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)

doc:
	doxygen Doxyfile