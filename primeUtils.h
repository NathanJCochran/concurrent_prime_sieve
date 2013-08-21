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
#define NUM_COLUMNS 10 

//Structs:
struct sieve_args {
    int * psbl_primes;
    unsigned int max;
    unsigned int low;
    unsigned int high;
};

struct options {
    unsigned int max;
    int concurrency;
    int q_set;
};


void get_options(int argc, char **argv, struct options *opts);
int get_concurrency(char *concr_str);
unsigned int get_max_prime(char *max_str);
void print_primes(int *primes, int max);
unsigned int get_low(int num_threads, unsigned int max, int thread_num);
unsigned int get_high(int num_threads, unsigned int max, int thread_num);
unsigned int get_interval(int num_threads, unsigned int max);
unsigned int get_sqrt_32(unsigned int max);
void pre_sieve(int *psbl_primes, unsigned int max);
void thread_sieve(struct sieve_args *sa);
void sieve_multiples(int *psbl_primes, unsigned int factor, unsigned int max);
void sieve_multiples_in_range(int *psbl_primes, unsigned int factor, unsigned int low, unsigned int high);
unsigned int get_first_multiple_gt(unsigned int factor, unsigned int low);
unsigned long get_min(unsigned long a, unsigned long b);
void set_bit(int *array, unsigned int pos);
void clear_bit(int *array, unsigned int pos);
int check_bit(int *array, unsigned int pos);
