/**
 * \file experimental/bgq/cnclassroute/cnclassroute.c
 * \brief ???
 */
#include <stdio.h>
#include "cnclassroute.h"

void build_node_classroute(CR_RECT_T *world, CR_COORD_T *worldroot, CR_COORD_T *me,
					CR_RECT_T *comm, int dim0, ClassRoute_t *cr) {
	int d, dim;
	ClassRoute_t cr0 = {0};

	for (dim = 0; dim < CR_NUM_DIMS; ++dim) {
		d = dim0 + dim;
		if (d >= CR_NUM_DIMS) d -= CR_NUM_DIMS;
		if (CR_COORD_DIM(me,d) <= CR_COORD_DIM(worldroot,d)) {
			if (CR_COORD_DIM(me,d) > CR_COORD_DIM(CR_RECT_LL(comm),d)) {
				cr0.input |= CR_LINK(d,CR_SIGN_NEG);
			}
		}
		if (CR_COORD_DIM(me,d) >= CR_COORD_DIM(worldroot,d)) {
			if (CR_COORD_DIM(me,d) < CR_COORD_DIM(CR_RECT_UR(comm),d)) {
				cr0.input |= CR_LINK(d,CR_SIGN_POS);
			}
		}
		if (CR_COORD_DIM(me,d) < CR_COORD_DIM(worldroot,d)) {
			if (CR_COORD_DIM(me,d) < CR_COORD_DIM(CR_RECT_UR(comm),d)) {
				cr0.output |= CR_LINK(d,CR_SIGN_POS);
				break;
			}
		} else if (CR_COORD_DIM(me,d) > CR_COORD_DIM(worldroot,d)) {
			if (CR_COORD_DIM(me,d) > CR_COORD_DIM(CR_RECT_LL(comm),d)) {
				cr0.output |= CR_LINK(d,CR_SIGN_NEG);
				break;
			}
		}
	}
	*cr = cr0;
}

/**
 * \brief Determine if two coordinates are the same (in all dimensions)
 *
 * \param[in] c0	First coordinate to compare
 * \param[in] c1	Second coordinate to compare
 * \retrun	"true" (non-zero) if coordinates are the same
 */
static int eq_coords(CR_COORD_T *c0, CR_COORD_T *c1) {
	int d;
	for (d = 0; d < CR_NUM_DIMS; ++d) {
		if (CR_COORD_DIM(c0,d) != CR_COORD_DIM(c1,d)) return 0;
	}
	return 1;
}

/**
 * \brief Determine if any down-tree nodes are in communicator
 *
 * Recursive routine. Traverses down-tree and if a link has no local contributors
 * then the link is pruned.
 *
 * Must build each classroute, so needs enough information to do that. This also
 * builds the top-level classroute (for the actual node in question). All down-tree
 * classroutes are temporary - discarded before return.
 *
 * \param[in] world	The entire partition rectangle
 * \param[in] worldroot	The root coordinates
 * \param[in] me	Coordinates of current node
 * \param[in] comm	The sub-communicator rectangle
 * \param[in] exclude	Coordinates of nodes NOT in communicator
 * \param[in] nexclude	Number of coordinates in 'exclude'
 * \param[in] dim0	Primary dimension for classroute algorithm
 * \param[in,out] cr	Classroute (top level only)
 * \param[in] level	Recursion level
 * \return	number of local contributors beneath current node
 */
static int find_local_contrib(CR_RECT_T *world, CR_COORD_T *worldroot, CR_COORD_T *me,
			CR_RECT_T *comm, CR_COORD_T *exlcude, int nexclude,
			int dim0, ClassRoute_t *cr, int level) {
	uint32_t l, m;
	int n, s, t;
	CR_COORD_T c0, c1;
	ClassRoute_t cr1;
	static int signs[] = {
		[CR_SIGN_POS] = 1,
		[CR_SIGN_NEG] = -1,
	};
	if (level) {
		for (n = 0; n < nexclude; ++n) {
			if (eq_coords(&exlcude[n], me)) {
				/* NOT local contributor... */
				break;
			}
		}
		if (n >= nexclude) {
			/* we ARE a local contributor, so no need to go further */
			return 1;
		}
	}
	build_node_classroute(world, worldroot, me, comm, dim0, &cr1);
	c0 = *me;
	l = cr1.input;
	t = 0;
	for (n = 0; n < CR_NUM_DIMS; ++n) {
		for (s = 0; s < 2; ++s) {
			m = CR_LINK(n, s);
			if (l & m) {
				c1 = c0;
				CR_COORD_DIM(&c1,n) += signs[s];
				/* assume no errors! */
				int f = find_local_contrib(world, worldroot, &c1,
						comm, exlcude, nexclude,
						dim0, NULL, level + 1);
				if (!f) cr1.input &= ~m;
				t += f;
			}
		}
	}
	if (cr) *cr = cr1;
	return t;
}

void build_node_classroute_sparse(CR_RECT_T *world, CR_COORD_T *worldroot, CR_COORD_T *me,
				CR_RECT_T *comm, CR_COORD_T *exlcude, int nexclude,
				int dim0, ClassRoute_t *cr) {
	/*
	 * traverse down-tree and determine if any local contributors exist.
	 * unfortunately, we don't have any classroutes but our own, so we must
	 * build each down-tree node's classroute on the fly.
	 */
	find_local_contrib(world, worldroot, me, comm, exlcude, nexclude, dim0, cr, 0);
}

