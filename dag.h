/*
   Copyright (C) 2010 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-16, giovedì 16:32 (CET) Massimo Lauria"
   Time-stamp: "2010-12-16, giovedì 23:18 (CET) Massimo Lauria"

   Description::

   Direct acyclic graph implementation, with managing functions.  This
   implementation is mostly focused on sparse graphs.

*/

#ifndef DAG_H
#define DAG_H
/* Preamble */

#include <stdlib.h>
#include "common.h"


/* {{{ Basic datastructures:

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

  /* Precomputed data for efficient managing */
  size_t source_number;
  size_t sink_number;

  Vertex *sources;
  Vertex *sinks;

} DAG;

extern DAG* create_piramid_graph(int h);
extern DAG* copy_graph(DAG *src);
extern void dispose_graph(DAG *ptr);

extern void print_graph(DAG *p,
                        void (*vertex_label_to_string)(char*,size_t,Vertex));

extern void print_dot_graph(DAG *p,
                            char *name,
                            char* options,
                            char**vertex_options,
                            void (*vertex_label_to_string)(char*,size_t,Vertex) );


DAG *product_graph(DAG *inner,DAG *outer);

#endif /* DAG_H */
