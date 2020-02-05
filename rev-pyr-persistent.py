#!/usr/bin/env python
"""Estimate the reversible pebbling number for Pyramids

Uses the recurrence relation:


p(a+b) = a + 1 + max(p(a-1),p(b))`
"""

# Max height with x extra pebbles 
def g(x):
    if x<=0: return 0
    if x==1: return 0
    return 2**(g(x-1)+x-2)+g(x-1)

# Cost of the pyramid of height h
def persistent_pyr_cost(h):
    assert h>=0
    i=1
    while(g(i)<h): i=i+1
    return h+i

MaxHeight= 640

CostTable=[0]*(MaxHeight+1)

for n in range(1,MaxHeight+1):
    CostTable[n]=persistent_pyr_cost(n)

print("     H |   Cost | Delta | ")
for n in range(1,MaxHeight+1):
    print(" %5d | %5d | %3d | "%(n,
                                 CostTable[n],
                                 CostTable[n]-CostTable[n-1]))
