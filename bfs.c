/**
  Massimo Lauria, 2010, 2011, 2012, 2013

  Implementation of  a Breadth-First-Search  for the Black-White  or a
  Reversible Pebbling of a directed acyclic graph.

  @file 
  
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "dag.h"
#include "dsbasic.h"
#include "pebbling.h"
#include "hashtable.h"
#include "statistics.h"

extern void print_dot_PebbleConfiguration(const DAG *g, const PebbleConfiguration *peb,
                                          char *name,char* options);


/**************************************
 * Utilities for pebbling dictionary
 **************************************/

/*
 * The size of the hash is just a bold assumption. I definitely need a
 * serious hash map implementation.
 *
 */
#define HASH_TABLE_SPACE_SIZE    CONFIG_HASHSIZE

/*
 * To use  the dictionary  with PebbleConfiguration  we must  tell the
 * dictionary how to hash them.
 *
 */
size_t hashPebbleConfiguration(void *data) {
  assert(data);
  PebbleConfiguration *ptr=(PebbleConfiguration *)data;

#if BLACK_WHITE_PEBBLING
  return  (size_t)(ptr->white_pebbled * 0x9e3779b9 + ptr->black_pebbled);
#else
  return  (size_t)(ptr->black_pebbled * 0x9e3779b9);
#endif

}

/*
 * An "equality"  function for  pebbling configurations. We  just want
 * configuration to match if the pebble  set is the same (and the sink
 * has  been touched).   We  DON'T want  to  differentiate because  of
 * pebbling cost,  since the collistion is  what we use to  update the
 * cost function.
 *
 */
Boolean  samePebbleConfiguration(void *A,void *B) {

  assert(A);
  assert(B);

  PebbleConfiguration *pA,*pB;
  pA=(PebbleConfiguration*)A;
  pB=(PebbleConfiguration*)B;

  if (pA->black_pebbled != pB->black_pebbled) return FALSE;

#if BLACK_WHITE_PEBBLING
  if (pA->white_pebbled != pB->white_pebbled) return FALSE;
#endif

  if (pA->sink_touched  != pB->sink_touched ) return FALSE;
  return TRUE;
}

void  freePebbleConfiguration(void *data) {
  assert(data);
  dispose_PebbleConfiguration((PebbleConfiguration *)data);
}


/*
 * Runtime consistency checks. 
 *
 * We  verify  that the  elements  in  the dictionary  corresponds  to
 * consistent  pebbling configurations.  Also the  full dictionary  is
 * read  in order  to  produce an  histogram with  the  length of  the
 * buckets. Knowing the length of the buckets is a decent indicator of
 * the  quality of  the hash  function  (at least  for the  particular
 * problem in hand).
 *
 * Since the cost of this test is very expensive, the histogram is not
 * produced even when assertion are enabled.
 */
Boolean CheckRuntimeConsistency(DAG *g,Dict *dict) {

  assert(g);
  assert(dict);
  assert(isconsistent_DAG(g));
  assert(isconsistentDict(dict));
  assert(g->size < BITTUPLE_SIZE);

#ifdef HASHTABLE_DEBUG

  if (!isconsistentDict(dict)) return FALSE;

  LinkedList cur,*l;
  int cnt;
  unsigned long long int tot_cnt=0;

  /* Hash map hops histogram */
  unsigned long long int histogram[40];
  for(int i=0;i<40;i++) { histogram[i]=0; }

  /* Count the elements in each bucket */
  for(size_t i=0;i<dict->size;i++) {

    l=dict->buckets[i];
    cnt=0;

    /* Chech if all elements are appropriate pebblings */
    for(resetSL(l);iscursorvalidSL(l);nextSL(l)) {
      cnt++;
      isconsistent_PebbleConfiguration(g,getSL(l));

      /* No two equal configurations in the bucket */
      forkcursorSL(l,&cur);
      nextSL(&cur);
      while(iscursorvalidSL(&cur)) {
        if (dict->eq_function(getSL(l),getSL(&cur))) return FALSE;
        nextSL(&cur);
      }
      
    }
    if (cnt<39) histogram[cnt]++;
    else histogram[39]++;
    tot_cnt += cnt;
  }
  fprintf(stderr,"Hashed elemenents: %15llu\n",tot_cnt);
  for(int i=0;i<39;i++) { fprintf(stderr," %2d  = % 15llu\n",i,histogram[i]); }
  fprintf(stderr," ... = % 15llu\n",histogram[39]);

#endif  /* HASHTABLE_DEBUG */
  
  return TRUE;
}


