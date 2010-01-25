/**
 * \file experimental/bgq/cnclassroute/xmi/cnclassroute.h
 * \brief ???
 */
/* this should all be in xmi.h? */

#ifndef __experimental_bgq_cnclassroute_xmi_cnclassroute_h__
#define __experimental_bgq_cnclassroute_xmi_cnclassroute_h__

#include <sys/types.h>
#include <stdint.h>

#include "sys/xmi.h"

static const uint16_t cr_links[][2] = {
[CR_AXIS_A][CR_SIGN_POS] = COLLECTIVE_CLASS_ROUTE_ENABLE_AP,
[CR_AXIS_A][CR_SIGN_NEG] = COLLECTIVE_CLASS_ROUTE_ENABLE_AM,
[CR_AXIS_B][CR_SIGN_POS] = COLLECTIVE_CLASS_ROUTE_ENABLE_BP,
[CR_AXIS_B][CR_SIGN_NEG] = COLLECTIVE_CLASS_ROUTE_ENABLE_BM,
[CR_AXIS_C][CR_SIGN_POS] = COLLECTIVE_CLASS_ROUTE_ENABLE_CP,
[CR_AXIS_C][CR_SIGN_NEG] = COLLECTIVE_CLASS_ROUTE_ENABLE_CM,
[CR_AXIS_D][CR_SIGN_POS] = COLLECTIVE_CLASS_ROUTE_ENABLE_DP,
[CR_AXIS_D][CR_SIGN_NEG] = COLLECTIVE_CLASS_ROUTE_ENABLE_DM,
[CR_AXIS_E][CR_SIGN_POS] = COLLECTIVE_CLASS_ROUTE_ENABLE_EP,
[CR_AXIS_E][CR_SIGN_NEG] = COLLECTIVE_CLASS_ROUTE_ENABLE_EM,
};
#define CR_LINK(n,s)	cr_links[n][s]

typedef xmi_coord_t CR_COORD_T;	// this code ignores the 'network' field!
#define CR_COORD_DIM(c,d)	(c)->u.n_torus.coords[d]

typedef struct {
	CR_COORD_T ll;
	CR_COORD_T ur;
} CR_RECT_T;
#define CR_RECT_LL(r)	(&(r)->ll)
#define CR_RECT_UR(r)	(&(r)->ur)

typedef ClassRoute_t CR_ROUTE_T;
#define CR_ROUTE_UP(cr)		(cr)->output
#define CR_ROUTE_DOWN(cr)	(cr)->input
#define CR_ROUTE_ID(cr)		(cr)->id
#define CR_ROUTE_VC(cr)		(cr)->output

#endif // __experimental_bgq_cnclassroute_xmi_cnclassroute_h__
