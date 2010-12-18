/*
   Copyright (C) 2010 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-16, giovedì 16:36 (CET) Massimo Lauria"
   Time-stamp: "2010-12-18, sabato 18:05 (CET) Massimo Lauria"

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

#include <stdio.h>

#define ERRMSG(str) \
  { fprintf(stderr,"ERROR at [%s,%d]: %s",__FILE__,__LINE__,str); }

#define WARNMSG(str) \
  { fprintf(stderr,"WARNING at [%s,%d]: %s",__FILE__,__LINE__,str); }

#define ASSERT_NOTNULL(p) {                     \
    if (!(p)) {                                 \
      ERRMSG("NOT NULL assertion failure.");    \
      exit(-1);                                 \
  }                                             \
  }

#define ASSERT_NULL(p) {                        \
    if (p) {                                    \
      ERRMSG("NULL assertion failure.");        \
      exit(-1);                                 \
  }                                             \
  }

#define ASSERT_TRUE(p) {                        \
    if (!(p)) {                                 \
      ERRMSG("TRUE assertion failure.");        \
      exit(-1);                                 \
  }                                             \
  }


#define ASSERT_FALSE(p) {                       \
    if (p) {                                    \
      ERRMSG("NULL assertion failure.");        \
      exit(-1);                                 \
  }                                             \
  }


#else

#define ERRMSG(str) {}
#define WARNMSG(str) {}

#define ASSERT_NOTNULL(p) {}
#define ASSERT_NULL(p)    {}

#define ASSERT_TRUE(p)    {}
#define ASSERT_FALSE(p)   {}


#endif


#endif /* COMMON_H  */
