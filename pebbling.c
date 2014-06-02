/*
   Copyright (C) 2010, 2011, 2012, 2013, 2014 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-17, venerdì 12:01 (CET) Massimo Lauria"
   Time-stamp: "2014-06-02, 08:05 (EDT) Massimo Lauria"

   Description::

   Pebbling of a graphs: we implement the main data structure for a
   pebbling, and basic management functions, to recognise which
   vertices in the graph can be pebbled and which white pebble can be
   removed.

*/

/* Preamble */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "common.h"
#include "dag.h"
#include "pebbling.h"


/**
 * Create a new empty pebbling of a given length.
 *
 * @param length the length of the pebbling to be allocated.
 *
 * @return a pointer to a the configuration.
 */
Pebbling *new_Pebbling(size_t length) {

  assert(length>0);
  Pebbling *ptr=(Pebbling*)malloc(sizeof(Pebbling));

  assert(ptr);
  ptr->steps =(Vertex*)malloc(sizeof(Vertex)*length);
  assert(ptr->steps);

  ptr->cost =0;
  return ptr;
}


/**
 * Create a copy of a Pebbling data structure.
 *
 * @param src the pebbling to be copied.
 *
 * @return a pointer to a the new pebbling.
 */
Pebbling *copy_Pebbling(const Pebbling *src) {

  assert(src);
  assert(src->length>0);

  Pebbling *ptr=(Pebbling*)malloc(sizeof(Pebbling));
  assert(ptr);

  ptr->length = src->length;
  ptr->cost = src->cost;

  ptr->steps =(Vertex*)malloc(sizeof(Vertex)*ptr->length);
  assert(ptr->steps);

  memcpy(ptr->steps,src->steps,ptr->length*sizeof(Vertex));

  return ptr;
}


/**
 * Destroy the Pebbling data structure.
 *
 * @param ptr pointer to the pebbling to be disposed.
 *
 */
void dispose_Pebbling(Pebbling *ptr) {
  assert(ptr);
  if (ptr->length>0) {
    assert(ptr->steps);
    free(ptr->steps);
  } else
    assert(ptr->steps==NULL);
  free(ptr);
}


/**
   Pebbling Configurations
 */


/**
 * Create a new empty pebble configuration allocated on the heap
 * memory.
 *
 * @return a pointer to a the configuration.
 */
PebbleConfiguration *new_PebbleConfiguration(void) {

  PebbleConfiguration *ptr=(PebbleConfiguration*)malloc(sizeof(PebbleConfiguration));

  assert(ptr);

#if WHITE_PEBBLES
  ptr->white_pebbled=0;
#endif

#if BLACK_PEBBLES  
  ptr->black_pebbled=0;
#endif

  ptr->used_pebbles = 0;
  
  ptr->sink_touched =FALSE;

  ptr->pebbles=0;

  ptr->previous_configuration=NULL;
  ptr->last_changed_vertex=0;

  return ptr;
}


/**
 * Allocate a copy of a pebble configuration.
 *
 * @param src a pointer to the configuration to be copied.
 *
 * @return a pointer to a new configuration.
 */
PebbleConfiguration *copy_PebbleConfiguration(const PebbleConfiguration *src) {

  PebbleConfiguration *dst=(PebbleConfiguration*)malloc(sizeof(PebbleConfiguration));

  assert(src);
  assert(dst);

#if WHITE_PEBBLES
  dst->white_pebbled=src->white_pebbled;
#endif

#if BLACK_PEBBLES  
  dst->black_pebbled=src->black_pebbled;
#endif

  dst->used_pebbles = src->used_pebbles;
  
  dst->sink_touched=src->sink_touched;

  dst->pebbles=src->pebbles;

  dst->previous_configuration=src->previous_configuration;
  dst->last_changed_vertex=src->last_changed_vertex;

  return dst;
}

void dispose_PebbleConfiguration(PebbleConfiguration *ptr) {
  assert(ptr);
  free(ptr);
}

