CXX=g++
CC=gcc
INCDIR=../src
CFLAGS=-Wall -I$(INCDIR) -g


all: pqtest

priority_queue.o: ../src/priority_queue.c 
	$(CC) $(CFLAGS) -c -o $@ $^

pqtest: priority_queue_test.c priority_queue.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm pqtest