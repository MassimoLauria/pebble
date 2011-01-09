/*
  Massimo Lauria, 2010

  This incomplete  program is supposed  to compute the  black pebbling
  number of a graph.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "common.h"
#include "dag.h"
#include "pebbling.h"
#include "hashtable.h"



#define HASH_TABLE_SPACE_SIZE    0x0FFF

/*
 * To  use the dictionary  with PebbleConfiguration  we must  tell the
 * dictionary how to compare two configurations and how to hash them.
 */
size_t   hashPebbleConfiguration(void *data) {

  if (data==NULL) return 0;

  PebbleConfiguration *ptr=(PebbleConfiguration *)data;
  size_t hash = ptr->white_pebbled ^ ptr->black_pebbled;
  return hash;
}

Boolean  samePebbleConfiguration(void *A,void *B) {
  if (A==NULL && B==NULL) { return TRUE; } /* Both null */
  if (A==NULL || B==NULL) { return FALSE;} /* Just one null */

  PebbleConfiguration *pA,*pB;
  pA=(PebbleConfiguration*)A;
  pB=(PebbleConfiguration*)B;

  /* Compare the  important data; we just want  configuration to match
     if the  pebble set is the  same.  We DON'T  want to differentiate
     because of pebbling cost, since  the collistion is what we use to
     update the cost function. */
  if (pA->black_pebbled != pB->black_pebbled) return FALSE;
  if (pB->white_pebbled != pB->white_pebbled) return FALSE;

  return TRUE;
}


/*
 *  The graph of configuration is  a directed graph in which each node
 *  is  indexed by  an hash  of the  configuration it  represents, and
 *  contains the  array of the HASHED configuration  that follows.  We
 *  cache the number of pebbles  required to reach a configuration and
 *  the  previous  configuration  in   the  chain.   Notice  that  the
 *  plausible configurations are  not that many.  So we  need to use a
 *  sparse representation.
 */
/* A configuration is sane only if linked to configurations in the
   appropriate dictionary */
Boolean CheckDictConsistency(DAG *g,PebbleConfiguration *s,Dict *dict) {

  ASSERT_NOTNULL(g);
  ASSERT_NOTNULL(s);
  ASSERT_NOTNULL(dict);

  ASSERT_TRUE(isconsistent_DAG(g));
  ASSERT_TRUE(isconsistent_PebbleConfiguration(g,s));
  ASSERT_TRUE(isconsistentDict(dict));

  ASSERT_TRUE(g->size < MAX_VERTICES);

  return TRUE;
}

void output_pebbling_strategy(DAG *g,PebbleConfiguration *ptr) {
  while(ptr) {
    print_dot_Pebbling(g,ptr,"X",NULL);
    ptr=ptr->previous_configuration;
  }
}


/* Each pebble configuration has a  number of neightbours less than or
   equal to  the number of vertices.   For each vertex  you can either
   add  a pebble  (if  possible)  or remove  one  (if present).
*/

PebbleConfiguration *compute_new_configuration(Vertex v, DAG *g,PebbleConfiguration *old) {

  PebbleConfiguration *nconf=NULL;

  if (isactive(v,g,old)==TRUE && isblack(v,g,old)==FALSE) {
  /* If an unpebbled vertex can be pebbled, place a pebble on it */

    nconf=copy_PebbleConfiguration(old);
    nconf->black_pebbled |= (0x1 << v);
    nconf->pebbles     += 1;
    if (nconf->pebbles > nconf->pebble_cost)
      nconf->pebble_cost = nconf->pebbles;
    return nconf;

  } else if (isblack(v,g,old))  {
  /* If it is a black pebbled vertex, remove it */
    nconf=copy_PebbleConfiguration(old);
    nconf->black_pebbled ^= (0x1 << v);
    nconf->pebbles     -= 1;
  }

  return nconf;
}



/* Explore the space of pebbling strategies.  The output is given as a
   sequence of vertices, because at  any point in a pebbling, there is
   a unique minimal move that can  be performed on a vertex, given its
   status.

   If there is  a pebble on the vertex, such  pebble shoud be removed.
   If there  is not, then either a  white or a black  pebble should be
   added.  In case both are possible, it is always convenient to add a
   black one.

   Notice that  any pebbling  induce a dual  pebbling with  a reversed
   sequence  of vertices.   Thus the  output of  this function  can be
   interpreted in both directions.

   INPUT:

         -- DAG: the graph  to pebble (with few vertices  and a single
            sink).

         -- upper_bound:   the  maximum   number  of  pebbles  in  the
                         configuration,   if   such   number  is   not
                         sufficient,  the  the  computation will  fail
                         gracefully without finding the pebblingx */
