CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -pthread

all: PageReplacement

PageReplacement: PageReplacement.o
	$(CC) $(CFLAGS) -o $@ $^

PageReplacement.o: PageReplacement.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o PageReplacement
