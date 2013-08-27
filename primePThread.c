/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * primePThread.c
 * Operating Systems - CS311_400
 * Author: Nathan Cochran
 * Date: 8/26/2013
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "primeUtils.h"

//Functions Prototypes:
long nano_diff(struct timespec * begin, struct timespec * end);
pthread_t *create_threads(int num, int *psbl_primes, unsigned int max);
void join_threads(int n, pthread_t *threads);

int main(int argc, char * argv[]) {
    int * psbl_primes;
    pthread_t * threads;
    struct options opts;
    struct timespec begin, end;

    clock_gettime(CLOCK_REALTIME, &begin);
   
    //Get command line arguments:
    get_options(argc, argv, &opts);

    //Allocate memory for the bit array:
    psbl_primes = calloc(((opts.max/32)+1), sizeof(int));

    //By definition, 0 and 1 are not a prime numbers:
    set_bit(psbl_primes, 0);
    set_bit(psbl_primes, 1);

    //Call pre_sieve (this marks off all non-primes less than sqrt(max)):
    pre_sieve(psbl_primes, opts.max);

    //Create threads, let finish, join:
    threads = create_threads(opts.concurrency, psbl_primes, opts.max);
    join_threads(opts.concurrency, threads);

    //Print primes:
    if(!opts.q_set) {
        print_primes(psbl_primes, opts.max);
    }

    //Free heap space used for the bit array:
    free(psbl_primes);

    clock_gettime(CLOCK_REALTIME, &end);

    if(TIME) {
        printf("Time: %ld\n", nano_diff(&begin, &end));
    }

    return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Returns the difference (in nanoseconds) between two timespecs
 * Param:   struct timespec * begin -  The first time
 * Param:   struct timespec * end -  The second time
 * Return:  long -  The difference in nanoseconds
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
long nano_diff(struct timespec * begin, struct timespec * end) {
    return (BILLION * (end->tv_sec - begin->tv_sec) + (end->tv_nsec - begin->tv_nsec));
}

pthread_t * create_threads(int num, int * psbl_primes, unsigned int max) {
    int i;
    struct sieve_args * sa;
    pthread_t * threads = malloc(num * sizeof(pthread_t));

    for(i=0; i<num; i++) {

        //Argument struct for the sieve function:
        sa = malloc(sizeof(struct sieve_args));
        sa->psbl_primes = psbl_primes;
        sa->low = get_low(num, max, i);
        sa->high = get_high(num, max, i);
        sa->max = max;

        printf("Thread: %d, Max: %u, Low: %u, High: %u\n", i, max, sa->low, sa->high);

        //Create a new sieve thread:
        if((errno = pthread_create(&threads[i], NULL, (void *)thread_sieve, (void *) sa)) != 0) {
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