/* {{{ */ void pebbling_strategy(DAG *g,unsigned int upper_bound) {

  /* Dictionary data structure */
  Dict *D = newDict(HASH_TABLE_SPACE_SIZE);
  D->key_function = hashPebbleConfiguration;
  D->eq_function  = samePebbleConfiguration;
  ASSERT_TRUE(isconsistentDict(D));

  /* PROLOGUE ----------------------------------- */
  if (g->size > MAX_VERTICES) {
    fprintf(stderr,
            "Error in search procedure: the graph is too "
            "big for the optimized data structures.");
    exit(-1);
  }

  if (g->sink_number!=1) {
    fprintf(stderr,
            "Error in search procedure: the graph has more than "
            "one sink vertex.");
    exit(-1);
  }

  /* SEARCH ALGORITHM */
  /* The first attempt  to implement this program will  use a standard
     graph reachability algorithm for directed graphs */

  /* Since  graphs of  configuration will  be  very large,  we try  to
     represent  a configuration  with the  smallest  memory footprint.
     Furthermore  there  will  be  a  lot  of  useless  or  non  valid
     configurations, thus we produce configurations on demand.  We use
     an hash to access configurations.

     N.B. As a  futue option: we could use a ZDD  for keeping track of
     visited configurations.
  */

  /* Initial status of the search procedure */
  Queue *Q=newSL();
  LinkedList *Trash=newSL();
  DictQueryResult res;
  PebbleConfiguration *nptr,*ptr=new_PebbleConfiguration();
  PebbleConfiguration *final=NULL;
  /* Initial status of dictionary */
  writeDict(D,ptr);
  enqueue(Q,ptr);
#ifdef DEBUG
  int i=0; char buffer[20];   /* Buffer for printing debug
                                 informations. */
#endif


  /* Pick a pebbling status from the queue, produce the followers, and
     put in the queue the ones that haven't been analized yet or the
     one with a better cost.*/
  for(resetSL(Q);!isemptySL(Q);delete_and_nextSL(Q)) {
    ASSERT_TRUE(isconsistentDict(D));
    ptr=(PebbleConfiguration*)getSL(Q);

#ifdef DEBUG
    /* Print pebbling configuration */
    i++;
    sprintf(buffer,"A%d",i);
    print_dot_Pebbling(g, ptr,buffer,NULL);
#endif


    /* Since the value of `upper_bound' is updated during the process,
       we need to check again if queued element have too many pebbles. */
    if (ptr->pebble_cost > upper_bound) {
      appendSL(Trash,ptr);
      continue;
    }
    ASSERT_TRUE(ptr->pebbles <= upper_bound);


    /* Check if final configuration has been reached, and update the
       upper bound. */
    if ((isblack(g->sinks[0],g,ptr))
        &&
        (final==NULL || final->pebble_cost > ptr->pebble_cost)) {
      final=ptr;
      upper_bound = ptr->pebble_cost;
      continue; /* Correct only for black pebbles */
    }

    /* Otherwise compute the next configurations */
    for(Vertex v=0;v<g->size;v++) {


      /* New configurations */
      nptr=compute_new_configuration(v,g,ptr);

      if (nptr==NULL) continue;

      if (nptr->pebble_cost > upper_bound) { /* Is it above the upper bound? */
        dispose_PebbleConfiguration(nptr);
        continue;
      }

      /* Valid new configurations */
      res=queryDict(D,nptr);

      if (res.value==NULL ||
          ((PebbleConfiguration*)res.value)->pebble_cost > nptr->pebble_cost ) {
        /* New or improved configuration */
        writeDict(D,nptr);
        enqueue(Q,nptr);
        nptr->previous_configuration = ptr;
        nptr->last_changed_vertex = v;
      } else {
        /* No improvements */
        dispose_PebbleConfiguration(nptr);
      }

    }

    /* Save the analyzed configuration for later retrieval */
    appendSL(Trash,ptr);
  }



  /* EPILOGUE --------------------------------- */
  /* free memory, free Mandela! */
  output_pebbling_strategy(g,final);
}
/* }}} */


/*
 *  The example test program creates two pyramid graphs and produces
 *  the OR-product graph of them.  Then it prints the DOT
 *  representation of such graphs.
 */

int main(int argc, char *argv[])
{

  DAG *A=piramid(3);
  pebbling_strategy(A,10);
  exit(0);
}



