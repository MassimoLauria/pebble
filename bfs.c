/*
  Massimo Lauria, 2010, 2011, 2012, 2013

  Implementation of a Breadth-First-Search for the Black-White
  Pebbling of a directed acyclic graph.

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
#include "timedflags.h"
#include "statistics.h"

#define HASH_TABLE_SPACE_SIZE    0x07FFFFF


/*
 * To use the dictionary with PebbleConfiguration we must tell the
 * dictionary how to compare two configurations and how to hash
 * them. Optionslly also how to deallocate them from memory.
 */
size_t   hashPebbleConfiguration(void *data) {

  assert(data);
  PebbleConfiguration *ptr=(PebbleConfiguration *)data;
  return  (size_t)(ptr->white_pebbled * 0x9e3779b9 + ptr->black_pebbled);
 }

Boolean  samePebbleConfiguration(void *A,void *B) {

  assert(A);
  assert(B);

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

void  freePebbleConfiguration(void *data) {
  assert(data);
  dispose_PebbleConfiguration((PebbleConfiguration *)data);
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

  assert(g);
  assert(dict);

  assert(isconsistent_DAG(g));
  assert(isconsistentDict(dict));

  assert(g->size < BITTUPLE_SIZE);

#ifdef HASHTABLE_DEBUG

  if (!isconsistentDict(dict)) return FALSE;

  LinkedList cur,*l;

  unsigned long long int histogram[40];
  for(int i=0;i<40;i++) { histogram[i]=0; }
  int cnt;
  unsigned long long int tot_cnt=0;


  for(size_t i=0;i<dict->size;i++) {

    l=dict->buckets[i];
    cnt=0;
    /* Chech if all elements are appropriates pebblings. */
    for(resetSL(l);iscursorvalidSL(l);nextSL(l)) {
      cnt++;
      isconsistent_PebbleConfiguration(g,getSL(l));
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
#endif
  return TRUE;
}


/**
   Finalize the a black-white persistent pebbling

   In the case of persistent pebbling, we computed the pebbling
   looking for a transition from a configuration with a single while
   pebble on the sink, to a configuration containing only black
   pebbles.  It's dual configuration is indeed the persistent
   pebbling we are looking for, so we are going to reverse it.

   @param dag the graph we are pebbling

   @param final a pointer to the final configuration found by the BFS.
   
   @return Pebbling the finalized pebbling.
*/
Pebbling *finalize_persistent_pebbling(const DAG *graph,
                                       PebbleConfiguration *final) {
  fprintf(stderr,"Finalization\n");
  assert(graph);
  assert(final);
  assert(isfinal(graph,final));
#if REVERSIBLE
  assert(0);
#endif

  PebbleConfiguration *ptr=NULL;
  Pebbling *solution=NULL;
  size_t length=0;
  Vertex i=0;
  
  /* compute the length of the pebbling, without clean up */
  ptr = final;
  while(ptr!=NULL) {
    length++;
    ptr=ptr->previous_configuration;
  }
  length -= 1; /* initial conf is not a step */
  length += final->pebbles; /* clean up black pebbles */

  /* solution to be filled */
  solution = new_Pebbling(length);
  solution->cost   = final->pebble_cost;
  solution->length = length;

  /* Reversing the pebbling: final clean up corresponds to
     pebbling placements at the beginning. */
  i=0;
  for(Vertex v=0;v<graph->size;v++) {
    if (isblack(v,graph,final)) { solution->steps[i]=v; ++i; }
  }

  /* Reversing the actual pebbling */
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
   Finalize the a black-white pebbling, by removing residual black
   pebbles.

   @param dag the graph we are pebbling

   @param final a pointer to the final configuration found by the BFS.
   
   @return Pebbling the finalized pebbling.
*/
Pebbling *finalize_pebbling(const DAG *graph,
                                       PebbleConfiguration *final) {
  assert(graph);
  assert(final);
  assert(isfinal(graph,final));
#if REVERSIBLE
  assert(0);
#endif

  PebbleConfiguration *ptr=NULL;
  Pebbling *solution=NULL;
  size_t length=0;
  Vertex i=0;

  /* compute the length of the pebbling, without clean up */
  ptr = final;
  while(ptr!=NULL) {
    length++;
    ptr=ptr->previous_configuration;
  }
  length -= 1; /* initial conf is not a step */
  length += final->pebbles; /* clean up black pebbles */

  /* solution to be filled */
  solution = new_Pebbling(length);
  solution->cost   = final->pebble_cost;
  solution->length = length;

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
  assert(isfinal(graph,final));
#if !REVERSIBLE
  assert(0);
#endif

  PebbleConfiguration *ptr=NULL;
  Pebbling *solution=NULL;
  size_t length=0;
  Vertex i=0;
  Vertex j=0;

  /* compute the length of the pebbling */
  ptr = final;
  while(ptr!=NULL) {
    length++;
    ptr=ptr->previous_configuration;
  }
  length -= 1;
  length *= 2;

  /* solution to be filled */
  solution = new_Pebbling(length);
  solution->cost   = final->pebble_cost;
  solution->length = length;

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
   Explore the space of pebbling strategies.  The output is given as a
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

   Persistent pebbling is actually measured by placing a white pebble
   on the sink and by trying to complete the pebbling.

   INPUT:

   @param DAG the graph  to pebble (with few vertices  and a single
   sink).

   @param bottom the initial cap to the pebble number. It is raised
   one by one up to the value of `top'.

   @param top the maximum number of pebbles in the configuration,
   if such number is not sufficient, the the computation
   will fail gracefully without finding the pebbling.

   @param persisten_pebbling: whether we count the black white
   pebbling number for a pebbling which
   leaves a black pebble in the sink.

   @return a pebbling

 */
Pebbling *bfs_pebbling_strategy(DAG *g,
                                           unsigned int bottom,
                                           unsigned int top,
                                           Boolean persistent_pebbling) {

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

  if (bottom < 1 ) { bottom=1; } /* We start with at least space 1
                                    pebbling. */

  if (bottom > top ) {
    fprintf(stderr,
            "Pebbling number range is empty: [%u, %u]",bottom,top);
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

     N.B. As a future option: we could use a ZDD  for keeping track of
     visited configurations.
  */

  Pebbling* solution=NULL;

  /* Dictionary data structure */
  Dict *D = newDict(HASH_TABLE_SPACE_SIZE);
  DictQueryResult res;               /* Query results */
  D->key_function = hashPebbleConfiguration;
  D->eq_function  = samePebbleConfiguration;
  D->dispose_function = freePebbleConfiguration;

  /* Data structures for BFS */
  Queue               *Q=newSL(); /* Configuration to be processed immediately.   */
  Queue       *BoundaryQ=newSL(); /* Configuration to be processed the next round */

  /* Initial empty configuration */
  PebbleConfiguration *initial=new_PebbleConfiguration();
  unsigned int upper_bound=bottom;

  /* Data structure for statistics collection */
  STATS_CREATE(Stat);

#if (!WHITE_PEBBLES)
  persistent_pebbling = 0;
#endif

  /* To compute persistent pebbling put a white pebble on top and
     try to finish the pebbling. This is dual of a pebbling which
     starts with an empty configuration and finishes with a black
     pebble on the sink. We already verified that the graph has a
     single sink.
  */
  if (persistent_pebbling) {  placewhite(g->sinks[0],g,initial); }

  /* Initial configuration may contains a pebble and so be right on the boundary */
  if (initial->pebbles == upper_bound && upper_bound<top) {
    enqueue(BoundaryQ,initial);
    STATS_INC(Stat,delayed);
  }

  /* Put the initial configuration in line to be processed. */
  enqueue  (Q,initial);
  writeDict(D,&res,initial);


  /* Additional variables */
  PebbleConfiguration *ptr  =NULL;    /* Configuration to be processed */
  PebbleConfiguration *nptr =NULL;    /* Configuration to be queued for later processing (maybe) */
  PebbleConfiguration *final=NULL;    /* Final configuration */

  /* Consistency test of data structures */
  assert(isconsistentDict(D));
  assert(isconsistentSL(Q));

  STATS_SET(Stat,first_queuing,1);
  STATS_SET(Stat,queued,1);
  STATS_SET(Stat,dict_size,D->size);

#if PRINT_RUNNING_STATS==1
  Counter tmp;
  for(int p=0;p<=top;p++) {
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
    if (p!=top)
      { STATS_ADD(Stat,search_space,2*tmp);} /* Sink maybe pebbled or not */
    else
      { STATS_ADD(Stat,search_space,tmp); }
#endif
  }
#endif


  /* Pick a pebbling status from the queue, produce the followers, and
     put in the queue the ones that haven't been analized yet or the
     one with a better cost.*/
  for(resetSL(Q); !isemptySL(Q) || !isemptySL(BoundaryQ) ; pop(Q)) {

    if (isemptySL(Q)) {  /* No more element to be processed... raise the upper bound. */
      if (upper_bound<top) {

        STATS_REPORT(Stat,"\nRaising the upper bound from %u to %u / %u\n",
                     upper_bound,upper_bound+1,top);

        upper_bound++;
        disposeSL(Q);
        Q=BoundaryQ;
        BoundaryQ=newSL();
      } else {
        final = NULL;
        goto epilogue;           /* The whole search space has been explored */
      }
    }

    if (print_running_stats_flag) {
      STATS_ADD(Stat,clock,timedflags_clock_freq);
      STATS_REPORT(Stat,
                   "\nClock %llu: Report for graph on %u vertices, upper bound=%u:",
                   STATS_GET(Stat,clock),
                   g->size,
                   upper_bound);
      print_running_stats_flag=0;
    }

    /* Get an element from the queue */
    ptr=(PebbleConfiguration*)getSL(Q);
    assert(isconsistent_PebbleConfiguration(g,ptr));
    assert(ptr->pebble_cost <= upper_bound);
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
        assert(nptr->pebble_cost <= upper_bound);

        unsafe_noquery_writeDict(D,&res,nptr); /* Mark as encountered (put in the dictionary) */

        if (isfinal(g,nptr)) {               /* Is it the end of the search? */
          final=nptr;
          goto epilogue;
        }

        if (nptr->pebbles == upper_bound && upper_bound<top) {
          /* Boundary configuration will be
             reprocessed with a larger upper bound */
          enqueue(BoundaryQ,nptr);
          STATS_INC(Stat,delayed);
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

    }/* End of loop on one-step reachable configurations */

  }/* Queue of configurations empty */

 epilogue:

  /* In the case of persistent pebbling, we computed the pebbling
     looking for a transition from a configuration with a single while
     pebble on the sink, to a configuration containing only black
     pebbles.  It's dual configuration is indeed the persistent
     pebbling we are looking for, so we are going to reverse it.
  */
#if !REVERSIBLE
  if (persistent_pebbling) {

    solution = final ? finalize_persistent_pebbling(g,final) : NULL ;
    
  } else {

    solution = final ? finalize_pebbling(g,final) : NULL ;
    
  }
#else
  solution = final ? finalize_reversible_pebbling(g,final) : NULL ;
#endif

  
  STATS_REPORT(Stat,"%s","FINAL REPORT:\n\n");

#ifdef HASHTABLE_DEBUG
  assert(!CheckRuntimeConsistency(g,D));
#endif


  /* Free the memory of the data structures */
  if (Q) disposeSL(Q);
  if (BoundaryQ) disposeSL(BoundaryQ);
  disposeDict(D);

  return solution;
}

