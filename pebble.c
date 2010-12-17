/*
  Massimo Lauria, 2010

  This incomplete  program is supposed  to compute the  black pebbling
  number of a graph.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "common.h"
#include "dag.h"

#define EMPTY_STATUS  0x0
#define BLACK_PEBBLE  0x1
#define WHITE_PEBBLE  0x2
#define SINK_VERTEX   0x4
#define SOURCE_VERTEX 0x8





/*
 *  The graph of configuration is  a directed graph in which each node
 *  is  indexed by  an hash  of the  configuration it  represents, and
 *  contains the  array of the HASHED configuration  that follows.  We
 *  cache the number of pebbles  required to reach a configuration and
 *  the  previous  configuration  in   the  chain.   Notice  that  the
 *  plausible configurations are  not that many.  So we  need to use a
 *  sparse representation.
 */
#define MAX_VERTICES sizeof(BitTuple)*CHAR_BIT
#define MAX_PEBBLES  10
/*#define HASH_SIZE    0x0FFFFFFF*/
#define HASH_SIZE    0x0FFFFF
#define HASH_WASTE_RANGE  0x0FFFF
#define ROTATE_RIGHT_AMOUNT (sizeof(BitTuple)*CHAR_BIT/2)
/*
 *  The basic data structure for  a pebbling is essentially a bunch of
 *  bit vectors, implemented as  an array of integers, which represent
 *  the sets of black and white pebbles in the configuration.
 *
 *  The aux pointer is used to realize the buckets in the hash (Yes, I
 *  know.   Auxiliary  data  shouldn't   go  inside  the  status  data
 *  structure).   Notice  that  aux  data  are not  hashed,  thus  two
 *  idential  configuration   which  different  auxiliary   data  will
 *  collide, which is what we want.
 */
typedef struct _PebblingStatus {

  /* Since the configuration space can be large, we use integers of
   specific size for as bit tuples of fixed length.  The specific
   integer type is defined here. */
  BitTuple white_pebbled;    /* White pebbled vertices */
  BitTuple black_pebbled;    /* Black pebbled vertices */
  BitTuple active_vertices;  /* Vertices  that   can  possibly  change
                                their status in the next round */
  int pebbles;

  /* Information regarding the  configuration transition:  what is the
     pebble cost  needed to reach  the present configuration?  What is
     the actual  previous configuration? What was the  vertex we acted
     on to reach the present configuration? */
  int pebble_cost;
  struct _PebblingStatus *previous_configuration;
  Vertex last_changed_vertex;

  /* Link for bucket management */
  struct _PebblingStatus* next;
} PebblingStatus;

typedef struct _PebblingStatusList {
  PebblingStatus *ptr;
  struct _PebblingStatusList* next;
} PebblingStatusList;

void copy_pebbling_status(PebblingStatus *src,PebblingStatus *dst) {

  ASSERT_NOTNULL(src);
  ASSERT_NOTNULL(dst);

  dst->white_pebbled=src->white_pebbled;
  dst->black_pebbled=src->black_pebbled;
  dst->active_vertices=src->active_vertices;
  dst->pebbles=src->pebbles;

  dst->pebble_cost=src->pebble_cost;
  dst->previous_configuration=src->previous_configuration;
  dst->last_changed_vertex=src->last_changed_vertex;

  /* dst->next=src->next; */
}



/* The dictionary  has a very  simple implementation.  It is  an array
   lists, each list is indexed by  an hash which is reduced modulo the
   size  of the  array.   Notice that  for  performance we  statically
   allocate the array, and the  actual size is between 15/16 and 16/16
   of the allocated size.

   The dictionary also keeps a list of elements in the buckets. It is
   sorted with respect to the time of insertion in the dictionary.
 */
typedef struct {

  PebblingStatus *pool[HASH_SIZE];
  long unsigned int pool_size;

} ConfigurationDictionary;

