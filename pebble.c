/*
  Massimo Lauria, 2010
  
  This incomplete program is supposed to compute
  the black white pebbling number of a graph.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define EMPTY_STATUS  0x0
#define BLACK_PEBBLE  0x1
#define WHITE_PEBBLE  0x2
#define SINK_VERTEX   0x4
#define SOURCE_VERTEX 0x8

/* Check wheter a pointer is NULL, in such case abort */
#define ASSERT_NOTNULL(p) { if (!(p)) exit(-1); }

/* {{{ Basic datastructures:
   a Vertex is encoded as an int, 
   a VertexList is a classic adjiacency list of vertices */
typedef unsigned int Vertex;
typedef unsigned int VertexInfo;
typedef struct _VertexList { Vertex idx;  struct _VertexList *next; } VertexList;

/* Vertices are given in some topological order, so the unique sink is the last
   vertex.

   The representation of the graph is a standard adjiacency list representation. Each
   vertex has both a list of incoming and outgoing edges.  
   
   An array may or may not be present (i.e. it is null). If not null
   it contains the status of the vertices. */
typedef struct { 

  size_t size;     /* Number of vertices in the DAG */

  VertexList **in;  /* Vector of adiacency lists */
  VertexList **out;
  
  VertexInfo *info;  /* Additional information about each vertex, its use is application dependent. */

} DAG;


/* 
 *  The graph of configuration is a directed graph in which each node
 *  is indexed by an hash of the configuration it represents, and
 *  contains the array of the HASHED configuration that follows.  We
 *  cache the number of pebbles required to reach a configuration and
 *  the previous configuration in the chain.  Notice that the
 *  plausible configurations are not that many.  So we need to use a
 *  sparse representation.
 */
#define MAX_VERTICES sizeof(long unsigned int)*CHAR_BIT
#define MAX_PEBBLES  10
/*#define HASH_SIZE    0x0FFFFFFF*/
#define HASH_SIZE    0x0FFFFF
#define HASH_WASTE_RANGE  0x0FFFF
#define ROTATE_RIGHT_AMOUNT (sizeof(long unsigned int)*CHAR_BIT/2)
/*  
 *  The basic data structure for a pebbling is essentially a bunch of
 *  bit vectors, implemented as an array of integers, which represent
 *  the sets of black and white pebbles in the configuration.
 *  
 *  The aux pointer is used to realize the buckets in the hash (Yes, I
 *  know. Auxiliary data shouldn't go inside the status data
 *  structure).  Notice that aux data are not hashed, thus two
 *  idential configuration which different auxiliary data will
 *  collide, which is what we want.
 */ 
typedef struct _PebblingStatus {

  long unsigned int white_pebbled;
  long unsigned int black_pebbled;
  long unsigned int active_vertices;
  int pebbles;

  /* How did we reach this configuration */
  int pebble_cost;
  struct _PebblingStatus *previous_configuration;

  struct _PebblingStatus* next;
} PebblingStatus;

void copy_pebbling_status(PebblingStatus *src,PebblingStatus *dst) {

  ASSERT_NOTNULL(src);
  ASSERT_NOTNULL(dst);

  dst->white_pebbled=src->white_pebbled;
  dst->black_pebbled=src->black_pebbled;
  dst->active_vertices=src->active_vertices;
  dst->pebbles=src->pebbles;
  
  dst->pebble_cost=src->pebble_cost;
  dst->previous_configuration=src->previous_configuration;
  dst->next=src->next;
}



/* The dictionary has a very simple implementation.  It is an array
   lists, each list is indexed by an hash which is reduced modulo the
   size of the array.  Notice that for performance we statically
   allocate the array, and the actual size is between 15/16 and 16/16
   of the allocated size.  */
typedef struct {
  PebblingStatus *pool[HASH_SIZE];
  int key;
} ConfigurationDictionary;

/* 
 * When the dictionary is queried the result is simply the index of
 * the bucket in which the record hashes, and a pointer to a matching
 * occurrence in the dictionary, if there is any. A null pointer
 * represent no matching record.
 */
typedef struct {
  long unsigned int bucket_idx;
  PebblingStatus *occurrence;
} DictionaryQueryResult;



extern long int random(void);

void dictionary_init(ConfigurationDictionary* d) {
  long unsigned int r;

  ASSERT_NOTNULL(d);
  
  r=random() % HASH_WASTE_RANGE;
  d->key = HASH_SIZE - HASH_WASTE_RANGE + r; 
}

