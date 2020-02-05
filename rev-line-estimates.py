#!/usr/bin/env python
"""Estimate the reversible pebbling number for lines.
   
   The recurrence covers persistent and non persistent pebbling.
   
Uses the recurrence relation:

p(a+b) <= 1 + max(p(a-1),p(b))
c(a+b) <= max(p(a-1),1+c(b))


"""

MaxHeight= 80


CTable=[0]*(MaxHeight+1)
PTable=[0]*(MaxHeight+1)
CSplit=[[0]]*(MaxHeight+1)
PSplit=[[0]]*(MaxHeight+1)

CTable[0]=1
CTable[1]=2
CTable[2]=2
CTable[3]=3
CTable[4]=3

PTable[0]=1
PTable[1]=2
PTable[2]=3
PTable[3]=3
PTable[4]=4

assert(MaxHeight > 4)

for n in range(5,MaxHeight+1):
    CTable[n] = n+1
    PTable[n] = n+1
    
    new_pvalue , new_cvalue  = 0,0
    for a in range(1,n):
        new_pvalue = 1 + max(PTable[a-1],PTable[n-a])
        new_cvalue = max(PTable[a-1],1+CTable[n-a])

        if new_cvalue < CTable[n]:
            CTable[n]  = new_cvalue
            CSplit[n] = [a]
        elif new_cvalue == CTable[n]:
            CSplit[n] += [a]
            
        if new_pvalue < PTable[n]:
            PTable[n]  = new_pvalue
            PSplit[n] = [a]
        elif new_pvalue == PTable[n]:
            PSplit[n] += [a]
 
print("     H |   Cost | Delta | Split ")
for n in range(1,MaxHeight+1):
    print(" %5d | %5d | %3d | %d-%d "%(n,
                                       CTable[n],
                                       CTable[n]-CTable[n-1],
                                       min(CSplit[n]),
                                       max(CSplit[n])))

print("     H |   Pers | Delta | Split ")
for n in range(1,MaxHeight+1):
    print(" %5d | %5d | %3d | %d-%d "%(n,
                                       PTable[n],
                                       PTable[n]-PTable[n-1],
                                       min(PSplit[n]),
                                       max(PSplit[n])))