/*
 * When the  dictionary is queried the  result is simply  the index of
 * the bucket in which the record  hashes, and a pointer to a matching
 * occurrence  in the  dictionary, if  there  is any.  A null  pointer
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
  d->pool_size = HASH_SIZE - HASH_WASTE_RANGE + r;

}

DictionaryQueryResult dictionary_find(ConfigurationDictionary* d,PebblingStatus *s) {
  long unsigned int h;
  PebblingStatus *t;
  DictionaryQueryResult res={ 0UL, NULL };

  ASSERT_NOTNULL(d);
  ASSERT_NOTNULL(s);

  /* Compute the hash and then find the position in the array */
  h= ( s->white_pebbled >> ROTATE_RIGHT_AMOUNT )
    | ( s->white_pebbled << ( sizeof(BitTuple)*CHAR_BIT - ROTATE_RIGHT_AMOUNT) )
    | s->black_pebbled;
  h = h % d->pool_size;

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
 *  added to the dictionary, otherwise an old one is updated.  The
 *  function returns true if there has been a change in the
 *  dictionary.
 */
int dictionary_update(ConfigurationDictionary* d,PebblingStatus *s) {
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
    n->last_changed_vertex=s->last_changed_vertex;

    n->next=d->pool[res.bucket_idx];
    d->pool[res.bucket_idx]=n;

    return 1;

  } else if (res.occurrence->pebble_cost > s->pebble_cost) {
    /* The configuration occur, so we update the old record if there's the need. */
    res.occurrence->pebble_cost=s->pebble_cost;
    res.occurrence->previous_configuration=s->previous_configuration;
    res.occurrence->last_changed_vertex=s->last_changed_vertex;

    return 1; /* Changes in the cost */
  } else
    return 0; /* No changes */
}

/* A configuration is sane only if linked to configurations in the
   appropriate dictionary */
void assert_status_sanity(DAG *g,PebblingStatus *s,ConfigurationDictionary *dict) {
  DictionaryQueryResult q;
  int i;
  int pebbled;
  int covered=0;

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
  /* Check that the number of indicated pebbles is correct. */
  pebbled=0;
  BitTuple bmask=0x1;
  for(i=0;i<g->size;i++) {
    if (s->white_pebbled & bmask ) pebbled++;
    if (s->black_pebbled & bmask ) pebbled++;
    bmask = bmask << 1;
  }
  if (s->pebbles != pebbled) {
    fprintf(stderr,"ERROR: A configuration has wrong pebble count.");
    exit(-1);
  }
  /* Check that the active pebbles are all correct (i.e. a
     vertex is active if and only if both the predecessors are
     pebbled. */
  for (i=0;i<g->size;i++) {
    covered=1;
    for(int j=0;j<g->indegree[i];j++) {
      /* Is the predecessor pebbled? */
      if ( (s->white_pebbled | s->black_pebbled) & ( 0x1 << g->in[i][j] )) continue;
      else { covered=0; break; }
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


#ifdef FALSE
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
#endif
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

         -- DAG: the graph to pebble (with few vertices and a single sink).
         -- upper_bound: the maximum number of pebbles in the configuration,
                         if such number is not sufficient, the the computation will fail gracefully
                         without finding the pebbling
*/
/* {{{ */ void pebbling_strategy(DAG *g,unsigned int upper_bound) {

  /* Dictionary data tructure */
  ConfigurationDictionary dict;
  /* Initial and Temporaty Status */
  PebblingStatus initial_status={0UL,0UL,0UL,0,0,NULL,0,NULL};

  /* PROLOGUE ----------------------------------- */
  if (g->size > MAX_VERTICES) {
    fprintf(stderr,
            "Error in search procedure: the graph is too "
            "big for the optimized data structures.");
    exit(-1);
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

  /* Pick a pebbling status from the queue, produce the followers, and
     put in the queue the ones that haven't been analized yet */




  /* EPILOGUE --------------------------------- */
  /* free memory, free Mandela! */
}
/* }}} */



/*
 *  The example test program creates two pyramid graphs and produces
 *  the OR-product graph of them.  Then it prints the DOT
 *  representation of such graphs.
 */

int main(int argc, char *argv[])
{
  DAG *A,*B,*C;
  A=create_piramid_graph(3);
  B=create_piramid_graph(4);


  C=product_graph(A,B);
  print_dot_graph(A,"A",NULL,NULL,NULL);
  print_dot_graph(B,"B",NULL,NULL,NULL);
  print_dot_graph(C,"C",NULL,NULL,NULL);

  dispose_graph(A);
  dispose_graph(B);
  dispose_graph(C);

  exit(0);
}



