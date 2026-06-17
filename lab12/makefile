CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGETS = task1 task2_client task2_server task3

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

run1: task1
	./task1

run2: task2_server task2_client
	./task2_server &
	sleep 1
	./task2_client
	wait

run3: task3
	./task3

run: run1 run2 run3

clean:
	rm -f $(TARGETS)

.PHONY: all clean run1 run2 run3 run