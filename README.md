concurrent prime sieve
----------------------

This pair of programs (primePThread and primeMProc) use parallel threads and processes, respectively, to calculate all prime numbers up to UINT_MAX.  Each thread/process sieves a portion of a bit-array representing all possible prime numbers, according to the [Sieve of Eratosthenes](http://en.wikipedia.org/wiki/Sieve_of_Eratosthenes) algorithm.

Besides outputting a list of primes, each program also outputs the time it took to complete the computation, making this pair of programs particularly useful for comparing the relative benefits of different concurrency levels, as well as the pros/cons of using threads vs. processes for parallelization.  A simple test script - test.py - is included.  This python script runs many successive executions of the program with different concurrency levels and max primes, outputting the average times for each case.  For meaningful results, it should be run on a computer with more than one CPU (a computer which is capable of true parallelization).

For further algorithmic details, as well as an analysis of the test results, see the expanation pdf included in the repo.

#### Compilation:

primePThread: `make primePThread`

primeMProc: `make primeMProc`

both: `make`

#### Usage:

    Usage: primeMProc/primePThread [-q] [-c concurrency] [-m max prime]
    Options:
      -q: Toggle quiet mode (primes will not be displayed - only the time it took)
      -c: The number of concurrent threads/processes to run
      -m: The max prime - all primes up to this will be calculated