/**
   Finalize a black-white or reversible persistent pebbling.

   The breath-first-search  process we use  to find the  pebbling does
   not produce a  complete one, in particular a clean  up phase can be
   appended afterward.

   In  the  case of  persistent  pebbling,  we computed  the  pebbling
   looking for a  transition from a configuration with  a single white
   pebble  on  the sink,  to  a  configuration containing  only  black
   pebbles.  It's dual configuration is indeed the persistent pebbling
   we are looking for, so we are going to reverse it.

   The  black pebbles  in the  final  configuration of  the input  are
   translated to white  pebbles in the output. A  preamble sequence of
   white  pebble placements  is added  to make  the pebbling  formally
   correct.
   
   INPUT:
   
   @param dag the graph we are pebbling

   @param final a pointer to the final configuration found by the BFS.

   OUTPUT:

   @return Pebbling the finalized pebbling.
*/
Pebbling *finalize_persistent_pebbling(const DAG *graph,
                                       PebbleConfiguration *final) {
  assert(graph);
  assert(final);

  PebbleConfiguration *ptr=NULL;
  Pebbling *solution=NULL;
  size_t length=0;
  int    cost =0;
  Vertex i=0;
  
  /* compute the length of the pebbling, without clean up */
  ptr = final;
  while(ptr!=NULL) {
    length++;
    cost = MAX(cost,configurationcost(graph, ptr));
    ptr=ptr->previous_configuration;
  }
  length -= 1; /* initial conf is not a step */

#if BLACK_WHITE_PEBBLING
  /* there may be black pebbles left to clean up which translates into
     white placement after reversing the pebbling.
   */
  length += final->pebbles;
#endif 
  
  /* solution to be filled */
  solution = new_Pebbling(length);
  solution->length = length;
  solution->cost   = cost;

  
  i=0;
#if BLACK_WHITE_PEBBLING
  /* Preamble phase: place some white pebbles, which correspond to the
     black pebbles cleaned at the end of the original pebbling. */
  for(Vertex v=0;v<graph->size;v++) {
    if (isblack(v,graph,final)) { solution->steps[i]=v; ++i; }
  }
#endif
  
  /* Reverse the actual pebbling */
  assert(i == final->pebbles);
  ptr = final;
  while(ptr->previous_configuration!=NULL) {
    solution->steps[i] = ptr->last_changed_vertex;
    ptr=ptr->previous_configuration;
    i++;
  }

  return solution;
}






/**
   Finalize the a black or black-white pebbling, by removing residual
   black pebbles.

   @param dag the graph we are pebbling

   @param final a pointer to the final configuration found by the BFS.
   
   @return Pebbling the finalized pebbling.
*/
Pebbling *finalize_pebbling(const DAG *graph,
                                       PebbleConfiguration *final) {
  assert(graph);
  assert(final);

#if REVERSIBLE_PEBBLING
  assert(0);
#endif

  PebbleConfiguration *ptr=NULL;
  Pebbling *solution=NULL;
  size_t length=0;
  int cost = 0;
  Vertex i=0;

  /* compute the length of the pebbling, without clean up */
  ptr = final;
  while(ptr!=NULL) {
    length++;
    cost = MAX(cost,configurationcost(graph, ptr));
    ptr=ptr->previous_configuration;
  }
  length -= 1; /* initial conf is not a step */
  length += final->pebbles; /* clean up black pebbles */

  /* solution to be filled */
  solution = new_Pebbling(length);
  solution->length = length;
  solution->cost   = cost;

  /* Load the steps in the solution vector
     (steps are in reverse order) */
  i = length - final->pebbles;
  ptr = final;
  while(ptr->previous_configuration!=NULL) {
    solution->steps[--i] = ptr->last_changed_vertex;
    ptr=ptr->previous_configuration;
  }
  assert(i==0);

  /* Reversing the pebbling: final clean up corresponds to
     pebbling placements at the beginning. */
  i=length;
  for(Vertex v=0;v<graph->size;v++) {
    if (isblack(v,graph,final)) { solution->steps[--i]=v; }
  }


  return solution;
}