DictionaryQueryResult dictionary_find(ConfigurationDictionary* d,PebblingStatus *s) {
  long unsigned int h;
  PebblingStatus *t;
  DictionaryQueryResult res={ 0UL, NULL };

  ASSERT_NOTNULL(d);
  ASSERT_NOTNULL(s);
  
  /* Compute the hash and then find the position in the array */
  h= ( s->white_pebbled >> ROTATE_RIGHT_AMOUNT ) 
    | ( s->white_pebbled << ( sizeof(long unsigned int)*8 - ROTATE_RIGHT_AMOUNT) ) 
    | s->black_pebbled;
  h = h % d->key;

  t=d->pool[h];
  while(t) {
    if ( (t->white_pebbled == s->white_pebbled ) &&
         (t->black_pebbled == s->black_pebbled ) ) break;
  }

  res.occurrence = t;
  res.bucket_idx = h;

  return res;
}

/* 
 *  The update function has the following semantic: if the
 *  configuration is absent from the dictionary, then a new record is
 *  added to the dictionary, otherwise an old one is updated.  
 */
DictionaryQueryResult dictionary_update(ConfigurationDictionary* d,PebblingStatus *s) {
  DictionaryQueryResult res={ 0UL, NULL };
  PebblingStatus *n;

  ASSERT_NOTNULL(d);
  ASSERT_NOTNULL(s);

  res = dictionary_find(d,s);

  if (res.occurrence==NULL) { 
    /* The configuration does not occur in the dictionary */
    n = (PebblingStatus*)malloc(sizeof(PebblingStatus));
    n->white_pebbled= s->white_pebbled;
    n->black_pebbled= s->black_pebbled;
    n->active_vertices= s->active_vertices;
    n->pebble_cost  = s->pebble_cost;
    n->previous_configuration=s->previous_configuration;
    
    n->next=d->pool[res.bucket_idx];
    d->pool[res.bucket_idx]=n;
  
  } else if (res.occurrence->pebble_cost > s->pebble_cost) {
    /* The configuration occur, so we update the old record if there's the need. */
    res.occurrence->pebble_cost=s->pebble_cost;
    res.occurrence->previous_configuration=s->previous_configuration;
  }
  return res;
}

/* A configuration is sane only if linked to configurations in the
   appropriate dictionary */
void assert_status_sanity(DAG *g,PebblingStatus *s,ConfigurationDictionary *dict) {
  DictionaryQueryResult q;
  int i;
  int covered=0;
  VertexList *p;

  ASSERT_NOTNULL(dict);
  ASSERT_NOTNULL(s);
  if (g->size > MAX_VERTICES) ASSERT_NOTNULL(NULL);

  if (s->next!=NULL) {
    q=dictionary_find(dict,s->next);
    if (q.occurrence != s->next) {
      fprintf(stderr,"A configuration is linked to some other which is not in the dictionary.");
      exit(-1);
    }
  }
  if (s->previous_configuration!=NULL) {
    q=dictionary_find(dict,s->previous_configuration);
    if (q.occurrence != s->previous_configuration) {
      fprintf(stderr,"A configuration is linked to some other which is not in the dictionary.");
      exit(-1);
    }
  }
  /* FIXME: Check that the number of indicated pebbles is correct. */
  /* Check that the active pebbles are all correct (i.e. a
     vertex is active if and only if both the predecessors are
     pebbled. */
  for (i=0;i<g->size;i++) {
    p=g->in[i];
    covered=1;
    while(p) {
      /* Is the predecessor pebbled? */
      if ( (s->white_pebbled | s->black_pebbled) & ( 0x1 << p->idx )) continue;
      else { covered=0; break; }
      p=p->next;
    }
    /* Should the predecessors be pebbled? */
    if ( s->active_vertices & (0x1 << i) ) {
      if (!covered) {
        fprintf(stderr,"ERROR: A configuration has an active vertex with some unpebbled predecessor.");
        exit(-1);  }
    } else {
      if (covered) {
        fprintf(stderr,"ERROR: A configuration has an non-active vertex with all pebbled predecessor.");
        exit(-1);  }
    }
  }
}



/* This is intended to receive an adjacency list, and to insert the
   vertex in el in the appropriated position, maintaining the
   decreasing ordering of the adjacency list.  */
