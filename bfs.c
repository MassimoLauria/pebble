/*
  Massimo Lauria, 2010

  Implementation of a Breadth-First-Search for the Black-White
  Pebbling fo A Graph.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "common.h"
#include "dag.h"
#include "dsbasic.h"
#include "pebbling.h"
#include "hashtable.h"
#include "timedflags.h"
#include "statistics.h"

#define HASH_TABLE_SPACE_SIZE    0x01FFFFF
#define REPORT_INTERVAL          20


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
  if (pA->sink_touched  != pB->sink_touched ) return FALSE;
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
Boolean CheckRuntimeConsistency(DAG *g,Dict *dict) {

  ASSERT_NOTNULL(g);
  ASSERT_NOTNULL(dict);

  ASSERT_TRUE(isconsistent_DAG(g));
  ASSERT_TRUE(isconsistentDict(dict));

  ASSERT_TRUE(g->size < BITTUPLE_SIZE);

#ifdef HASHTABLE_DEBUG

  if (!isconsistentDict(dict)) return FALSE;

  LinkedList cur,*l;

  for(size_t i=0;i<dict->size;i++) {

    l=dict->buckets[i];

    /* Chech if all elements are appropriates pebblings. */
    for(resetSL(l);iscursorvalidSL(l);nextSL(l)) {
      isconsistent_PebbleConfiguration(g,getSL(l));
      forkcursorSL(l,&cur);
      nextSL(&cur);
      while(iscursorvalidSL(&cur)) {
        if (dict->eq_function(getSL(l),getSL(&cur))) return FALSE;
        nextSL(&cur);
      }
    }

  }
#endif
  return TRUE;
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
                         gracefully without finding the pebbling. */
