/*
   Copyright (C) 2010, 2011 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-17, venerdì 12:01 (CET) Massimo Lauria"
   Time-stamp: "2011-01-13, giovedì 22:40 (CET) Massimo Lauria"

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


inline Boolean place_white_heuristics_cut(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  if (c->previous_configuration==NULL) return FALSE;
  if (v==c->last_changed_vertex) return FALSE;

  /* Never place two pebbles on vertices with decreasing
     rank. Notice that successors always have bigger rank.  */
  if (ispebbled(c->last_changed_vertex,g,c) && (v < c->last_changed_vertex)) return TRUE;

  return FALSE;
}

inline Boolean place_black_heuristics_cut(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  if (c->previous_configuration==NULL) return FALSE;
  if (v==c->last_changed_vertex) return FALSE;

  /* Never place two pebbles on vertices with decreasing
     rank. Notice that successors always have bigger rank.  */
  if (ispebbled(c->last_changed_vertex,g,c) && (v < c->last_changed_vertex)) return TRUE;

  return FALSE;
}

inline Boolean delete_white_heuristics_cut(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  Vertex w=c->last_changed_vertex;

  if (c->previous_configuration==NULL) return FALSE;
  if (v==w) return FALSE;

  /* Never remove pebbles on vertices with increasing
     rank. Notice that predecessors always have smaller rank.  */
  if (!ispebbled(w,g,c) && (v > w)) return TRUE;

  /* If a white pebble removal is after a placement, ... */
  /* if (ispebbled(w,g,c)) { */
  /*   /\* either the placed pebble is necessary for the removal ... *\/ */
  /*   if ((BITTUPLE_UNIT << w) & g->pred_bitmasks[v]) return FALSE; */
  /*   /\* or the white pebble was necessary for the placement ... *\/ */
  /*   if (((BITTUPLE_UNIT << w) & g->succ_bitmasks[v]) && isblack(w,g,c)) return FALSE; */
  /*   return TRUE; */
  /* } */

  return FALSE;
}

inline Boolean delete_black_heuristics_cut(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  if (c->previous_configuration==NULL) return FALSE;
  if (v==c->last_changed_vertex) return FALSE;

  /* Never remove pebbles on vertices with increasing
     rank. Notice that predecessors always have smaller rank.  */
  if (ispebbled(c->last_changed_vertex,g,c) && (v > c->last_changed_vertex)) return TRUE;

  /* /\* If a black pebble removal is after a placement, then the placed */
  /*    vertex must be a black pebbled successor *\/ */
  /* if (ispebbled(c->last_changed_vertex,g,c)) { */
  /*   if (!isblack(c->last_changed_vertex,g,c)) return TRUE; */
  /*   if (!((BITTUPLE_UNIT << c->last_changed_vertex) & g->succ_bitmasks[v])) return TRUE; */
  /* } */

  return FALSE;
}




PebbleConfiguration *new_PebbleConfiguration(void) {

  PebbleConfiguration *ptr=(PebbleConfiguration*)malloc(sizeof(PebbleConfiguration));

  ASSERT_NOTNULL(ptr);

  ptr->white_pebbled=0;
  ptr->black_pebbled=0;

  ptr->sink_touched =FALSE;

  ptr->pebble_cost=0;
  ptr->pebbles=0;

  ptr->previous_configuration=NULL;
  ptr->last_changed_vertex=0;

  return ptr;
}


