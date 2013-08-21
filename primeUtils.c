#include "primeUtils.h"

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

unsigned int get_low(int num_threads, unsigned int max, int thread_num) {
    unsigned long intvl, sqr;

    //If this is the first thread, return the square root + 1 (where pre_sieve left off):
    if(thread_num == 0) {
        sqr = (unsigned int)(sqrt((float) max));
        return sqr;
    }
    else {
        sqr = get_sqrt_32(max);
        intvl = get_interval(num_threads, max);
        return get_min((sqr + (thread_num * intvl)), max);
    }
}

unsigned int get_high(int num_threads, unsigned int max, int thread_num) {
    unsigned long intvl, sqr;

    intvl = get_interval(num_threads, max);
    sqr = get_sqrt_32(max);
    return get_min(sqr + ((thread_num+1) * intvl), max);
}

unsigned int get_interval(int num_threads, unsigned int max) {
    unsigned int intvl, sqr;

    //Returns an interval size which:
    //  1. Evenly splits up the work amongst the various threads
    //  2. Is divisble by 32
    sqr = (unsigned int)(sqrt((float) max));
    intvl = ((max-sqr)/num_threads);
    intvl = (intvl/32 + 1) * 32;
    return intvl;
}

unsigned int get_sqrt_32(unsigned int max) {
    unsigned int sqr;

    //Get actual square root (truncated to int):
    sqr = (unsigned int)(sqrt((float) max));

    //If the square root is already divisible by 32, return it:
    if((sqr % 32) == 0) {
        return sqr;
    }

    //Else, return the next number which is divisible by 32:
    else {
        return ((sqr/32) + 1) * 32;
    }
}

void pre_sieve(int * psbl_primes, unsigned int max) {
    unsigned int i, sqr, sqr2;

    sqr = (unsigned int)(sqrt((float) max));
    sqr2 = (unsigned int)(sqrt((float) sqr));

    for(i=0; i<=sqr2; i++) {
        if(!check_bit(psbl_primes, i)) {
            sieve_multiples(psbl_primes, i, sqr);
        }
    }
}

void thread_sieve(struct sieve_args * sa) {
    unsigned int i, sqr;
    sqr = (unsigned int)(sqrt((float) sa->max));

    for(i=0; i<=sqr; i++) {
        if(!check_bit(sa->psbl_primes, i)) {
            sieve_multiples_in_range(sa->psbl_primes, i, sa->low, sa->high);
        }
    }
    free(sa);
}

void sieve_multiples(int * psbl_primes, unsigned int factor, unsigned int max) {
    unsigned int i;

    for(i = (2*factor); i <= max; i += factor) {
        set_bit(psbl_primes, i);
    }
}

void sieve_multiples_in_range(int * psbl_primes, unsigned int factor, unsigned int low, unsigned int high) {
    unsigned long i;

    for(i = get_first_multiple_gt(factor, low); i <= high; i += factor) {
        set_bit(psbl_primes, i);
    }
}

unsigned int get_first_multiple_gt(unsigned int factor, unsigned int low) {
    unsigned int i = (low/factor) + 1;
    return i*factor;
}

unsigned long get_min(unsigned long a, unsigned long b) {
    if(a<b)
        return a;
    else
        return b;
}

void set_bit(int * array, unsigned int pos) {
    array[pos/32] |= 1 << (pos%32);
}

void clear_bit(int * array, unsigned int pos) {
    array[pos/32] &= ~(1 << (pos%32));
}

int check_bit(int * array, unsigned int pos) {
    return ((array[pos/32] & (1 << (pos%32))) != 0);
}
