/*
   Copyright (C) 2010 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-17, venerdì 17:11 (CET) Massimo Lauria"
   Time-stamp: "2010-12-18, sabato 18:11 (CET) Massimo Lauria"

   Description::

   Basic implementation of simple data structures as queue, lists, deque.


*/

/* Preamble */
#include <stdlib.h>

#include "common.h"
#include "dsbasic.h"


/********************************************************************************
                     LINKED LISTS
 ********************************************************************************/

Boolean isconsistentSL(LinkedList *list) {

  ASSERT_NOTNULL(list);

  if (list->head==NULL || list->tail==NULL) {
    if (list->head ||
        list->tail ||
        list->cursor || list->before_cursor) return FALSE;
  }

  /* Is tail the last element */
  if (list->tail && list->tail->next!=NULL) return FALSE;

  /* TODO: test for circularity */

  return TRUE;
}

Boolean isemptySL(LinkedList *list) {
  ASSERT_NOTNULL(isconsistentSL(list));
  return (list->head==NULL);
}

Boolean iscursorvalidSL(LinkedList *list) {
  return (list->cursor!=NULL);
}



/* Create an empty linked list */
LinkedList *newSL(void) {
  LinkedList *list;
  list=(LinkedList*)malloc(sizeof(LinkedList));

  ASSERT_NOTNULL(list);

  list->head=NULL;
  list->tail=NULL;
  list->cursor=NULL;
  list->before_cursor =NULL;

  return list;
}

/*
   If the cursor is after the tail, it is considered to be invalid.
   Thus an empty list is not considered to have a cursor.

   Add an element either before or after the cursor.

   If the cursor is at the head, the new element will be the
   new head. The new cursor position depends on the flag `before'
*/
void insertSL(LinkedList *l,void *data,Boolean before) {

  ASSERT_NOTNULL(l);
  ASSERT_NOTNULL(isconsistentSL(l));
  ASSERT_TRUE(iscursorvalidSL(l));
  ASSERT_FALSE(isemptySL(l));

  /* Allocation */
  struct LinkedListHandle *lelem;
  lelem=(struct LinkedListHandle*)malloc(sizeof(struct LinkedListHandle));
  ASSERT_NOTNULL(lelem);

  /* Insertion */
  lelem->data=data;
  lelem->next=l->cursor;
  if (l->before_cursor) l->before_cursor->next=lelem;

  /* Update head and tails */
  if (lelem->next==l->head) {
    l->head=lelem;
  }
  if (l->tail->next==lelem) {
    l->tail=lelem;
  }
  if (before) {
    l->before_cursor=lelem;
    l->cursor = lelem->next;
  } else {
    l->cursor = lelem;
  }

  ASSERT_NOTNULL(isconsistentSL(l));

}

/* Notice that we do not make any check on data pointer. It can be as
   well NULL.  Appended elements will always be after any valid
   cursor. */
void appendSL(LinkedList *l,void *data) {

  ASSERT_NOTNULL(l);
  ASSERT_NOTNULL(isconsistentSL(l));

  /* Allocation */
  struct LinkedListHandle *lelem;
  lelem=(struct LinkedListHandle*)malloc(sizeof(struct LinkedListHandle));
  ASSERT_NOTNULL(lelem);
  lelem->data=data;
  lelem->next=NULL;

  if (l->tail) l->tail->next=lelem; /* Append it! */
  if (isemptySL(l)) l->head=lelem;    /* Possible new head */
  l->tail=lelem;                    /* New tail */

  ASSERT_NOTNULL(isconsistentSL(l));

}


void disposeSL(LinkedList *l) {
  struct LinkedListHandle *ch,*lh;
  ASSERT_NOTNULL(l);
  lh=l->head;
  while(lh) { ch=lh; lh=lh->next; free(ch); }
  free(lh);
}

/* Cursor managing */
void resetSL(LinkedList *l) {
  ASSERT_NOTNULL(l);
  l->cursor=l->head;
  l->before_cursor=NULL;
}

void nextSL(LinkedList *l) {
  ASSERT_NOTNULL(l);
  ASSERT_NOTNULL(l->cursor);
  l->before_cursor=l->cursor;
  l->cursor=l->cursor->next;
}

void *getSL(LinkedList *l) {
  ASSERT_NOTNULL(l);
  ASSERT_NOTNULL(l->cursor);
  return l->cursor->data;
}



/********************************************************************************
                     DOUBLE LINKED LISTS
 ********************************************************************************/

