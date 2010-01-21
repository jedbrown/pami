/**
 * \file experimental/bgq/cnclassroute/cnclassroute.h
 * \brief ???
 */
/* this should all be in xmi.h? */

#ifndef __experimental_bgq_cnclassroute_cnclassroute_h__
#define __experimental_bgq_cnclassroute_cnclassroute_h__

#define CR_SIGN_POS	0
#define CR_SIGN_NEG	1

/// this header must define:
///
/// CR_LINK(n,s)	Create bit-mask for n-th link (dim) in 's' direction (CR_SIGN_POS...)
///
/// CR_NUM_DIMS		Number of dimensions in network
/// CR_DIM_NAMES	String of single-char dimension IDs (names) - e.g. "XYZ" or "ABCDE"
///
/// CR_COORD_T			datatype for a coordinate (CR_NUM_DIMS)
/// CR_COORD_DIM(coordp,dim)	accessor for coordp->[dim] (must work for both set/get)
///
/// CR_RECT_T		datatype for a rectangle (two coordinates)
/// CR_RECT_LL(rectp)	accessor for rectp->lower-left-coord (get/put/CR_COORD_DIM())
/// CR_RECT_UR(rectp)	accessor for rectp->upper-right-coord (get/put/CR_COORD_DIM())
///
/// CR_ROUTE_T		datatype for classroute 
/// CR_ROUTE_UP(crp)	accessor for up-link bits in classroute
/// CR_ROUTE_DOWN(crp)	accessor for down-link bits in classroute
/// CR_ROUTE_NETMASK	mask of bits in links that are network connections
///
#include "xmi/cnclassroute.h" // replace with $(TARGET)/cnclassroute.h ...

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
extern void build_node_classroute(CR_RECT_T *world, CR_COORD_T *worldroot, CR_COORD_T *me,
				CR_RECT_T *comm, int dim0, CR_ROUTE_T *cr);

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
extern void build_node_classroute_sparse(CR_RECT_T *world, CR_COORD_T *worldroot, CR_COORD_T *me,
				CR_RECT_T *comm, CR_COORD_T *exlcude, int nexclude,
				int dim0, CR_ROUTE_T *cr);

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
extern void pick_world_root_pair(CR_RECT_T *world, CR_COORD_T *worldroot1, CR_COORD_T *worldroot2,
								int *pri_dim);

#endif // __experimental_bgq_cnclassroute_cnclassroute_h__