/* This function checks if the configuration of pebbles referenced by
   ptr is a consistent configuration for the graph referenced by
   graph, which in turns must be consistent. */
Boolean isconsistent_PebbleConfiguration(const DAG *graph,const PebbleConfiguration *ptr) {

  assert(ptr);
  assert(graph);

  /* The size of the graph and the number of sinks must fit with the
     pebbling representation. */
  if (graph->size > BITTUPLE_SIZE) return FALSE;
  if (graph->sink_number != 1)    return FALSE;

  /* The configuration must not have both a white and a black pebble
     on the same vertex */
#if WHITE_PEBBLES && BLACK_PEBBLES
  if (ptr->white_pebbled & ptr->black_pebbled) return FALSE;
#endif

  /* If there's a pebble on the sink, sink is touched */
#if BLACK_PEBBLES
  if (ptr->black_pebbled & (BITTUPLE_UNIT << graph->sinks[0])) {
    if (!ptr->sink_touched) return FALSE;
  }
#endif

#if WHITE_PEBBLES
if (ptr->white_pebbled & (BITTUPLE_UNIT << graph->sinks[0])) {
    if (!ptr->sink_touched) return FALSE;
  }
#endif
  
  /* If sink is touched in the previous conf, then it must be later */
  if (ptr->previous_configuration!=NULL &&
      ptr->previous_configuration->sink_touched==TRUE &&
      ptr->sink_touched==FALSE
      ) return FALSE;

  /* Count the number of pebbles in the bitvector and check
     consistency with the pebble counter */
  unsigned int counter=0;

  for(size_t i=0; i<graph->size; i++) {
#if WHITE_PEBBLES    
    if GETBIT(ptr->white_pebbled,i) counter++;
#endif

#if BLACK_PEBBLES
    if GETBIT(ptr->black_pebbled,i) counter++;
#endif

  }

  if (counter!=ptr->pebbles) return FALSE;

  /* Check that the mask is clean in the residual bits */
  BitTuple mask = ptr->used_pebbles;

#if WHITE_PEBBLES
  mask |= ptr->white_pebbled;
#endif
  
#if BLACK_PEBBLES
  mask |= ptr->black_pebbled;
#endif
  
  if (BITTUPLE_SIZE > graph->size) {
    if ( mask & /* bit mentioned in the configuration */
         ~((BITTUPLE_UNIT << graph->size) -1) )      /* A bitmask which is set only on the high bits */
      return FALSE;
  }

  return TRUE;
}


inline int configurationcost(const DAG *g,const PebbleConfiguration *c) {

  assert(isconsistent_DAG(g));
  assert(isconsistent_PebbleConfiguration(g,c));

  int cost = 0;
  BitTuple pebbled = 0;

#if WHITE_PEBBLES
  pebbled |= c->white_pebbled;
#endif

#if BLACK_PEBBLES
  pebbled |= c->black_pebbled;
#endif
  
  for(Vertex v=0; v<g->size; v++) {
    if (GETBIT(pebbled,v)==1) cost++;
  }
  return cost;
}


/* ------------------------------ Manipulation of pebble status -----------------------------*/

#if BLACK_PEBBLES

/* Black */
inline void deleteblack(const Vertex v,const DAG *g,PebbleConfiguration *const c) {

  assert(isconsistent_DAG(g));
  assert(isconsistent_PebbleConfiguration(g,c));
  assert(v<g->size);
  assert(isblack(v,g,c));

  RESETBIT(c->black_pebbled,v);
  RESETBIT(c->used_pebbles,v);
  c->pebbles       -= 1;
#if REVERSIBLE
  c->used_pebbles |= g->pred_bitmasks[v];
#endif
}

/* Determines if a vertex is black pebbled according to a specific
   configuration */
inline Boolean isblack(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  assert(isconsistent_DAG(g));
  assert(isconsistent_PebbleConfiguration(g,c));
  assert(v<g->size);

  return GETBIT(c->black_pebbled,v);
}

