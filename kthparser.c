/*
   Copyright (C) 2013 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2013-04-11, 21:15 (CEST) Massimo Lauria"
   Time-stamp: "2013-04-11, 23:02 (CEST) Massimo Lauria"

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

extern void dag_precompute_data(DAG *digraph);


/* Parse a graph in kth format
   
   input: file stream
   
   output: a DAG object
 */
DAG *kthparser(FILE *input) {

  DAG *dag=NULL;
  
  char buffer[101];
  char *residual_buffer=NULL;
  int  temp=0;

  size_t vertices=0;
  Vertex read = 0;
  Vertex position = 0;
  Vertex last_predecessor = 0;

  while (feof(input)==0) {
    
    temp=fscanf(input,"%100s",&buffer);
    if (temp==-1) break;
    
    if (buffer[0]=='c') { /* Comment line */
      fprintf(stderr,"comment line\n");
      do { temp=getc(input); } while(temp!=-1 && temp!='\n');
      continue;
    }

    /* Read a vertex number */
    temp=sscanf(buffer,"%u",&read);
    if (temp==-1 || read==0) break;

    residual_buffer=buffer+temp;
    if (strlen(residual_buffer)==0) { /* string have been completely consumed */
      residual_buffer=NULL;
    } 

    if (vertices==0) { /* This is the vertex number */
      
      vertices = read;
      position = 0;
      fprintf(stderr,"GOOD: vertex number %u\n",read);
     
    } else if ( read < position ) { /* Still reading vertex specification */
      
      if (last_predecessor >= read) {
        fprintf(stderr,"ERROR: predecessors must be in ascending order (%u >= %u)\n",
                last_predecessor,read);
        exit(-1);
      }

      fprintf(stderr,"GOOD: %u preceeds %u\n",read,position);
      last_predecessor = read;

    } else if (read == position ){
      fprintf(stderr,"ERROR: self loop on vertex %u (b:%s)\n",position,buffer);
      exit(-1);
 
    } else if (read > vertices ){
      fprintf(stderr,"ERROR: too large vertex %u\n",read);
      exit(-1);

    } else if (read == position+1) { /* This is a vertex specification. Pass the : */
    
      fprintf(stderr,"GOOD: Read specification for %u\n",read);
      
      last_predecessor = 0;
      position = position +1;

      temp=fscanf(input,"%100s",&buffer);
      if (temp==-1 || strcmp(buffer,":")!=0) {
        fprintf(stderr,"ERROR: expecting : specification for vertex %u (b:%s)\n",position,buffer);
        exit(-1);
      }
      fprintf(stderr,"GOOD: Read : from %s\n",buffer);

    } else {
      fprintf(stderr,"ERROR: missing vertex %u (b:%s)\n",position+1,buffer);
      exit(-1);
    }
  }

  if (vertices==0)
    fprintf(stderr,"ERROR: positive number of vertices expected (b:%s)\n",buffer);
 
  if (position != vertices) {
    fprintf(stderr,"ERROR: unexpected end of file.",vertices); 
  }

  /* Finalize the object */
  exit(0);
  dag_precompute_data(dag);
  assert(isconsistent_DAG(dag));
  return dag;
}

