/*
  Massimo Lauria, 2010

  This incomplete  program is supposed  to compute the  black-white pebbling
  number of a graph.

*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include "common.h"
#include "dag.h"
#include "timedflags.h"
#include "pebbling.h"
#include "bfs.h"

#define REPORT_INTERVAL          5

#define USAGEMESSAGE "\n\
Usage: %s [-hZ] -p<int> -b<int> \n\
\n\
       -h     help message;\n\
       -p N   height of the pyramid graph (mandatory);\n\
       -b M   maximum number of pebbles (mandatory);\n\
       -Z     search for a 'persistent pebbling' (optional).\n\
       -t     find shortest pebbling within space limits, instead of minimizing space (optional).\n\
"


/**
 *  The example test program creates two pyramid graphs and produces
 *  the OR-product graph of them.  Then it prints the DOT
 *  representation of such graphs.
 *
 */
int main(int argc, char *argv[])
{

  int pebbling_bound=0;
  int pyramid_height=0;
  int optimize_time=0;
  int persistent_pebbling=0;
  int option_code=0;

  unsigned int bottom=0;
  unsigned int top=0;


  /* PebbleConfiguration *solution=NULL; */
  Pebbling *solution=NULL;

  /* Parse option to set Pyramid height,
     pebbling upper bound. */
  while((option_code = getopt(argc,argv,"hZtb:p:"))!=-1) {
    switch (option_code) {
    case 'h':
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(0);
      break;
    case 'Z':
      persistent_pebbling=1;
      break;
    case 't':
      optimize_time=1;
      break;
    case 'b':
      pebbling_bound=atoi(optarg);
      if (pebbling_bound>0) break;
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(-1);
      break;
    case 'p':
      pyramid_height=atoi(optarg);
      if (pyramid_height>0) break;
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(-1);
      break;
    case '?':
    default:
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(-1);
    }
  }

  if (pebbling_bound==0 || pyramid_height==0) {
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(-1);
  }

  if (optimize_time) {
    bottom=pebbling_bound;
    top=pebbling_bound;
  } else {
    bottom=1;
    top=pebbling_bound;
  }

  install_timed_flags(REPORT_INTERVAL);

  DAG *C=pyramid(pyramid_height);

  solution=bfs_pebbling_strategy(C,bottom,top,persistent_pebbling);

  if (solution) {
    print_dot_Pebbling(C,solution);
    fprintf(stderr,"Graph does have a pebbling of cost %u and length %u.\n",solution->cost,solution->length);
  } else {
    fprintf(stderr,"Graph does not have a pebbling of cost %u.\n",pebbling_bound);
  }

  /* Clean up */
  dispose_DAG(C);
  if (solution) dispose_Pebbling(solution);
}
