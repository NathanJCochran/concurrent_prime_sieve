#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <pthread.h>
#include "primeUtils.h"

struct sieve_args {
    int * psbl_primes;
    unsigned int max;
};

pthread_t * create_threads(int n, int * psbl_primes, unsigned int max);
void join_threads(int n, pthread_t * threads);
void sieve(struct sieve_args * s_args);
void sieve_multiples(int * psbl_primes, int factor, unsigned int max);
void lock_mutex(pthread_mutex_t * mtx); 
void unlock_mutex(pthread_mutex_t * mtx);

static unsigned int count = 0;
static pthread_mutex_t count_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t bits_mtx = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char * argv[]) {
    int * psbl_primes;
    pthread_t * threads;
    struct options opts;
    
    //Get command line arguments:
    get_options(argc, argv, &opts);

    //Allocate heap space for the bit array:
    psbl_primes = calloc(((opts.max/32)+1), sizeof(int));
    
    //By definition, 0 and 1 are not a prime numbers:
    set_bit(psbl_primes, 0);
    set_bit(psbl_primes, 1);

    //Create threads, let finish, join:
    threads = create_threads(opts.concr, psbl_primes, opts.max);
    join_threads(opts.concr, threads);

    //Print primes:
    if(!opts.q_set) {
        print_primes(psbl_primes, opts.max);
    }

    //Free heap space used for the bit array:
    free(psbl_primes);

    return 0;
}

pthread_t * create_threads(int n, int * psbl_primes, unsigned int max) {
    int i;
    struct sieve_args s_args;

    pthread_t * threads = malloc(n * sizeof(pthread_t));

    s_args.psbl_primes = psbl_primes;
    s_args.max = max;

    for(i=0; i<n; i++) {
        if((errno = pthread_create(&threads[i], NULL, (void *)sieve, (void *) &s_args)) != 0) {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
    }
    return threads;
}

void join_threads(int n, pthread_t * threads) {
    int i;
    for(i=0; i<n; i++) {
        if((errno = pthread_join(threads[i], NULL)) != 0) {
            perror("Error joining threads");
            exit(EXIT_FAILURE);
        }
    }
    free(threads);
}


void sieve(struct sieve_args * s_args) {
    unsigned int i, sqr;
    sqr = (unsigned int)(sqrt((float) s_args->max));

    do {
        lock_mutex(&count_mtx);
        i=count++;
        unlock_mutex(&count_mtx);
        
        if(!check_bit(s_args->psbl_primes, i)) {
            sieve_multiples(s_args->psbl_primes, i, s_args->max);
        }   
    } while(count<=sqr);
}

void sieve_multiples(int * psbl_primes, int factor, unsigned int max) {
    int i;
    for(i=2; (i*factor)<=max; i++) {
        lock_mutex(&bits_mtx);
        set_bit(psbl_primes, i*factor);
        unlock_mutex(&bits_mtx);
    }   
}

void lock_mutex(pthread_mutex_t * mtx) {
    if((errno = pthread_mutex_lock(mtx)) != 0) {
        perror("Error locking mutex");
        exit(EXIT_FAILURE);
    }
}

void unlock_mutex(pthread_mutex_t * mtx) {
    if((errno = pthread_mutex_unlock(mtx)) != 0) {
        perror("Error unlocking mutex");
        exit(EXIT_FAILURE);
    }
}
