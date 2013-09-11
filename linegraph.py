#!/usr/bin/env python

import sys

N=int(sys.argv[1])

print "c line graph of length %d (with %d vertices)" % (N,N+1)
print N+1


if N >= 0:
    print "1 :"

for i in range(2,N+2):
    print "%d : %d" % (i,i-1)
