/**
 * \file experimental/cnclassroute/cnclassroute.h
 * \brief ???
 */
/* this should all be in xmi.h? */

#ifndef __experimental_cnclassroute_cnclassroute_h__
#define __experimental_cnclassroute_cnclassroute_h__

#include <sys/types.h>
#include <stdint.h>

#include "kernel/Collective.h"
#define XMI_MAX_DIMS 5
#define XMI_DIM_NAMES	"ABCDE"

#define CR_SIGN_POS	0
#define CR_SIGN_NEG	1

static uint16_t cr_links[][2] = {
[0][CR_SIGN_POS] = COLLECTIVE_CLASS_ROUTE_ENABLE_AP,
[0][CR_SIGN_NEG] = COLLECTIVE_CLASS_ROUTE_ENABLE_AM,
[1][CR_SIGN_POS] = COLLECTIVE_CLASS_ROUTE_ENABLE_BP,
[1][CR_SIGN_NEG] = COLLECTIVE_CLASS_ROUTE_ENABLE_BM,
[2][CR_SIGN_POS] = COLLECTIVE_CLASS_ROUTE_ENABLE_CP,
[2][CR_SIGN_NEG] = COLLECTIVE_CLASS_ROUTE_ENABLE_CM,
[3][CR_SIGN_POS] = COLLECTIVE_CLASS_ROUTE_ENABLE_DP,
[3][CR_SIGN_NEG] = COLLECTIVE_CLASS_ROUTE_ENABLE_DM,
[4][CR_SIGN_POS] = COLLECTIVE_CLASS_ROUTE_ENABLE_EP,
[4][CR_SIGN_NEG] = COLLECTIVE_CLASS_ROUTE_ENABLE_EM,
};
#define CR_LINK(n,s)	cr_links[n][s]

typedef struct {
	size_t dims;
	size_t coords[XMI_MAX_DIMS];
} coord_t;

typedef struct {
	coord_t ll;
	coord_t ur;
} rect_t;

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
extern void build_node_classroute(rect_t *world, coord_t *worldroot, coord_t *me,
				rect_t *comm, int dim0, ClassRoute_t *cr);

/**
 * \brief Compute classroute for node 'me' when rectangle is sparse
 *
 * \param[in] world	Entire partition rectangle (COMM_WORLD)
 * \param[in] worldroot	Coordinates of root node in COMM_WORLD
 * \param[in] me	Coordinates of my node
 * \param[in] comm	Rectangle to produce classroute for
 * \param[in] exlcude	Array of coords for nodes excluded from comm
 * \param[in] nexlcude	Number of coords in 'exclude'
 * \param[in] dim0	Starting dimension (iterate circular increasing)
 * \param[out] cr	Classroute bitmaps
 */
#ifdef SUPPORT_SPARSE_RECTANGLE
extern void build_node_classroute_sparse(rect_t *world, coord_t *worldroot, coord_t *me,
				rect_t *comm, coord_t *exlcude, int nexclude,
				int dim0, ClassRoute_t *cr);
#endif /* SUPPORT_SPARSE_RECTANGLE */

/**
 * Pick a pair of roots for 'world' rectangle that will not conflict
 *
 *
 *
 * \param[in] world		Entire partition rectangle (COMM_WORLD)
 * \param[out] worldroot1	Coordinates of 1st root node in COMM_WORLD
 * \param[out] worldroot2	Coordinates of 2nd root node in COMM_WORLD
 * \param[out] pri_dim		Primary dimension for building classroute
 */
extern void pick_world_root_pair(rect_t *world, coord_t *worldroot1, coord_t *worldroot2,
								int *pri_dim);

#endif
