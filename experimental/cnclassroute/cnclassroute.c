#include "cnclassroute.h"

void build_node_classroute(rect_t *world, rect_t *comm, coord_t *me, classroute_t *cr) {
	int d, dims = world->ll.dims;
	classroute_t cr0 = {0};
	int m;

	// assert(ll->dims == ur->dims == me->dims);
	for (d = 0; d < dims; ++d) {
		m = (world->ur.coords[d] - world->ll.coords[d] + 1) / 2;
		m += world->ll.coords[d];
		if (me->coords[d] < m) {
			if (me->coords[d] > comm->ll.coords[d]) {
				cr0.dn_tree |= CR_LINK(d,CR_SIGN_NEG);
			}
			if (me->coords[d] < comm->ur.coords[d]) {
				cr0.up_tree |= CR_LINK(d,CR_SIGN_POS);
				break;
			}
		} else if (me->coords[d] == m) {
			if (me->coords[d] > comm->ll.coords[d]) {
				cr0.dn_tree |= CR_LINK(d,CR_SIGN_NEG);
			}
			if (me->coords[d] < comm->ur.coords[d]) {
				cr0.dn_tree |= CR_LINK(d,CR_SIGN_POS);
			}
		} else if (me->coords[d] > m) {
			if (me->coords[d] < comm->ur.coords[d]) {
				cr0.dn_tree |= CR_LINK(d,CR_SIGN_POS);
			}
			if (me->coords[d] > comm->ll.coords[d]) {
				cr0.up_tree |= CR_LINK(d,CR_SIGN_NEG);
				break;
			}
		}
	}
	*cr = cr0;
}
