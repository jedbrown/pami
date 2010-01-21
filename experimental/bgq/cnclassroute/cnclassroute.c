/**
 * \file experimental/bgq/cnclassroute/cnclassroute.c
 * \brief ???
 */
#include <stdio.h>
#include "cnclassroute.h"

void build_node_classroute(CR_RECT_T *world, CR_COORD_T *worldroot, CR_COORD_T *me,
					CR_RECT_T *comm, int dim0, CR_ROUTE_T *cr) {
	int d, dim;
	CR_ROUTE_T cr0 = {0};

	for (dim = 0; dim < CR_NUM_DIMS; ++dim) {
		d = dim0 + dim;
		if (d >= CR_NUM_DIMS) d -= CR_NUM_DIMS;
		if (CR_COORD_DIM(me,d) <= CR_COORD_DIM(worldroot,d)) {
			if (CR_COORD_DIM(me,d) > CR_COORD_DIM(CR_RECT_LL(comm),d)) {
				CR_ROUTE_DOWN(&cr0) |= CR_LINK(d,CR_SIGN_NEG);
			}
		}
		if (CR_COORD_DIM(me,d) >= CR_COORD_DIM(worldroot,d)) {
			if (CR_COORD_DIM(me,d) < CR_COORD_DIM(CR_RECT_UR(comm),d)) {
				CR_ROUTE_DOWN(&cr0) |= CR_LINK(d,CR_SIGN_POS);
			}
		}
		if (CR_COORD_DIM(me,d) < CR_COORD_DIM(worldroot,d)) {
			if (CR_COORD_DIM(me,d) < CR_COORD_DIM(CR_RECT_UR(comm),d)) {
				CR_ROUTE_UP(&cr0) |= CR_LINK(d,CR_SIGN_POS);
				break;
			}
		} else if (CR_COORD_DIM(me,d) > CR_COORD_DIM(worldroot,d)) {
			if (CR_COORD_DIM(me,d) > CR_COORD_DIM(CR_RECT_LL(comm),d)) {
				CR_ROUTE_UP(&cr0) |= CR_LINK(d,CR_SIGN_NEG);
				break;
			}
		}
	}
	*cr = cr0;
}

static int eq_coords(CR_COORD_T *c0, CR_COORD_T *c1) {
	int d;
	for (d = 0; d < CR_NUM_DIMS; ++d) {
		if (CR_COORD_DIM(c0,d) != CR_COORD_DIM(c1,d)) return 0;
	}
	return 1;
}

/* recursive routine */
static int find_local_contrib(CR_RECT_T *world, CR_COORD_T *worldroot, CR_COORD_T *me,
			CR_RECT_T *comm, CR_COORD_T *exlcude, int nexclude,
			int dim0, CR_ROUTE_T *cr, int level) {
	uint32_t l, m;
	int n, s, t;
	CR_COORD_T c0, c1;
	CR_ROUTE_T cr1;
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
	l = CR_ROUTE_DOWN(&cr1);
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
				if (!f) CR_ROUTE_DOWN(&cr1) &= ~m;
				t += f;
			}
		}
	}
	if (cr) *cr = cr1;
	return t;
}

void build_node_classroute_sparse(CR_RECT_T *world, CR_COORD_T *worldroot, CR_COORD_T *me,
				CR_RECT_T *comm, CR_COORD_T *exlcude, int nexclude,
				int dim0, CR_ROUTE_T *cr) {
	/*
	 * traverse down-tree and determine if any local contributors exist.
	 * unfortunately, we don't have any classroutes but our own, so we must
	 * build each down-tree node's classroute on the fly.
	 */
	find_local_contrib(world, worldroot, me, comm, exlcude, nexclude, dim0, cr, 0);
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
	*pri_dim = min_dim;
	*worldroot1 = root;
	CR_COORD_DIM(worldroot1,min_dim) = CR_COORD_DIM(CR_RECT_LL(world),min_dim);
	*worldroot2 = root;
	CR_COORD_DIM(worldroot2,min_dim) = CR_COORD_DIM(CR_RECT_UR(world),min_dim);
}

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

static struct cr_allocation *cr_alloc[BGQ_COLLECTIVE_MAX_CLASSROUTES] = { 0 };

/**
 * \brief Allocate a classroute for rectangle
 *
 * Will re-use classroute ID for identical rectangles, or where a
 * classroute has no overlapping rectangles in use at the moment.
 *
 * \param[in] subcomm	The rectangle for classroute to be created
 *
 * \return 0..N = classroute, -1 = no space
 */
int get_classroute_id(CR_RECT_T *subcomm) {
	int x, z = -1;
	struct cr_allocation *new, *crp;

	for (x = 0; x < BGQ_COLLECTIVE_MAX_CLASSROUTES; ++x) {
		if (cr_alloc[x] == NULL) {
			if (z == -1) z = x;
			continue;
		}
		for (crp = cr_alloc[x]; crp; crp = crp->cr_peer) {
			int y = rect_compare(&crp->rect, subcomm);
			if (y == -1) break; // can't use this classroute...
			if (y == 0 || crp->cr_peer == NULL) {
				new = malloc(sizeof(struct cr_allocation));
				new->rect = *subcomm;
				CR_ROUTE_ID(&new->classroute) = x;
				CR_ROUTE_UP(&new->classroute) = 0;
				CR_ROUTE_DOWN(&new->classroute) = 0;
				// CR_ROUTE_VC(&new->classroute) = vc;
				new->cr_peer = crp->cr_peer;
				crp->cr_peer = new;
				return x;
			}
		}
	}
	// no matching or disjoint peers found... add new entry...
	if (z != -1) {
		new = malloc(sizeof(struct cr_allocation));
		new->rect = *subcomm;
		CR_ROUTE_ID(&new->classroute) = z;
		CR_ROUTE_UP(&new->classroute) = 0;
		CR_ROUTE_DOWN(&new->classroute) = 0;
		// CR_ROUTE_VC(&new->classroute) = vc;
		new->cr_peer = crp->cr_peer;
		cr_alloc[z] = new;
	}
	return z;
}

/**
 * \brief Release rectangle from use as a classroute
 *
 * Assumes that 'subcomm' previously succeeded when passed to get_classroute_id().
 *
 * \param[in] subcomm	The rectangle for classroute to be removed
 *
 * \return 0 = classroute still in use, 1..N = classroute X - 1, -1 = not found
 */
int release_classroute_id(CR_RECT_T *subcomm) {
	int x, z = -1;
	struct cr_allocation *crp, *crq;

	for (x = 0; x < BGQ_COLLECTIVE_MAX_CLASSROUTES; ++x) {
		crq = NULL;
		for (crp = cr_alloc[x]; crp; crp = crp->cr_peer) {
			int y = rect_compare(&crp->rect, subcomm);
			if (y == 0) {
				if (z == -1) {
					z = x;
					// found it - or one just like it...
					// every instance must have the same ID or
					// this doesn't work
					if (crq == NULL) {
						cr_alloc[x] = crp->cr_peer;
					} else {
						crq->cr_peer = crp->cr_peer;
					}
					// keep searching, for duplicates...
				} else {
					// second identical rectangle...
					// classroute still in use...
					return 0;
				}
			}
			crq = crp;
		}
		if (z != -1) {
			// found exactly one... DCRs should be cleared...
			return z + 1; // good indicator???
		}
	}
	return -1; // error - rectangle must have existed...
}
