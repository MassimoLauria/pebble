/*
   Copyright (C) 2010, 2011, 2012, 2013, 2014 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2010-12-16, giovedì 17:03 (CET) Massimo Lauria"
   Time-stamp: "2014-06-02, 09:09 (EDT) Massimo Lauria"

   Description::

   Implementation of Direct acyclic graph functionality.


*/

/* Preamble */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "dag.h"


/********************************************************************************
                     DATA STRUCTURE MANAGEMENT FUNCTIONS
 ********************************************************************************/

/*
   Graph structure is mostly used statically. Once the graph has been
   defined, it is not modified anymore.  Thus we can do precomputing
   of some information.  */
void dag_precompute_data(DAG *digraph) {

  assert(digraph->sinks  ==NULL);
  assert(digraph->sources==NULL);

  /* Count the number of sources */
  digraph->sink_number=0;
  digraph->source_number=0;

  for(size_t i=0;i<digraph->size;i++) {
    if (digraph->indegree[i]  == 0) digraph->source_number++;
    if (digraph->outdegree[i] == 0) digraph->sink_number++;
  }

  assert(digraph->source_number >0);
  assert(digraph->sink_number   >0);

  /* Allocate the space for source and sink arrays */
  digraph->sinks=(Vertex*)malloc(sizeof(Vertex)*(digraph->sink_number));
  digraph->sources=(Vertex*)malloc(sizeof(Vertex)*(digraph->source_number));

  /* Now save the sources and sinks */
  size_t source_extend=0;
  size_t sink_extend=0;
  for(size_t i=0;i<digraph->size;i++) {
    if (digraph->indegree[i] == 0) digraph->sources[source_extend++]=i;
    if (digraph->outdegree[i] == 0) digraph->sinks[sink_extend++]=i;
  }

  /* Computes the predecessors and successors bitmaks */
  if (digraph->size > BITTUPLE_SIZE) {
    digraph->pred_bitmasks = NULL;
    digraph->succ_bitmasks = NULL;
  } else {
    digraph->pred_bitmasks=(BitTuple*)malloc(sizeof(BitTuple)*(digraph->size));
    digraph->succ_bitmasks=(BitTuple*)malloc(sizeof(BitTuple)*(digraph->size));
    for(Vertex v=0;v<digraph->size;v++) {
      digraph->pred_bitmasks[v]=BITTUPLE_ZERO;
      digraph->succ_bitmasks[v]=BITTUPLE_ZERO;
      for(size_t j=0;j<digraph->indegree[v];j++) {
        assert(digraph->in[v][j] < v );
        digraph->pred_bitmasks[v] |= (BITTUPLE_UNIT << digraph->in[v][j]);
      }
      for(size_t j=0;j<digraph->outdegree[v];j++) {
        assert(digraph->out[v][j] > v);
        digraph->succ_bitmasks[v] |= (BITTUPLE_UNIT << digraph->out[v][j]);
      }
    }
  }
}


/* This builds a copy of a DAG.

   N.B. The clone of an inconsistent graph maybe inconsistent, but the
   inconsistency may be different. */