VertexList *sorted_insertion(VertexList *l,Vertex el) {
  VertexList *v,*t; 
  
  v=(VertexList*)malloc(sizeof(VertexList));
  ASSERT_NOTNULL(v);
  v->idx  = el;
  /* v->next is not initialized because it will be set in any case */

  /* HEAD Insertion*/
  if ( (l==NULL) || (el < l->idx) ) {
    v->next=l;
    return v;
  }
  
  /* Inner insertion invariant: the new element is always >= than the element 
     pointed by t */
  t=l;
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

  d->info= (VertexInfo*)calloc( d->size, sizeof(VertexInfo) );

  ASSERT_NOTNULL(d->in );
  ASSERT_NOTNULL(d->out);
  ASSERT_NOTNULL(d->info);
  
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


/* This builds a copy of a DAG.  

   N.B. The clone of an inconsistent graph is always consistent, but
   may fail to represent the original inconsistent graph. */

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

  d->info = (VertexInfo*)calloc( d->size, sizeof(VertexInfo) );


  ASSERT_NOTNULL(d->in );
  ASSERT_NOTNULL(d->out);
  ASSERT_NOTNULL(d->info);

  /* If there are outgoing edges, we add the respective arcs, notice
  that this insertion do not check consistency of the original graph.
  If the original graph is not well formed (e.g. it has outgoing edges
  but not the respective incoming edges) it adds nevertheless.
  */
  if (orig->out) { 
    for(i=0; i < d->size; ++i) { 
      t=orig->out[i];
      while(t) { add_arc(d,i,t->idx); t=t->next; } 
    } 
  }
  
  /* We also clone the infos on the vertices. */
  if (orig->info) {
    memcpy(d->info,orig->info,d->size);
  }
  return d;
}
/* }}} */
             
             
/* Destroys a DAG structure, freeing memory */
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
  /* Info */
  if (p->info) free(p->info);

  /* Dispose the main data structure */
  free(p);
} 
/* }}} */

/* This is the default way a label is assigned to a vertex idx Given a
   buffer and a size limit, it writes on the buffer the label for the
   vertex v.  The default impletemtation is to map numbers in their
   string representation.
 */
extern int snprintf(char* buf,size_t size, const char *format, ... );
void default_vertex_label_hash(char* buf,size_t l,Vertex v) {
  snprintf(buf,l,"%d",v);
}


/* Prints a string representation of the DAG */
/* {{{ */ void print_graph(DAG *p, void (*vertex_label_hash)(char*,size_t,Vertex)) {
  int i;
  VertexList *h;
  char label_buffer[20];
  
  /* Ignore null graphs */
  if (!p) return;

  /* If no fucntion for computing labels is specified, then we use the
     default one, which turn a number in its string respresentation */
  if (!vertex_label_hash) vertex_label_hash=default_vertex_label_hash;

  for (i = 0; i < p->size; ++i) {
    
    if (p->info) {
      printf("[");
      if (p->info[i] & BLACK_PEBBLE) {
        printf("B");
      } else if (p->info[i] & WHITE_PEBBLE) {
        printf("W");
      } else {
        printf(" ");
      }
      printf("] ");
    }
    vertex_label_hash(label_buffer,20,i);
    printf("%s ",label_buffer); 
    /* Incoming edges */
    if (p->in) {
      printf("I:"); 
      h=p->in[i];
      while(h) { 
        vertex_label_hash(label_buffer,20,h->idx);
        printf(" %s",label_buffer); 
        h=h->next;
      }
    }
  
    /* Outgoing edges */
    if (p->out) { 
      printf(" O:"); 
      h=p->out[i];
      while(h) { 
        vertex_label_hash(label_buffer,20,h->idx);
        printf(" %s",label_buffer); 
        h=h->next;
      }
    }
    printf("\n");
  }
}
/* }}} */ 



/* Prints a representation of the DAG, which can be used by DOT and
 * graphviz.
 */
void print_dot_graph(DAG *p,char *name,char* options,void (*vertex_label_hash)(char*,size_t,Vertex) ) {
  int i;
  VertexList *h;
  char label_buffer[20];
  
  /* Ignore null graphs */
  if (!p) return;
  
  /* If no function for computing labels is specified, then we use the
   * default one, which turn a number in its string
   * representation. (setq c-block-comment-prefix "*")
   */
  if (!vertex_label_hash) vertex_label_hash=default_vertex_label_hash;
 
  printf("digraph %s {\n",name);
  printf("\t rankdir=BT;\n");
  if (options) printf("%s\n",options);
  


  /* Print the eventual info */
  for (i = 0; i < p->size; ++i) {

    /* Vertex identifier */
    printf("\t %d [",i);
    
    /* Start with vertex info */
    vertex_label_hash(label_buffer,20,i);
    printf("label=%s,penwidth=2,shape=circle,style=filled,fixedsize=true",label_buffer); 

    /* if there are no pebbling information, move to the next
       vertex */
    if (!p->info) { printf("]\n"); continue; }

    if (p->info[i] & BLACK_PEBBLE) {
      printf(",color=gray,fontcolor=white,fillcolor=black");
    } else if (p->info[i] & WHITE_PEBBLE) {
      printf(",color=gray,fontcolor=black,fillcolor=white");
    } else {
      printf(",color=gray,fontcolor=black,fillcolor=lightgray");
    }
    printf("]\n");
  }
 
  
  for (i = 0; i < p->size; ++i) {
   
    /* Outgoing edges */
    if (p->out) {
      vertex_label_hash(label_buffer,20,i);
      printf("\t /* Arcs outgoing from %s (key %d)*/ \n",label_buffer,i);
      h=p->out[i];
      while(h) {
        printf("\t %d -> %d;\n",i,h->idx); 
        h=h->next;
      }
      printf("\n");
    }
  }

  printf("}\n");

}
/* }}} */


