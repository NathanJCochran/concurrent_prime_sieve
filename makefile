CC=gcc
DEBUG=-g
CFLAGS=$(DEBUG) -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement
PROGS=primePThread

all: $(PROGS)

primePThread: primePThread.o primeUtils.o
	$(CC) $(CFLAGS) -o $@ primePThread.o primeUtils.o -pthread -lm

primePThread.o: primePThread.c
	$(CC) $(CFLAGS) -c $^

primeUtils.o: primeUtils.c
	$(CC) $(CFLAGS) -c $^ -lm

clean:
	rm -f $(PROGS) *.o *~

