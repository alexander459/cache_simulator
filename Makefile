CC=gcc
CFLAGS=-g

all: generator simulator

generator: src/generator.o
	$(CC) $(CFLAGS) $^ -o $@

simulator: src/simulator.o
	$(CC) $(CFLAGS) $^ -o $@

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f ./src/*.o
	-rm -f simulator generator
