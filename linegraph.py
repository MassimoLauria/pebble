#!/usr/bin/env python

import sys

N=int(sys.argv[1])

print "c line graph with %d vertices" % N
print N


if N > 0:
    print "1 :"

for i in range(2,N+1):
    print "%d : %d" % (i,i-1)
