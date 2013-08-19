#include "primeUtils.h"

void get_options(int argc, char ** argv, struct options * opts) {
    char opt;
    
    //Default option values:
    opts->max = UINT_MAX;
    opts->concr = DFL_CONCUR;
    opts->q_set = false;

    //Get any command line option values:
    while((opt = getopt(argc, argv, "qc:m:")) != -1) {
        switch (opt) {
            case 'q':
                opts->q_set = true;
                break;
            case 'c':
                opts->concr = get_concurrency(optarg);
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

int get_concurrency(char * optarg) {
    int concr;

    errno = 0;
    concr = (int) strtol(optarg, NULL, 10);

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

unsigned int get_max_prime(char * optarg) {
    long max;

    errno = 0;
    max = strtol(optarg, NULL, 10);

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

void set_bit(int * array, int pos) {
    array[pos/32] |= 1 << (pos%32);
}

void clear_bit(int * array, int pos) {
    array[pos/32] &= ~(1 << (pos%32));
}

int check_bit(int * array, int pos) {
    return ((array[pos/32] & (1 << (pos%32))) != 0);
}