inline void placeblack(const Vertex v,const DAG *g,PebbleConfiguration *const c) {

  assert(isconsistent_DAG(g));
  assert(isconsistent_PebbleConfiguration(g,c));
  assert(v<g->size);
  assert(!ispebbled(v,g,c));
  assert(isactive(v,g,c));

  SETBIT(c->black_pebbled,v);
  c->pebbles       += 1;
  c->used_pebbles |= g->pred_bitmasks[v];

  if (v==g->sinks[0] && !c->sink_touched) {
    c->sink_touched = TRUE;
    SETBIT(c->used_pebbles,v);
  }
}


#endif /* BLACK_PEBBLES */

#if WHITE_PEBBLES

/* White */
inline void deletewhite(const Vertex v,const DAG *g,PebbleConfiguration *const c) {

  assert(isconsistent_DAG(g));
  assert(isconsistent_PebbleConfiguration(g,c));
  assert(v<g->size);
  assert(iswhite(v,g,c));
  assert(isactive(v,g,c));

  RESETBIT(c->white_pebbled,v);
  RESETBIT(c->used_pebbles,v);
  c->pebbles       -= 1;
  c->used_pebbles |= g->pred_bitmasks[v];
}


/* Determines if a vertex is white pebbled according to a specific
   configuration */
inline Boolean iswhite(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  assert(isconsistent_DAG(g));
  assert(isconsistent_PebbleConfiguration(g,c));
  assert(v<g->size);

  return GETBIT(c->white_pebbled,v);
}

inline void placewhite(const Vertex v,const DAG *g,PebbleConfiguration *const c) {

  assert(isconsistent_DAG(g));
  assert(isconsistent_PebbleConfiguration(g,c));
  assert(v<g->size);
  assert(!ispebbled(v,g,c));

  SETBIT(c->white_pebbled,v);
  c->pebbles       += 1;

  if (v==g->sinks[0] && !c->sink_touched) {
    c->sink_touched=TRUE;
    SETBIT(c->used_pebbles,v);
  }
}

#endif /* WHITE_PEBBLES */

/* Vertex Statuses */

/* Determines if a vertex is pebbled according to a specific
   configuration */
inline Boolean ispebbled(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  assert(isconsistent_DAG(g));
  assert(isconsistent_PebbleConfiguration(g,c));
  assert(v<g->size);

#if BLACK_PEBBLES && WHITE_PEBBLES
  return GETBIT(c->white_pebbled | c->black_pebbled,v);
#elif BLACK_PEBBLES
  return GETBIT(c->black_pebbled,v);
#elif WHITE_PEBBLES
  return GETBIT(c->white_pebbled,v);
#else
  return FALSE;
#endif
}


/* Determines if there is a pebble on all predecessors of a given
   vertex */
inline Boolean isactive(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  assert(isconsistent_DAG(g));
  assert(isconsistent_PebbleConfiguration(g,c));
  assert(v<g->size);

#if BLACK_PEBBLES && WHITE_PEBBLES
  return (((c->white_pebbled | c->black_pebbled) & g->pred_bitmasks[v])
          == g->pred_bitmasks[v])?TRUE:FALSE;
#elif BLACK_PEBBLES
  return ((c->black_pebbled & g->pred_bitmasks[v])
          == g->pred_bitmasks[v])?TRUE:FALSE;
#elif WHITE_PEBBLES
  return ((c->white_pebbled & g->pred_bitmasks[v])
          == g->pred_bitmasks[v])?TRUE:FALSE;
#else
  return FALSE;
#endif
}


inline Boolean isused(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  assert(isconsistent_DAG(g));
  assert(isconsistent_PebbleConfiguration(g,c));
  assert(v<g->size);

  return GETBIT(c->used_pebbles,v);
}

/* Persistent pebbling is realized by placing a pebble on top and then
   by reaching the empty configuration. In both black-white
   andreversible pebble the actual pebbling is inverted to result in
   the desired pebbling that ends with a pebble on the sink.  */
