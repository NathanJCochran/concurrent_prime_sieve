/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * primeMProc.c
 * Operating Systems - CS311_400
 * Author: Nathan Cochran
 * Date: 8/26/2013
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "primeUtils.h"

//Function Prototypes:
long nano_diff(struct timespec * begin, struct timespec * end);
int *map_prime_shm(int shm_fd, unsigned int max);
int create_prime_shm(void);
int open_prime_shm(unsigned int max);
void unlink_prime_shm(void);
void unmap_prime_shm(void *addr, unsigned int max);
pid_t *create_children(int num_procs, unsigned int max);
pid_t create_child(int low, int high, unsigned int max);
void wait_on_children(int num_procs, pid_t *pids);
void sigint_handler(int sig);
void install_sigint_handler(void);

int main(int argc, char * argv[]) {
    int shm_fd;
    int * psbl_primes;
    pid_t * pids;
    struct options opts;
    struct timespec begin, end;

    clock_gettime(CLOCK_REALTIME, &begin);

    //Get command line arguments:
    get_options(argc, argv, &opts);

    //Install sigint_handler():
    install_sigint_handler();

    //Create shared memory object (bit array):
    shm_fd = create_prime_shm();
    psbl_primes = map_prime_shm(shm_fd, opts.max);

    //By definition, 0 and 1 are not a prime numbers:
    set_bit(psbl_primes, 0); 
    set_bit(psbl_primes, 1);
    
    //Call pre_sieve (this marks off all non-primes less than sqrt(max)):
    pre_sieve(psbl_primes, opts.max);

    //The shared memory region is removed for demonstrative
    //purposes - i.e. so that the child processes
    //can connect to the shared memory region by name,
    //instead of just inheriting the mapping or fd.
    unmap_prime_shm(psbl_primes, opts.max);
    close(shm_fd);

    //Create child processes, let finish, join:
    pids = create_children(opts.concurrency, opts.max);
    wait_on_children(opts.concurrency, pids);

    //Print primes:
    if(!opts.q_set) {

        //Re-open the shared memory region:
        shm_fd = open_prime_shm(opts.max);
        psbl_primes = map_prime_shm(shm_fd, opts.max);

        //Print primes:
        print_primes(psbl_primes, opts.max);

        //Close the shared memory region:
        unmap_prime_shm(psbl_primes, opts.max);
        close(shm_fd);
    }

    //Unlink the shared memory object:
    unlink_prime_shm();

    clock_gettime(CLOCK_REALTIME, &end);

    if(TIME) {
        printf("Time: %ld\n", nano_diff(&begin, &end));
    }

    return EXIT_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Returns the difference, in nanoseconds, between the two timespecs
 * Param:   struct timespec * begin -  The earlier timespec
 * Param:   struct timespec * end -  The later timespec
 * Return:  long -  The different, in nanoseconds
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
long nano_diff(struct timespec * begin, struct timespec * end) {
    return (BILLION * (end->tv_sec - begin->tv_sec) + (end->tv_nsec - begin->tv_nsec));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Creates a shared memory object called "prime_shm", returns the fd
 * Param:   void
 * Return:  int -  The file descriptor representing the shared memory object
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int create_prime_shm(void) {
    int shm_fd;

    //Create shared memory object:
    if((shm_fd = shm_open("prime_shm", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
        perror("Error creating shared memory object");
        exit(EXIT_FAILURE);
    }

    return shm_fd;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Opens the shared memory object called "prime_shm", returns the fd
 * Param:   unsigned int max -  The max prime
 * Return:  int -  The file descriptor representing the shared memory object
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int open_prime_shm(unsigned int max) {
    int shm_fd;

    //Open shared memory object:
    if((shm_fd = shm_open("prime_shm", O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
        perror("Error opening shared memory object");
        exit(EXIT_FAILURE);
    }

    return shm_fd;;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Maps the shared memory object prime_shm to a region of memory, returning the address
 * Param:   int shm_fd -  The file descriptor of the shared memory object to map
 * Param:   unsigned int max -  The max prime
 * Return:  int * -  The address of the newly mapped shared memory object
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int * map_prime_shm(int shm_fd, unsigned int max) {
    int size;
    int * addr;

    size = (max/8) + 1;

    //Resize:
    if(ftruncate(shm_fd, size) == -1) {
        perror("Error truncating shared memory object");
        exit(EXIT_FAILURE);
    }

    //Map to region in calling process's memory:
    if((addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED) {
        perror("Error mapping shared memory object");
        exit(EXIT_FAILURE);
    }

    //Return addr of shared region:
    return addr;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Unmaps the shared memory object prime_shm from the region in memory specified by addr
 * Param:   void * addr -  The address of the region to unmap
 * Param:   unsigned int max -  The max prime
 * Return:  void
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void unmap_prime_shm(void * addr, unsigned int max) {
    int size;
    size = (max/8) + 1;

    //Unmap region of memory:
    if(munmap(addr, size) == -1) {
        perror("Error unmapping shared memory region");
        exit(EXIT_FAILURE);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Unlinks the prime_shm shared memory object, permanently removing it
 * Param:   void
 * Return:  void
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void unlink_prime_shm(void) {
    if(shm_unlink("prime_shm") == -1) {
        perror("Error unlinking shared memory object");
        exit(EXIT_FAILURE);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Creates num_procs subprocesses to sieve the bit array
 * Param:   int num_procs -  The number of subprocesses to create
 * Param:   unsigned int max -  The max prime
 * Return:  pid_t * -  List of pids of the newly created children
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
pid_t * create_children(int num_procs, unsigned int max) {
    int i;
    unsigned int low, high;
    pid_t * pids;

    pids = malloc(num_procs * sizeof(pid_t));

    for(i=0; i<num_procs; i++) {        
        low = get_low(num_procs, max, i);
        high = get_high(num_procs, max, i);
        pids[i] = create_child(low, high, max);
    }

    return pids;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Creates a child process which seives the numbers in the range specified by low and high
 * Param:   int low -  The lower bound of the interval to sieve
 * Param:   int high -  The upper bound of the interval to sieve
 * Param:   unsigned int max -  The max prime
 * Return:  pid_t -  The pid of the newly created process
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
pid_t create_child(int low, int high, unsigned int max) {
    int shm_fd;
    pid_t pid;
    struct sieve_args * sa;

    switch (pid = fork()) {

        //Error case:
        case -1:
            perror("Error forking child process");
            exit(EXIT_FAILURE);

        //Child case:
        case 0:

            //Argument struct for the sieve function:
            sa = malloc(sizeof(struct sieve_args));
            shm_fd = open_prime_shm(max);
            sa->psbl_primes = map_prime_shm(shm_fd, max);
            sa->low = low;
            sa->high = high;
            sa->max = max;
            
            //Call sieve function:
            thread_sieve(sa);

            exit(EXIT_SUCCESS);

        //Parent case:
        default: 
            return pid;
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Waits on all of the children specified in pids
 * Param:   int num_procs -  The number of children to wait on
 * Param:   pid_t * pids -  The pids of the children to wait on
 * Return:  void
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void wait_on_children(int num_procs, pid_t * pids) {
    int i;

    //Wait on each child in the pids array:
    for(i=0; i<num_procs; i++) {
        if(waitpid(pids[i], NULL, 0) == -1) {
            perror("Error waiting for child process");
            exit(EXIT_FAILURE);
        }
    }

    //Free the array of pids:
    free(pids);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Handler for SIGINT - unlinks the shared memory region before terminating
 * Param:   int sig -  The signal
 * Return:  void
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void sigint_handler(int sig) {
    unlink_prime_shm();
    exit(EXIT_SUCCESS);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Installs the handler for SIGINT
 * Param:   void
 * Return:  void
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void install_sigint_handler(void) {
    struct sigaction sigint;

    sigint.sa_handler = sigint_handler;
    sigemptyset(&sigint.sa_mask);
    sigint.sa_flags = 0;

    sigaction(SIGINT, &sigint, NULL);
}









    
