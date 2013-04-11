/*
   Copyright (C) 2013 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2013-04-11, 21:15 (CEST) Massimo Lauria"
   Time-stamp: "2013-04-12, 01:33 (CEST) Massimo Lauria"

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

typedef struct _vl { struct _vl* next; Vertex v; } vl;

/* Parse a graph in kth format
   
   input: file stream
   
   output: a DAG object
 */
DAG *kthparser(FILE *input) {

  DAG *dag=NULL;
  
  char buffer[101];
  int  temp=0;

  size_t vertices=0;
  Vertex read = 0;
  Vertex position = 0;
  Vertex last_predecessor = 0;

  size_t *indegree=NULL;
  size_t *outdegree=NULL;
  
  vl **predecessors=NULL;
  vl *insertpoint=NULL;
  
  while (feof(input)==0) {
    
    temp=fscanf(input,"%100s",&buffer);
    if (temp==-1) break;
    
    if (buffer[0]=='c') { /* Comment line */
      fprintf(stderr,"PARSER: comment line\n");
      do { temp=getc(input); } while(temp!=-1 && temp!='\n');
      continue;
    }

    /* Read the vertex number */
    temp=sscanf(buffer,"%u",&read);
    if (temp==-1 || read==0) break;

    if (vertices==0) { /* This is the vertex number */
      
      vertices = read;
      position = 0;
      fprintf(stderr,"PARSER: there are %u vertices.\n",read);

      indegree = (size_t*)calloc(vertices,sizeof(size_t));
      outdegree = (size_t*)calloc(vertices,sizeof(size_t));
      predecessors = (vl**)malloc(vertices*sizeof(vl*));
      for (Vertex v=0; v < vertices; v++) {
        predecessors[v] = (vl*)malloc(sizeof(vl));
        predecessors[v]->v = 0;
        predecessors[v]->next = NULL;
      }
    } else if ( read < position ) { /* Still reading vertex specification */
      
      if (last_predecessor >= read) {
        fprintf(stderr,"PARSER ERROR: predecessors must be in ascending order.\n",
                last_predecessor,read);
        exit(-1);
      }

      fprintf(stderr,"PARSER: Edge %u -> %u\n",read,position);
      last_predecessor = read;
      
      indegree[position-1]++;
      outdegree[read-1]++;

      insertpoint->next=(vl*)malloc(sizeof(vl));
      insertpoint = insertpoint->next;
      assert(insertpoint);
      insertpoint->v = read-1; /* Internally vertices are counted from 0 */
      insertpoint->next = NULL;

    } else if (read == position ){
      fprintf(stderr,"PARSER ERROR: self loop on vertex %u.\n",position);
      exit(-1);
 
    } else if (read > vertices ){
      fprintf(stderr,"PARSER ERROR: out of range vertex %u.\n",read);
      exit(-1);

    } else if (read == position+1) { /* This is a vertex specification. Pass the : */
    
      fprintf(stderr,"PARSER: start reading predecessors of %u.\n",read);
      
      last_predecessor = 0;
      position = position +1;
      insertpoint = predecessors[position-1];

      temp=fscanf(input,"%100s",&buffer);
      if (temp==-1 || strcmp(buffer,":")!=0) {
        fprintf(stderr,"PARSER ERROR: expecting : before predecessors for vertex %u.\n",position,buffer);
        exit(-1);
      }

    } else {
      fprintf(stderr,"PARSER ERROR: Vertex %u missing.\n",position+1);
      exit(-1);
    }
  }

  if (vertices==0)
    fprintf(stderr,"PARSER ERROR: positive number of vertices expected.\n");
 
  if (position != vertices) {
    fprintf(stderr,"PARSER ERROR: unexpected end of file at vertex %u.\n",position); 
  }


  /* Build dag data structure */
  dag=(DAG*)malloc(sizeof(DAG));
  assert(dag);
  assert(vertices>0);

  /* Set to null sinks and source vector */
  dag->pred_bitmasks=NULL;
  dag->succ_bitmasks=NULL;
  dag->sources=NULL;
  dag->sinks=NULL;

  /* Size of the vertex set */
  dag->size=vertices;

  /* Allocation of degree information */
  dag->in  = (Vertex**)malloc( dag->size*sizeof(Vertex*) );
  dag->out = (Vertex**)malloc( dag->size*sizeof(Vertex*) );

  assert(dag->in );
  assert(dag->out);

  dag->indegree  = (size_t*)calloc( dag->size,sizeof(size_t) );
  dag->outdegree = (size_t*)calloc( dag->size,sizeof(size_t) );

  assert(dag->indegree );
  assert(dag->outdegree);

  /* Allocation of neighbour informations */
  for(Vertex v=0;v<dag->size;v++) {
    dag->in[v]  = (Vertex*)malloc( (indegree[v]) *sizeof(Vertex) );
    dag->out[v] = (Vertex*)malloc( (outdegree[v])*sizeof(Vertex) );
    assert(dag->in [v]);
    assert(dag->out[v]);
  }

  /* Load vertices */
  Vertex u=0;
  for (Vertex v=0;v<vertices;v++) {
    insertpoint = predecessors[v];
    while(insertpoint->next){
      u=insertpoint->next->v;
      insertpoint = insertpoint->next;
      dag->in[v][ dag->indegree[v]++ ] = u;
      dag->out[u][ dag->outdegree[u]++ ] = v;
    }
  }

  /* Print the object */
  /* for (Vertex v=0;v<vertices;v++) { */
  /*   fprintf(stderr,"Vertex %u: in=%u, out=%u\n",v+1,indegree[v],outdegree[v]); */
  /*   insertpoint = predecessors[v]; */
  /*   while(insertpoint->next){ */
  /*     fprintf(stderr,"%u ",insertpoint->next->v+1); */
  /*     insertpoint = insertpoint->next; */
  /*   } */
  /*   fprintf(stderr," -> %u\n",v+1); */
  /* } */

  /* Free temporary structures */
  if (indegree)  free(indegree);
  if (outdegree) free(outdegree);
  for (Vertex v=0;v<vertices;v++) {
    while(predecessors[v]){
      insertpoint = predecessors[v];
      predecessors[v] = predecessors[v]->next;
      free(insertpoint);
    }
  }
  free(predecessors);

  dag_precompute_data(dag);
  /* print_DAG(dag,NULL); */
  assert(isconsistent_DAG(dag));
  return dag;
}

