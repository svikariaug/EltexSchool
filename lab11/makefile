CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_XOPEN_SOURCE=500
LDFLAGS = -pthread
TARGET = task
SRC = task.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean