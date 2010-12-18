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



#define HASH_TABLE_SPACE_SIZE    0x0FFFFF
#define HASH_WASTE_RANGE  0x0FFFF
#define ROTATE_RIGHT_AMOUNT (sizeof(BitTuple)*CHAR_BIT/2)


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


/* Explore the space of pebbling strategies.
   The output is given as a sequence of vertices, because
   at any point in a pebbling, there is a unique minimal move that can be
   performed on a vertex, given its status.

   If there is a pebble on the vertex, such pebble shoud be removed.
   If there is not, then either a white or a black pebble should be added.
   In case both are possible, it is always convenient to add a black one.

   Notice that any pebbling induce a dual pebbling with a reversed sequence of vertices.
   Thus the output of this function can be interpreted in both directions.

   INPUT:

         -- DAG: the graph to pebble (with few vertices and a single sink).
         -- upper_bound: the maximum number of pebbles in the configuration,
                         if such number is not sufficient, the the computation will fail gracefully
                         without finding the pebbling
*/
/* {{{ */ void pebbling_strategy(DAG *g,unsigned int upper_bound) {

  /* Dictionary data structure */

  /* Initial and Temporaty Status */

  /* PROLOGUE ----------------------------------- */
  if (g->size > MAX_VERTICES) {
    fprintf(stderr,
            "Error in search procedure: the graph is too "
            "big for the optimized data structures.");
    exit(-1);
  }

  /* SEARCH ALGORITHM */
  /* The first attempt to implement this program will use a standard
     graph reachability algorithm for directed graphs */

  /* Since graphs of configuration will be very large, we try to
     represent a configuration with the smallest memory footprint.
     Furthermore there will be a lot of useless or non valid
     configurations, thus we produce configurations on demand.
     We use an hash to access configurations.

     N.B. As a futue option: we could use a ZDD for keeping track of visited
     configurations.
  */
  //dictionary_init(&dict);
  //dictionary_update(&dict,&initial_status);
  //assert_status_sanity(g,&initial_status,&dict);

  /* Pick a pebbling status from the queue, produce the followers, and
     put in the queue the ones that haven't been analized yet */




  /* EPILOGUE --------------------------------- */
  /* free memory, free Mandela! */
}
/* }}} */



/*
 *  The example test program creates two pyramid graphs and produces
 *  the OR-product graph of them.  Then it prints the DOT
 *  representation of such graphs.
 */

int main(int argc, char *argv[])
{
  DAG *A,*B,*C;
  A=piramid(3);
  B=piramid(4);


  C=orproduct(A,B);
  print_dot_DAG(A,"A",NULL,NULL,NULL);
  print_dot_DAG(B,"B",NULL,NULL,NULL);
  print_dot_DAG(C,"C",NULL,NULL,NULL);

  dispose_DAG(A);
  dispose_DAG(B);
  dispose_DAG(C);

  exit(0);
}



