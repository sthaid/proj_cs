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
// travelling sales person algorithm comparison
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <time.h>

// defines

#define MAX_CITY                  64
#define MAX_CITY_BRUTE_FORCE      13
#define MAX_CITY_DYN_PROG         22
#define MAX_CITY_NEAREST_NEIGHBOT MAX_CITY
#define MAX_CITY_BOUNDED          MAX_CITY

#define MAX_ALG (sizeof(alg_table)/sizeof(alg_t))

// typedefs

typedef struct {
    int (*proc)(int start_city, long visit_city);
    char * name;
    int max_city;
} alg_t;

typedef struct {
    int x;
    int y;
    int dist[MAX_CITY];
    int sort[MAX_CITY];
} city_t;

// prototypes

long microsec_timer(void);
inline void swap(int * a, int * b);

int tsp_brute_force(int start_city, long visit_city_bits);
int tsp_dyn_prog(int start_city, long visit_city_bits);
int tsp1(int start_city, long visit_city_bits, bool dyn_prog);

int tsp_nearest_neighbor(int start_city, long visit_city_bits);
int tsp_bounded(int start_city, long visit_city_bits);
int tsp2(int start_city, long visit_city_bits, int num_cities_to_check, int * city_chosen);

// variables

city_t city[MAX_CITY];
int    max_city;

alg_t alg_table[] = {
    { tsp_brute_force,      "brute_force",       MAX_CITY_BRUTE_FORCE },
    { tsp_dyn_prog,         "dyn_prog",          MAX_CITY_DYN_PROG },
    { tsp_bounded,          "bounded",           MAX_CITY_BOUNDED },
    { tsp_nearest_neighbor, "nearest_neighbor",  MAX_CITY_NEAREST_NEIGHBOT },
        };

// -----------------  MAIN  ---------------------------------------------------

