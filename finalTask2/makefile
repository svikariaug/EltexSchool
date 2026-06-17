CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = taxi
SRC = taxi.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean run run-file