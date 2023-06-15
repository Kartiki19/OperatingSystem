CC = g++
CFLAGS = -std=c++11 -Wall -Wextra -pthread

all: prodcon

prodcon: prodcon.o
	$(CC) $(CFLAGS) -o $@ $^

prodcon.o: prodcon.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o prodcon
