CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGETS = task1 task2 task3

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

run1: task1
	./task1

run2: task2
	./task2

run3: task3
	./task3

run: $(TARGETS)
	./task1
	./task2
	./task3

clean:
	rm -f $(TARGETS)

.PHONY: all clean run1 run2 run3 run