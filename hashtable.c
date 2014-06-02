/*
   Copyright (C) 2010, 2011, 2012, 2014 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-18, sabato 01:23 (CET) Massimo Lauria"
   Time-stamp: "2014-06-02, 09:10 (EDT) Massimo Lauria"

   Description::

   An very rought implementation of hash table.


*/

/* Preamble */
#include <stdlib.h>
#include <assert.h>

#include "common.h"
#include "dsbasic.h"
#include "hashtable.h"

extern long int random(void);

Boolean isconsistentDict(Dict *d) {
  assert(d);
  assert(d->size <= d->allocation);
  assert(d->buckets);

  assert(d->key_function);
  assert(d->eq_function);

#if defined(HASHTABLE_DEBUG)
  LinkedList *l;

  for(size_t i=0;i<d->size;i++) {

    l=d->buckets[i];

    assert(isconsistentSL(l));

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

  LinkedList *ll;
  void       *ptr;

  assert(d);

  /* Remove from memory all objects in the dictionary */
  if (d->dispose_function!=NULL) {
    for(size_t i=0;i<d->size;i++) {
      ll=d->buckets[i];
      resetSL(ll);
      while(iscursorvalidSL(ll)) {
        ptr=getSL(ll);
        d->dispose_function(ptr);
        nextSL(ll);
      }
    }
  }

  /* Remove data structure from memory */
  for(size_t i=0;i<d->size;i++) disposeSL(d->buckets[i]);
  free(d->buckets);
  free(d);
}


Dict *newDict(size_t allocation) {

  Dict *d=(Dict *)malloc(sizeof(Dict));
  assert(d);

  size_t r=random() % (allocation >> 4);
  d->allocation = allocation;
  d->size = allocation - (allocation >> 4) + r;

  d->key_function = NULL;
  d->eq_function  = NULL;
  d->dispose_function = NULL;

  d-> buckets = (LinkedList**)calloc(d->size,sizeof(LinkedList*));
  for(size_t i=0;i<d->size;i++) {
    d->buckets[i]=newSL();
  }
  assert(d->buckets);
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

  assert(d);
  assert(result);
  assert(d->eq_function);
  LinkedList *ll=NULL;

  /* Compute the hash and then find the position in the array */
  result->key    = d->key_function(data);
  result->bucket = result->key % d->size;
  result->value  = NULL;
  result->hops=0;

  Boolean (*cmp)(void *,void *)=d->eq_function;
  void *ptr;

  ll=d->buckets[result->bucket];
  resetSL(ll);
  while(iscursorvalidSL(ll)) {

    ptr=getSL(ll);

    if (  cmp(data,ptr)  ) {
      result->value=ptr;
      return;
    }
    result->hops++;
    nextSL(ll);
  }
  assert(result->value==NULL);
  return;
}


/*
 *  It is equivalent to write in behaviour, but assumes it does not
 *  query the dictionary before insertion. It assumes that the result
 *  of such query is in `result'.
 */
void unsafe_noquery_writeDict(Dict *d,DictQueryResult *const result,void *data) {

  assert(d);
  assert(result);
  assert(result->key    == d->key_function(data));
  assert(result->bucket == result->key % d->size);

  LinkedList      *ll  = d->buckets[result->bucket];
  assert(ll);

  if (result->value==NULL) {
    /* The configuration does not occur in the dictionary */
    appendSL(ll,data);
  } else {
    /* The configuration occur, so we update the old record if there's the need. */
    /* We use an internal of LinkedList!! */
    assert(d->eq_function(data,result->value));
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

  assert(d);
  assert(result);

  queryDict(d,result,data);
  unsafe_noquery_writeDict(d,result,data);

}

/**
 * Print the histogram of the hashtable
 *
 * @param stream Output file
 * @param d Pointer to the dictionary
 */
void histogramDict(FILE *stream,Dict *d) {

  unsigned int len;
  unsigned int tot;

  struct LinkedListHandle *cursor;

  if (d==NULL) return;

  fprintf(stream,"HASHTABLE_HISTOGRAM\n");
  fprintf(stream,"SIZE %lu\nALLOCATION %lu\n",d->size,d->allocation);

  tot=0;
  for(size_t i=0;i<d->size;i++) {
    if (i % 50 == 0) fprintf(stream,"\n");
    len=0;
    cursor=d->buckets[i]->head;
    while(cursor) { len++; cursor=cursor->next;}
    tot+=len;
    fprintf(stream,"%u ",len);
  }

  fprintf(stream,"\nHASHTABLE_HISTOGRAM\n");
}