int main(int argc, char **argv)
{
    int i, j, k;

    // set line buffering
    setlinebuf(stdout);

    // get number of city
    if (argc != 2 || 
        sscanf(argv[1], "%d", &max_city) != 1 ||
        max_city < 2 || max_city > MAX_CITY)
    {
        printf("usage: tsp <max_city>\n"
               "  max_city in range 2 to %d\n",
               MAX_CITY);
        exit(1);
    }

    // generate locations of city in 1000x1000 coordinate grid
    for (i = 0; i < max_city; i++) {
        city[i].x = (long)rand() * 1000 / RAND_MAX;
        city[i].y = (long)rand() * 1000 / RAND_MAX;
    }

    // gernerate distances to other cities
    for (i = 0; i < max_city; i++) {
        for (j = 0; j < max_city; j++) {
            int dx = city[i].x - city[j].x;
            int dy = city[i].y - city[j].y;
            city[i].dist[j] = sqrt(dx*dx + dy*dy);
        }
    }

    // generate list of other cities sorted by distance
    for (i = 0; i < max_city; i++) {
        int dist[max_city], min_idx, min_dist;

        memcpy(dist, city[i].dist, sizeof(dist));
        for (j = 0; j < max_city; j++) {
            city[i].sort[j] = j;
        }

        for (j = 0; j < max_city; j++) {
            min_dist = min_idx = -1;
            for (k = j; k < max_city; k++) {
                if (min_idx == -1 || dist[k] < min_dist) {
                    min_idx = k;
                    min_dist = dist[k];
                }
            }
            if (min_idx == -1) {
                printf("ERROR min_idx is -1\n");
                exit(1);
            }
            swap(&dist[j], &dist[min_idx]);
            swap(&city[i].sort[j], &city[i].sort[min_idx]);
        }
    }

#if 0
    // debug print city info
    for (i = 0; i < max_city; i++) {
        printf("city[%d] %3d,%3d: ", i, city[i].x, city[i].y);
        for (j = 0; j < max_city; j++) {
            printf("%3d ", city[i].dist[j]);
        }
        printf("\n");
        printf("    ");
        for (j = 0; j < max_city; j++) {
            printf("%3d ", city[i].sort[j]);
        }
        printf("\n");
    }
#endif

    // loop over tsp algorithms
    printf("max_city %d ...\n", max_city);
    for (i = 0; i < MAX_ALG; i++) {
        alg_t * alg = &alg_table[i];
        bool do_run;
        int distance=0;
        long start_time, duration=0;

        // run the algorithm
        do_run = (max_city <= alg->max_city);
        if (do_run) {
            int start_city = 0;
            long visit_city_bits = (2 * (1L << (max_city-1)) -1) & ~(1L << start_city);

            start_time = microsec_timer();
            distance = alg->proc(start_city, visit_city_bits);
            duration = microsec_timer() - start_time;
        }

        // print result
        if (do_run) {
            printf("  %-16s %12d %3ld.%06ld\n",
                   alg->name, distance,
                   duration/1000000, duration%1000000);
        } else {
            printf("  %-16s %12s\n",
                   alg->name, "not_run");
        }
    }

    return 0;
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

inline void swap(int * a, int * b) 
{
    int tmp = *a;

    *a = *b;
    *b = tmp;
}


// -----------------  TSP ALG: BRUTE FORCE AND DYN_PROG  ---------------------------

int saved_solution[MAX_CITY_DYN_PROG][1L<<MAX_CITY_DYN_PROG];

int tsp_brute_force(int start_city, long visit_city_bits)
{
    return tsp1(start_city, visit_city_bits, false);
}

int tsp_dyn_prog(int start_city, long visit_city_bits)
{
    return tsp1(start_city, visit_city_bits, true);
}

int tsp1(int start_city, long visit_city_bits, bool dyn_prog)
{
    int next_city, distance;
    int min_distance = INT32_MAX;

    // if dynamic programming and first call then zero saved_solution
    if (dyn_prog && start_city == 0) {
        int i;
        for (i = 0; i < max_city; i++) {
            bzero(saved_solution[i], (1L<<max_city)*sizeof(int));
        }
    }

    // verify start_city not in visit_city_bits
    if (visit_city_bits & (1L << start_city)) {
        printf("ERROR: start_city=%d visit_city_bits=0x%lx\n",
               start_city, visit_city_bits);
        exit(1);
    }

    // if no cities to be visitted then return 0 distance
    if (visit_city_bits == 0) {
        return 0;
    }

    // if using dynamic programming and saved solution available then return that
    if (dyn_prog && saved_solution[start_city][visit_city_bits]) {
        return saved_solution[start_city][visit_city_bits];
    }

    // loop over all cities to be visitted and find minimum distance
    for (next_city = 0; next_city < max_city; next_city++) {
        // if we don't visit this city then contine
        if ((visit_city_bits & (1L << next_city)) == 0) {
            continue;
        }

        // determine distance for the path starting at next_city
        distance = city[start_city].dist[next_city] +
                   tsp1(next_city, visit_city_bits & ~(1L << next_city), dyn_prog);

        // if this is a minimum distance then save it
        if (distance < min_distance) {
            min_distance = distance;
        }
    }

    // save this solution
    if (dyn_prog) {
        saved_solution[start_city][visit_city_bits] = min_distance;
    }

#if 0
    // debug print the number of used slots in the saved_solution array
    if (dyn_prog && start_city == 0) {
        long sc, vsb, used=0, not_used=0;;
        
        for (sc = 0; sc < max_city; sc++) {
            for (vsb = 0; vsb < (1L<<max_city); vsb++) {
                if (saved_solution[sc][vsb] != 0) {
                    used++;
                } else {
                    not_used++;
                }
            }
        }
        printf("saved_solution entries used=%ld not_used=%ld percent_used=%ld\n",
               used, not_used, 100 * used / (used + not_used));
    }
#endif

    // return the minimum distance
    return min_distance;
}

// -----------------  TSP ALG: APPROXIMATIONS  -------------------------------------

int tsp_nearest_neighbor(int start_city, long visit_city_bits)
{
    return tsp2(start_city, visit_city_bits, 1, NULL);
}

int tsp_bounded(int start_city, long visit_city_bits)
{
    int route_city = start_city;
    int total_distance = 0;
    int city_chosen;

    while (visit_city_bits) {
        // determine the next city on the route, using N nearest neighbors on 
        // first call to tsp2, and N-1 on subsequent recursion
        tsp2(route_city, visit_city_bits, 10, &city_chosen);

        // validate city_chosen
        if (city_chosen < 1 || city_chosen >= max_city) {
            printf("ERROR city_chosen %d\n", city_chosen);
            exit(1);
        }

#if 0
        // debug print
        printf("%d -> %d dist=%d  total_dist=%d\n",
            route_city, city_chosen, 
            city[route_city].dist[city_chosen], 
            total_distance + city[route_city].dist[city_chosen]);
#endif

        // update vars to prepare for next loop, which will determine
        // the next city on the route
        total_distance += city[route_city].dist[city_chosen];
        route_city = city_chosen;
        visit_city_bits &= ~(1L << route_city);
    }

    // return total_distance
    return total_distance;
}

int tsp2(int start_city, long visit_city_bits, int num_cities_to_check, int * city_chosen)
{
    int i, next_city, distance;
    int num_cities_checked = 0;
    int min_distance = INT32_MAX;
    int best_city = -1;

    // verify start_city not in visit_city_bits
    if (visit_city_bits & (1L << start_city)) {
        printf("ERROR: start_city=%d visit_city_bits=0x%lx\n",
               start_city, visit_city_bits);
        exit(1);
    }

    // verify num_cities_to_check 
    if (num_cities_to_check < 1) {
        printf("ERROR: num_cities_to_check %d\n", num_cities_to_check);
        exit(1);
    }

    // if no cities to be visitted then return 0 distance
    if (visit_city_bits == 0) {
        if (city_chosen != NULL) {
            *city_chosen = best_city;
        }
        return 0;
    }

    // loop over all cities to be visitted and find minimum distance
    for (i = 0; i < max_city; i++) {
        // set next_city from the list of cities sorted by distance
        next_city = city[start_city].sort[i];

        // if we don't visit this city then contine
        if ((visit_city_bits & (1L << next_city)) == 0) {
            continue;
        }

        // determine distance using next_city
        distance = city[start_city].dist[next_city] + 
                   tsp2(next_city, 
                        visit_city_bits & ~(1L << next_city),
                        num_cities_to_check > 1 ? num_cities_to_check-1 : 1,
                        NULL);

        // if this is a minimum distance then save it
        if (distance < min_distance) {
            min_distance = distance;
            best_city = next_city;
        }

        // if number of cities checked is at limit then break
        if (++num_cities_checked == num_cities_to_check) {
            break;
        }
    }

    // verify best_city is set
    if (best_city == -1) {
        printf("ERROR best_city not set\n");
        exit(1);
    }

    // return the minimum distance and city_chosen
    if (city_chosen != NULL) {
        *city_chosen = best_city;
    }
    return min_distance;
}

