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

typedef xmi_coord_t CR_COORD_T;	// this code ignores the 'network' field!
#define CR_COORD_DIM(c,d)	(c)->u.n_torus.coords[d]

typedef struct {
	CR_COORD_T ll;
	CR_COORD_T ur;
} CR_RECT_T;
#define CR_RECT_LL(r)	(&(r)->ll)
#define CR_RECT_UR(r)	(&(r)->ur)

#endif // __experimental_bgq_cnclassroute_xmi_cnclassroute_h__