DAG* copy_DAG(const DAG *src) {

  DAG *d;

  assert(src);

  d=(DAG*)malloc(sizeof(DAG));

  assert(d);

  /* Allocation for arcs specification */
  d->size = src->size;

  d->in  = (Vertex**)malloc( d->size*sizeof(Vertex*) );
  d->out = (Vertex**)malloc( d->size*sizeof(Vertex*) );

  assert(d->in );
  assert(d->out);

  /* Copy the degree specifications */
  d->indegree  = (size_t*)malloc( d->size*sizeof(size_t) );
  d->outdegree = (size_t*)malloc( d->size*sizeof(size_t) );

  assert(d->indegree );
  assert(d->outdegree);

  memcpy(d->indegree,src->indegree,d->size);
  memcpy(d->outdegree,src->outdegree,d->size);

  /*
     If the original graph is not well formed (e.g. it has outgoing
     edges but not the respective incoming edges) we copy the same
     defects.
  */
  
  /* Copy of incoming neighbour information */
  Vertex v;
  for(v=0;v<d->size;v++) {

    if (d->indegree[v]==0) { d->in[v]=NULL; continue; }

    d->in[v]  = (Vertex*)malloc( (d->indegree[v]) *sizeof(Vertex) );
    assert(d->in [v]  );
    assert(src->in [v]);
    memcpy(d->in[v],src->in[v],d->size);
  }

  /* Copy of outgoing neighbour information */
  for(v=0;v<d->size;v++) {

    if (d->outdegree[v]==0) { d->out[v]=NULL; continue; }

    d->out[v] = (Vertex*)malloc( (d->outdegree[v])*sizeof(Vertex) );
    assert(d->out   [v]);
    assert(src->out [v]);
    memcpy(d->out[v],src->out[v],d->size);
  }

  dag_precompute_data(d);

  assert(isconsistent_DAG(d)); /* Construction should be sound */
  return d;
}


/* Destroys a DAG structure, freeing memory */
void dispose_DAG(DAG* p) {

  /* Ignore null graphs */
  if (p==NULL) return;

  Vertex v; /* index variable */

  /* Remove incoming arcs arrays */
  if (p->in!=NULL) {
    for(v=0;v<p->size;v++)
      if (p->in[v]!=NULL) free(p->in[v]);
    free(p->in);
  }
  /* Remove outgoing arcs arrays */
  if (p->out!=NULL) {
    for(v=0;v< p->size ;v++)
      if (p->out[v]!=NULL) free(p->out[v]);
    free(p->out);
  }

  /* Dispose other arays */
  if (p->indegree  !=NULL) free(p->indegree );
  if (p->outdegree !=NULL) free(p->outdegree);
  if (p->sources   !=NULL) free(p->sources);
  if (p->sinks     !=NULL) free(p->sinks  );
  if (p->pred_bitmasks !=NULL) free(p->pred_bitmasks);
  if (p->succ_bitmasks !=NULL) free(p->succ_bitmasks);

  /* Dispose the main data structure */
  free(p);
}

/* TODO: Check that the data structure representing the graph is sound,
   meaning that any outgoing edge is related with a corresponding
   incoming edge, that the degrees have been computed appropriately.
   All checks are rather exprensive but this is not an issue for the
   application of pebbling. */
Boolean isconsistent_DAG(const DAG *ptr) {

  assert(ptr);
  assert(ptr->size > 0 );

  assert(ptr->indegree );
  assert(ptr->outdegree);

  assert(ptr->in);
  assert(ptr->out);

  assert(ptr->sinks);
  assert(ptr->sources);

  /* Bitmasks are present iff they are big enough */
  if (ptr->size <= BITTUPLE_SIZE) {
    assert(ptr->pred_bitmasks);
    assert(ptr->succ_bitmasks);
  } else {
    assert(ptr->pred_bitmasks==NULL);
    assert(ptr->succ_bitmasks==NULL);
  }
  return TRUE;
}


/********************************************************************************
                     I/O FUNCTIONS
 ********************************************************************************/

/* Prints a string representation of the DAG in kth format */
void fprint_DAG(FILE *outfile,const DAG *p,const char* prefix) {
  Vertex v,w;

  /* Ignore null graphs */
  if (!p) return;

  assert(p->indegree);
  assert(p->in);
  assert(p->outdegree);
  assert(p->out);

  for (v = 0; v < p->size; ++v) {
    
    if (prefix) fprintf(outfile,"%s",prefix);

    fprintf(outfile,"%lu :",v+1);

    for(w=0;w < p->indegree[v]; w++) {
      fprintf(outfile," %lu",p->in[v][w]+1);
    }
    fprintf(outfile,"\n");
  }
}



