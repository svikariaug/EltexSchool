CC = gcc

CFLAGS = -Wall -Wextra -fPIC

LDFLAGS = -L.

TARGET = calculator

LIB = libcalc.so

LIB_OBJS = add.o sub.o mul.o div.o

MAIN_OBJ = main.o

all: $(LIB) $(TARGET)

$(LIB): $(LIB_OBJS)
	$(CC) -shared -o $(LIB) $(LIB_OBJS)

add.o: add.c calc.h
	$(CC) $(CFLAGS) -c add.c -o add.o

sub.o: sub.c calc.h
	$(CC) $(CFLAGS) -c sub.c -o sub.o

mul.o: mul.c calc.h
	$(CC) $(CFLAGS) -c mul.c -o mul.o

div.o: div.c calc.h
	$(CC) $(CFLAGS) -c div.c -o div.o

$(MAIN_OBJ): main.c calc.h
	$(CC) $(CFLAGS) -c main.c -o $(MAIN_OBJ)

$(TARGET): $(MAIN_OBJ)
	$(CC) $(MAIN_OBJ) $(LDFLAGS) -lcalc -o $(TARGET)

run: $(TARGET)
	LD_LIBRARY_PATH=. ./$(TARGET)

clean:
	rm -f $(LIB_OBJS) $(MAIN_OBJ) $(LIB) $(TARGET)

.PHONY: all clean rebuild run install