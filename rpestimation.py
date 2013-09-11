#!/usr/bin/env python
"""Estimate the reversible pebbling number for Pyramids

Uses the recurrence relation:

p(a+b) = a + 1 + max(p(a-1),p(b))`
"""

MaxHeight= 640


CostTable=[0]*(MaxHeight+1)
SplitTable=[[0]]*(MaxHeight+1)

CostTable[0]=1
CostTable[1]=2
CostTable[2]=2
CostTable[3]=3
CostTable[4]=3

assert(MaxHeight > 4)

for n in range(5,MaxHeight+1):
    CostTable[n] = n
    new_value  = 0
    for a in range(1,n):
        new_value = 1 + max(CostTable[a-1],CostTable[n-a])
        if new_value < CostTable[n]:
            CostTable[n]  = new_value
            SplitTable[n] = [a]
        elif new_value == CostTable[n]:
            SplitTable[n] += [a]
 

print "     H |   Cost | Delta | Split "
for n in range(1,MaxHeight+1):
    print " %5d | %5d | %3d | %d-%d "%(n,
                                      CostTable[n],
                                      CostTable[n]-CostTable[n-1],
                                       min(SplitTable[n]),
                                       max(SplitTable[n]))
