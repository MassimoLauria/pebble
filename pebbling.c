/*
   Copyright (C) 2010 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-17, venerdì 12:01 (CET) Massimo Lauria"
   Time-stamp: "2010-12-18, sabato 18:47 (CET) Massimo Lauria"

   Description::

   Pebbling of a graphs: we implement the main data structure for a
   pebbling, and basic management functions, to recognise which
   vertices in the graph can be pebbled and which white pebble can be
   removed.

*/

/* Preamble */
#include <stdlib.h>
#include "common.h"
#include "dag.h"
#include "pebbling.h"

PebbleConfiguration *copy_PebbleConfiguration(const PebbleConfiguration *src) {

  PebbleConfiguration *dst=(PebbleConfiguration*)malloc(sizeof(PebbleConfiguration));

  ASSERT_TRUE(src);
  ASSERT_NOTNULL(dst);

  dst->white_pebbled=src->white_pebbled;
  dst->black_pebbled=src->black_pebbled;

  dst->pebbles=src->pebbles;

  dst->pebble_cost=src->pebble_cost;
  dst->previous_configuration=src->previous_configuration;
  dst->last_changed_vertex=src->last_changed_vertex;

  return dst;
}

void dispose_PebbleConfiguration(PebbleConfiguration *ptr) {
  ASSERT_NOTNULL(ptr);
  free(ptr);
}

/* This function checks if the configuration of pebbles referenced by
   ptr is a consistent configuration for the graph referenced by
   graph, which in turns must be consistent. */
Boolean isconsistent_PebbleConfiguration(const DAG *graph,const PebbleConfiguration *ptr) {

  ASSERT_NOTNULL(ptr);
  ASSERT_NOTNULL(graph);

  /* The configuration must support the size of the graph */
  if (graph->size > MAX_VERTICES) return FALSE;

  /* The configuration must not have both a white and a black pebble
     on the same vertex */
  if (ptr->white_pebbled | ptr->black_pebbled) return FALSE;


  /* Count the number of pebbles in the bitvector and check
     consistency with the pebble counter */
  unsigned int counter=0;

  for(size_t i=0; i<graph->size; i++) {
    if (ptr->white_pebbled & (0x1 << i)) counter++;
    if (ptr->black_pebbled & (0x1 << i)) counter++;
  }

  if (counter!=ptr->pebbles) return FALSE;

  /* Check that the mask is clean in the residual bits */
  if (MAX_VERTICES != graph->size) {
    if ( (ptr->white_pebbled | ptr->black_pebbled) &  /* The OR of the bitsequences */
         ~((1 << graph->size) -1) )                   /* A bitmask which is set only on the high bits */
      return FALSE;
  }
  /* Everything's fine */
  return TRUE;
}

/* Determines if a vertex is black pebbled according to a specific
   configuration */
inline Boolean isblack(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  ASSERT_TRUE(isconsistent_DAG(g));
  ASSERT_TRUE(isconsistent_PebbleConfiguration(g,c));
  ASSERT_TRUE(v<g->size);

  return (c->black_pebbled & (1<<v))?TRUE:FALSE;
}

/* Determines if a vertex is white pebbled according to a specific
   configuration */
inline Boolean iswhite(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  ASSERT_TRUE(isconsistent_DAG(g));
  ASSERT_TRUE(isconsistent_PebbleConfiguration(g,c));
  ASSERT_TRUE(v<g->size);

  return (c->white_pebbled & (1<<v))?TRUE:FALSE;
}


/* Determines if a vertex is pebbled according to a specific
   configuration */
inline Boolean ispebbled(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  ASSERT_TRUE(isconsistent_DAG(g));
  ASSERT_TRUE(isconsistent_PebbleConfiguration(g,c));
  ASSERT_TRUE(v<g->size);

  return ((c->white_pebbled | c->black_pebbled) & (1<<v))?TRUE:FALSE;
}

/* Determines if there is a pebble on all predecessors of agiven
   vertex */
Boolean isactive(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  ASSERT_TRUE(isconsistent_DAG(g));
  ASSERT_TRUE(isconsistent_PebbleConfiguration(g,c));
  ASSERT_TRUE(v<g->size);

  for(size_t i=0;i<g->indegree[v];i++) {
    if (!ispebbled(g->in[v][i],g,c)) return FALSE;
  }
  return TRUE;
}



/* Print a graph with a pebble configuration, with dot. */
void print_dot_Pebbling(const DAG *g, PebbleConfiguration *peb,
                        char *name,char* options) {
  ASSERT_TRUE(isconsistent_DAG(g));
  ASSERT_TRUE(isconsistent_PebbleConfiguration(g,peb));

  char *vertexopts[g->size];

  for(Vertex v=0;v<g->size;v++) {
    if (iswhite(v,g,peb)) {
      vertexopts[v]=",color=gray,fontcolor=black,fillcolor=white";
    } else if (isblack(v,g,peb)) {
      vertexopts[v]=",color=gray,fontcolor=white,fillcolor=black";
    } else {
      vertexopts[v]=",color=gray,fontcolor=black,fillcolor=lightgray";
    }
  }
  print_dot_DAG(g,name,options,vertexopts,NULL);
}

