/*
   Copyright (C) 2013, 2014 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2013-04-11, 21:15 (CEST) Massimo Lauria"
   Time-stamp: "2014-06-02, 11:43 (EDT) Massimo Lauria"

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

#ifndef NDEBUG
#define PARSERDEBUG(fmt,...) fprintf(stderr, "c PARSER DEBUG:" fmt "\n", __VA_ARGS__) 
#else
#define PARSERDEBUG(fmt,...) 
#endif

#define PARSERERROR(fmt,...) fprintf(stderr, "c PARSER ERROR:" fmt "\n", __VA_ARGS__) 


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
    
    temp=fscanf(input,"%100s",(char*)&buffer);
    if (temp==-1) break;
    
    if (buffer[0]=='c') { /* Comment line */
      PARSERDEBUG("%s","comment line");
      do { temp=getc(input); } while(temp!=-1 && temp!='\n');
      continue;
    }

    /* Read the vertex number */
    temp=sscanf(buffer,"%lu",&read);
    if (temp==-1 || read==0) break;

    if (vertices==0) { /* This is the vertex number */
      
      vertices = read;
      position = 0;
      if (read>1) PARSERDEBUG("there are %lu vertices",read);
      else if (read==1) PARSERDEBUG("there is %u vertex",1);

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
        PARSERERROR("Predecessors must be in ascending order. %lu predecessor of %lu",last_predecessor,read);
        exit(EXIT_FAILURE);
      }

      PARSERDEBUG("Edge %lu -> %lu",read,position);
      last_predecessor = read;
      
      indegree[position-1]++;
      outdegree[read-1]++;

      insertpoint->next=(vl*)malloc(sizeof(vl));
      insertpoint = insertpoint->next;
      assert(insertpoint);
      insertpoint->v = read-1; /* Internally vertices are counted from 0 */
      insertpoint->next = NULL;

    } else if (read == position ){
      PARSERERROR("self loop on vertex %lu.",position);
      exit(EXIT_FAILURE);
 
    } else if (read > vertices ){
      PARSERERROR("out of range vertex %lu.",read);
      exit(EXIT_FAILURE);

    } else if (read == position+1) { /* This is a vertex specification. Pass the : */
    
      PARSERDEBUG("Start reading predecessors of %lu.",read);
      
      last_predecessor = 0;
      position = position +1;
      insertpoint = predecessors[position-1];

      temp=fscanf(input,"%100s",(char*)&buffer);
      if (temp==-1 || strcmp(buffer,":")!=0) {
        PARSERERROR("expecting \":\" before predecessors for vertex %lu.",position);
        exit(EXIT_FAILURE);
      }

    } else {
      PARSERERROR("Vertex %lu missing.",position+1);
      exit(EXIT_FAILURE);
    }
  }

  if (vertices==0)
    PARSERERROR("%s","positive number of vertices expected.");
 
  if (position != vertices) {
    PARSERERROR("unexpected end of file at vertex %lu.",position); 
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
  assert(isconsistent_DAG(dag));
  return dag;
}

