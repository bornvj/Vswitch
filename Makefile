CC = gcc
CFLAGS = -g -O0 -fsanitize=address -fno-omit-frame-pointer
LDFLAGS = -fsanitize=address
TARGET = switch

SRC = src/main.c src/frame.c src/record.c src/tools.c src/command.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)

doc:
	doxygen Doxyfile