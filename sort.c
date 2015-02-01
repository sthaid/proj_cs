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
// sort algorithm comparison
//

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//#define QUICK_SORT_UNIT_TEST

#define MAX_ELEMENTS 100000000  // 100 million

int initial_array[MAX_ELEMENTS];
int array[MAX_ELEMENTS];

bool sorted_okay(int * array, int elements, int cksum_expected);
int  checksum(int * array, int elements);
void print_array(char * s, int * array, int elements, int pivot_idx);
long microsec_timer(void);

void bubble_sort(int * array, int elements);
void selection_sort(int * array, int elements);
void insertion_sort(int * array, int elements);
void quick_sort(int * array, int elements);
void merge_sort(int * array, int elements);

#ifdef QUICK_SORT_UNIT_TEST
void quick_sort_unit_test();
#endif

// -----------------  MAIN  ----------------------------------------------

int main(int argc, char **argv)
{
    int i, alg, elements, cksum;
    long start_us, duration_us;

    static struct {
        char * name;
        void (*proc)(int * array, int elements);
        int max_elements_to_test;
    } alg_table[] = {
          { "Selection", selection_sort,   100000 },
          { "Insertion", insertion_sort,   100000 },
          { "Bubble",    bubble_sort,      100000 },
          { "Quick",     quick_sort,    100000000 },  // 100 million
          { "Merge",     merge_sort,    100000000 },  // 100 million
                                   };

    #define MAX_ALG (sizeof(alg_table) / sizeof(alg_table[0]))

    // set line buffering
    setlinebuf(stdout);

#ifdef QUICK_SORT_UNIT_TEST
    // unit test quick sort with small sort arrays
    quick_sort_unit_test();
#endif

    // initialize the array to be sorted with random values
    for (i = 0; i < MAX_ELEMENTS; i++) {
        initial_array[i] = rand(); 
    }

    // loop over sort algorithms
    for (alg = 0; alg < MAX_ALG; alg++) {
        // verify max_alements_to_test in range
        if (alg_table[alg].max_elements_to_test > MAX_ELEMENTS) {
            printf("max_elements_to_test %d too big\n",
                   alg_table[alg].max_elements_to_test);
            exit(10);
        }
    
        // loop over number of elements to sort
        printf("%s Sort ...\n", alg_table[alg].name);
        for (elements = 100; 
             elements <= alg_table[alg].max_elements_to_test; 
             elements *= 10) 
        {
            // init array to be sorted, and compute checksum
            memcpy(array, initial_array, elements*sizeof(int));
            cksum = checksum(array, elements);

            // sort array
            start_us = microsec_timer();
            alg_table[alg].proc(array,elements);
            duration_us = microsec_timer() - start_us;

            // print time it took to do the sort
            printf("%10d: %3d.%06d\n",
                   elements,
                   (int)(duration_us/1000000), (int)(duration_us%1000000));

            // check the sorted result
            if (!sorted_okay(array, elements, cksum)) {
                exit(1);
            }                
        }
        printf("\n");
    }

    // exit
    return 0;
}

bool sorted_okay(int * array, int elements, int cksum_expected)
{
    int i, cksum_actual;

    for (i = 0; i < elements-1; i++) {
        if (array[i] > array[i+1]) {
            printf("ERROR array[%d] = %d %d\n",
                i, array[i], array[i+1]);
            return false;
        }
    }

    cksum_actual = checksum(array, elements);
    if (cksum_expected != cksum_actual) {
        printf("ERROR cksum expected=%d actual=%d\n",
            cksum_expected, cksum_actual);
        return false;
    }

    return true;
}

int checksum(int * array, int elements)
{
    int sum=0, i;

    for (i = 0; i < elements; i++) {
        sum += array[i];
    }
    return sum;
}