/* Utility for product_graph */
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
  p->info= (VertexInfo* )calloc( A*B, sizeof(VertexInfo)  );

  ASSERT_NOTNULL(p->in );
  ASSERT_NOTNULL(p->out);
  ASSERT_NOTNULL(p->info);
  
  /* Several copies of inner graphs are produced and glued together */
  for(i=0;i<B;i++) {
    h=clone_graph(inner);
    offset_graph(h,i*A);
    memcpy(p->in + i*A, h->in , A*sizeof(VertexList*) );
    memcpy(p->out+ i*A, h->out, A*sizeof(VertexList*) );
    /* Dispose the cloned copy. Notice that the lists in the cloned copy
       are not freed, since we keep them attached to the product graph
       adjacency list.  */
    free(h->in);
    free(h->out);
    free(h->info);
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

   INPUT: 
   
         -- DAG: the graph to pebble.
         -- upper_bound: the maximum number of pebbles in the configuration,
                         if such number is not sufficient, the the computation will fail gracefully 
                         without finding the pebbling
*/   
/* {{{ */ void pebbling_strategy(DAG *g,unsigned int upper_bound) {
  Vertex i;
  
  /* Dictionary data tructure */
  ConfigurationDictionary dict;
  /* Initial and Temporaty Status */
  PebblingStatus initial_status={0UL,0UL,0UL,0,0,NULL,NULL};
  /* PebblingStatus temp_status   ={0UL,0UL,0UL,0,0,NULL,NULL}; */

  /* Pebbling sequence */
  
  /* Cursors */
  VertexList **in,**out;
  
  /* List of vertices, to keep sorted */
  VertexList *sources=NULL;
  Vertex     sink;

  /* PROLOGUE ----------------------------------- */
  if (g->size > MAX_VERTICES) {
    fprintf(stderr,"Error in search procedure: the graph is too big for the optimized data structures.");
    exit(-1);
  } 
  /* Find the sink and the sources */
  sink=g->size; /* Dummy value for sink, it signals no sinks has been found yet */
  out=g->out; /* Cache pointers */
  in =g->in;
  for(i=g->size-1; i+1 > 0; --i) {
    
    /* A sink is found */
    if (out[i]==NULL) {
      if (sink < g->size ) {
        printf("Sorry, DAGs with multiple sinks are not supported");
        ASSERT_NOTNULL(NULL);
      } 
      else sink=i;
    }
    
    /* A source is found, notice that insertion in sorted list 
       could cost up to the length of the list, but we scan the vertices backwards
       thus all insertions are on the HEAD of the list, which costs O(1) */
    if (in[i]==NULL) {
      sources=sorted_insertion(sources,i);
      /* All source nodes are active for the initial status */
      initial_status.active_vertices |= (1 << i);
    }

  }

  /* SEARCH ALGORITHM */
  /* The first attempt to implement this program will use a standard
     graph reachability algorithm for directed graphs */ 

  /* Since graphs of configuration will be very large, we try to
     represent a configuration with the smallest memory footprint.
     Furthermore there will be a lot of useless or non valid
     configurations, thus we produce configurations on demand.
     We use an hash to access configurations.

     N.B. As a futue option: we could use a ZDD for keeping track of visited
     configurations.
  */
  dictionary_init(&dict);
  dictionary_update(&dict,&initial_status);
  assert_status_sanity(g,&initial_status,&dict);  

  /* EPILOGUE --------------------------------- */
  /* free memory, free Mandela! */
  dispose_list(sources);
}
/* }}} */



/* 
 *  The example test program creates two pyramid graphs and produces
 *  the OR-product graph of them.  Then it prints the DOT
 *  representation of such graphs.
 */ 

int main(int argc, char *argv[])
{
  DAG *A,*B,*P;
  A=create_piramid_graph(2);
  B=create_piramid_graph(3);

  
  print_dot_graph(A,"A",NULL,NULL);
  print_dot_graph(B,"B",NULL,NULL);

  P=product_graph(B,A);
 
  print_dot_graph(P,"AxB",NULL,NULL);

  pebbling_strategy(B,3);

  dispose_graph(A);
  dispose_graph(B);
  dispose_graph(P);
  return 0;
}


    
