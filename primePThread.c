#include "primeUtils.h"

//Functions Prototypes:
pthread_t *create_threads(int num, int *psbl_primes, unsigned int max);
void join_threads(int n, pthread_t *threads);

int main(int argc, char * argv[]) {
    int * psbl_primes;
    pthread_t * threads;
    struct options opts;
    
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

    return 0;
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