void print_array(char * s, int * array, int elements, int pivot_idx)
{
    int i;

    printf("%s: ", s);
    for (i = 0; i < elements; i++) {
        if (i == pivot_idx) {
            printf("*%d* ", array[i]);
        } else {
            printf("%d ", array[i]);
        }
    }
    printf("\n");
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

// -----------------  SORT ROUTINES  -------------------------------------

inline void swap(int * x, int * y) 
{
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

void bubble_sort(int * array, int elements)
{
    bool swaps;
    int  i;

    // return if no sorting needed
    if (elements <= 1) {
        return;
    }

    // sort by swapping neighbots until a full scan of 
    // the array is accomplished without any swaps
    do {
        swaps = false;
        for (i = 0; i < elements-1; i++) {
            if (array[i] > array[i+1]) {
                swap(&array[i], &array[i+1]);
                swaps = true;
            }
        }
    } while (swaps);
}

void selection_sort(int * array, int elements)
{
    int start, i, min_val, min_idx;

    // return if no sorting needed
    if (elements <= 1) {
        return;
    }

    // search the array for the minimum value, and swap the minimum value
    // with the value at the start of array;  increment start of array and repeat
    for (start = 0; start < elements-1; start++) {
        min_val = array[start];
        min_idx = start;
        for (i = start+1; i < elements; i++) {
            if (array[i] < min_val) {
                min_val = array[i];
                min_idx = i;
            }
        }
        swap(&array[start], &array[min_idx]);
    }
}

void insertion_sort(int * array, int elements)
{
    int i,j,val;

    // return if no sorting needed
    if (elements <= 1) {
        return;
    }

    // inspect successive elements of the array for an element that is out 
    // order;  when an out of order element is found then scan backward to 
    // find where it belongs;  move memory to moake room for the out of
    // order element and place the out of order element where it belongs
    for (i = 1; i < elements; i++) {
        // if element is in correct order then continue
        if (array[i] >= array[i-1]) {
            continue;
        }

        // val is the out of order element
        val = array[i];

        // search backward until correct place found for val;
        // when done with this search, j index is the desired location
        for (j = i-1; j > 0; j--) {
            if (val >= array[j-1]) {
                break;
            }
        }

        // move memory to make space at location j for val
        memmove(array+j+1, array+j, (i-j)*sizeof(int));

        // place val at location j
        array[j] = val;
    }
}

void quick_sort(int * array, int elements)
{
    int left_idx, right_idx;
    int pivot_val;
    int pivot_idx;

#if 0
    // if number of elements is small then sort via insertion sort
    if (elements < 10) {
        insertion_sort(array, elements);
        return;
    }
#else
    // if no sorting needed then return
    if (elements <= 2) {
        if (elements == 2 && array[0] > array[1]) {
            swap(&array[0], &array[1]);
        }
        return;
    }
#endif

    // XXX printf("%d - %d %d %d\n", elements, array[0], array[1], array[2]);

    // select pivot value, and swap it to the end
    pivot_val = array[elements/2];
    swap(&array[elements/2], &array[elements-1]);

    // reorder array such that values less than pivot are on the left and
    // values >= pivot are on the right
    left_idx = 0;
    right_idx = elements-2;
    while (true) {
        if (left_idx == right_idx) {
            break;
        }
        if (array[left_idx] >= pivot_val) {
            swap(&array[left_idx], &array[right_idx]);
            right_idx--;
        } else {
            left_idx++;
        }
    }

    // determine the pivot_idx, this is the index at which 
    // all array elements from pivot_idx and above are >= pivot_val
    //
    // we know ...
    // - everything below the left_idx is < pivot_val, because left_idx
    //   is incremnted on that condition
    // - everything above right_idx is >= pivot_val, because values 
    //   were swapped on that condition
    // - left_idx == right_idx
    //
    // we don't know the state of the value of left_idx; so all that is
    // needed to set the pivot_idx is to check the state of array[left_idx]
    pivot_idx = (array[left_idx] > pivot_val
                 ? left_idx : left_idx + 1);
    // XXX printf("pv %d  li %d  pi %d\n", pivot_val, left_idx, pivot_idx);

    // quick sort the left and the right
    quick_sort(array, pivot_idx);
    quick_sort(array+pivot_idx, elements-pivot_idx);
}

void merge_sort(int * array, int elements)
{
    int left_idx, right_idx, merge_idx;
    int * merged;

#if 0
    // if number of elements is small then sort via insertion sort
    if (elements < 10) {
        insertion_sort(array, elements);
        return;
    }
#else
    // if no sorting needed then return
    if (elements <= 1) {
        return;
    }
#endif

    // sort the left half and the right half seperately
    merge_sort(array, elements/2);
    merge_sort(array+elements/2, elements-elements/2);


    // merge the sorted left and right halfs into a single sorted list
    merged = malloc(elements * sizeof(int));
    left_idx = 0; 
    right_idx = elements/2;
    merge_idx = 0;
    while (true) {
        // if merged array is full then we're done
        if (merge_idx == elements) {
            break;
        }

        // if nothing remains on the left side then 
        // copy the remaining from the right side, and
        // we're done
        if (left_idx == elements/2) {
            memcpy(&merged[merge_idx], 
                   &array[right_idx], 
                   (elements-merge_idx)*sizeof(int));
            break;
        }

        // if nothing remains on the right side then 
        // copy the remaining from the left side, and
        // we're done
        if (right_idx == elements) {
            memcpy(&merged[merge_idx], 
                   &array[left_idx], 
                   (elements-merge_idx)*sizeof(int));
            break;
        }

        // take value from the left side or right side as appropriate
        if (array[left_idx] < array[right_idx]) {
            merged[merge_idx++] = array[left_idx++];
        } else {
            merged[merge_idx++] = array[right_idx++];
        }
    }
    memcpy(array, merged, elements * sizeof(int));
    free(merged);
}

// -----------------  QUICK SORT UNIT TEST  ------------------------------

#ifdef QUICK_SORT_UNIT_TEST
int rand_n(int n)
{
    return (long)rand() * n / RAND_MAX;
}


void quick_sort_unit_test()
{
    int i, j;
    int elements;
    uint64_t count=0;

    printf("%s starting\n", __func__);

    while (true) {
        elements = rand_n(6);
        for (i = 0; i < elements; i++) {
            array[i] = rand_n(6);
        }

        quick_sort(array,elements);

        for (i = 0; i < elements-1; i++) {
            if (array[i] > array[i+1]) {
                for (j = 0; j < elements; j++) {
                    printf("%d ", array[j]);
                }
                printf("  OOPS\n");
                exit(1);
            }
        }

        count++;
        if ((count % 1000000) == 0) {
            printf("%s ok count %d million\n", __func__, (int)(count/1000000));
        }
    }
}
#endif