/* Prints a representation of the DAG, which can be used by DOT and
 * graphviz. Vertices are indexed starting from 1, despite being
 * indexed from zero in the internal representation.
 */
void fprint_dot_DAG(FILE *outfile,
                    const DAG *p,
                    char *name,
                    char* options,
                    char **vertex_options) {
  Vertex i,j;

  /* Ignore null graphs */
  if (!p) return;

  assert(p->indegree);
  assert(p->in);
  assert(p->outdegree);
  assert(p->out);

  fprintf(outfile,"digraph %s {\n",name);
  fprintf(outfile,"\t rankdir=BT;\n");
  if (options) fprintf(outfile,"%s\n",options);



  /* Print the vertex infos */
  for (i = 0; i < p->size; ++i) {

    /* Vertex identifier */
    fprintf(outfile,"\t %lu [",i+1);

    /* Start with vertex info */
    fprintf(outfile,"label=%lu,penwidth=2,shape=circle,style=filled,fixedsize=true",i+1);

    /* if there are no vertex dependent options, move to the next
       vertex */
    if (vertex_options && vertex_options[i])
      fprintf(outfile,",%s",vertex_options[i]);
    fprintf(outfile,"]\n");
  }


  for (i = 0; i < p->size; ++i) {

    /* Outgoing edges */
    fprintf(outfile,"\t /* Arcs outgoing from %lu */ \n",i+1);

    for(j=0;j<p->outdegree[i];j++)
      fprintf(outfile,"\t %lu -> %lu;\n",i+1,p->out[i][j]+1);

    fprintf(outfile,"\n");
  }

  fprintf(outfile,"}\n");

}
/* }}} */


/********************************************************************************
                     STRUCTURE BUILDING FUNCTIONS
 ********************************************************************************/

/* A classic case for pebbling is the complete tree */
/* {{{ */ DAG* tree(int height) {

  Vertex u,v;
  DAG *d=(DAG*)malloc(sizeof(DAG));
  assert(d);
  assert(height>=0);

  /* Set to null sinks and source vector */
  d->pred_bitmasks=NULL;
  d->succ_bitmasks=NULL;
  d->sources=NULL;
  d->sinks=NULL;

  /* Size of the vertex set */
  d->size=(1<<height) + ((1 << height) - 1);

  /* Allocation of degree information */

  d->in  = (Vertex**)malloc( d->size*sizeof(Vertex*) );
  d->out = (Vertex**)malloc( d->size*sizeof(Vertex*) );

  assert(d->in );
  assert(d->out);

  d->indegree  = (size_t*)calloc( d->size,sizeof(size_t) );
  d->outdegree = (size_t*)calloc( d->size,sizeof(size_t) );

  assert(d->indegree );
  assert(d->outdegree);

  /* Computing of degree information */
  for(v=0; v < (1<<height); v++)       d->indegree[v]=0;  /* Source elements */
  for(v=(1<<height); v < d->size; v++) d->indegree[v]=2;  /* Non source elements */
  for(v=0; v < d->size-1; v++)         d->outdegree[v]=1; /* Non sink elements */
  d->outdegree[v]=0;

  /* Allocation of neighbour informations */
  for(v=0;v<d->size;v++) {
    d->in[v]  = (Vertex*)malloc( (d->indegree[v]) *sizeof(Vertex) );
    d->out[v] = (Vertex*)malloc( (d->outdegree[v])*sizeof(Vertex) );
    assert(d->in [v]);
    assert(d->out[v]);
  }

  /* Incoming vertices */
  /* Notice that in a level by level enumeration from root to leafs,
     pred(x)={2x+1,2x+2}
     Thus in our enumeration: pred(x)={N-1-2*(N-x),N-2*(N-x)}
  */
  u=0;
  for(v=(1<<height); v < d->size; v++) { /* Non source elements */
    d->in[v][0]=u++;
    d->in[v][1]=u++;
  }
  u=(1<<height);
  for(v=0; v < d->size-2; v+=2) { /* Non sink elements */
    d->out[v  ][0]=u;
    d->out[v+1][0]=u;
    u++;
  }

  dag_precompute_data(d);
  assert(isconsistent_DAG(d)); /* Construction should be sound */
  return d;
}
/* }}} */