/**
   Finalize the a reversible pebbling, by repeating the discovered pebbling backward.

   @param dag the graph we are pebbling

   @param final a pointer to the final configuration found by the BFS.
   
   @return Pebbling the finalized pebbling.
*/
Pebbling *finalize_reversible_pebbling(const DAG *graph,
                                       PebbleConfiguration *final) {
  assert(graph);
  assert(final);

#if !REVERSIBLE_PEBBLING
  assert(0);
#endif

  PebbleConfiguration *ptr=NULL;
  Pebbling *solution=NULL;
  size_t length=0;
  int cost = 0;
  Vertex i=0;
  Vertex j=0;

  /* compute the length of the pebbling */
  ptr = final;
  while(ptr!=NULL) {
    length++;
    cost = MAX(cost,configurationcost(graph, ptr));
    ptr=ptr->previous_configuration;
  }
  length -= 1;
  length *= 2;

  /* solution to be filled */
  solution = new_Pebbling(length);
  solution->length = length;
  solution->cost   = cost;

  /* Load the steps in the solution vector
     (steps are in reverse order) */
  i = length/2;
  j = length/2;
  ptr = final;
  while(ptr->previous_configuration!=NULL) {
    solution->steps[--i] = ptr->last_changed_vertex;
    solution->steps[j++] = ptr->last_changed_vertex;
    ptr=ptr->previous_configuration;
  }
  assert(i==0);
  assert(j==length);
  
  return solution;
}




/**
   Explore the space of pebbling strategies.

   We employ a simple breadth-first-search exploration in the graph of
   pebbling configurations, checking if a final configuration
   (e.g. sink has been touched and there are no white pebbles) is
   reachable. The space is explored by looking for pebbling with at
   most N pebbles.
   
   Persistent black-white pebbling is actually measured by placing a
   white pebble on the sink and by trying to complete the
   pebbling. Then the resulting pebbling is reversed.
   
   Since  we are  going  to  explore many  configurations,  we try  to
   represent  a  configuration  with the  smallest  memory  footprint.
   Furthermore  there  will   be  a  lot  of  useless   or  non  valid
   configurations, thus we produce configurations on demand.  We use a
   dictionary to keep track of previously visited configurations.

   N.B. As a future option: we could use a ZDD  for keeping track of
   visited configurations.
   
   INPUT:

   @param DAG the graph  to pebble (with few vertices  and a single
   sink).

   @param upper_bound: the maximum number of pebbles in the
   configurations, if such number is not sufficient, the the
   computation will fail gracefully without finding the pebbling.

   @param persisten_pebbling: whether we count the black white
   pebbling number for a pebbling which leaves a black pebble in the
   sink.

   OUTPUT:

   The output is given as a sequence of vertices, because at any point
   in a pebbling, there is a unique minimal move that can be performed
   on a vertex, given its status.

   If there is  a pebble on the vertex, such  pebble shoud be removed.
   If there is  not, then either a  white or a black  pebble should be
   added.  In  case both  are allowed,  it is always  better to  add a
   black one.

   Notice  that in  case of  black-white or  reversible pebbling,  any
   pebbling  induces  a dual  pebbling  with  a reversed  sequence  of
   vertices.  Thus the  output of this function can  be interpreted in
   both directions.
   
   @return a pebbling if exists, NULL otherwise.

 */
