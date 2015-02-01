/*
Copyright (c) 2015 Steven Haid

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//
// knapsack algorithm comparison
//

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ITEM    100
#define MAX_VALUE  1000
#define MAX_WEIGHT 1000

typedef struct {
    int value;
    int weight;
} item_t;

item_t item[MAX_ITEM+1];
int    max_item;
bool   taken[MAX_ITEM+1];

int top_down(int num_item, int capacity, bool dynamic_programming);
int bottom_up(int num_item, int capacity);
int approximation(int num_item, int capacity);

void print_results(char * s, int value, long duration_us);
long microsec_timer(void);

// ----------------------------------------------------------------------  

int main(int argc, char ** argv) 
{
    int value, i, avg_item_weight, avg_item_value;
    long start_us, duration_us;

    // loop over different number of item
    for (max_item = 10; max_item <= MAX_ITEM; max_item += 10) {
        // generate array of item weights and values, these are generated
        // based on max_item so that not all items will fit in the knapsack
        avg_item_weight = 0;
        avg_item_value = 0;
        for (i = 1; i <= max_item; i++) {
            item[i].value  = 4L * MAX_VALUE  / max_item * rand() / RAND_MAX + 1;
            item[i].weight = 4L * MAX_WEIGHT / max_item * rand() / RAND_MAX + 1;
            avg_item_weight += item[i].weight;
            avg_item_value  += item[i].value;
        }
        avg_item_weight /= max_item;
        avg_item_value  /= max_item;

        // print info on this group of tests
        printf("max_item=%d max_weight=%d avg_item_weight=%d avg_item_value=%d\n",
               max_item, MAX_WEIGHT, avg_item_weight, avg_item_value);

        // solve via brute force, cut this off at 30 becasue it takes too long
        if (max_item <= 30) {
            start_us = microsec_timer();
            value = top_down(max_item, MAX_WEIGHT, false);
            duration_us = microsec_timer() - start_us;
            print_results("top_down_no_dp", value, duration_us);
        }

        // solve via top down dynamic prog
        start_us = microsec_timer();
        value = top_down(max_item, MAX_WEIGHT, true);
        duration_us = microsec_timer() - start_us;
        print_results("top_down_dp", value, duration_us);

        // solve via bottom up dynamic prog
        start_us = microsec_timer();
        value = bottom_up(max_item, MAX_WEIGHT);
        duration_us = microsec_timer() - start_us;
        print_results("bottom_up", value, duration_us);

        // solve via v/w aproximation
        start_us = microsec_timer();
        value = approximation(max_item, MAX_WEIGHT);
        duration_us = microsec_timer() - start_us;
        print_results("approximation", value, duration_us);

        printf("\n");
    }

    return 0;
}

void print_results(char * s, int value, long duration_us)
{
    int num_taken=0, weight_taken=0, value_taken=0;
    int i;

    // determine the number of items taken and the weight & value taken
    for (i = 1; i <= max_item; i++) {
        if (taken[i]) {
            num_taken++;
            weight_taken += item[i].weight;
            value_taken += item[i].value;
        }
    }

    // print result
    printf("  %-14s duration=%ld.%06ld taking num=%-2d weight=%-4d value=%-4d\n",
        s, 
        duration_us/1000000, duration_us%1000000,
        num_taken,
        weight_taken,
        value_taken);

    // validate that the value returned is consistent with the total
    // values of the items selected
    if (value != value_taken) {
        printf("ERROR value=%d not equal value_taken=%d\n",
            value, value_taken);
        exit(1);
    }
}

long microsec_timer(void)
{
    static int first_call = 1;
    static struct timespec tf;
    struct timespec t;

    if (first_call) {
        clock_gettime(CLOCK_MONOTONIC_RAW,&tf);
        first_call = 0;
        return 0;
    }

    clock_gettime(CLOCK_MONOTONIC_RAW,&t);
    return (t.tv_sec - tf.tv_sec) * 1000000 +
           (t.tv_nsec - tf.tv_nsec) / 1000;
}

// ----------------------------------------------------------------------  

// these are used by the top_down and bottom_up routines
int  saved_value[MAX_ITEM+1][MAX_WEIGHT+1];
bool saved_taken[MAX_ITEM+1][MAX_WEIGHT+1];

int top_down(int num_item, int capacity, bool dynamic_programming)
{
    int value_take=0, value_not_take;
    bool first_call = (num_item == max_item);

    // in first call reset the saved value arrays
    if (first_call) {
        bzero(saved_value, sizeof(saved_value));
        bzero(saved_taken, sizeof(saved_taken));
    }

    // check for end of recursion
    if (num_item == 0) {
        return 0;
    }

    // when dynamic programming is selected then we make use of the
    // saved value from solved subproblem
    if (dynamic_programming && saved_value[num_item][capacity]) {
        return saved_value[num_item][capacity];
    }

    // determine the value if we take or don't take the item
    if (capacity >= item[num_item].weight) {
        value_take = item[num_item].value + 
                     top_down(num_item-1, capacity-item[num_item].weight,
                     dynamic_programming);
    }
    value_not_take = top_down(num_item-1, capacity, dynamic_programming);

    // check which is better, to take or not to take
    if (value_take > value_not_take) {
        saved_value[num_item][capacity] = value_take;
        saved_taken[num_item][capacity] = true;
    } else {
        saved_value[num_item][capacity] = value_not_take;
        saved_taken[num_item][capacity] = false;
    }

    // if about to return on the original call then scan through the 
    // saved_taken array to determine which items were taken
    if (first_call) {
        int c=MAX_WEIGHT, n;
        for (n = max_item; n > 0; n--) {
            if (saved_taken[n][c]) {
                taken[n] = true;
                c -= item[n].weight;
            } else {
                taken[n] = false;
            }
        }
    }

    // return the value
    return saved_value[num_item][capacity];
}

int bottom_up(int num_item, int capacity)
{
    int value_take, value_not_take, i, c;

    // init the saved value arrays
    bzero(saved_value, sizeof(saved_value));
    bzero(saved_taken, sizeof(saved_taken));

    // loop over items and capacity
    for (i = 1; i <= num_item; i++) {
        for (c = 0; c <= capacity; c++) {
            // if we can take this item then determine value if taken
            if (item[i].weight <= c) {
                value_take = item[i].value +
                             saved_value[i-1][c-item[i].weight];
            } else {
                value_take = 0;
            }

            // if we don't take this item then determine value for not taking
            value_not_take = saved_value[i-1][c];

            // check which is better, to take or not to take
            if (value_take > value_not_take) {
                saved_value[i][c] = value_take;
                saved_taken[i][c] = true;
            } else {
                saved_value[i][c] = value_not_take;
                saved_taken[i][c] = false;
            }
        }
    }

    // scan through the saved_taken array to determine which
    // items were taken
    for (c = capacity, i = num_item; i > 0; i--) {
        if (saved_taken[i][c]) {
            taken[i] = true;
            c -= item[i].weight;
        } else {
            taken[i] = false;
        }
    }

    // return value
    return saved_value[num_item][capacity];
}

int approximation(int num_item, int capacity)
{
    int x[num_item+1];
    int max, max_i, i;
    int value;

    // init
    value = 0;
    memset(taken, 0, sizeof(taken));

    // init array 'x' which is the ratio of value to weight
    for (i = 1; i <= num_item; i++) {
        x[i] = 1000000 * item[i].value / item[i].weight;
    }

    // loop
    while (true) {
        // find the largest value to weight ratio
        max = max_i = 0;
        for (i = 1; i <= num_item; i++) {
            if (x[i] > max) {
                max = x[i];
                max_i = i;
            }
        }

        // if none found then break
        if (max_i == 0) {
            break;
        }

        // zero the value-to-weight entry because we're now done with it
        x[max_i] = 0;

        // if the found item doesn't fit then continue, maybe
        // there will be others found that will still fit
        if (item[max_i].weight > capacity) {
            continue;
        }

        // update capacity and value and set taken flag
        capacity     -= item[max_i].weight;
        value        += item[max_i].value;
        taken[max_i]  = true;
    }

    // return the total value
    return value;
}
    
