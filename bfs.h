/*
   Copyright (C) 2010, 2011, 2012 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2011-01-13, giovedì 15:29 (CET) Massimo Lauria"
   Time-stamp: "2012-06-04, 01:38 (CEST) Massimo Lauria"

   Description::

   Implementation of a bradth-first-search procedure for black-white
   pebbling.

*/


/* Preamble */
#ifndef  BFS_H
#define  BFS_H

#include "common.h"
#include "dag.h"
#include "pebbling.h"


/* Code */
extern Pebbling* bfs_pebbling_strategy(DAG *digraph,
                                                  unsigned int bottom,
                                                  unsigned int top,
                                                  Boolean persistent_pebbling);

#endif /* BFS_H */
