/*
   Copyright (C) 2010 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-16, giovedì 16:36 (CET) Massimo Lauria"
   Time-stamp: "2010-12-20, lunedì 17:50 (CET) Massimo Lauria"

   Description::

   Header containing declaring common functionalities, like lists and queues.

*/

#ifndef COMMON_H
#define COMMON_H
/* Preamble */
#include <stdlib.h>
#include <stdio.h>


/* Bit and Boolean types declarations */
/* Several bit tuples  are used in the code.
 */
typedef long long unsigned int BitTuple;
typedef unsigned char          Boolean;

#define TRUE  1
#define FALSE 0




#ifdef DEBUG

/* Assertion for DEBUG purposes:  If DEBUG constant is not defined,
   then such assertions result in an empty statement. Otherwise they
   will test the correspondent condition and stop the program with an
   appropriate error message.

 */

/* Error and warning messages, useful for debugging */
#define ERRMSG(str) \
  { fprintf(stderr,"ERROR at [%s,%d]: %s",__FILE__,__LINE__,str); }

#define WARNMSG(str) \
  { fprintf(stderr,"WARNING at [%s,%d]: %s",__FILE__,__LINE__,str); }


#define ASSERT_NOTNULL(p) {                       \
    if (!(p)) {                                   \
      ERRMSG("NOT NULL assertion failed.\n");     \
      exit(-1);                                   \
    }                                             \
  }

#define ASSERT_NULL(p) {                        \
    if (p) {                                    \
      ERRMSG("NULL assertion failed.\n");       \
      exit(-1);                                 \
  }                                             \
  }

#define ASSERT_TRUE(p) {                        \
    if (!(p)) {                                 \
      ERRMSG("TRUE assertion failed.\n");       \
      exit(-1);                                 \
    }                                           \
  }


#define ASSERT_FALSE(p) {                       \
    if (p) {                                    \
      ERRMSG("NULL assertion failed.\n");       \
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
