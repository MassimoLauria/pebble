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

#define REPORT_INTERVAL          5

#define USAGEMESSAGE "\n\
Usage: %s [-thZ] -b<int> [ -p<int> | -2<int> | -i <input> | -i <input1> -O <input2> ] \n\
\n\
       -h     help message;\n\
       -Z     search for a 'persistent pebbling' (optional).\n\
       -t     find shortest pebbling within space limits, instead of minimizing space (optional).\n\
       -f     output format {graphviz,text} (default: graphviz).\n\
\n\
       -b M   maximum number of pebbles (mandatory);\n\
\n\
    input graph:\n\
       -p N         pyramid graph of height N;\n\
       -2 N         binary tree height N;\n\
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
  FILE *input_file=NULL;
  FILE *input_file_aux=NULL;

  int input_directives = 0;

  int optimize_time=0;
  int persistent_pebbling=0;
  int option_code=0;

  int output_graphviz=1; /* graphviz is the default output format */

  unsigned int bottom=0;
  unsigned int top=0;


  /* PebbleConfiguration *solution=NULL; */
  Pebbling *solution=NULL;

  /* Parse option to set Pyramid height,
     pebbling upper bound. */
  while((option_code = getopt(argc,argv,"hZtb:p:2:f:i:O:"))!=-1) {
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

  /* Text output for pebbling has not being implemented yet */
    if (!output_graphviz) {
      fprintf(stderr,"Output format \'text\' not implemented.\n",pebbling_bound);
      exit(-1);
    }

  /* Search space interval*/
  if (optimize_time) {
    bottom=pebbling_bound;
    top=pebbling_bound;
  } else {
    bottom=1;
    top=pebbling_bound;
  }

  /* Timer for reporting progress */
  install_timed_flags(REPORT_INTERVAL);

  /* Produce or read input graph */
  DAG *C=NULL;
  if (pyramid_height>0) {
  
    C=pyramid(pyramid_height);
  
  } else if (tree_height>0) {
    
    C=tree(tree_height);
  
  } else {
    C=kthparser(input_file);
    /*fprintf(stderr,"Parser for KTH input not implemented",argv[0]);
    exit(-1);*/
    fclose(input_file);
  }

  /* Read second input graph if needed */
  if (input_file_aux) {
    DAG *OUTER=C;
    DAG *INNER=kthparser(input_file_aux);
    C = orproduct(OUTER,INNER);
    dispose_DAG(OUTER);
    dispose_DAG(INNER);
    fclose(input_file_aux);
  }
  
  solution=bfs_pebbling_strategy(C,bottom,top,persistent_pebbling);

  if (solution) {
    fprintf(stderr,"Graph does have a pebbling of cost %u and length %u.\n",solution->cost,solution->length);
    if (output_graphviz) print_dot_Pebbling(C,solution);
    else fprintf(stderr,"Output format \'text\' not implemented.\n",pebbling_bound);
  } else {
    fprintf(stderr,"Graph does not have a pebbling of cost %u.\n",pebbling_bound);
  }

  /* Clean up */
  dispose_DAG(C);
  if (solution) dispose_Pebbling(solution);
}