/* A classic case for pebbling is the pyramid graph */
/* {{{ */ DAG* pyramid(int h) {
/*

           14
          / \
         12   13
        / \ / \
       9   10  11      this graph has HEIGHT 4
      / \ / \ / \
     5   6   7   8
    / \ / \ / \ / \
   0   1   2   3   4

*/

  int i,j,v;
  DAG *d=(DAG*)malloc(sizeof(DAG));
  assert(d);
  assert(h>=0);

  /* Set to null sinks and source vector */
  d->pred_bitmasks=NULL;
  d->succ_bitmasks=NULL;
  d->sources=NULL;
  d->sinks=NULL;


  /* Allocation of degree information */
  d->size = (h+2)*(h+1) / 2;

  d->in  = (Vertex**)malloc( d->size*sizeof(Vertex*) );
  d->out = (Vertex**)malloc( d->size*sizeof(Vertex*) );

  assert(d->in );
  assert(d->out);

  d->indegree  = (size_t*)calloc( d->size,sizeof(size_t) );
  d->outdegree = (size_t*)calloc( d->size,sizeof(size_t) );

  assert(d->indegree );
  assert(d->outdegree);

  /* Computing of degree information */
  v=0;
  for (i=0; i < h; ++i)
  {
    d->outdegree[v]=1;      /* First of each level has only an arc */
    v++;
    for (j = 1; j < h-i; ++j) {
      d->outdegree[v]=2;
      v++;
    }
    d->outdegree[v]=1;     /* Last of each level has only an arc */
    v++;
  }
  for(v=h+1;v<d->size;v++) d->indegree[v]=2; /* Indegree is 2 for any non source */

  /* Allocation of neighbour informations */
  for(v=0;v<d->size;v++) {
    d->in[v]  = (Vertex*)malloc( (d->indegree[v]) *sizeof(Vertex) );
    d->out[v] = (Vertex*)malloc( (d->outdegree[v])*sizeof(Vertex) );
    assert(d->in [v]);
    assert(d->out[v]);
  }

  /* Fix the incoming vertices */
  v=0;
  for (i=0; i < h; ++i)
  {
    /* First of each level has only an arc */
    d->out      [v][0]=v+h+1-i;
    d->in [v+h+1-i][0]=v      ;
    v++;
    for (j = 1; j < h-i; ++j) {

      d->out      [v][0]=v+h-i;   /* Left arc */
      d->in [v+h  -i][1]=v;

      d->out      [v][1]=v+h+1-i; /* Right arc */
      d->in [v+h+1-i][0]=v;
      v++;
    }
    d->out      [v][0]=v+h-i;  /* Last of each level has only an arc */
    d->in [v+h  -i][1]=v;
    v++;
  }

  dag_precompute_data(d);
  assert(isconsistent_DAG(d)); /* Construction should be sound */
  return d;
}
/* }}} */

