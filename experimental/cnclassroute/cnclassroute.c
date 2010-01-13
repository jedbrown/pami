#include "cnclassroute.h"

/**
 * \brief Compute classroute for node "me"
 *
 * \param[in] world	Entire partition rectangle (COMM_WORLD)
 * \param[in] worldroot	Coordinates of root node in COMM_WORLD
 * \param[in] me	Coordinates of my node
 * \param[in] comm	Rectangle to produce classroute for
 * \param[in] dim0	Starting dimension (iterate circular in + direction)
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
 *
 * If a non-rectangle communicator is being used, the circumscribing rectangle
 * must participate in the classroute creation. All nodes in the rectangle are
 * part of the classroute and only nodes in the communicator have local contribution,
 * except for leafs/branches with no local contribution which must be pruned from
 * the resulting classroute. In order for a node to know it must be pruned, or
 * that it is at a prune point, the node may have to create the entire classroute
 * (for all nodes) and evaluate it's position in it - or at least may have to
 * create the classroute down-tree from itself.
 */
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
