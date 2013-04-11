/*
   Copyright (C) 2013 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2013-04-11, 21:15 (CEST) Massimo Lauria"
   Time-stamp: "2013-04-11, 21:45 (CEST) Massimo Lauria"

   Description::

   Implementation of Direct acyclic graph functionality.


*/

/* Preamble */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "dsbasic.h"
#include "dag.h"

extern  dag_precompute_data(DAG *digraph);


/* Parse a graph in kth format
   
   input: file stream
   
   output: a DAG object
 */
void kthparser(FILE *input) {

  DAG *dag=NULL;
  
  char buffer[101];
  int  test=0;
  int  temp=0;

  size_t vertices=0;
  Vertex next = 0;

  while (feof(input)==0) {
    
    test=fscanf(input,"%100s",&buffer);
    
    if (strcmp(buffer,"c")==0) { /* Comment line */
      fprint(stderr,"comment line\n");
      do { temp=getc(input) } while(temp!=-1 && temp!='\n');
      continue;
    }

    if (next==0) { /* Waiting for the vertex number */
      temp=snscanf(buffer,100,"%u",&vertices);
      if (temp==-1) {
        fprint(stderr,"ERROR: number of vertices expected");
      } 
      fprint(stderr,"Number of vertices %u.",vertices);
      next = 1;
    }
  }
  
  /* Finalize the object */
  exit(0);
  dag_precompute_data(dag);
  assert(isconsistent_DAG(dag));
  return dag;
}

int main() {
  VVI g,rg;
  parsekth(cin,g,rg);
  int n=g.size();
  int m=0;
  FOR (i,0,n) m+= g[i].size();
  printf("Parsed a graph with %d vertexes and %d edges\n", n, m);
}
