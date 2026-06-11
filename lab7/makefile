CC = gcc
CFLAGS = -Wall -Wextra
TARGET = calculator
LIB = libcalc.a
OBJS = main.o add.o sub.o mul.o div.o

all: $(TARGET)

$(TARGET): $(OBJS)
	ar rcs $(LIB) add.o sub.o mul.o div.o
	$(CC) $(CFLAGS) main.o -L. -lcalc -o $(TARGET)

%.o: %.c calc.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(LIB) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run