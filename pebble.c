/*
  Massimo lauria, 2010
  
  This incomplete program is supposed to compute
  the black white pebbling number of a graph.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BLACK_PEBBLE  0x1
#define WHITE_PEBBLE  0x2
#define SINK_VERTEX   0x4
#define SOURCE_VERTEX 0x8

/* Check wheter a pointer is NULL, in such case abort */
#define ASSERT_NOTNULL(p) { if (!(p)) exit(-1); }

/* {{{ Basic datastructures:
   a Vertex is encoded as an int, 
   a VertexList is a classic adjiacency list of vertices */
typedef int Vertex;
typedef struct _VertexList { Vertex idx;  struct _VertexList *next; } VertexList;

/* Vertices are given in some topological order, so the unique sink is the last
   vertex.

   The representation of the graph is a standard adjiacency list representation. Each
   vertex has both a list of incoming and outgoing edges.  
*/
typedef struct { 

  int size;     /* Number of vertices in the DAG */

  VertexList **in;  /* Vector of adiacency lists */
  VertexList **out;

} DAG;





VertexList *sorted_insertion(VertexList *l,Vertex el) {
  VertexList *v,*t; 
  
  v=(VertexList*)malloc(sizeof(VertexList));
  ASSERT_NOTNULL(v);
  v->idx  = el;
  
  /* HEAD Insertion*/
  if ( (l==NULL) || (el < l->idx) ) {
    v->next=l;
    return v;
  }
  
  /* Inner insertion invariant: the new element is always >= then the element 
     pointed by t */
  while ( (t->next!=NULL) && (el >= t->next->idx) ) t=t->next;
  v->next=t->next;
  t->next=v;
  return l;
}

VertexList *sorted_update(VertexList *l,VertexList *data) {
  
  VertexList *head;
  VertexList *v;
  VertexList *cursor; 
  Vertex el;

  /* At most one HEAD insertion may happen */
  head=l;
  if ( (head=NULL) || (data->idx < head->idx) ) {
    v=(VertexList*)malloc(sizeof(VertexList));
    ASSERT_NOTNULL(v);
    v->idx = data->idx;
    v->next = head;
    head=v;
    data=data->next;
  }
  
  /* All successive insertions are after the HEAD */
  cursor=head;
  while(data) {
    el = data->idx;

    v=(VertexList*)malloc(sizeof(VertexList));
    ASSERT_NOTNULL(v);
    v->idx = data->idx;
    
    /* Inner insertion invariant: the new element is always >= then the element 
       pointed by t */
    while ( (cursor->next!=NULL) && (el >= cursor->next->idx) ) cursor=cursor->next;
    v->next=cursor->next;
    cursor->next=v;

    data=data->next;
  }
  return head;
}

void dispose_list(VertexList *l) {
  VertexList *c;
  while(l) { c=l; l=l->next; free(c); }
}

/* }}} */

/* Utility function for populating a graph with arcs */
/* {{{ */ void add_arc(DAG* dag,Vertex source,Vertex dest) {
  
  /* Some sanitiy checks */
  ASSERT_NOTNULL(dag);
  ASSERT_NOTNULL(dag->in);
  ASSERT_NOTNULL(dag->out);

  if ((source >= dag->size) 
      || (dest >= dag->size)) return;
  
  /* An incoming arcs to dest and an outgoing arc from source */
  dag->in[dest]  = sorted_insertion(dag->in[dest],source);
  dag->out[source] = sorted_insertion(dag->out[source],dest);

  return;
} 
/* }}} */

