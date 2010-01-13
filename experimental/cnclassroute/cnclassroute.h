/**
 * \file experimental/cnclassroute/cnclassroute.h
 * \brief ???
 */
/* this should all be in xmi.h? */

#ifndef __experimental_cnclassroute_cnclassroute_h__
#define __experimental_cnclassroute_cnclassroute_h__

#include <sys/types.h>
#include <stdint.h>

#define XMI_MAX_DIMS 8
#define XMI_DIM_NAMES	"ABCDEFGH"

typedef struct {
	size_t dims;
	size_t coords[XMI_MAX_DIMS];
} coord_t;

typedef struct {
	coord_t ll;
	coord_t ur;
} rect_t;

typedef struct {
	uint32_t dn_tree;
	uint32_t up_tree;
} classroute_t;

#define CR_SIGN_POS	0
#define CR_SIGN_NEG	1

#define CR_LINK(n,s)	(1 << (n * 2 + s))

#endif
