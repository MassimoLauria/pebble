/*
  Massimo Lauria, 2010, 2001, 2012, 2013

  This incomplete  program is supposed  to compute the  black-white pebbling
  number of a graph.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "common.h"
#include "dag.h"
#include "timedflags.h"
#include "pebbling.h"
#include "bfs.h"


#define USAGEMESSAGE "\n\
Usage: %s [-thZ] -b<int> [ -p<int> | -2<int> | -i <input> ] [-O <input2> ] \n\
\n\
       -h     help message;\n\
       -Z     search for a 'persistent pebbling' (optional, only useful for black/white pebbling).\n\
       -t     find shortest pebbling within space limits, instead of minimizing space (optional).\n\
       -f     output format {graphviz,text} (default: graphviz).\n\
\n\
       -b M   maximum number of pebbles (mandatory);\n\
\n\
    input graph:\n\
       -p N         pyramid graph of height N;\n\
       -2 N         binary tree height N;\n\
       -c N         chain of length N;\n\
       -i <input>   load input file in KTH format.\n\
       -O <input2>  OR product between input graph and <input2>.\n\
\n\
KTH input format is a top to bottom topologically sorted\n\
representation of a DAG. N is a *positive* integer, and N indexed\n\
lines follows, each containing the index of the vertex, and a space\n\
separated list of predecessors for that vertex. The predecessors must\n\
have smaller indices.\n\
The file may start with comment lines prefixed by charachter \'c\'.\n\
\n\
Example:\n\
\n\
c\n\
c This is a DAG of 5 vertices\n\
c\n\
5\n\
1  :\n\
2  : \n\
3  : 1  \n\
4  : 3  \n\
5  : 2  4\n"


/* 
 *  Open the input file. '-' represent standard input.
 */
FILE *openfile(const char* filename) {

  FILE *f = NULL;
  
  if (strlen(filename)==1 && filename[0]=='-') return stdin;
  
  f = fopen(filename,"r");
  if (f==NULL) {
    fprintf(stderr, "ERROR: unable to open file \"%s\"",filename);
    exit(-1);
    }
  return f;
}

extern DAG *kthparser(FILE *stream);

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
  int tree_height=0;
  int chain_length=0;
  
  FILE *input_file=NULL;
  FILE *input_file_aux=NULL;

  int input_directives = 0;
  char graph_name[100];
  
  int optimize_time=0;
  int persistent_pebbling=0;
  int option_code=0;

  int output_graphviz=1; /* graphviz is the default output format */

  unsigned int cost=0;


  /* PebbleConfiguration *solution=NULL; */
  Pebbling *solution=NULL;

  /* Parse option to set Pyramid height,
     pebbling upper bound. */
  while((option_code = getopt(argc,argv,"hZtb:p:2:c:f:i:O:"))!=-1) {
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
    /* Input */
    case 'p':
      pyramid_height=atoi(optarg);
      if (pyramid_height>0) {input_directives++; break;}
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(-1);
      break;
    case '2':
      tree_height=atoi(optarg);
      if (tree_height>0) {input_directives++; break;}
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(-1);
      break;
    case 'c':
      chain_length=atoi(optarg);
      if (chain_length>0) {input_directives++; break;}
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(-1);
      break;
    case 'i':
      input_file=openfile(optarg);
      input_directives++;
      break;
    case 'O':
      input_file_aux=openfile(optarg);
      if (input_file!=stdin || input_file_aux!=stdin) break;
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(-1);
      break;
      /* Output format */
    case 'f':
      if (strcmp(optarg,"text")==0) output_graphviz = 0;
      else if (strcmp(optarg,"graphviz")==0) output_graphviz = 1;
      else {
        fprintf(stderr,USAGEMESSAGE,argv[0]);
        exit(-1);
      }
      break;
    case '?':
    default:
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(-1);
    }
  }

  /* Test for valid command line */
  if (pebbling_bound==0) {
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(-1);
  }

  /* Only one input */
  if (input_directives > 1) {
    fprintf(stderr,USAGEMESSAGE,argv[0]);
    exit(-1);
  } 
  if (input_directives == 0) input_file = stdin;


  /* Timer for reporting progress */
#if PRINT_STATS_INTERVAL > 0
  install_timed_flags(PRINT_STATS_INTERVAL);
#endif
  
  /* Produce or read input graph */
  DAG *C=NULL;
  if (pyramid_height>0) {
  
    C=pyramid(pyramid_height);
    snprintf(graph_name, 100, "Pyramid of height %d",pyramid_height);
  
  } else if (tree_height>0) {
    
    C=tree(tree_height);
    snprintf(graph_name, 100, "Tree of height %d",tree_height);
  
  } else if (chain_length>0) {
    
    C=path(chain_length);
    snprintf(graph_name, 100, "Chain of height %d",chain_length);

  } else {
    C=kthparser(input_file);
    fclose(input_file);
    snprintf(graph_name, 100, "The graph in input");
  }

  /* Read second input graph if needed */
  if (input_file_aux) {
    DAG *OUTER=C;
    DAG *INNER=kthparser(input_file_aux);
    C = orproduct(OUTER,INNER);
    dispose_DAG(OUTER);
    dispose_DAG(INNER);
    fclose(input_file_aux);
    snprintf(graph_name, 100, "This OR product");
  }

  /* Output on stderr if graphviz output*/
  FILE *out_file=NULL;
  if (output_graphviz) out_file=stderr;
  else out_file=stdout;
    
  /* Search space interval*/
  cost= optimize_time ? pebbling_bound : 1;

  while ( (cost <= pebbling_bound) && !solution ) {
    fprintf(out_file, "c Search for pebbling of cost %d\n",cost);
    solution=bfs_pebbling_strategy(C,cost,persistent_pebbling);
    cost++;  
  }

  /* Print solution */
  if (solution) {

    fprintf(out_file,"c %s has a pebbling of cost %u and length %u.\n",
            graph_name,solution->cost,solution->length);

    fprintf(out_file,"s SATISFIABLE\n");

    if (output_graphviz) print_dot_Pebbling(C,solution);
    else print_text_Pebbling(C,solution);

  } else {

    fprintf(stderr,"c %s does not have a pebbling of cost %u.\n",
            graph_name,pebbling_bound);

    fprintf(out_file,"s UNSATISFIABLE\n");
  }

  /* Clean up */
  dispose_DAG(C);
  if (solution) dispose_Pebbling(solution);

  /* Exit codes as in SAT competitions */
  if (solution) exit(20);
  else exit(10);
}
