/*
  Massimo Lauria, 2010, 2001, 2012, 2013

  Given a graph and a pebble bound, produces a QBF that claims that
  the graph has a reversilble pebbling which cost is withing
  that bound.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "common.h"
#include "dag.h"
#include "config.h"


#define USAGEMESSAGE "\n\
Usage: %s [-h] -b<int> [ -p<int> | -2<int> | -c<int> | -i <input> ]\n\
\n\
       -h     help message;\n\
\n\
       -b M   maximum number of pebbles (mandatory);\n\
\n\
    input graph:\n\
       -p N         pyramid graph of height N;\n\
       -2 N         binary tree height N;\n\
       -c N         chain of length N;\n\
       -i <input>   load input file in KTH format.\n\
\n\
KTH input format is a source to sync topologically sorted\n\
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
FILE *openinputfile(const char* filename) {

  FILE *f = NULL;
  
  if (strlen(filename)==1 && filename[0]=='-') return stdin;
  
  f = fopen(filename,"r");
  if (f==NULL) {
    fprintf(stderr, "c ERROR: unable to open input file \"%s\"",filename);
    exit(EXIT_FAILURE);
    }
  return f;
}

/* 
 *  Open the output file.
 */
FILE *openoutputfile(const char* filename) {

  FILE *f = NULL;
  
  f = fopen(filename,"w");
  if (f==NULL) {
    fprintf(stderr, "c ERROR: unable to open output file \"%s\"",filename);
    exit(EXIT_FAILURE);
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

  int input_directives = 0;
  char graph_name[100];
  
  int option_code=0;

  /* Parse option to set Pyramid height,
     pebbling upper bound. */
  while((option_code = getopt(argc,argv,"hb:p:2:c:f:i:g:"))!=-1) {
    switch (option_code) {
    case 'h':
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(EXIT_SUCCESS);
      break;
    case 'b':
      pebbling_bound=atoi(optarg);
      if (pebbling_bound>0) break;
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(EXIT_FAILURE);
      break;
    /* Input */
    case 'p':
      pyramid_height=atoi(optarg);
      if (pyramid_height>0) {input_directives++; break;}
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(EXIT_FAILURE);
      break;
    case '2':
      tree_height=atoi(optarg);
      if (tree_height>0) {input_directives++; break;}
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(EXIT_FAILURE);
      break;
    case 'c':
      chain_length=atoi(optarg);
      if (chain_length>0) {input_directives++; break;}
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(EXIT_FAILURE);
      break;
    case 'i':
      input_file=openinputfile(optarg);
      input_directives++;
      break;
    case '?':
    default:
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  /* Test for valid command line */
  if (pebbling_bound==0) {
      fprintf(stderr,USAGEMESSAGE,argv[0]);
      exit(EXIT_FAILURE);
  }

  /* Only one input */
  if (input_directives > 1) {
    fprintf(stderr,USAGEMESSAGE,argv[0]);
    exit(EXIT_FAILURE);
  } 
  if (input_directives == 0) input_file = stdin;

  
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
    snprintf(graph_name, 100, "Input graph");
  }

  /* Re-output the input graph */
  printf("c ===== input DAG ======\n");
  printf("c c %s\n", graph_name);
  fprint_DAG(stdout,C,"c ");
  printf("c ======================\n");
  printf("c Dymon-Tompa rounds: %d\n", pebbling_bound);
  printf("c ======================\n");
  
  /* Output the QBF */

  /* Clean up and exit */
  dispose_DAG(C);
  exit(EXIT_SUCCESS);
}