/* Build a path. */
/* {{{ */ DAG* path(int n) {
/*

   0 --> 1 --> 2 --> 3 --> 4  a path of length 4.

*/

  int v;
  DAG *d=(DAG*)malloc(sizeof(DAG));
  assert(d);

  /* Set to null sinks and source vector */
  d->pred_bitmasks=NULL;
  d->succ_bitmasks=NULL;
  d->sources=NULL;
  d->sinks=NULL;

  /* Allocation of degree information */
  d->size = n+1;

  d->in  = (Vertex**)malloc( d->size*sizeof(Vertex*) );
  d->out = (Vertex**)malloc( d->size*sizeof(Vertex*) );

  assert(d->in );
  assert(d->out);

  d->indegree  = (size_t*)calloc( d->size,sizeof(size_t) );
  d->outdegree = (size_t*)calloc( d->size,sizeof(size_t) );

  assert(d->indegree );
  assert(d->outdegree);

  /* Computing of degree information */
  for(v=0; v < n; ++v)
  {
    d->outdegree[v] =1;      /* Except for last vertex: outdegree is 1 */
    d->indegree[v+1]=1;      /* Except for first vertex: indegree is 1 */
  }

  /* Allocation of neighbour informations */
  for(v=0;v<d->size;v++) {
    d->in[v]  = (Vertex*)malloc( (d->indegree[v]) *sizeof(Vertex) );
    d->out[v] = (Vertex*)malloc( (d->outdegree[v])*sizeof(Vertex) );
    assert(d->in [v]);
    assert(d->out[v]);
  }

  /* Fix the incoming vertices */
  for(v=0; v < n; ++v)
  {
    d->out[v][0] =v+1;      /* Except for last vertex: outdegree is 1 */
    d->in[v+1][0]=v;        /* Except for first vertex: indegree is 1 */
  }

  dag_precompute_data(d);
  assert(isconsistent_DAG(d)); /* Construction should be sound */
  return d;
}
/* }}} */


/* Build a star. */
/* {{{ */ DAG* leader(int n) {
/*

   0 ---\
   1 ----\
   2 -----> 6   All sources but one, and a sink.
   3 ----/
   4 ---/
   5 --/
*/

  int v;
  DAG *d=(DAG*)malloc(sizeof(DAG));
  assert(d);

  /* Set to null sinks and source vector */
  d->pred_bitmasks=NULL;
  d->succ_bitmasks=NULL;
  d->sources=NULL;
  d->sinks=NULL;

  /* Allocation of degree information */
  d->size = n+1;

  d->in  = (Vertex**)malloc( d->size*sizeof(Vertex*) );
  d->out = (Vertex**)malloc( d->size*sizeof(Vertex*) );

  assert(d->in );
  assert(d->out);

  d->indegree  = (size_t*)calloc( d->size,sizeof(size_t) );
  d->outdegree = (size_t*)calloc( d->size,sizeof(size_t) );

  assert(d->indegree );
  assert(d->outdegree);

  /* Computing of degree information */
  for(v=0; v < n; ++v)
  {
    d->outdegree[v]=1;      /* Except for last vertex: outdegree is 1 */
    d->indegree [v]=0;      /* Except for last vertex: indegree is  0 */
  }
  d->outdegree[n]=0;
  d->indegree [n]=n;

  /* Allocation of neighbour informations */
  for(v=0;v<d->size;v++) {
    d->in[v]  = (Vertex*)malloc( (d->indegree[v]) *sizeof(Vertex) );
    d->out[v] = (Vertex*)malloc( (d->outdegree[v])*sizeof(Vertex) );
    assert(d->in [v]);
    assert(d->out[v]);
  }

  /* Fix the incoming vertices */
  for(v=0; v < n; ++v)
  {
    d->out[v][0] = n;      /* Except for last vertex: outdegree is 1 */
    d-> in[n][v] = v;
  }
  dag_precompute_data(d);
  assert(isconsistent_DAG(d)); /* Construction should be sound */
  return d;
}
/* }}} */



