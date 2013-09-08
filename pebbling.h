/*
   Copyright (C) 2010, 2011, 2012, 2013 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-17, venerdì 12:03 (CET) Massimo Lauria"
   Time-stamp: "2013-09-08, 22:01 (CEST) Massimo Lauria"

   Description::

   Pebbling data structure.

*/


/* Preamble */
#ifndef  PEBBLING_H
#define  PEBBLING_H

#include <stdlib.h>
#include "common.h"
#include "dag.h"



/**
 *  A pebble configuration is represented by a bitset of at most
 *  MAX_VERTICES elements.  This allows to represent the pebbling in
 *  small space, which is the most important thing since the
 *  configuration may be a large number.
 */
typedef struct _PebbleConfiguration {

  /* Since the configuration space can be large, we use integers of
   specific size as bit tuples of fixed length.  The specific integer
   type is defined here. */
  BitTuple white_pebbled;    /* White pebbled vertices */
  BitTuple black_pebbled;    /* Black pebbled vertices */
  BitTuple useful_pebbles;   /* Pebbled used for other pebbles */

  Boolean sink_touched;

  unsigned int pebbles;  /* Number of pebbles in the pebbling */

  /* Information regarding the  configuration transition:  what is the
     pebble cost  needed to reach  the present configuration?  What is
     the actual  previous configuration? What was the  vertex we acted
     on to reach the present configuration? */
  unsigned int pebble_cost;             

  /* information about the transition */
  struct _PebbleConfiguration *previous_configuration;
  Vertex last_changed_vertex;

} PebbleConfiguration;

/**
 *  A pebbling can be described as a sequence of vertices, since given
 *  a vertex in the graph, of all action possible there is at most one
 *  which is legal and locally optimal.  We also include pebbling
 *  length and cost in order to avoid the recomputation.
 */
typedef struct {

  Vertex *steps;
  size_t  length;
  int     cost;

} Pebbling;


extern Pebbling*  new_Pebbling(size_t length);
extern Pebbling*  copy_Pebbling(const Pebbling *src);
extern void       dispose_Pebbling(Pebbling*);

extern PebbleConfiguration*  new_PebbleConfiguration();
extern PebbleConfiguration* copy_PebbleConfiguration(const PebbleConfiguration *src);
extern void              dispose_PebbleConfiguration(PebbleConfiguration *ptr);
extern Boolean      isconsistent_PebbleConfiguration(const DAG *graph,const PebbleConfiguration *ptr);

extern Boolean isfinal(const DAG *g,const PebbleConfiguration *c);


/* Vertices operations */
extern Boolean iswhite    (const Vertex v, const DAG *g, const PebbleConfiguration *c);
extern void    placewhite (const Vertex v, const DAG *g, PebbleConfiguration *const c);
extern void    deletewhite(const Vertex v, const DAG *g, PebbleConfiguration *const c);

extern Boolean isblack    (const Vertex v, const DAG *g, const PebbleConfiguration *c);
extern void    placeblack (const Vertex v, const DAG *g, PebbleConfiguration *const c);
extern void    deleteblack(const Vertex v, const DAG *g, PebbleConfiguration *const c);

extern Boolean ispebbled(const Vertex v, const DAG *g, const PebbleConfiguration *c);
extern Boolean isactive (const Vertex v, const DAG *g, const PebbleConfiguration *c);
extern Boolean isuseful (const Vertex v, const DAG *g, const PebbleConfiguration *c);




/**
   Produce a neighbor configuration 
   
   Given a configuration, produces the neighbor configuration obtained
   operating on vertex v. This is well defined since there is at most
   one action which is sensible to do on a given vertex.

   In some cases some legal configuration are not produced if the
   resulting pebbling would be clearly sub optimal (this heuristics
   are put in place to reduce the runtime and the growth of the search
   space).
   
   OUTPUT:

   @return a pebble configuration or NULL
*/
extern PebbleConfiguration *next_PebbleConfiguration(const Vertex v,
                                                     const DAG *g,
                                                     const PebbleConfiguration *old,
                                                     unsigned int max_pebbles);


/* Pretty Print functions */
extern void print_dot_Pebbling(const DAG *g, const Pebbling *ptr);


#endif /* PEBBLING_H */