Boolean isconsistentDL(DLinkedList *list) {

  ASSERT_NOTNULL(list);

  /* If anyone is NULL, both must be */
  if (list->head==NULL || list->tail==NULL) {
    if (list->head || list->tail) return FALSE;
  }

  /* Are tail and head at the extremes? */
  if (list->tail && list->tail->next!=NULL) return FALSE;
  if (list->head && list->head->prev!=NULL) return FALSE;

  /* Check that prev and next matches */
  if (list->head==NULL) return TRUE;

  struct DLinkedListHandle *ptr=list->head;
  while(ptr->next) {
    if (ptr->next->prev != ptr) return FALSE;
  }

  return TRUE;
}

Boolean isemptyDL(DLinkedList *list) {
  ASSERT_NOTNULL(isconsistentDL(list));
  return (list->head==NULL);
}

Boolean iscursorvalidDL(DLinkedList *list) {
  return (list->cursor!=NULL);
}


/* Create an empty double linked list */
DLinkedList *newDL(void) {
  DLinkedList *dlist;
  dlist=(DLinkedList*)malloc(sizeof(DLinkedList));

  ASSERT_NOTNULL(dlist);

  dlist->head=NULL;
  dlist->tail=NULL;

  return dlist;
}

/*
   If the cursor is after the tail or before the head, it is
   considered to be invalid.  Thus an empty list is not considered to
   have a cursor.

   Add an element either before or after the cursor.

   If the cursor is at the head, the new element will be the
   new head. The new cursor position depends on the flag `before'
*/
void insertDL(DLinkedList *l,void *data,Boolean before) {

  ASSERT_NOTNULL(l);
  ASSERT_NOTNULL(isconsistentDL(l));
  ASSERT_TRUE(iscursorvalidDL(l));
  ASSERT_FALSE(isemptyDL(l));

  /* Allocation */
  struct DLinkedListHandle *lelem;
  lelem=(struct DLinkedListHandle*)malloc(sizeof(struct DLinkedListHandle));
  ASSERT_NOTNULL(lelem);

  /* Insertion */
  lelem->data=data;
  lelem->next=l->cursor;
  lelem->prev=l->cursor->prev;
  if (l->cursor->prev) l->cursor->prev->next=lelem;
  l->cursor->prev=lelem;

  /* Update head and tails */
  if (lelem->next==l->head) {
    l->head=lelem;
  }
  if (lelem->prev==l->tail) {
    l->tail=lelem;
  }
  if (!before) {
    l->cursor = lelem;
  }

  ASSERT_NOTNULL(isconsistentDL(l));

}

/* Notice that we do not make any check on data pointer. It can be as
   well NULL.  We use on implementation and we use macros to define
   two different functions for attaching either to the head or to the
   tail.
 */
void extendDL(DLinkedList *l,void *data,Boolean to_tail) {

  ASSERT_NOTNULL(l);
  ASSERT_NOTNULL(isconsistentDL(l));

  /* Allocation */
  struct DLinkedListHandle *lelem;
  lelem=(struct DLinkedListHandle*)malloc(sizeof(struct DLinkedListHandle));
  ASSERT_NOTNULL(lelem);
  lelem->data=data;
  lelem->next=NULL;
  lelem->prev=NULL;

  if (to_tail) { /* Append to the tail */

    lelem->prev = l->tail;
    if (l->tail) l->tail->next=lelem; /* Append it */
    if (isemptyDL(l)) l->head=lelem; /* Possible new head */
    l->tail = lelem;                  /* New tail */

  } else {       /* Append to the head  */

    lelem->next = l->head;
    if (l->head) l->head->prev=lelem; /* Append it */
    if (isemptyDL(l)) l->tail=lelem; /* Possibile new tail */
    l->head = lelem;                  /* New head */

  }
  ASSERT_NOTNULL(isconsistentDL(l));
}


void disposeDL(DLinkedList *dl) {
  struct DLinkedListHandle *ch,*lh;

  ASSERT_NOTNULL(dl);
  ASSERT_NOTNULL(isconsistentDL(dl));

  lh=dl->head;
  while(lh) { ch=lh; lh=lh->next; free(ch); }
  free(lh);
}


void resetDL(DLinkedList *l) {
  ASSERT_NOTNULL(l);
  l->cursor=l->head;
}

void nextDL(DLinkedList *l) {
  ASSERT_NOTNULL(l);
  ASSERT_NOTNULL(l->cursor);
  l->cursor=l->cursor->next;
}

void prevDL(DLinkedList *l) {
  ASSERT_NOTNULL(l);
  ASSERT_NOTNULL(l->cursor);
  l->cursor=l->cursor->prev;
}


void *getDL(DLinkedList *l) {
  ASSERT_NOTNULL(l);
  ASSERT_NOTNULL(l->cursor);
  return l->cursor->data;
}