/* Build the OR product of two DAGs

   The OR product means that there is a vertex (i,j) for every vertex
   i of the outer graph and vertex j of the inner graph.

   The predecessors of each (i,j) are the (i,j') for j' predecessor of j
   plus (i',s) for each inner sink s and i' predecessor of i in the
   outer graph.

*/
/* {{{ */ DAG *orproduct(const DAG *outer,const DAG *inner) {
  DAG *p=NULL;

  Vertex i,j;
  Vertex xo,xi,x;
  size_t So,Si,S;

  assert(inner);
  assert(outer);

  /* Size of the new graph */
  Si=inner->size;
  So=outer->size;
  S =  So * Si;

  /* Initial structure */
  p=(DAG*)malloc(sizeof(DAG));
  assert(p);
  p->size=S;

  /* Set to null sinks and source vector */
  p->pred_bitmasks=NULL;
  p->succ_bitmasks=NULL;
  p->sources=NULL;
  p->sinks=NULL;

  /* Basic arrays */
  p->in  = (Vertex**)malloc( S*sizeof(Vertex*) );
  p->out = (Vertex**)malloc( S*sizeof(Vertex*) );
  assert(p->in );
  assert(p->out);
  p->indegree  = (size_t*)malloc( S*sizeof(size_t) );
  p->outdegree = (size_t*)malloc( S*sizeof(size_t) );
  assert(p->indegree );
  assert(p->outdegree);

  /* Computes the degrees */

  /* Number of inner edges */
  for(xo=0;xo<So;xo++) {
    for(xi=0;xi<Si;xi++) {
      p->indegree [xo*Si+xi]  = inner->indegree[xi]; // + (outer->indegree[xo])*inner->sink_number;
      p->outdegree[xo*Si+xi]  = inner->outdegree[xi];
    }
  }
  /* Incoming edges from predecessors' sinks */
  for(xo=0;xo<So;xo++) {
    for(xi=0;xi<Si;xi++) {
      p->indegree [xo*Si+xi]  += (outer->indegree[xo])*inner->sink_number;
    }
  }
  /* Outgoing edges to successors' sources */
  for(xo=0;xo<So;xo++) {
    for(i=0;i<inner->sink_number;i++) {
      p->outdegree[xo*Si+inner->sinks[i]] += Si*(outer->outdegree[xo]);
    }
  }

  /* Allocation of in/out arrays */
  for(x=0;x<S;x++) {
    p->in[x]  = (Vertex*)malloc(p->indegree[x]*sizeof(Vertex));
    p->out[x] = (Vertex*)malloc(p->outdegree[x]*sizeof(Vertex*));
  }

  /* Build the network of links */

  /* Setup inner arcs */
  for(xo=0;xo<So;xo++) {
    for(xi=0;xi<Si;xi++) {
      /* Incoming */
      for(i=0;i<inner->indegree[xi] ;i++)  p->in[xo*Si+xi][i] =inner->in[xi][i]+xo*Si;
      /* Outgoing */
      for(i=0;i<inner->outdegree[xi];i++)  p->out[xo*Si+xi][i]=inner->out[xi][i]+xo*Si;
    }
  }

  /* Edges between outer copies to be connected */
  Vertex yo,yi;

  /* Incoming edges from predecessors' sinks */
  for(yo=0;yo<So;yo++)
    for(i=0;i<outer->indegree[yo];i++) {
      xo=outer->in[yo][i];
      /* (xo -> yo) is the outer pair to be connected */
      for(yi=0;yi<Si;yi++) for(j=0;j<inner->sink_number;j++)  {
          xi=inner->sinks[j];
          /* (xi -> yi) is the inner pair to be connected */
          p->in[yo*Si+yi][inner->indegree[yi]+i*inner->sink_number+j] = xo*Si+xi;
      }
    }

  /* Outgoing edges to successors' vertices */
  for(xo=0;xo<So;xo++)
    for(i=0;i<outer->outdegree[xo];i++) {
      yo=outer->out[xo][i];
      /* (xo -> yo) is the outer pair to be connected */
      for(j=0;j<inner->sink_number;j++) for(yi=0;yi<Si;yi++)  {
          xi=inner->sinks[j];
          /* (xi -> yi) is the inner pair to be connected
             notice that inner->outdegree[x0] should be always 0.
           */
          p->out[xo*Si+xi][inner->outdegree[xi]+i*Si+yi] = yo*Si+yi;
      }
    }


  dag_precompute_data(p);
  assert(isconsistent_DAG(p)); /* Construction should be sound */
  return p;
}
/* }}} */