PebbleConfiguration *bfs_pebbling_strategy(DAG *g,unsigned int upper_bound) {

  /* PROLOGUE ----------------------------------- */
  if (g->size > BITTUPLE_SIZE) {
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

  /* Dictionary data structure */
  Dict *D = newDict(HASH_TABLE_SPACE_SIZE);
  DictQueryResult res;               /* Query results */
  D->key_function = hashPebbleConfiguration;
  D->eq_function  = samePebbleConfiguration;

  /* Data structures for BFS */
  Queue          *Q=newSL();    /* Configuration to be processed */
  LinkedList *Trash=newSL();    /* Fully processed configurations */

  /* Initial empty configuration */
  PebbleConfiguration *empty=new_PebbleConfiguration();
  writeDict(D,&res,empty);
  enqueue  (Q,empty);

  /* Additional variables */
  PebbleConfiguration *ptr =NULL;    /* Configuration to be processed */
  PebbleConfiguration *nptr=NULL;    /* Configuration to be queued for later processing (maybe) */
  PebbleConfiguration *final=NULL;   /* Cheapest final configuration so far */

  /* Consistency test of data structures */
  ASSERT_TRUE(isconsistentDict(D));
  ASSERT_TRUE(isconsistentSL(Q));

  /* Data structure for statistics collection */
  STATS_CREATE(Stat);
  STATS_SET(Stat,first_queuing,1);
  STATS_SET(Stat,queued,1);

#ifdef PRINT_RUNNING_STATS
  Counter tmp;
  for(int p=0;p<=upper_bound;p++) {
    tmp=1; /* How many configurations with p pebbles? */
    /* We compute binom(n,p)*2^p, with n=the size of the graph and p
       the number of pebbles.  We first computing 2^p*n!/(n-p)! which
       is integer, then we divide for p!. In this way all intermediate
       values are integers. Since p << n there is no much danger of
       overflow if the total count does not overflow. */
#if WHITE_PEBBLES==1 && BLACK_PEBBLES==1
    for(int k=1;k<=p;k++) { tmp *= 2*( g->size - k + 1); }
    for(int k=2;k<=p;k++) { tmp /= k; }
    STATS_ADD(Stat,search_space,2*tmp); /* Sink maybe touched or not */
#else
    for(int k=1;k<=p;k++) { tmp *= ( g->size - k + 1); }
    for(int k=2;k<=p;k++) { tmp /= k; }
    if (p!=upper_bound)
      { STATS_ADD(Stat,search_space,2*tmp);} /* Sink maybe pebbled or not */
    else
      { STATS_ADD(Stat,search_space,tmp); }
#endif
  }
#endif


  /* Pick a pebbling status from the queue, produce the followers, and
     put in the queue the ones that haven't been analized yet or the
     one with a better cost.*/
  for(resetSL(Q);!isemptySL(Q);delete_and_nextSL(Q)) {

#ifdef PRINT_RUNNING_STATS
    if (print_running_stats_flag) {
      STATS_ADD(Stat,clock,REPORT_INTERVAL);
      fprintf(stderr,"\nClock %llu: Report for graph on %u vertices, upper bound=%u:",
              STATS_GET(Stat,clock),
              g->size,
              upper_bound);
      if (!CheckRuntimeConsistency(g,D)) {
        fprintf(stderr,"\nWARNING!!! HASHTABLE INCONSISTENT!!!");
      }
      fprintf(stderr,"\n\n");
      STATS_REPORT(Stat);
      print_running_stats_flag=0;
    }
#endif

    /* Get an element from the queue */
    ptr=(PebbleConfiguration*)getSL(Q);
    ASSERT_TRUE(isconsistent_PebbleConfiguration(g,ptr));

    /* Check because `upper_bound' is updated during the process */
    if (ptr->pebble_cost > upper_bound) {
      appendSL(Trash,ptr);
      STATS_INC(Stat,queued_and_discarded);
      continue;
    }

    /* This configuration is going to be processed */
    STATS_INC(Stat,processed);

    /* Check if it is final */
    if (isfinal(g,ptr)
        &&
        (final==NULL || final->pebble_cost > ptr->pebble_cost)) {
      final=ptr;
      upper_bound = ptr->pebble_cost;
      STATS_INC(Stat,final);
      continue;
    }

    /* Otherwise compute the next configurations */
    for(Vertex v=0;v<g->size;v++) {

      /* Produce a new configurations */
      nptr=next_PebbleConfiguration(v,g,ptr);
      if (nptr==NULL) {
        continue;
      } else {
        STATS_INC(Stat,offspring);
      }


      if (nptr->pebble_cost > upper_bound) { /* Is it above the upper bound? */
        STATS_INC(Stat,above_upper_bound);
        dispose_PebbleConfiguration(nptr);
        continue;
      }

      /* Evaluate the configuration... */
      queryDict(D,&res,nptr);
      STATS_INC(Stat,dict_queries);
      STATS_ADD(Stat,dict_hops,res.hops);

      if (res.value==NULL)  {  /* Never encountered before */
        nptr->previous_configuration = ptr;
        nptr->last_changed_vertex = v;
        unsafe_noquery_writeDict(D,&res,nptr);
        enqueue(Q,nptr);
        STATS_INC(Stat,queued);
        STATS_INC(Stat,first_queuing);
        STATS_INC(Stat,dict_misses);
        STATS_INC(Stat,dict_writes);
      } else if (((PebbleConfiguration*)res.value)->pebble_cost > nptr->pebble_cost ) {
                              /* Cost improvement */
        nptr->previous_configuration = ptr;
        nptr->last_changed_vertex = v;
        unsafe_noquery_writeDict(D,&res,nptr);
        enqueue(Q,nptr);
        STATS_INC(Stat,queued);
        STATS_INC(Stat,requeuing);
        STATS_INC(Stat,dict_writes);
      } else {               /* Old with no improvements */
        STATS_INC(Stat,suboptimal);
        dispose_PebbleConfiguration(nptr);
      }

    }

    /* Save the analyzed configuration for later retrieval */
    appendSL(Trash,ptr);
  }



  /* EPILOGUE --------------------------------- */
  /* free memory, free Mandela! */
#ifdef PRINT_RUNNING_STATS
  fprintf(stderr,"FINAL REPORT:\n\n");
  if (!CheckRuntimeConsistency(g,D)) {
    fprintf(stderr,"\nWARNING!!! HASHTABLE INCONSISTENT!!!");
  }
  STATS_REPORT(Stat);
#endif
  return final;
}

