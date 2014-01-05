concurrent_prime_sieve
----------------------

This pair of programs (primePThread and primeMProc) use parallel threads and processes, respectively, to calculate all 
prime numbers up to UINT_MAX.  Each thread/process sieves a portion of a bit-array representing all possible prime numbers,
according to the [sieve of eratosthenes](http://en.wikipedia.org/wiki/Sieve_of_Eratosthenes) algorithm.

For further details, see the expanation pdf included in the repo.

#### Compilation:

primePThread: `make primePThread`

primeMProc: `make primeMProc'

both: `make`

#### Execution:

    Usage: ./primeMProc [-q] [-c concurrency] [-m max prime]
    Options:
      -q: Toggle quiet mode
      -c: The number of concurrent threads/processes to run
      -m: The max prime - all primes up to this will be calculated
