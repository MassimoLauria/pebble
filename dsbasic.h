/*
   Copyright (C) 2010, 2011 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-17, venerdì 17:13 (CET) Massimo Lauria"
   Time-stamp: "2011-01-12, mercoledì 16:53 (CET) Massimo Lauria"

   Description::

   Basic Data Structure Implementation: list, queue, deque, stack.


*/


/* Preamble */
#ifndef  DSBASIC_H
#define  DSBASIC_H

#include <stdlib.h>

/********************************************************************************
                     LINKED LISTS
 ********************************************************************************/


/* Basic list memory cell */
struct LinkedListHandle {
  void   *data;
  struct LinkedListHandle *next;
};

/* Linked list object */
typedef struct  {
  struct LinkedListHandle *head;
  struct LinkedListHandle *tail;

  struct LinkedListHandle *cursor;
  struct LinkedListHandle *before_cursor;  /* In simple lists we need this */

} LinkedList;


Boolean isconsistentSL(LinkedList *list);
Boolean isemptySL(LinkedList *list);
Boolean iscursorvalidSL(LinkedList *list);

LinkedList *newSL();
void disposeSL(LinkedList *l);

void insertSL(LinkedList *l,void *data,Boolean before);
void appendSL(LinkedList *l,void *data);
void delete_and_nextSL(LinkedList *l);
void pushSL(LinkedList *l,void *data);

void resetSL(LinkedList *l);
void nextSL(LinkedList *l);
void *getSL(LinkedList *l);

/********************************************************************************
                     DOUBLE LINKED LISTS
 ********************************************************************************/

/* Double linked list memory cell */
struct DLinkedListHandle {
  void   *data;
  struct DLinkedListHandle *next;
  struct DLinkedListHandle *prev;
};

/* Double Linked list object */
typedef struct  {
  struct DLinkedListHandle *head;
  struct DLinkedListHandle *tail;

  struct DLinkedListHandle *cursor;

} DLinkedList;

Boolean isconsistentDL(DLinkedList *list);
Boolean isemptyDL(DLinkedList *list);
Boolean iscursorvalidDL(DLinkedList *list);

DLinkedList* newDL();
void disposeDL(DLinkedList *l);

void insertDL(DLinkedList *l,void *data,Boolean before);
void extendDL(DLinkedList *l,void *data,Boolean to_tail);
void delete_and_rightDL(DLinkedList *l);
void delete_and_leftDL (DLinkedList *l);


#define appendheadDL(dl,d) (extendDL((dl),(d),0))
#define appendDL(dl,d)     (extendDL((dl),(d),1))


void resetDL(DLinkedList *l);
void nextDL(DLinkedList *l);
void *getDL(DLinkedList *l);


/********************************************************************************
                     OTHER BASIC DATA STRUCTURES
 ********************************************************************************/

typedef LinkedList  Queue;
typedef DLinkedList Deque;
typedef LinkedList  Stack;

/* Queue is realized with LinkedList in which the cursos always stays
   at the beginning. */
#define enqueue(q,d) (appendSL((q),(d)))
#define pop(q)       (delete_and_nextSL(q))

/* Deque is realized with Double LinkedList */
#define  leftenqueue(dq,d) (extendDL((dq),(d),0))
#define rightenqueue(dq,d) (extendDL((dq),(d),1))

/* Stack is realized with LinkedList */
#define  push(st,d) (pushSL((q),(d)))


#endif /* DSBASIC_H */
