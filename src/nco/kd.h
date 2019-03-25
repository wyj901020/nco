/*
 * Headers for k-d tree geometric data structure
 */

#ifndef KD_HEADER
#define KD_HEADER

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <assert.h>
#include <limits.h>

#include <math.h>

#include <sys/param.h>

#include "nco.h"
#include "nco_mmr.h" /* Memory management */
#include "nco_omp.h" /* OpenMP utilities */

#define KD_LEFT		0
#define KD_BOTTOM	1
#define KD_RIGHT	2
#define KD_TOP		3
#define KD_BOX_MAX	4

/* Return values */
#define KD_OK		1
#define KD_NOMORE	2

#define KD_NOTIMPL	-3
#define KD_NOTFOUND	-4 

/* Fatal Faults */
#define KDF_M		0	/* Memory fault    */
#define KDF_ZEROID	1	/* Insert zero     */
#define KDF_MD		2	/* Bad median      */
#define KDF_F		3	/* Father fault    */
#define KDF_DUPL	4	/* Duplicate entry */
#define KDF_UNKNOWN	99	/* Unknown error   */


#define ERR_BUF_SIZE	4096
#define STACK_SIZE	100

#define KD_LOSON	0
#define KD_HISON	1


#define KD_INIT_STACK	15	/* Initial size of stack                */

#define	KD_THIS_ONE	-1	/* Indicates going through this element */
#define KD_DONE		2	/* Entirely done searching this element */

#define PATH_INIT	50
#define PATH_INCR	10

#ifndef DBL_MAX
# define DBL_MAX         (1.79769313486232e+30) 
#endif


/* The minmax kd_box limits of a wrapped polgon are split in two and both are inserted into the kd tree
   They both have the same poly_sct pointer as elem->item.  We need a minor change in find_item() to allow
   this duplicatation of item */
#define ALLOW_DUPLICATE_ITEM 1


/* define macros */
#define KD_SIZE(val)	(val)->size

#define BOXINTERSECT(b1, b2) \
  (((b1)[KD_RIGHT] >= (b2)[KD_LEFT]) && \
   ((b2)[KD_RIGHT] >= (b1)[KD_LEFT]) && \
   ((b1)[KD_TOP] >= (b2)[KD_BOTTOM]) && \
   ((b2)[KD_TOP] >= (b1)[KD_BOTTOM]))

/*
 * Destructively replaces the next item of list1 with list2.
 * Returns modified list1.
 */
#define RCDR(list1, list2) \
  (   (list1) ? ((  (list1)->sons[0] = (list2)), (list1)) : (list2)  )


/* maybe we  have sys/param maybe we dont */
#ifndef MIN
#define MIN(a, b)	((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#endif

#define KDABS(a)		((a) < 0 ? -(a) : (a))

#define KD_GROWSIZE(s)	10	/* Linear expansion  */

/* all the typedefs in one place */ 

//extern char *kd_pkg_name;	/* For error handling */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct kd_dummy_defn {
    int dummy;
} kd_dummy;

typedef kd_dummy *kd_tree;
typedef kd_dummy *kd_gen;


typedef double kd_box[4];
typedef double *kd_box_r;

typedef int kd_status;
typedef char* kd_generic;



typedef struct KDElem_defn {
    kd_generic item;		/* Actual item at this node */
    kd_box size;		/* Size of item             */
    double lo_min_bound;		/* Lower minimum boundary   */
    double hi_max_bound;		/* High maximum boundary    */
    double other_bound;		/* Discriminator dependent  */
    struct KDElem_defn *sons[2];/* Children                 */
} KDElem;

typedef struct KDTree_defn {
    KDElem *tree;		/* K-d tree itself      */
    int item_count;		/* Number of nodes in tree */
    int dead_count;		/* Number of dead nodes */
    kd_box extent;      /* extents for the entire tree */
    int items_balanced; /* how many where in the tree when built */
} KDTree;

typedef struct kd_save {
    short disc;			/* Discriminator             */
    short state;		/* Current state (see above) */
    KDElem *item;		/* Element saved             */
    kd_box Bp;          /* for nearest neighbor, a saved bounds info */
    kd_box Bn;          /* for nearest neighbor, a saved bounds info */
} KDSave;

typedef struct kd_state {
    kd_box extent;		/* Search area 		     */
    short stack_size;		/* Allocated size of stack   */
    short top_index;		/* Top of the stack          */
    KDSave *stk;		/* Stack of active states    */
} KDState;


typedef struct kd_priority
{
	double dist;
	kd_generic elem;
} kd_priority;

typedef struct KDPpriority
{
	double dist;
	KDElem *elem;
} KDPriority;


