/*
   Copyright (C) 2010, 2011 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-18, sabato 01:23 (CET) Massimo Lauria"
   Time-stamp: "2011-01-13, giovedì 12:08 (CET) Massimo Lauria"

   Description::

   An very rought implementation of hash table.


*/

/* Preamble */
#include <stdlib.h>

#include "common.h"
#include "dsbasic.h"
#include "hashtable.h"

extern long int random(void);

Boolean isconsistentDict(Dict *d) {
  ASSERT_NOTNULL(d);
  ASSERT_TRUE(d->size <= d->allocation);
  ASSERT_NOTNULL(d->buckets);

  ASSERT_NOTNULL(d->key_function);
  ASSERT_NOTNULL(d->eq_function);

#if defined(HASHTABLE_DEBUG)
  LinkedList *l;

  for(size_t i=0;i<d->size;i++) {

    l=d->buckets[i];

    ASSERT_TRUE(isconsistentSL(l));

    /* Chech if all elements have the same hash and if they are in the
       appropriate bucket. It uses linked list internals. */
    for(resetSL(l);iscursorvalidSL(l);nextSL(l)) {
      if ((d->key_function(getSL(l)) % d->size)!=i) {
        return FALSE;
      }
    }
  }
#endif
  return TRUE;
}


void disposeDict(Dict *d) {

  if (d==NULL) return;

  for(size_t i=0;i<d->size;i++) {
    disposeSL(d->buckets[i]);
  }

  free(d->buckets);
  free(d);

}


Dict *newDict(size_t allocation) {

  Dict *d=(Dict *)malloc(sizeof(Dict));
  ASSERT_NOTNULL(d)

  size_t r=random() % (allocation >> 4);
  d->allocation = allocation;
  d->size = allocation - (allocation >> 4) + r;

  d->key_function = NULL;
  d->eq_function  = NULL;

  d-> buckets = (LinkedList**)calloc(d->size,sizeof(LinkedList*));
  for(size_t i=0;i<d->size;i++) {
    d->buckets[i]=newSL();
  }
  ASSERT_NOTNULL(d->buckets);
  return d;

}

/*
   The query consist of a pointer to an object. The unique key of such
   object is computed.  The corresponding bucket is explored, to find
   out if the object exists there. A DictQueryResult object with value
   field set to NULL means the object is not in the dictionary. The
   cursor of the LinkedList which implements the bucket stays on the
   found element.
 */
void queryDict(Dict* d,DictQueryResult *const result,void *data) {

  ASSERT_NOTNULL(d);
  ASSERT_NOTNULL(result);
  LinkedList      *ll=NULL;

  /* Compute the hash and then find the position in the array */
  result->key    = d->key_function(data);
  result->bucket = result->key % d->size;
  result->value  = NULL;
  result->hops=0;

  ll=d->buckets[result->bucket];
  resetSL(ll);
  while(iscursorvalidSL(ll)) {
    if (  d->eq_function(data,getSL(ll))  ) {
      result->value=getSL(ll);
      return;
    }
    result->hops++;
    nextSL(ll);
  }
  ASSERT_NULL(result->value);
  return;
}


/*
 *  It is equivalent to write in behaviour, but assumes it does not
 *  query the dictionary before insertion. It assumes that the result
 *  of such query is in `result'.
 */
void unsafe_noquery_writeDict(Dict *d,DictQueryResult *const result,void *data) {

  ASSERT_NOTNULL(d);
  ASSERT_NOTNULL(result);
  ASSERT_TRUE(result->key    == d->key_function(data));
  ASSERT_TRUE(result->bucket == result->key % d->size);

  LinkedList      *ll  = d->buckets[result->bucket];
  ASSERT_NOTNULL(ll);

  if (result->value==NULL) {
    /* The configuration does not occur in the dictionary */
    appendSL(ll,data);
  } else {
    /* The configuration occur, so we update the old record if there's the need. */
    /* We use an internal of LinkedList!! */
    ASSERT_TRUE(d->eq_function(data,result->value));
    ll->cursor->data=data;
  }
  return;
}

/*
 *  The update function has the following semantic: if the
 *  configuration is absent from the dictionary, then a new record is
 *  added to the dictionary, otherwise an old one is overwritten.  The
 *  DictQueryResult object is filled with the result of the query of
 *  the old value (may be NULL) query.
 */
void writeDict(Dict *d,DictQueryResult *const result,void *data) {

  ASSERT_NOTNULL(d);
  ASSERT_NOTNULL(result);

  queryDict(d,result,data);
  unsafe_noquery_writeDict(d,result,data);

}
