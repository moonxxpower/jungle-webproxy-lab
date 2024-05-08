CC = gcc
CFLAGS = -O2 -Wall -I ..

all: adder adder_homework

adder: adder.c
	$(CC) $(CFLAGS) -o adder adder.c

adder_homework: adder_homework.c
	$(CC) $(CFLAGS) -o adder_homework adder_homework.c

clean:
	rm -f adder adder_homework *~
