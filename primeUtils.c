/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * primeUtils.c
 * Operating Systems - CS311_400
 * Author: Nathan Cochran
 * Date: 8/26/2013
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "primeUtils.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Get command line options: [-q: quiet mode, -m: max prime, -c: concurrency]
 * Param:   int argc -  Number of arguments
 * Param:   char ** argv -  Argument vector
 * Param:   struct options * opts -  Output parameter specifying the options given
 * Return:  void
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void get_options(int argc, char ** argv, struct options * opts) {
    char opt;
    
    //Default option values:
    opts->max = UINT_MAX;
    opts->concurrency = DFL_CONCUR;
    opts->q_set = false;

    //Get any command line option values:
    while((opt = getopt(argc, argv, "qc:m:")) != -1) {
        switch (opt) {
            case 'q':
                opts->q_set = true;
                break;
            case 'c':
                opts->concurrency = get_concurrency(optarg);
                break;
            case 'm':
                opts->max = get_max_prime(optarg);
                break;
            default:
                printf("Usage: %s [-q (quiet mode)] [-c concurrency] [-m max prime]\n", argv[0]);
                exit(EXIT_SUCCESS);
                break;
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Takes the concurrency (-c) option argument, validates it, returns it as an int
 * Param:   char * concr_str -  The string representation of the concurrency
 * Return:  int -  Integer representation of the concurrency
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int get_concurrency(char * concr_str) {
    int concr;

    errno = 0;
    concr = (int) strtol(concr_str, NULL, 10);

    if(errno != 0) {
        perror("Error converting string to integer");
        exit(EXIT_FAILURE);
    }

    if((concr < 1) || (concr > 50)) {
        printf("Error: Concurrency level (-c) is not in valid range (1 to 50).\n");
        exit(EXIT_SUCCESS);
    }

    return concr;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Takes the max prime (-p) option argument, validates it, and returns it as an int
 * Param:   char * max_str -  String representation of the max prime
 * Return:  unsigned int -  Integer representation of the max prime
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int get_max_prime(char * max_str) {
    long max;

    errno = 0;
    max = strtol(max_str, NULL, 10);

    if(errno != 0) {
        perror("Error converting string to integer");
        exit(EXIT_FAILURE);
    }

    if((max < 2) || (max > UINT_MAX)) {
        printf("Error: Max prime (-m) is not in valid range (2 to %u).\n", UINT_MAX);
        exit(EXIT_SUCCESS);
    }

    return (unsigned int) max;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Prints all primes up to max, as specified in the bit array primes
 * Param:   int * primes -  The bit array to read the primes from
 * Param:   int max -  The max prime
 * Return:  void
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void print_primes(int * primes, int max) {
    unsigned int i;
    unsigned int cnt = 0;

    for(i=1; i<=max; i++) {
        if(!check_bit(primes, i)) {
            cnt++;
            printf("%10u", i);

            if(cnt % NUM_COLUMNS == 0) {
                printf("\n");
            }
        }
    }
    printf("\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Returns the lower bound of the interval of the bit array that the thread, specified by thread_num, is responsible for
 * Param:   int num_threads -  The total number of threads
 * Param:   unsigned int max -  The max prime
 * Param:   int thread_num -  Specifies the thread in question (between 0 and num_threads)
 * Return:  unsigned int -  The lower bound of the interval this thread is responsible for
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int get_low(int num_threads, unsigned int max, int thread_num) {
    unsigned long intvl, sqr;

    //If this is the first thread, return the square root (where pre_sieve left off):
    if(thread_num == 0) {
        sqr = get_sqrt(max);
        return sqr;
    }
    else {
        sqr = get_sqrt32(max);
        intvl = get_interval(num_threads, max);
        return get_min((sqr + (thread_num * intvl)), max);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Returns the upper bound of the interval of the bit array that the thread, specified by thread_num, is responsible for
 * Param:   int num_threads -  The total number of threads
 * Param:   unsigned int max -  The max prime
 * Param:   int thread_num -  Number specifying the thread in question (between 0 and num_threads)
 * Return:  unsigned int -  The upper bound of the interval that the given thread is responsible for
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int get_high(int num_threads, unsigned int max, int thread_num) {
    unsigned long intvl, sqr;

    intvl = get_interval(num_threads, max);
    sqr = get_sqrt32(max);
    return get_min(sqr + ((thread_num+1) * intvl), max);
}


//Returns an interval size which:
//  1. Evenly splits up the work amongst the various threads
//  2. Is divisble by 32
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Returns the size of the interval that each thread is responsible for
 * Param:   int num_threads -  The number of threads
 * Param:   unsigned int max -  The max prime
 * Return:  unsigned int -  The size of the interval that each thread is responsible for
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int get_interval(int num_threads, unsigned int max) {
    unsigned int intvl;

    intvl = ((max - get_sqrt32(max))/num_threads);
    intvl = (intvl/32 + 1) * 32;
    return intvl;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Returns the square root, rounded down to the nearest int, of the value passed in
 * Param:   unsigned int max -  The value to get the square root of (i.e., the max prime)
 * Return:  unsigned int -  The square root of the value
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int get_sqrt(unsigned int max) {
    return (unsigned int)(sqrt((double)max));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Returns the square root of the given value rounded up to the next number cleanly divisible by 32 (the size of an int)
 * Param:   unsigned int max -  The max prime
 * Return:  unsigned int -  The square root rounded up to the next interval of 32
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int get_sqrt32(unsigned int max) {
    unsigned int sqr;

    //Get actual square root:
    sqr = get_sqrt(max);

    //If the square root is already divisible by 32, return it:
    if((sqr % 32) == 0) {
        return sqr;
    }

    //Else, return the next number which is divisible by 32:
    else {
        return ((sqr/32) + 1) * 32;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Sieves the primes up to the square root of the max (thereby giving the numbers that need to be sieved out of the entire bit array)
 * Param:   int * psbl_primes -  Bit array representing all numbers up to the max
 * Param:   unsigned int max -  The max prime
 * Return:  void
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void pre_sieve(int * psbl_primes, unsigned int max) {
    unsigned int i, sqr, sqr2;

    sqr = get_sqrt(max);
    sqr2 = (unsigned int)(sqrt((float) sqr));

    for(i=0; i<=sqr2; i++) {
        if(!check_bit(psbl_primes, i)) {
            sieve_multiples(psbl_primes, i, sqr);
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Sieves numbers from the interval specified by low and high (in the argument struct)
 * Param:   struct sieve_args * sa -  Gives the bit array, max prime, high and low bounds for this thread's work
 * Return:  void
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void thread_sieve(struct sieve_args * sa) {
    unsigned int i, sqr;
    sqr = get_sqrt(sa->max);

    for(i=0; i<=sqr; i++) {
        if(!check_bit(sa->psbl_primes, i)) {
            sieve_multiples_in_range(sa->psbl_primes, i, sa->low, sa->high);
        }
    }
    free(sa);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Sieves all multiples of factor, up to max, out of the bit array (except factor itself)
 * Param:   int * psbl_primes -  Bit array representing all numbers up to max
 * Param:   unsigned int factor -  The number whose multiples will be sieved out
 * Param:   unsigned int max -  The max prime
 * Return:  void
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void sieve_multiples(int * psbl_primes, unsigned int factor, unsigned int max) {
    unsigned int i;

    for(i = (2*factor); i <= max; i += factor) {
        set_bit(psbl_primes, i);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Sieves all multiples of the given factor from the range specified by high and low
 * Param:   int * psbl_primes -  Bit array representing all numbers up to max
 * Param:   unsigned int factor -  The number whose multiples will be sieved out
 * Param:   unsigned int low -  The lower bound of the range to sieve
 * Param:   unsigned int high -  The upper bound of the range to sieve
 * Return:  void
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void sieve_multiples_in_range(int * psbl_primes, unsigned int factor, unsigned int low, unsigned int high) {
    unsigned long i;

    for(i = get_first_multiple_gt(factor, low); i <= high; i += factor) {
        set_bit(psbl_primes, i);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Returns the first multiple of the given factor that's greater than low
 * Param:   unsigned int factor -  The factor whose multiple is being sought
 * Param:   unsigned int low -  The multiple returns will be the first that's greater than this number
 * Return:  unsigned int -  The first multiple greater than low
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int get_first_multiple_gt(unsigned int factor, unsigned int low) {
    unsigned int i = (low/factor) + 1;
    return i*factor;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Returns the minimum of the two values (or either, on a tie)
 * Param:   unsigned long a -  A value
 * Param:   unsigned long b -  Another value
 * Return:  unsigned long -  The minimum of the two values
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned long get_min(unsigned long a, unsigned long b) {
    if(a<b)
        return a;
    else
        return b;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Sets the bit specified by pos in the given bit array
 * Param:   int * array -  The bit array
 * Param:   unsigned int pos -  The position of the bit to set
 * Return:  void
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void set_bit(int * array, unsigned int pos) {
    array[pos/32] |= 1 << (pos%32);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Clears the bit specified by pos in the given bit array
 * Param:   int * array -  The bit array
 * Param:   unsigned int pos -  The position of the bit to clear
 * Return:  void
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void clear_bit(int * array, unsigned int pos) {
    array[pos/32] &= ~(1 << (pos%32));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Returns 1 if the bit specified by pos is set, 0 if not
 * Param:   int * array -  The bit array
 * Param:   unsigned int pos -  The position of the bit to check
 * Return:  int -  1 if the bit is set, 0 if not
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int check_bit(int * array, unsigned int pos) {
    return ((array[pos/32] & (1 << (pos%32))) != 0);
}
