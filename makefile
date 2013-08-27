CC=gcc
DEBUG=-g
CFLAGS=$(DEBUG) -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement
PROGS=primePThread primeMProc

all: $(PROGS)

primePThread: primePThread.o primeUtils.o
	$(CC) $(CFLAGS) -o $@ primePThread.o primeUtils.o -pthread -lrt -lm

primePThread.o: primePThread.c
	$(CC) $(CFLAGS) -c $^
    
primeMProc: primeMProc.o primeUtils.o
	$(CC) $(CFLAGS) -o $@ primeMProc.o primeUtils.o -lrt -lm

primeMProc.o: primeMProc.c
	$(CC) $(CFLAGS) -c $^ -lrt

primeUtils.o: primeUtils.c
	$(CC) $(CFLAGS) -c $^ -lm

clean:
	rm -f $(PROGS) *.o *~

