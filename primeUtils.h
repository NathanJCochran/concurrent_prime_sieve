#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>

#define DFL_CONCUR 1
#define NUM_COLUMNS 9 

struct options {
    unsigned int max;
    int concr;
    int q_set;
};

void get_options(int argc, char **argv, struct options * opts);
int get_concurrency(char *optarg);
unsigned int get_max_prime(char *optarg);
void print_primes(int *bits, int max);
void set_bit(int *array, int pos);
void clear_bit(int *array, int pos);
int check_bit(int *array, int pos);
