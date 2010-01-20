/**
 * \file experimental/cnclassroute/cnclassroute.c
 * \brief ???
 */
#include <stdio.h>
#include "cnclassroute.h"

void build_node_classroute(rect_t *world, coord_t *worldroot, coord_t *me,
					rect_t *comm, int dim0, classroute_t *cr) {
	// assert(ll->dims == ur->dims == me->dims);
	int d, dim, dims = world->ll.dims;
	classroute_t cr0 = {0};

	for (dim = 0; dim < dims; ++dim) {
		d = dim0 + dim;
		if (d >= dims) d -= dims;
		if (me->coords[d] <= worldroot->coords[d]) {
			if (me->coords[d] > comm->ll.coords[d]) {
				cr0.dn_tree |= CR_LINK(d,CR_SIGN_NEG);
			}
		}
		if (me->coords[d] >= worldroot->coords[d]) {
			if (me->coords[d] < comm->ur.coords[d]) {
				cr0.dn_tree |= CR_LINK(d,CR_SIGN_POS);
			}
		}
		if (me->coords[d] < worldroot->coords[d]) {
			if (me->coords[d] < comm->ur.coords[d]) {
				cr0.up_tree |= CR_LINK(d,CR_SIGN_POS);
				break;
			}
		} else if (me->coords[d] > worldroot->coords[d]) {
			if (me->coords[d] > comm->ll.coords[d]) {
				cr0.up_tree |= CR_LINK(d,CR_SIGN_NEG);
				break;
			}
		}
	}
	*cr = cr0;
}

#ifdef SUPPORT_SPARSE_RECTANGLE

static int eq_coords(coord_t *c0, coord_t *c1) {
	int d;
	for (d = 0; d < c0->dims; ++d) {
		if (c0->coords[d] != c1->coords[d]) return 0;
	}
	return 1;
}

/* recursive routine */
static int find_local_contrib(rect_t *world, coord_t *worldroot, coord_t *me,
			rect_t *comm, coord_t *exlcude, int nexclude,
			int dim0, classroute_t *cr, int level) {
#warning Untested code
	uint32_t l, m;
	int n, s, t;
	coord_t c0, c1;
	classroute_t cr1;
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
	l = cr1.dn_tree;
	t = 0;
	for (n = 0; n < world->ll.dims; ++n) {
		for (s = 0; s < 2; ++s) {
			m = CR_LINK(n, s);
			if (l & m) {
				c1 = c0;
				c1.coords[n] += signs[s];
				/* assume no errors! */
				int f = find_local_contrib(world, worldroot, &c1,
						comm, exlcude, nexclude,
						dim0, NULL, level + 1);
				if (!f) cr1.dn_tree &= ~m;
				t += f;
			}
		}
	}
	if (cr) *cr = cr1;
	return t;
}

void build_node_classroute_sparse(rect_t *world, coord_t *worldroot, coord_t *me,
				rect_t *comm, coord_t *exlcude, int nexclude,
				int dim0, classroute_t *cr) {
	/*
	 * traverse down-tree and determine if any local contributors exist.
	 * unfortunately, we don't have any classroutes but our own, so we must
	 * build each down-tree node's classroute on the fly.
	 */
	find_local_contrib(world, worldroot, me, comm, exlcude, nexclude, dim0, cr, 0);
}

#endif /* SUPPORT_SPARSE_RECTANGLE */

void pick_world_root_pair(rect_t *world, coord_t *worldroot1, coord_t *worldroot2,
								int *pri_dim) {
	int x;
	int min = 99999999;
	int min_dim = -1;
	coord_t root;
	for (x = 0; x < world->ll.dims; ++x) {
		int size = world->ur.coords[x] - world->ll.coords[x] + 1;
		root.coords[x] = world->ll.coords[x] + size / 2;
		if (size > 1 && size <= min) {
			min = size;
			min_dim = x;
		}
	}
	root.dims = world->ll.dims;
	*pri_dim = min_dim;
	*worldroot1 = root;
	worldroot1->coords[min_dim] = world->ll.coords[min_dim];
	*worldroot2 = root;
	worldroot2->coords[min_dim] = world->ur.coords[min_dim];
}
