/*
   Copyright (C) 2010, 2011, 2013, 2014 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-16, giovedì 16:32 (CET) Massimo Lauria"
   Time-stamp: "2014-03-18, 15:26 (CET) Massimo Lauria"

   Description::

   Direct acyclic graph implementation, with managing functions.  This
   implementation is mostly focused on sparse graphs.

*/

#ifndef DAG_H
#define DAG_H
/* Preamble */

#include <stdlib.h>
#include "common.h"


/* Basic data structures:

   a Vertex is encoded as an int,
   a VertexList is a classic adjiacency list of vertices

*/

typedef size_t Vertex;

/* The  representation of  the  graph is  a  standard adjiacency  list
   representation.  Each  vertex  has  both  a list  of  incoming  and
   outgoing edges.

   An info array may  or may not be present (i.e. it  is null). If not
   null it contains the status of the vertices.

   This representation is very static and is not apt to be modified
   much during the program.  This is not an issue, since we are mostly
   interested in analysis of a static graph.

 */
typedef struct {

  size_t size;     /* Number of vertices in the DAG */

  size_t *indegree;
  size_t *outdegree;

  Vertex **in;      /* Vector of incoming vertices */
  Vertex **out;     /* Vector of outgoing vertices */

  /* The following data is precomputed for efficiency */

  BitTuple *pred_bitmasks;  /* Bitmask indicating the predecessors of a vertex */
  BitTuple *succ_bitmasks;  /* Bitmask indicating the predecessors of a vertex */

  size_t source_number;
  size_t sink_number;

  Vertex *sources;
  Vertex *sinks;

} DAG;

/* Data structure management functions */
extern Boolean isconsistent_DAG(const DAG *ptr);
extern DAG*    copy_DAG(const DAG *src);
extern void    dispose_DAG(DAG* ptr);

/* I/O Functions */
extern void print_DAG(const DAG *p);

extern void print_dot_DAG(const DAG *p,
                            char *name,
                            char *options,
                            char**vertex_options);


/* Structure building functions */
extern DAG* pyramid(int h);
extern DAG* path(int n);
extern DAG* leader(int n);
extern DAG* tree(int h);

extern DAG* orproduct(const DAG *outer,const DAG *inner);

#endif /* DAG_H */







