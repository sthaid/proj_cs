
TARGETS = knapsack sort tsp

CC = gcc
CFLAGS = -g -O2 -Wall -lm -lrt

all: $(TARGETS)

#
# build rules
#

knapsack: knapsack.c
	$(CC) $(CFLAGS) $< -o $@

sort: sort.c
	$(CC) $(CFLAGS) $< -o $@

tsp: tsp.c
	$(CC) $(CFLAGS) $< -o $@

#
# clean rule
#

clean:
	rm -f $(TARGETS)