void    init_persistent_pebbling(const DAG *g, PebbleConfiguration *const c) {

  assert(isconsistent_DAG(g));
  assert(isconsistent_PebbleConfiguration(g,c));

  assert(g->sink_number==1);
  assert(!ispebbled(g->sinks[0],g,c));
#if REVERSIBLE
  SETBIT(c->black_pebbled,g->sinks[0]);
#elif WHITE_PEBBLES
  SETBIT(c->black_pebbled,g->sinks[0]);
#else
  assert(0);
#endif
  
  c->pebbles       += 1;
  c->sink_touched = TRUE;
  SETBIT(c->used_pebbles,g->sinks[0]);
}


/* Determines if the configuration is final for a visiting
   pebbling. In a visiting pebbling we start with an empty
   configuration an we try to touch the sink. And reach a
   configuration without white pebbles. */
inline Boolean isfinal_visiting(const DAG *g,const PebbleConfiguration *c) {

  assert(isconsistent_DAG(g));
  assert(isconsistent_PebbleConfiguration(g,c));

#if WHITE_PEBBLES
  if (c->white_pebbled!=0) return FALSE;
#endif

  if (c->sink_touched==TRUE) return TRUE;
  else return FALSE;
}

/* Determines if the configuration is final for a persistent
   pebbling. In a persistent pebbling we start with a configuration
   with a hard to remove pebble on the sink (i.e. makes sense for
   white and reversible) and we try to reach the configuration with no
   hard to remove pebbles. */
inline Boolean isfinal_persistent(const DAG *g,const PebbleConfiguration *c) {

  assert(isconsistent_DAG(g));
  assert(isconsistent_PebbleConfiguration(g,c));
  assert(c->sink_touched);
  
#if WHITE_PEBBLES
  if (c->white_pebbled!=0) return FALSE;
#elif REVERSIBLE
  if (c->black_pebbled!=0) return FALSE;
#else
  assert(FALSE);           /* persistent pebbling makes sense only for REVERSIBLE and WHITE_PEBBLE */
#endif

  return TRUE;
}



/* Print a graph with a pebble configuration, with dot.  If the `peb'
   is NULL it does assume that the pebbling to be printed is empty.
   */
void fprint_dot_PebbleConfiguration(FILE *outfile,const DAG *g, const PebbleConfiguration *peb,
                        char *name,char* options) {
  assert(isconsistent_DAG(g));
  assert(peb==NULL || isconsistent_PebbleConfiguration(g,peb));

  char *vertexopts[g->size];

  for(Vertex v=0;v<g->size;v++) {

#if WHITE_PEBBLES
    if (peb && iswhite(v,g,peb)) {
      vertexopts[v]="color=gray,fontcolor=black,fillcolor=white";
      continue;
    }
#endif

#if BLACK_PEBBLES
    if (peb && isblack(v,g,peb)) {
      vertexopts[v]="color=gray,fontcolor=white,fillcolor=black";
      continue;
    }
#endif
    
    vertexopts[v]="color=gray,fontcolor=black,fillcolor=lightgray";

  }
  fprint_dot_DAG(outfile,g,name,options,vertexopts);
}


