/*
   Copyright (C) 2010, 2011, 2012 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-16, giovedì 16:36 (CET) Massimo Lauria"
   Time-stamp: "2012-06-04, 19:10 (CEST) Massimo Lauria"

   Description::

   Header containing declaring common functionalities, like lists and queues.

*/

#ifndef COMMON_H
#define COMMON_H
/* Preamble */
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

/* Bit and Boolean types declarations */
/* Several bit tuples  are used in the code.
 */
typedef long long unsigned int BitTuple;
typedef long long unsigned int Counter;
typedef unsigned char          Boolean;

#define BITTUPLE_UNIT 0x1ULL       /* A bitmask compatible with BitTuple, of value 1 */
#define BITTUPLE_ZERO 0x0ULL       /* A bitmask compatible with BitTuple, of value 0 */
#define BITTUPLE_FULL (~0x0ULL)    /* A bitmask compatible with BitTuple, of value 1 */
#define BITTUPLE_SIZE sizeof(BitTuple)*CHAR_BIT

#define TRUE  1
#define FALSE 0

#define   GETBIT(tuple,i) (((tuple) & (BITTUPLE_UNIT << (i))) ? TRUE : FALSE )
#define   SETBIT(tuple,i) ((tuple)|=(BITTUPLE_UNIT << (i)))
#define RESETBIT(tuple,i) ((tuple)&= ~(BITTUPLE_UNIT << (i)))


#endif /* COMMON_H  */