Pebbling *bfs_pebbling_strategy(DAG *g,
                                           unsigned int upper_bound,
                                           Boolean persistent_pebbling) {

  /* PROLOGUE ----------------------------------- */
  if (g->size > BITTUPLE_SIZE) {
    fprintf(stderr,
            "Error in search procedure: the graph is too "
            "big for the optimized data structures.");
    exit(EXIT_FAILURE);
  }

  if (g->sink_number!=1) {
    fprintf(stderr,
            "Error in search procedure: the graph has more than "
            "one sink vertex.");
    exit(EXIT_FAILURE);
  }

  if (upper_bound < 1) { return NULL; } /* No pebbling with zero pebbles */

  
#if (!BLACK_WHITE_PEBBLING && !REVERSIBLE_PEBBLING)
  persistent_pebbling = 0;
#endif
  
  /* Collect statistic on the running */
  STATS_CREATE(Stat);

  /* END OF PROLOGUE ----------------------------------- */
  

  /* Data structures for BFS */
  PebbleConfiguration *initial=new_PebbleConfiguration();
  Queue               *Q=newSL();
  Dict *D = newDict(HASH_TABLE_SPACE_SIZE);

  /* Dictionary setup */
  DictQueryResult res;
  D->key_function = hashPebbleConfiguration;
  D->eq_function  = samePebbleConfiguration;
  D->dispose_function = freePebbleConfiguration;
  
  /* Initial configuration for the BFS */
  enqueue  (Q,initial);
  writeDict(D,&res,initial);
#if BLACK_WHITE_PEBBLING || REVERSIBLE_PEBBLING
  if (persistent_pebbling) {  init_persistent_pebbling(g, initial); }
#endif


  PebbleConfiguration *ptr  =NULL;    /* Configuration to be processed */
  PebbleConfiguration *nptr =NULL;    /* Configuration to be queued for later processing (maybe) */
  PebbleConfiguration *final=NULL;    /* final configuration */

  Boolean (*isfinal)(const DAG *, const PebbleConfiguration *); /*pointer to final configuration tester.*/ 

  if (persistent_pebbling)
    isfinal = isfinal_persistent;
  else 
    isfinal = isfinal_visiting;
  
  /* Consistency test of data structures */
  assert(isconsistentDict(D));
  assert(isconsistentSL(Q));

  STATS_SET(Stat,first_queuing,1);
  STATS_SET(Stat,queued,1);
  STATS_SET(Stat,dict_size,D->size);

  /* The breadth-first-search on the space of pebbling configurations.*/
  for(resetSL(Q); !isemptySL(Q); pop(Q)) {

    if (STATS_TIMER_OFF) {
      STATS_CLOCK_UPDATE(Stat);
      STATS_REPORT(Stat,
                   "\nClock %llu: Report for graph on %u vertices, upper bound=%u:\n",
                   STATS_GET(Stat,clock),
                   g->size,
                   upper_bound);
      STATS_TIMER_RESET();
    }

    /* Get an element from the queue */
    ptr=(PebbleConfiguration*)getSL(Q);
    assert(isconsistent_PebbleConfiguration(g,ptr));
    assert(!isfinal(g,ptr));
    STATS_INC(Stat,processed);

    /* Explore all configurations reachable in one step.  */
    for(Vertex v=0;v<g->size;v++) {

      nptr=next_PebbleConfiguration(v,g,ptr,upper_bound);
      if (nptr==NULL) continue; /* Step corresponding to vertex v is invalid/useless */

      STATS_INC(Stat,offspring);

      /* Find out if it has already been encountered (check in the dictionary) */
      queryDict(D,&res,nptr);
      STATS_INC(Stat,dict_queries);
      STATS_ADD(Stat,dict_hops,res.hops);

      if (res.value==NULL)  {  /* A configuration never encountered before */

        nptr->previous_configuration = ptr;  /* It's origin */
        nptr->last_changed_vertex = v;

        unsafe_noquery_writeDict(D,&res,nptr); /* Mark as encountered (put in the dictionary) */

        if (isfinal(g,nptr)) {               /* Is it the end of the search? */
          final=nptr;
          goto epilogue;
        }

        enqueue(Q,nptr);                       /* Put in queue for later processing */
        STATS_INC(Stat,queued);
        STATS_INC(Stat,first_queuing);

        STATS_INC(Stat,dict_misses);
        STATS_INC(Stat,dict_writes);

      } else {                                 /* Already encountered. No new information. */
        STATS_INC(Stat,suboptimal);
        dispose_PebbleConfiguration(nptr);
      }

    } /* End of neighborhood exploration */

  }/* queue of configurations is empty, end of BFS */


  Pebbling *solution=NULL;
  
epilogue:
  
  /* To get a formally correct pebbling we need to give final touch. */
  if (persistent_pebbling) {

    solution = final ? finalize_persistent_pebbling(g,final) : NULL ;

  } else {
#if REVERSIBLE_PEBBLING
    solution = final ? finalize_reversible_pebbling(g,final) : NULL ;
#else
    solution = final ? finalize_pebbling(g,final) : NULL ;
#endif
  }

  
  STATS_REPORT(Stat,"\nFINAL REPORT (clk. %llu): upper bound=%u:\n\n",
               STATS_GET(Stat,clock),
               upper_bound);

#ifdef HASHTABLE_DEBUG
  assert(!CheckRuntimeConsistency(g,D));
#endif


  /* Free the memory of the data structures */
  if (Q) disposeSL(Q);
  disposeDict(D);

  return solution;
}

