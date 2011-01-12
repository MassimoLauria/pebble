/*
  Massimo Lauria, 2010

  This incomplete  program is supposed  to compute the  black pebbling
  number of a graph.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <signal.h>
#include <unistd.h>

#include "common.h"
#include "dag.h"
#include "pebbling.h"
#include "hashtable.h"
#include "timedflags.h"


#define HASH_TABLE_SPACE_SIZE    0x0FFFFF
#define REPORT_INTERVAL          2


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
Boolean CheckDictConsistency(DAG *g,PebbleConfiguration *s,Dict *dict) {

  ASSERT_NOTNULL(g);
  ASSERT_NOTNULL(s);
  ASSERT_NOTNULL(dict);

  ASSERT_TRUE(isconsistent_DAG(g));
  ASSERT_TRUE(isconsistent_PebbleConfiguration(g,s));
  ASSERT_TRUE(isconsistentDict(dict));

  ASSERT_TRUE(g->size < BITTUPLE_SIZE);

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
void pebbling_strategy(DAG *g,unsigned int upper_bound) {

  /* Dictionary data structure */
  Dict *D = newDict(HASH_TABLE_SPACE_SIZE);
  D->key_function = hashPebbleConfiguration;
  D->eq_function  = samePebbleConfiguration;
  ASSERT_TRUE(isconsistentDict(D));

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
  int i=1; char buffer[20];   /* Buffer for printing debug
                                 informations. */
#endif

#ifdef PRINT_RUNNING_STATS
  unsigned long long int elapsed_seconds=0;
  unsigned long long int configurations_explored=0;
  unsigned long long int configurations_unique=0;
  unsigned long long int configurations_queued=0;
  unsigned long long int configurations_requeued=0;
  unsigned long long int configurations_boundary=0;

  unsigned long long int configurations_explored_lsec=1;
  unsigned long long int configurations_unique_lsec=1;
  unsigned long long int configurations_queued_lsec=1;
  unsigned long long int configurations_requeued_lsec=0;
  unsigned long long int configurations_boundary_lsec=0;
  unsigned long long int total_neighbours_lsec=0;
  unsigned long long int total_neighbours=0;

  unsigned long long int total_potential_configurations=0;

  int p,k;
  unsigned long long int fixed_size_confs;
  for(p=0;p<=upper_bound;p++) {
    fixed_size_confs=1; /* How many configurations with p pebbles? */
    /* We compute binom(n,p)*2^p, with n=the size of the graph and p
       the number of pebbles.  We first computing 2^p*n!/(n-p)! which
       is integer, then we divide for p!. In this way all intermediate
       values are integers. Since p << n there is no much danger of
       overflow if the total count does not overflow. */

    for(k=1;k<=p;k++) { fixed_size_confs *= 2*( g->size - k + 1); }
    for(k=2;k<=p;k++) { fixed_size_confs /= k; }
    total_potential_configurations += 2*fixed_size_confs; /* Sink maybe touched or not */
  }
