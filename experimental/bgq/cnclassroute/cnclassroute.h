/**
 * \file experimental/bgq/cnclassroute/cnclassroute.h
 * \brief ???
 */
/* this should all be in xmi.h? */

#ifndef __experimental_bgq_cnclassroute_cnclassroute_h__
#define __experimental_bgq_cnclassroute_cnclassroute_h__

#define CR_SIGN_POS	0
#define CR_SIGN_NEG	1

#define CR_AXIS_A       0
#define CR_AXIS_B       1
#define CR_AXIS_C       2
#define CR_AXIS_D       3
#define CR_AXIS_E       4
#define CR_NUM_DIMS     5
#define CR_DIM_NAMES    "ABCDE"

#include "kernel_impl.h"
#include "bqc/classroute.h"

#define GET_CR_ROUTE_VC(crp)	((crp)->input & BGQ_COLL_CLASS_INPUT_VC_MASK)
#define SET_CR_ROUTE_VC(crp,vc)	((crp)->input = ((crp)->input & ~BGQ_COLL_CLASS_INPUT_VC_MASK) | vc)

static const uint16_t cr_links[][2] = {
[CR_AXIS_A][CR_SIGN_POS] = BGQ_COLL_CLASS_LINK_AP,
[CR_AXIS_A][CR_SIGN_NEG] = BGQ_COLL_CLASS_LINK_AM,
[CR_AXIS_B][CR_SIGN_POS] = BGQ_COLL_CLASS_LINK_BP,
[CR_AXIS_B][CR_SIGN_NEG] = BGQ_COLL_CLASS_LINK_BM,
[CR_AXIS_C][CR_SIGN_POS] = BGQ_COLL_CLASS_LINK_CP,
[CR_AXIS_C][CR_SIGN_NEG] = BGQ_COLL_CLASS_LINK_CM,
[CR_AXIS_D][CR_SIGN_POS] = BGQ_COLL_CLASS_LINK_DP,
[CR_AXIS_D][CR_SIGN_NEG] = BGQ_COLL_CLASS_LINK_DM,
[CR_AXIS_E][CR_SIGN_POS] = BGQ_COLL_CLASS_LINK_EP,
[CR_AXIS_E][CR_SIGN_NEG] = BGQ_COLL_CLASS_LINK_EM,
};
#define CR_LINK(n,s)    cr_links[n][s]

/**
 *  this header must define:
 *
 *  CR_LINK(n,s)	Create bit-mask for n-th link (dim) in 's' direction (CR_SIGN_POS...)
 *
 *  CR_COORD_T			datatype for a coordinate (CR_NUM_DIMS)
 *  CR_COORD_DIM(coordp,dim)	accessor for coordp->[dim] (must work for both set/get)
 *
 *  CR_RECT_T		datatype for a rectangle (two coordinates)
 *  CR_RECT_LL(rectp)	accessor for rectp->lower-left-coord (get/put/CR_COORD_DIM())
 *  CR_RECT_UR(rectp)	accessor for rectp->upper-right-coord (get/put/CR_COORD_DIM())
 */
#include "xmi/cnclassroute.h" // replace with $(TARGET)/cnclassroute.h ...

/**
 * structure for managing "peer aware" classroute ID allocation
 */
struct cr_allocation {
	CR_RECT_T rect;
	ClassRoute_t classroute;
	struct cr_allocation *cr_peer;	// same classroute
};

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
				CR_RECT_T *comm, int dim0, ClassRoute_t *cr);

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
				int dim0, ClassRoute_t *cr);

/**
 * Pick a root for 'world' rectangle
 *
 * Used for sub-comms on exclusive virtual channel.
 *
 * \param[in] world		Entire partition rectangle (COMM_WORLD)
 * \param[out] worldroot	Coordinates of root node in COMM_WORLD
 * \param[out] pri_dim		Primary dimension for building classroute
 */
extern void pick_world_root(CR_RECT_T *world, CR_COORD_T *worldroot, int *pri_dim);

/**
 * Pick a pair of roots for 'world' rectangle that will not conflict
 *
 * Used for comm-world with shared virtual channel (system and user comm-world)
 *
 * \param[in] world		Entire partition rectangle (COMM_WORLD)
 * \param[out] worldroot1	Coordinates of 1st root node in COMM_WORLD
 * \param[out] worldroot2	Coordinates of 2nd root node in COMM_WORLD
 * \param[out] pri_dim		Primary dimension for building classroute
 */
extern void pick_world_root_pair(CR_RECT_T *world, CR_COORD_T *worldroot1, CR_COORD_T *worldroot2,
								int *pri_dim);

/**
 * \brief Find all possible classroutes for given rectangle
 *
 * Will re-use classroute ID for identical rectangles, if found.
 * Typically, this bitmap is ANDed with all other participants,
 * and the least-significant "1" bit indicates the first classroute
 * that all agree upon.
 *
 * \param[in] vc        Virtual channel to be used by classroute
 * \param[in] subcomm   The rectangle for classroute to be created
 * \return      bitmap of possible classroutes
 */
uint32_t get_classroute_ids(int vc, CR_RECT_T *subcomm, void **env);

/**
 * \brief Allocate a classroute for rectangle
 *
 * Will re-use classroute ID for identical rectangles, or where a
 * classroute has no overlapping rectangles in use at the moment.
 *
 * \param[in] id        Classroute ID
 * \param[in] vc        Virtual channel used by classroute
 * \param[in] subcomm   The rectangle for classroute to be created
 * \return      0 = re-using existing classroute, 1 = classroute DCRs must be set
 */
int set_classroute_id(int id, int vc, CR_RECT_T *subcomm, void **env);

/**
 * \brief Release rectangle from use as a classroute
 *
 * Assumes that 'subcomm' previously succeeded when passed to get_classroute_id().
 *
 * \param[in] id        Classroute ID
 * \param[in] vc        Virtual channel used by classroute
 * \param[in] subcomm   The rectangle for classroute to be removed
 * \return 0 = classroute still in use, 1 = classroute free (DCRs must be cleared)
 */
int release_classroute_id(int id, int vc, CR_RECT_T *subcomm, void **env);

#endif // __experimental_bgq_cnclassroute_cnclassroute_h__