void add_priority(int m, KDPriority *P, kd_box Xq, KDElem *elem);
int add_priority_intersect(int m, KDPriority *P, kd_box Xq, KDElem *elem);
int bounds_intersect(kd_box Xq, kd_box Bp, kd_box Bn);
int bounds_overlap_ball(kd_box Xq, kd_box Bp, kd_box Bn, int m, KDPriority *list);
void bounds_update(KDElem *elem, int disc, kd_box size);
KDElem *build_node(KDElem *items, int num, kd_box extent, int disc, int level, int max_level, KDElem **spares, int *treecount, double mean);
void collect_nodes(KDTree*, KDElem *, KDElem **, kd_box, long *, double *);
double coord_dist(double x, double y);
void del_elem(KDElem *elem, void (*delfunc)(kd_generic item));
kd_status del_element(KDTree *tree, KDElem *elem, int spot);
KDElem *find_item(KDElem *elem, int disc, kd_generic item, kd_box size, int search_p, KDElem *items_elem);
int get_min_max(KDElem *list, int disc, double *b_min, double *b_max);
KDElem *kd_new_node(kd_generic item, kd_box size, double lomin, double himax, double other, KDElem *loson, KDElem *hison);
char *kd_fault(int t);
KDElem *load_items(int (*itemfunc)(kd_generic arg, kd_generic *val, kd_box size), kd_generic arg, kd_box extent, int *length, double *mean);
int nodecmp(KDElem *a, KDElem *b, int disc);
void pr_tree(KDElem *elem, int disc, int depth);
void resolve(register KDElem **lo, register KDElem **eq, register KDElem **hi, int disc, double *lomean, double *himean, long *locount, long *hicount);
void sel_k(KDElem *items, double k, int disc, KDElem **lo, KDElem **eq, KDElem **hi, double *lomean, double *himean, long *locount, long *hicount);
void unload_items(KDTree*, KDElem **, kd_box, long *, double *);

void errRaise(const char *pkg, int code, const char *format);
void NEW_PATH(KDElem *elem);



/* Returns a textual description of a k-d error */


double KDdist(kd_box Xq, KDElem *elem);


void kd_badness(KDTree *realTree);
KDTree* kd_build(int (*itemfunc)(), kd_generic );
void kd_tree_badness_level(KDElem *elem, int level);
int kd_count (KDTree* tree);
KDTree* kd_create(void);
kd_status kd_delete(KDTree* , kd_generic , kd_box );
void kd_delete_stats(int *tries,int *levs);
void kd_destroy(KDTree *this_one, void (*delfunc)(kd_generic item));
char *kd_err_string(void);
int kd_finish (kd_gen);
void kd_insert(KDTree* , kd_generic , kd_box, kd_generic );
int kd_neighbour(KDElem *node, kd_box Xq, int m, KDPriority *list, kd_box Bp, kd_box Bn);
int kd_neighbour_intersect(KDElem *node, kd_box Xq, int m, KDPriority *list, kd_box Bp, kd_box Bn);
int kd_neighbour_intersect2(KDElem *node, int disc, kd_box Xq, int m, KDPriority *list);
int kd_neighbour_intersect2(KDElem *node, int disc, kd_box Xq, int m, KDPriority *list);
int kd_neighbour_intersect3(KDElem *node, int disc, kd_box Xq, int m, KDPriority *list, int stateH, int stateV );
int kd_nearest (KDTree* tree, double x, double y, int m, KDPriority **alist);
int kd_nearest_intersect(KDTree* realTree, kd_box Xq, int m, KDPriority *alist);
int kd_nearest_intersect_wrp(KDTree* realTree, kd_box Xq,kd_box Xr, int m, KDPriority *list);
kd_status kd_next (kd_gen , kd_generic *, kd_box);
void kd_print (KDTree*);
void kd_print_path(void);
void kd_print_nearest (KDTree* tree, double x, double y, int m);
void kd_push(KDState *gen, KDElem *elem, short disc);
void kd_pushb(KDState *gen, KDElem* elem, short dk, kd_box Bxn, kd_box Bxp);
kd_status kd_really_delete (KDTree* theTree, kd_generic data, kd_box old_size, int *num_tries, int *num_del);
KDTree* kd_rebuild ( KDTree* );
int kd_set_build_depth(int depth);
kd_gen kd_start (KDTree* tree, kd_box size);
kd_status kd_is_member(KDTree* , kd_generic , kd_box );
void kd_tree_badness(KDTree *tree, double *fact1, double *fact2, double *fact3, int *levs);

#ifdef __cplusplus
} /* end extern "C" */
#endif /* __cplusplus */
  

#endif /* KD_HEADER */