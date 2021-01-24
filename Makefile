
TARGETS = knapsack sort tsp

CC = gcc
CFLAGS = -g -O2 -Wall

all: $(TARGETS)

#
# build rules
#

knapsack: knapsack.c
	$(CC) $(CFLAGS) $< -o $@ -lm -lrt

sort: sort.c
	$(CC) $(CFLAGS) $< -o $@ -lm -lrt

tsp: tsp.c
	$(CC) $(CFLAGS) $< -o $@ -lm -lrt

#
# clean rule
#

clean:
	rm -f $(TARGETS)