/* A classic case for pebbling is the piramid graph */
/* {{{ */ DAG* create_piramid_graph(int h) {
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
  ASSERT_NOTNULL(d);

  /* Allocation of arcs lists */
  d->size = (h+2)*(h+1) / 2; 

  d->in  = (VertexList**)calloc( d->size, sizeof(VertexList*) );
  d->out = (VertexList**)calloc( d->size, sizeof(VertexList*) );

  ASSERT_NOTNULL(d->in );
  ASSERT_NOTNULL(d->out);
  
  v=0;
  for (i=0; i < h; ++i)
  {
    add_arc(d,v,v+h+1-i);        /* First of each level has only an arc */
    v++;
    for (j = 1; j < h-i; ++j) {
      add_arc(d,v,v+h  -i);      /* Backward arc */
      add_arc(d,v,v+h+1-i);      /* Forward arc */
      v++;     
    }
    add_arc(d,v,v+h-i);          /* Last of each level has only an arc */
    v++;
  }
  
  return d;
}
/* }}} */


/* This builds a copy of a DAG */
/* {{{ */ DAG* clone_graph(DAG *orig) {
  
  int i;
  VertexList *t;
  DAG *d;

  if (!orig) return NULL;

  d=(DAG*)malloc(sizeof(DAG));
  ASSERT_NOTNULL(d);

  /* Allocation of arcs lists */
  d->size = orig->size; 

  d->in  = (VertexList**)calloc( d->size, sizeof(VertexList*) );
  d->out = (VertexList**)calloc( d->size, sizeof(VertexList*) );

  ASSERT_NOTNULL(d->in );
  ASSERT_NOTNULL(d->out);

  if (!orig->out) return d;
  
  for (i=0; i < d->size; ++i)
  {
    t=orig->out[i];
    while(t) { add_arc(d,i,t->idx); t=t->next; }
  }
  
  return d;
}
/* }}} */
             
             
/* Destroy a DAG structure, freeing memory */
/* {{{ */ void dispose_graph(DAG *p) {
  int i;
  
  /* Ignore null graphs */
  if (!p) return;
  
  /* Incoming edges */
  if (p->in) {
    for (i = 0; i < p->size; ++i) dispose_list(p->in[i]);
    free(p->in);
  }
 /* Outgoing edges */
  if (p->out) {
    for (i = 0; i < p->size; ++i) dispose_list(p->out[i]);
    free(p->out);
  }
  free(p);
} 
/* }}} */


/* Prints a string representation of the DAG */
/* {{{ */ void print_graph(DAG *p) {
  int i;
  VertexList *h;
  
  /* Ignore null graphs */
  if (!p) return;

  for (i = 0; i < p->size; ++i) {
    printf("%d ",i); 
    /* Incoming edges */
    if (p->in) {
      printf("I:"); 
      h=p->in[i];
      while(h) { printf(" %d",h->idx); h=h->next;}
    }
  
    /* Outgoing edges */
    if (p->out) { 
      printf(" O:"); 
      h=p->out[i];
      while(h) { printf(" %d",h->idx); h=h->next;}
    }
    printf("\n");
  }
}
/* }}} */ 


/* Prints a representation of the DAG, which can be used by DOT (graphviz) */
/* {{{ */ void print_dot_graph(DAG *p,char *name,char* options) {
  int i;
  VertexList *h;
  
  /* Ignore null graphs */
  if (!p) return;

  printf("digraph %s {\n",name);
  if (options) printf("%s\n",options);

  for (i = 0; i < p->size; ++i) {
   
    /* Outgoing edges */
    if (p->out) {
      printf("\t /* Arcs outgoing from %d*/ \n",i);
      h=p->out[i];
      while(h) { 
        printf("\t %d -> %d ;\n",i,h->idx); 
        h=h->next;
      }
      printf("\n");
    }
  }

  printf("}\n");

}
/* }}} */


/* Utility forproduct_graph */
/* {{{ */ void offset_graph(DAG *p,int offset) {
  int i;
  VertexList *h;
  
  /* Ignore null graphs */
  if (!p) return;
  
  /* Incoming edges */
  if (p->in) {
    for (i = 0; i < p->size; ++i) {
      h=p->in[i];
      while(h) { h->idx += offset; h=h->next;}
    }
  }
 /* Outgoing edges */
  if (p->out) {
    for (i = 0; i < p->size; ++i) {
      h=p->out[i];
      while(h) { h->idx += offset; h=h->next;}
    }
  }
}
/* }}} */


