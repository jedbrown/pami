#include "cnclassroute.h"

/**
 * \brief Compute classroute for node "me"
 *
 * \param[in] world	Entire partition rectangle (COMM_WORLD)
 * \param[in] comm	Rectangle to produce classroute for
 * \param[in] me	Coordinates of my node
 * \param[out] cr	Classroute bitmaps
 *
 * Rectangles are defined by the coordinates of the lower-left and upper-right corners.
 * Note: upper-right is not size. Classroute consists of "up tree" bits and "down tree"
 * bits. Each bit represents a link. The "local contribution" link bit is not set here,
 * it is assumed it would always be "1".
 *
 * The classroute is based on the COMM_WORLD classroute, such that a given node
 * will have basically the same links in use for any classroute, except for nodes
 * on the inner edges of 'comm' (with respect to 'world').
 *
 * The COMM_WORLD classroute is as pictured here (2-D example):
 *
 *   O -> O -> O -> O <- O <- O
 *                  |
 *                  V
 *   O -> O -> O -> O <- O <- O
 *                  |
 *                  V
 *   O -> O -> O -> R <- O <- O
 *                  ^
 *                  |
 *   O -> O -> O -> O <- O <- O
 *                  ^
 *                  |
 *   O -> O -> O -> O <- O <- O
 *
 * Where 'R' is the root node, and dimension '0' is horizontal (and '1' is vertical).
 */
void build_node_classroute(rect_t *world, rect_t *comm, coord_t *me, classroute_t *cr) {
	// assert(ll->dims == ur->dims == me->dims);
	int d, dims = world->ll.dims;
	classroute_t cr0 = {0};
	int m;

	for (d = 0; d < dims; ++d) {
		m = (world->ur.coords[d] - world->ll.coords[d] + 1) / 2;
		m += world->ll.coords[d];
		if (me->coords[d] <= m) {
			if (me->coords[d] > comm->ll.coords[d]) {
				cr0.dn_tree |= CR_LINK(d,CR_SIGN_NEG);
			}
		}
		if (me->coords[d] >= m) {
			if (me->coords[d] < comm->ur.coords[d]) {
				cr0.dn_tree |= CR_LINK(d,CR_SIGN_POS);
			}
		}
		if (me->coords[d] < m) {
			if (me->coords[d] < comm->ur.coords[d]) {
				cr0.up_tree |= CR_LINK(d,CR_SIGN_POS);
				break;
			}
		} else if (me->coords[d] > m) {
			if (me->coords[d] > comm->ll.coords[d]) {
				cr0.up_tree |= CR_LINK(d,CR_SIGN_NEG);
				break;
			}
		}
	}
	*cr = cr0;
}