#endif

  /* Pick a pebbling status from the queue, produce the followers, and
     put in the queue the ones that haven't been analized yet or the
     one with a better cost.*/
  for(resetSL(Q);!isemptySL(Q);delete_and_nextSL(Q)) {
    ASSERT_TRUE(isconsistentDict(D));
    ptr=(PebbleConfiguration*)getSL(Q);

#ifdef DEBUG
    /* Print pebbling configuration */
    sprintf(buffer,"A%d",i);
    print_dot_Pebbling(g, ptr,buffer,NULL);
    i++;
#endif

#ifdef PRINT_RUNNING_STATS
    if (print_running_stats_flag) {
      /* Compute statistics from the heuristic */
      configurations_unique   += configurations_unique_lsec;
      configurations_explored += configurations_explored_lsec;
      configurations_queued   += configurations_queued_lsec;
      configurations_requeued   += configurations_requeued_lsec;
      configurations_boundary += configurations_boundary_lsec;
      total_neighbours        += total_neighbours_lsec;
      elapsed_seconds += REPORT_INTERVAL;

      fprintf(stderr,"\nGraph on %u vertices, upper bound=%u, clock %llu:\n\n",
              g->size,
              upper_bound,
              elapsed_seconds);

      fprintf(stderr,"TOTAL\n");
      fprintf(stderr,"[Explored] = %llu\n",configurations_explored);
      fprintf(stderr,"[  Unique] = %llu of %llu\n",configurations_unique,
              total_potential_configurations);
      fprintf(stderr,"[  Queued] = %llu\n",configurations_queued);
      fprintf(stderr,"[Requeued] = %llu\n",configurations_requeued);
      fprintf(stderr,"[Boundary] = %llu\n",configurations_boundary);
      fprintf(stderr,"[A.Neighb] = %llu\n",total_neighbours/configurations_explored);

      fprintf(stderr,"\nLAST INTERVAL\n");
      fprintf(stderr,"[Explored] = %llu\n",configurations_explored_lsec);
      fprintf(stderr,"[  Unique] = %llu\n",configurations_unique_lsec);
      fprintf(stderr,"[  Queued] = %llu\n",configurations_queued_lsec);
      fprintf(stderr,"[Requeued] = %llu\n",configurations_requeued_lsec);
      fprintf(stderr,"[Boundary] = %llu\n",configurations_boundary_lsec);
      fprintf(stderr,"[A.Neighb] = %llu\n",total_neighbours_lsec/configurations_explored_lsec);

      fprintf(stderr,"\nPER SECOND\n");
      fprintf(stderr,"[Explored] = %llu\n",configurations_explored/elapsed_seconds);
      fprintf(stderr,"[  Unique] = %llu\n",configurations_unique/elapsed_seconds);
      fprintf(stderr,"[  Queued] = %llu\n",configurations_queued/elapsed_seconds);
      fprintf(stderr,"[Requeued] = %llu\n",configurations_requeued/elapsed_seconds);
      fprintf(stderr,"[Boundary] = %llu\n",configurations_boundary/elapsed_seconds);

      configurations_explored_lsec=0;
      configurations_queued_lsec=0;
      configurations_unique_lsec=0;
      configurations_requeued_lsec=0;
      configurations_boundary_lsec=0;
      total_neighbours_lsec=0;

      print_running_stats_flag=0;
    }
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
    if (isfinal(g,ptr)
        &&
        (final==NULL || final->pebble_cost > ptr->pebble_cost)) {
      final=ptr;
      upper_bound = ptr->pebble_cost;
      continue; /* Correct only for black pebbles */
    }

    /* Otherwise compute the next configurations */
#ifdef PRINT_RUNNING_STATS
    configurations_explored_lsec += 1;
#endif

    for(Vertex v=0;v<g->size;v++) {


      /* New configurations */
      nptr=next_PebbleConfiguration(v,g,ptr);

      if (nptr==NULL) continue;

#ifdef PRINT_RUNNING_STATS
      total_neighbours_lsec +=1 ;
#endif


      if (nptr->pebble_cost > upper_bound) { /* Is it above the upper bound? */
#ifdef PRINT_RUNNING_STATS
        configurations_boundary_lsec += 1;
#endif
        dispose_PebbleConfiguration(nptr);
        continue;
      }

      /* Valid new configurations */
      res=queryDict(D,nptr);

      if (nptr->pebble_cost > upper_bound) {
#ifdef PRINT_RUNNING_STATS
        configurations_boundary_lsec +=1 ;
#endif
        dispose_PebbleConfiguration(nptr);
      } else if (res.value==NULL ||
                 ((PebbleConfiguration*)res.value)->pebble_cost > nptr->pebble_cost ) {
        /* New or improved configuration */
        writeDict(D,nptr);
        enqueue(Q,nptr);
#ifdef PRINT_RUNNING_STATS
        configurations_queued_lsec +=1 ;
        if (res.value!=NULL) configurations_requeued_lsec +=1;
        else configurations_unique_lsec +=1;
#endif
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
  print_dot_Pebbling_Path(g,final);
}


/*
 *  The example test program creates two pyramid graphs and produces
 *  the OR-product graph of them.  Then it prints the DOT
 *  representation of such graphs.
 */



int main(int argc, char *argv[])
{

  install_timed_flags(2,3);

  DAG *A=piramid(2);
  DAG *B=piramid(2);
  DAG *C=orproduct(A,B);
  pebbling_strategy(C,6);
  exit(0);
}

