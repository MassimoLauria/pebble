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
"


/*
 *  The example test program creates two pyramid graphs and produces
 *  the OR-product graph of them.  Then it prints the DOT
 *  representation of such graphs.
 */
int main(int argc, char *argv[])
{

  int pebbling_bound=0;
  int pyramid_height=0;
  int persistent_pebbling=0;
  int option_code=0;

  PebbleConfiguration *solution=NULL;


  /* Parse option to set Pyramid height,
     pebbling upper bound. */
  while((option_code = getopt(argc,argv,"hZb:p:"))!=-1) {
    switch (option_code) {
    case 'h':
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(0);
      break;
    case 'Z':
      persistent_pebbling=1;
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

  install_timed_flags(REPORT_INTERVAL);

  DAG *C=pyramid(pyramid_height);

  solution=bfs_pebbling_strategy(C,pebbling_bound,persistent_pebbling);

  if (solution) {
    fprintf(stderr,"Graph does have a pebbling of cost %u.\n",solution->pebble_cost);
    print_dot_Pebbling_Path(C,solution);
    exit(0);
  } else {
    fprintf(stderr,"Graph does not have a pebbling of cost %u.\n",pebbling_bound);
    exit(1);
  }
}

