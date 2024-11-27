CC = gcc
CFLAGS = -Wall -pthread
LDFLAGS = -pthread

PROGRAMS = q1 q2 q3 q4a q4b

all: $(PROGRAMS)

q1: q1.c
	$(CC) $(CFLAGS) -o $@ $^

q2: q2.c
	$(CC) $(CFLAGS) -o $@ $^ 

q3: q3.c
	$(CC) $(CFLAGS) -o $@ $^ 

q4a: q4a.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

q4b: q4b.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

clean:
	rm -f $(PROGRAMS)

.PHONY: all clean