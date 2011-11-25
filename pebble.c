/*
  Massimo Lauria, 2010

  This incomplete  program is supposed  to compute the  black-white pebbling
  number of a graph.

*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "dag.h"
#include "timedflags.h"
#include "pebbling.h"
#include "bfs.h"

#define REPORT_INTERVAL          5

#define USAGEMESSAGE "\n\
Usage: %s [-h] [-p N] [-b M] \n\
\n\
       -h     help message;\n\
       -p N   N>0 height of the pyramid graph;\n\
       -b M   M>0 maximum number of pebbles.\n\
"


/*
 *  The example test program creates two pyramid graphs and produces
 *  the OR-product graph of them.  Then it prints the DOT
 *  representation of such graphs.
 */
int main(int argc, char *argv[])
{

  int pebbling_bound=3;
  int pyramid_height=1;
  int option_code=0;

  PebbleConfiguration *solution=NULL;


  /* Parse option to set Pyramid height,
     pebbling upper bound. */
  while((option_code=getopt(argc,argv,"b:p:h"))!=-1) {
    switch (option_code) {
    case 'h':
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(0);
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

  install_timed_flags(REPORT_INTERVAL);

  DAG *C=pyramid(pyramid_height);

  solution=bfs_pebbling_strategy(C,pebbling_bound);

  if (solution) {
    fprintf(stderr,"Graph does have a pebbling of cost %u",solution->pebble_cost);
    print_dot_Pebbling_Path(C,solution);
    exit(0);
  } else {
    fprintf(stderr,"Graph does not have a pebbling of cost %u",pebbling_bound);
    exit(1);
  }
}

