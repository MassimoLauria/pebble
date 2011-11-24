/*
  Massimo Lauria, 2010

  This incomplete  program is supposed  to compute the  black-white pebbling
  number of a graph.

*/

#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "dag.h"
#include "timedflags.h"
#include "pebbling.h"
#include "bfs.h"

#define REPORT_INTERVAL          5

/*
 *  The example test program creates two pyramid graphs and produces
 *  the OR-product graph of them.  Then it prints the DOT
 *  representation of such graphs.
 */
int main(int argc, char *argv[])
{

  install_timed_flags(REPORT_INTERVAL);

  unsigned int UB=5;
  PebbleConfiguration *solution=NULL;

  /* DAG *A=leader(3); */
  /* DAG *B=piramid(2); */
  /* DAG *C=orproduct(A,B); */

  /* DAG *D=tree(3); */

  DAG *C=pyramid(5);

  solution=bfs_pebbling_strategy(C,UB);

  if (solution) {
    fprintf(stderr,"Graph does have a pebbling of cost %u",solution->pebble_cost);
    print_dot_Pebbling_Path(C,solution);
    exit(0);
  } else {
    fprintf(stderr,"Graph does not have a pebbling of cost %u",UB);
    exit(-1);
  }
}