void pick_world_root(CR_RECT_T *world, CR_COORD_T *worldroot, int *pri_dim) {
	int x;
	CR_COORD_T root;
	for (x = 0; x < CR_NUM_DIMS; ++x) {
		int size = CR_COORD_DIM(CR_RECT_UR(world),x) - CR_COORD_DIM(CR_RECT_LL(world),x) + 1;
		CR_COORD_DIM(&root,x) = CR_COORD_DIM(CR_RECT_LL(world),x) + size / 2;
	}
	*pri_dim = 0;
	*worldroot = root;
}

void pick_world_root_pair(CR_RECT_T *world, CR_COORD_T *worldroot1, CR_COORD_T *worldroot2,
								int *pri_dim) {
	int x;
	int min = 99999999;
	int min_dim = -1;
	CR_COORD_T root;
	for (x = 0; x < CR_NUM_DIMS; ++x) {
		int size = CR_COORD_DIM(CR_RECT_UR(world),x) - CR_COORD_DIM(CR_RECT_LL(world),x) + 1;
		CR_COORD_DIM(&root,x) = CR_COORD_DIM(CR_RECT_LL(world),x) + size / 2;
		if (size > 1 && size <= min) {
			min = size;
			min_dim = x;
		}
	}
	// assert(min_dim != -1);
	*pri_dim = min_dim;
	*worldroot1 = root;
	*worldroot2 = root;
	CR_COORD_DIM(worldroot1,min_dim) = CR_COORD_DIM(CR_RECT_LL(world),min_dim);
	CR_COORD_DIM(worldroot2,min_dim) = CR_COORD_DIM(CR_RECT_UR(world),min_dim);
}

/**
 * \brief Compare two rectangles
 *
 * \param[in] rect1	First rectangle to compare
 * \param[in] rect2	Second rectangle to compare
 * \return	0 = identical, 1 = disjoint, -1 = overlapping
 */
static int rect_compare(CR_RECT_T *rect1, CR_RECT_T *rect2) {
	int d, n = 0, o = 0;
	unsigned r1ll, r2ll, r1ur, r2ur;
	for (d = 0; d < CR_NUM_DIMS; ++d) {
		r1ll = CR_COORD_DIM(CR_RECT_LL(rect1),d);
		r2ll = CR_COORD_DIM(CR_RECT_LL(rect2),d);
		r1ur = CR_COORD_DIM(CR_RECT_UR(rect1),d);
		r2ur = CR_COORD_DIM(CR_RECT_UR(rect2),d);
		if (r1ll == r2ll && r1ur == r2ur) {
			++n;	/* identical */
			++o;	/* also counts as overlapping */
			continue;
		}
		if (r1ur >= r2ll && r1ll <= r2ur) {
			++o;	/* overlapping */
			continue;
		}
	}
	if (n == CR_NUM_DIMS) return 0;		/* identical rectangles */
	if (o == CR_NUM_DIMS) return -1;	/* overlapping */
	return 1; /* disjoint rectangles */
}

uint32_t get_classroute_ids(int vc, CR_RECT_T *subcomm, void **env) {
	int x;
	struct cr_allocation *crp;
	uint32_t free = 0;
	struct cr_allocation **cr_alloc = *env;

	if (!cr_alloc) {
		cr_alloc = malloc(BGQ_COLLECTIVE_MAX_CLASSROUTES * sizeof(struct cr_allocation *));
		memset(cr_alloc, 0, BGQ_COLLECTIVE_MAX_CLASSROUTES * sizeof(struct cr_allocation *));
		*env = cr_alloc;
	}
	/*
	 * assertion: if we find an identical rectangle that already has a classroute,
	 * then EVERYONE had better see the same thing. We return only one possible
	 * classroute in this case.
	 *
	 * \todo Consider virtual channel in search
	 */
	for (x = 0; x < BGQ_COLLECTIVE_MAX_CLASSROUTES; ++x) {
		if (cr_alloc[x] == NULL) {
			free |= (1 << x);
			continue;
		}
		/*
		 * since all rectangles in a classroute must be identical,
		 * no need to check them all, just the first.
		 */
		crp = cr_alloc[x];
		if (GET_CR_ROUTE_VC(&crp->classroute) != vc) {
			continue;
		}
		int y = rect_compare(&crp->rect, subcomm);
		if (y == 0) {
			/*
			 * identical rectangle already in use,
			 * return it's classroute
			 */
			return (1 << x);
		}
	}
	/*
	 * no matching rectangle found,
	 * return all unused classroutes
	 */
	return free;
}

int set_classroute_id(int id, int vc, CR_RECT_T *subcomm, void **env) {
	struct cr_allocation **cr_alloc = *env;
	struct cr_allocation *new, *crp;

	new = malloc(sizeof(struct cr_allocation));
	new->rect = *subcomm;
	new->classroute.id = id;
	new->classroute.output = 0;
	new->classroute.input = 0;
	SET_CR_ROUTE_VC(&new->classroute,vc);
	crp = cr_alloc[id];
	/* all are the same, just insert at front */
	new->cr_peer = crp;
	cr_alloc[id] = new;
	return (crp == NULL);
}

int release_classroute_id(int id, int vc, CR_RECT_T *subcomm, void **env) {
	struct cr_allocation **cr_alloc = *env;
	struct cr_allocation *crp;

	// assert(cr_alloc[id] != NULL);
	crp = cr_alloc[id];
	cr_alloc[id] = crp->cr_peer;
	free(crp);
	return (cr_alloc[id] == NULL);
}