/* Print a pebbling, using dot tool */
void fprint_dot_Pebbling(FILE *outfile,const DAG *g, const Pebbling *ptr) {

  PebbleConfiguration conf;
  
  conf.sink_touched=FALSE;
#if WHITE_PEBBLES
  conf.white_pebbled = 0;
#endif
#if BLACK_PEBBLES
  conf.black_pebbled = 0;
#endif
  conf.used_pebbles = 0;
  conf.pebbles = 0;
  conf.previous_configuration = NULL;
  conf.last_changed_vertex = 0;

  fprint_dot_PebbleConfiguration(outfile,g,NULL,"X",NULL);
  Vertex v;
  for (size_t i=0; i < ptr->length; ++i) {
    v=ptr->steps[i];
    /* The logical sequence of tests ensure correctness of the moves,
       assuming the pebbling is legal */

    /* Deletions */
#if BLACK_PEBBLES
    if (isblack(v,g,&conf)) {
      deleteblack(v,g,&conf);
      fprint_dot_PebbleConfiguration(outfile,g,&conf,"X",NULL);
      continue;
    }
#endif

#if WHITE_PEBBLES
    if (iswhite(v,g,&conf)) {
      deletewhite(v,g,&conf);
      fprint_dot_PebbleConfiguration(outfile,g,&conf,"X",NULL);
      continue;
    }
#endif

    /* Placements */
#if BLACK_PEBBLES
    if (isactive(v,g,&conf)) {
      placeblack(v,g,&conf);
      fprint_dot_PebbleConfiguration(outfile,g,&conf,"X",NULL);
      continue;
    }
#endif
    
#if WHITE_PEBBLES        
    placewhite(v,g,&conf);
    fprint_dot_PebbleConfiguration(outfile,g,&conf,"X",NULL);
#endif
  }
}

/* Print a pebbling, using dot tool */
void fprint_text_Pebbling(FILE *outfile,const DAG *g, const Pebbling *ptr) {

  PebbleConfiguration conf;
  
  conf.sink_touched=FALSE;
#if WHITE_PEBBLES
  conf.white_pebbled = 0;
#endif
#if BLACK_PEBBLES
  conf.black_pebbled = 0;
#endif
  conf.used_pebbles = 0;
  conf.pebbles = 0;
  conf.previous_configuration = NULL;
  conf.last_changed_vertex = 0;

  Vertex v;
  fprintf(outfile,"v");
  for (size_t i=0; i < ptr->length; ++i) {
    v=ptr->steps[i];
    /* The logical sequence of tests ensure correctness of the moves,
       assuming the pebbling is legal */

    /* Deletions */
#if BLACK_PEBBLES
    if (isblack(v,g,&conf)) {
      deleteblack(v,g,&conf);
      fprintf(outfile," -%u",v+1);
      continue;
    }
#endif

#if WHITE_PEBBLES
    if (iswhite(v,g,&conf)) {
      deletewhite(v,g,&conf);
      fprintf(outfile," -%u",v+1);
      continue;
    }
#endif

    /* Placements */
#if BLACK_PEBBLES
    if (isactive(v,g,&conf)) {
      placeblack(v,g,&conf);
      fprintf(outfile," %u",v+1);
      continue;
    }
#endif
    
#if WHITE_PEBBLES        
    placewhite(v,g,&conf);
    fprintf(outfile," %u",v+1);
#endif
  }

  fprintf(outfile,"\n");
}



/* Heuristics */

static inline Boolean place_white_heuristics_cut(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  if (c->previous_configuration==NULL) return FALSE;
  if (v==c->last_changed_vertex) return TRUE;

  /* Never place two pebbles on vertices with decreasing
     rank. Notice that successors always have bigger rank.  */
  if (ispebbled(c->last_changed_vertex,g,c) && (v < c->last_changed_vertex)) return TRUE;

  return FALSE;
}

static inline Boolean place_black_heuristics_cut(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  if (c->previous_configuration==NULL) return FALSE;
  if (v==c->last_changed_vertex) return TRUE;

  /* Never place two pebbles on vertices with decreasing
     rank. Notice that successors always have bigger rank.  */
  if (ispebbled(c->last_changed_vertex,g,c) && (v < c->last_changed_vertex)) return TRUE;

  return FALSE;
}