/* Build the OR product of two DAGs */
/* {{{ */ DAG *product_graph(DAG *inner,DAG *outer) {
  DAG *p=NULL;
  DAG *h=NULL;
  VertexList *t;
  Vertex x,y;
  int A,B;
  int i;
  if (!inner || !outer) return NULL;
  A=inner->size;
  B=outer->size;

  /* Initial structure */
  p=(DAG*)malloc(sizeof(DAG));
  ASSERT_NOTNULL(p);
  p->size=A*B;
  p->in  = (VertexList**)calloc( A*B, sizeof(VertexList*) );
  p->out = (VertexList**)calloc( A*B, sizeof(VertexList*) );

  ASSERT_NOTNULL(p->in );
  ASSERT_NOTNULL(p->out);
  
  /* Several copies of inner graphs are produced and glued together */
  for(i=0;i<B;i++) {
    h=clone_graph(inner);
    offset_graph(h,i*A);
    memcpy(p->in + i*A, h->in , A*sizeof(VertexList*) );
    memcpy(p->out+ i*A, h->out, A*sizeof(VertexList*) );
    /* Dispose cloned copy */
    free(h->in);
    free(h->out);
    free(h);
  }
  /* Add intermediate edges */
  for(i=0;i<B;i++) {
    t=outer->out[i];
    while(t) {
      x=i*A+A-1;
      for( y=A*(t->idx); y < A*(t->idx+1) ; y++ ) add_arc(p,x,y);
      t=t->next;
    }
  }
  return p;
}
/* }}} */

/* Explore the space of pebbling strategies.
   The output is given as a sequence of vertices, because
   at any point in a pebbling, there is a unique minimal move that can be 
   performed on a vertex, given its status. 
   
   If there is a pebble on the vertex, such pebble shoud be removed.
   If there is not, then either a white or a black pebble should be added. 
   In case both are possible, it is always convenient to add a black one.

   Notice that any pebbling induce a dual pebbling with a reversed sequence of vertices.
   Thus the output of this function can be interpreted in both directions.
*/   
/* {{{ */ void pebbling_strategy(DAG *g) {
  Vertex i;

  /* Pebbling sequence */
  
  /* Cursors */
  VertexList **in,**out;
  
  /* List of vertices, to keep sorted */
  VertexList *wpebbles=NULL,*bpebbles=NULL;
  VertexList *wremovable=NULL, *baddable=NULL;
  VertexList *sources;
  Vertex     sink;

  /* PROLOGUE ----------------------------------- */
  /* Find the sink and the sources */
  sink=g->size; /* Dummy value for sink, it signals no sinks has been found yet */
  out=g->out; /* Cache pointers */
  in =g->in;
  for(i=g->size-1; i>=0; --i) {
    
    /* A sink is found */
    if (out[i]==NULL) {
      if (sink >= g->size ) {
        printf("Sorry, DAGs with multiple sinks are not supported");
        ASSERT_NOTNULL(NULL);
      } 
      else sink=i;
    }
    
    /* A source is found, notice that insertion in sorted list 
       could cost up to the length of the list, but we scan the vertices backwards
       thus all insertions are on the HEAD of the list, which costs O(1) */
    if (in[i]==NULL) sources=sorted_insertion(sources,i);
  }

  /* SEARCH ALGORITHM */

  

  /* EPILOGUE --------------------------------- */
  /* free memory, free Mandela! */
  dispose_list(sources);
  dispose_list(wpebbles);
  dispose_list(bpebbles);
  dispose_list(wremovable);
  dispose_list(baddable);
}
/* }}} */

int main(int argc, char *argv[])
{
  DAG *A,*B,*P;
  A=create_piramid_graph(2);
  B=create_piramid_graph(3);

  P=product_graph(B,A);
  dispose_graph(A);
  dispose_graph(B);

  print_dot_graph(P,"prodotto",NULL);

  dispose_graph(P);
  return 0;
}


    
