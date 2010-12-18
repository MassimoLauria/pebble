/*
   Copyright (C) 2010 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-16, giovedì 16:36 (CET) Massimo Lauria"
   Time-stamp: "2010-12-17, venerdì 21:18 (CET) Massimo Lauria"

   Description::

   Header containing declaring common functionalities, like lists and queues.

*/

#ifndef COMMON_H
#define COMMON_H
/* Preamble */
#include <stdlib.h>

/* Code */

/* Type declarations */

/* Several bit tuples  are used in the code.
 */
typedef long long unsigned int BitTuple;
typedef unsigned char          Boolean;


#define TRUE  1
#define FALSE 0



/* Assertion for DEBUG purposes */
#ifdef DEBUG

#define ASSERT_NOTNULL(p) { if (!(p)) exit(-1); }
#define ASSERT_NULL(p)    { if (p) exit(-1); }

#define ASSERT_TRUE(p)    { if (!(p)) exit(-1); }
#define ASSERT_FALSE(p)    { if (p) exit(-1); }

#else

#define ASSERT_NOTNULL(p) {}
#define ASSERT_NULL(p)    {}

#define ASSERT_TRUE(p)    {}
#define ASSERT_FALSE(p)   {}


#endif


#endif /* COMMON_H  */