static inline Boolean delete_white_heuristics_cut(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  if (!isused(v, g, c)) return TRUE;

  Vertex w=c->last_changed_vertex;
  if (c->previous_configuration==NULL) return FALSE;
  if (v==w) return TRUE;  /* White pebble can't be placed and removed,
                             not even of the sink */

  /* Never remove pebbles on vertices with increasing
     rank. Notice that predecessors always have smaller rank.  */
  if (!ispebbled(w,g,c) && (v > w)) return TRUE;

  /* If a white pebble removal is after a placement, ... */
  if (ispebbled(w,g,c)) {
    /* either the placed pebble is necessary for the removal ... */
    if (GETBIT(g->pred_bitmasks[v],w)) return FALSE;
    /* or the white pebble was necessary for the placement ... */
    if (GETBIT(g->succ_bitmasks[v],w) && isblack(w,g,c)) return FALSE;
    return TRUE;
  } else {
    return FALSE;
  }

}

static inline Boolean delete_black_heuristics_cut(const Vertex v,const DAG *g,const PebbleConfiguration *c) {

  if (!isused(v, g, c)) return TRUE;

  Vertex w=c->last_changed_vertex;
  if (c->previous_configuration==NULL) return FALSE;
  if (v==w) return (g->sinks[0]!=w);    /* A black pebble may be
                                           placed and then removed iff
                                           it's on the sink. */

  /* Never remove pebbles on vertices with increasing
     rank. Notice that predecessors always have smaller rank.  */
  if (!ispebbled(w,g,c) && (v > w)) return TRUE;

  /* If a black pebble removal is after a placement, then the placed
     vertex must be a black pebbled successor, unless we are doing a
     reversible pebbling.
  */
#if !REVERSIBLE
  if (ispebbled(w,g,c)) {
    if (!isblack(w,g,c)) return TRUE;
    if (!GETBIT(g->succ_bitmasks[v],w)) return TRUE;
  }
#endif
  
  return FALSE;
}



/* Each pebble configuration has a number of neightbours less than or
   equal to the number of vertices.  For each vertex you can either
   add a pebble (if possible) or remove one (if present).  The
   possible move is unique for every vertex.
*/
PebbleConfiguration *next_PebbleConfiguration(const Vertex v,
                                              const DAG *g,
                                              const PebbleConfiguration *old,
                                              unsigned int max_pebbles) {

  PebbleConfiguration *nconf=NULL;

  assert(isconsistent_PebbleConfiguration(g,old));

#if BLACK_PEBBLES 
  
#if REVERSIBLE
  if ( isblack(v,g,old) && isactive(v,g,old)) {
#else
  if ( isblack(v,g,old) ) {
#endif

    if (delete_black_heuristics_cut(v,g,old)) return NULL;

    /* Delete BLACK */
    nconf=copy_PebbleConfiguration(old);
    deleteblack(v,g,nconf);

    assert(isconsistent_PebbleConfiguration(g,nconf));
    return nconf;
    }
#endif /* BLACK_PEBBLES */

#if WHITE_PEBBLES
  
  if ( iswhite(v,g,old) && isactive(v,g,old) ) { /* Delete WHITE */

    if (delete_white_heuristics_cut(v,g,old)) return NULL;

    nconf=copy_PebbleConfiguration(old);
    deletewhite(v,g,nconf);

    assert(isconsistent_PebbleConfiguration(g,nconf));
    return nconf;
  }
#endif
  
#if BLACK_PEBBLES
  if ( !ispebbled(v,g,old) && isactive(v,g,old) ) { /* Place BLACK */

    if (old->pebbles >= max_pebbles) return NULL;
    if (place_black_heuristics_cut(v,g,old)) return NULL;

    nconf=copy_PebbleConfiguration(old);

    placeblack(v,g,nconf);

    assert(isconsistent_PebbleConfiguration(g,nconf));
    return nconf;
  }
#endif

#if WHITE_PEBBLES
  if ( !ispebbled(v,g,old) && !isactive(v,g,old)) { /* Place WHITE */

    if (old->pebbles >= max_pebbles) return NULL;
    if (place_white_heuristics_cut(v,g,old)) return NULL;


    nconf=copy_PebbleConfiguration(old);
    placewhite(v,g,nconf);

    assert(isconsistent_PebbleConfiguration(g,nconf));
    return nconf;
  }
#endif

  /* No allowed operation on the chosen vertex. */
  return NULL;
}