PebbleConfiguration *copy_PebbleConfiguration(const PebbleConfiguration *src) {

  PebbleConfiguration *dst=(PebbleConfiguration*)malloc(sizeof(PebbleConfiguration));

  ASSERT_TRUE(src);
  ASSERT_NOTNULL(dst);

  dst->white_pebbled=src->white_pebbled;
  dst->black_pebbled=src->black_pebbled;

  dst->sink_touched=src->sink_touched;

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

  /* The size of the graph and the number of sinks must fit with the
     pebbling representation. */
  if (graph->size > BITTUPLE_SIZE) return FALSE;
  if (graph->sink_number != 1)    return FALSE;

  /* The configuration must not have both a white and a black pebble
     on the same vertex */
  if (ptr->white_pebbled & ptr->black_pebbled) return FALSE;

  /* Check that only the appropriate pebbles are used */
#if !BLACK_PEBBLES
  if (ptr->black_pebbled!=0) return FALSE;
#endif

#if !WHITE_PEBBLES
  if (ptr->white_pebbled!=0) return FALSE;
#endif

  /* If there's a pebble on the sink, sink is touched */
  if ((ptr->white_pebbled | ptr->black_pebbled) & (BITTUPLE_UNIT << graph->sinks[0])) {
    if (!ptr->sink_touched) return FALSE;
  }

  /* If sink is touche in the previous conf, then it must be later */
  if (ptr->previous_configuration!=NULL &&
      ptr->previous_configuration->sink_touched==TRUE &&
      ptr->sink_touched==FALSE
      ) return FALSE;

  /* Count the number of pebbles in the bitvector and check
     consistency with the pebble counter */
  unsigned int counter=0;

  for(size_t i=0; i<graph->size; i++) {
    if (ptr->white_pebbled & (BITTUPLE_UNIT << i)) counter++;
    if (ptr->black_pebbled & (BITTUPLE_UNIT << i)) counter++;
  }

  if (counter!=ptr->pebbles) return FALSE;

  /* Check that the mask is clean in the residual bits */
  if (BITTUPLE_SIZE != graph->size) {
    if ( (ptr->white_pebbled | ptr->black_pebbled) &  /* The OR of the bitsequences */
         ~((BITTUPLE_UNIT << graph->size) -1) )                   /* A bitmask which is set only on the high bits */
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

  return (c->black_pebbled & (BITTUPLE_UNIT<<v))?TRUE:FALSE;
}

/* Determines if a vertex is white pebbled according to a specific
   configuration */
inline Boolean iswhite(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  ASSERT_TRUE(isconsistent_DAG(g));
  ASSERT_TRUE(isconsistent_PebbleConfiguration(g,c));
  ASSERT_TRUE(v<g->size);

  return (c->white_pebbled & (BITTUPLE_UNIT<<v))?TRUE:FALSE;
}


/* Determines if a vertex is pebbled according to a specific
   configuration */
inline Boolean ispebbled(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  ASSERT_TRUE(isconsistent_DAG(g));
  ASSERT_TRUE(isconsistent_PebbleConfiguration(g,c));
  ASSERT_TRUE(v<g->size);

  return ((c->white_pebbled | c->black_pebbled) & (BITTUPLE_UNIT<<v))?TRUE:FALSE;
}

/* Determines if there is a pebble on all predecessors of agiven
   vertex */
inline Boolean isactive(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  ASSERT_TRUE(isconsistent_DAG(g));
  ASSERT_TRUE(isconsistent_PebbleConfiguration(g,c));
  ASSERT_TRUE(v<g->size);

  if (isblack(v,g,c)) return FALSE;

  return (((c->white_pebbled | c->black_pebbled) & g->pred_bitmasks[v])
          == g->pred_bitmasks[v])?TRUE:FALSE;

  /* for(size_t i=0;i<g->indegree[v];i++) { */
  /*   if (!ispebbled(g->in[v][i],g,c)) return FALSE; */
  /* } */
  /* return TRUE; */
}


/* Determines if the configuration is a final one */
inline Boolean isfinal(const DAG *g,const PebbleConfiguration *c) {

  ASSERT_TRUE(isconsistent_DAG(g));
  ASSERT_TRUE(isconsistent_PebbleConfiguration(g,c));

  if (c->white_pebbled==0 && c->sink_touched==TRUE) return TRUE;
  else return FALSE;
}



/* Print a graph with a pebble configuration, with dot.  If the `ped'
   is NULL it does assume that the pebbling to be printed is empty.
   */
void print_dot_Pebbling(const DAG *g, const PebbleConfiguration *peb,
                        char *name,char* options) {
  ASSERT_TRUE(isconsistent_DAG(g));
  ASSERT_TRUE(peb==NULL || isconsistent_PebbleConfiguration(g,peb));

  char *vertexopts[g->size];

  for(Vertex v=0;v<g->size;v++) {
    if (peb && iswhite(v,g,peb)) {
      vertexopts[v]="color=gray,fontcolor=black,fillcolor=white";
    } else if (peb && isblack(v,g,peb)) {
      vertexopts[v]="color=gray,fontcolor=white,fillcolor=black";
    } else {
      vertexopts[v]="color=gray,fontcolor=black,fillcolor=lightgray";
    }
  }
  print_dot_DAG(g,name,options,vertexopts,NULL);
}


void print_dot_Pebbling_Path(const DAG *g, const PebbleConfiguration *ptr) {
  while(ptr) {
    ASSERT_TRUE(isconsistent_PebbleConfiguration(g,ptr));
    print_dot_Pebbling(g,ptr,"X",NULL);
    ptr=ptr->previous_configuration;
  }
}


/* Each pebble configuration has a number of neightbours less than or
   equal to the number of vertices.  For each vertex you can either
   add a pebble (if possible) or remove one (if present).  The
   possible move is unique for every vertex.
*/
PebbleConfiguration *next_PebbleConfiguration(const Vertex v, const DAG *g, const PebbleConfiguration *old) {

  PebbleConfiguration *nconf=NULL;

  ASSERT_TRUE(isconsistent_PebbleConfiguration(g,old));

  if (isactive(v,g,old) &&  iswhite(v,g,old) ) { /* Delete WHITE */

    if (delete_white_heuristics_cut(v,g,old)) return NULL;

    nconf=copy_PebbleConfiguration(old);
    nconf->white_pebbled ^= (BITTUPLE_UNIT << v);
    nconf->pebbles     -= 1;

    ASSERT_TRUE(isconsistent_PebbleConfiguration(g,nconf));
    return nconf;
  }

#if BLACK_PEBBLES
  if (isactive(v,g,old) && !iswhite(v,g,old) ) { /* Place BLACK */

    if (place_black_heuristics_cut(v,g,old)) return NULL;

    nconf=copy_PebbleConfiguration(old);
    nconf->black_pebbled |= (BITTUPLE_UNIT << v);
    nconf->pebbles     += 1;

    if (nconf->pebbles > nconf->pebble_cost)
      nconf->pebble_cost = nconf->pebbles;
    if (v==g->sinks[0])
      nconf->sink_touched=TRUE;

    ASSERT_TRUE(isconsistent_PebbleConfiguration(g,nconf));
    return nconf;
  }
#endif

  if (!isactive(v,g,old) && isblack(v,g,old) ) { /* Delete BLACK */

    if (delete_black_heuristics_cut(v,g,old)) return NULL;

    nconf=copy_PebbleConfiguration(old);
    nconf->black_pebbled ^= (BITTUPLE_UNIT << v);
    nconf->pebbles     -= 1;

    ASSERT_TRUE(isconsistent_PebbleConfiguration(g,nconf));
    return nconf;
  }

#if WHITE_PEBBLES

#if BLACK_PEBBLES
  if (!isactive(v,g,old) && !iswhite(v,g,old)) { /* Place WHITE */
#else
  if (!iswhite(v,g,old)) { /* Place WHITE */
#endif

    if (place_white_heuristics_cut(v,g,old)) return NULL;

    nconf=copy_PebbleConfiguration(old);
    nconf->white_pebbled |= (BITTUPLE_UNIT << v);
    nconf->pebbles     += 1;
    if (nconf->pebbles > nconf->pebble_cost)
      nconf->pebble_cost = nconf->pebbles;
    if (v==g->sinks[0])
      nconf->sink_touched=TRUE;

    ASSERT_TRUE(isconsistent_PebbleConfiguration(g,nconf));
    return nconf;
  }
#endif
  /* No allowed operation on the chosen vertex. */
  return NULL;
}


