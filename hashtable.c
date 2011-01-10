/*
   Copyright (C) 2010, 2011 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-18, sabato 01:23 (CET) Massimo Lauria"
   Time-stamp: "2011-01-10, lunedì 13:04 (CET) Massimo Lauria"

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

  for(size_t i=0;i<d->size;i++) {
    if (!isconsistentSL(d->buckets[i])) return FALSE;
  }
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
DictQueryResult queryDict(Dict* d,void *data) {

  ASSERT_NOTNULL(d);

  DictQueryResult res={ 0UL, NULL,0,0 };
  LinkedList      *ll=NULL;

  /* Compute the hash and then find the position in the array */
  res.key    = d->key_function(data);
  res.bucket = res.key % d->size;
  res.value  = NULL;
  res.hops=0;

  ll=d->buckets[res.bucket];
  resetSL(ll);
  while(iscursorvalidSL(ll)) {
    if (  d->eq_function(data,getSL(ll))  ) {
      res.value=getSL(ll);
      return res;
    }
    res.hops++;
    nextSL(ll);
  }

  return res;
}

/*
 *  The update function has the following semantic: if the
 *  configuration is absent from the dictionary, then a new record is
 *  added to the dictionary, otherwise an old one is overwritten.  The
 *  function returns a pointer to the old one (NULL if any).
 */
void *writeDict(Dict *d,void *data) {

  DictQueryResult res;
  LinkedList      *ll;

  ASSERT_NOTNULL(d);

  res = queryDict(d,data);
  ll  = d->buckets[res.bucket];

  if (res.value==NULL) {
    /* The configuration does not occur in the dictionary */
    appendSL(d->buckets[res.bucket],data);
    return NULL;
  } else {
    /* The configuration occur, so we update the old record if there's the need. */
    /* We use an internal of LinkedList!! */
    ASSERT_NOTNULL(d->eq_function(data,res.value));
    ll->cursor->data=data;

    return res.value;
  }
}


