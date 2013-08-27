#!/usr/bin/python

import subprocess
import os
import shlex
import sys
import random

def test(command):

	proc = subprocess.Popen(shlex.split(command), stdout = subprocess.PIPE)
	out = proc.stdout.readlines()

	for line in out:
		print line,
		spl = line.split(": ")
		if spl[0] == "Time":
			print line
			time = spl[1].strip("\n")
	return float(time)

def main():

    progs = ["./primePThread", "./primeMProc"]
    concurrency = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20]
    limits = range(500000000, 4294967296, 500000000) + [4294967296]

    num_runs = 2

    avg_times = dict((limit, dict((c, 0) for c in concurrency)) for limit in limits)

    for prog in progs:
        for run in range(0, num_runs):
            print "\nRun #" + str(run+1) + ":"
            for limit in limits:
                for c in concurrency:
    			    cmd = " ".join([prog, " -c ", str(c), " -m ", str(limit), " -q "])
    			    print cmd + ":"
    			    avg_times[limit][c] += test(cmd)

	# Print results:
	print "\nAverage times (nanoseconds): "
    for prog in progs:
    	for run in range(0, num_runs):
            print "\nRun #" + str(run+1) + ":"
            for limit in limits:
                for c in concurrency:
                    cmd = " ".join([prog, " -c ", str(c), " -m ", str(limit), " -q "])
                    print cmd + ":"
                    avg_times[limit][c] = avg_times[limit][c]/num_runs
                    print avg_times[limit][c]

if __name__ == "__main__":
	main();
