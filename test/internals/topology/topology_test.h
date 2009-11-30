/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file test/internals/topology/topology_test.h
 * \brief Test the generic topology features
 */

#ifndef __test_internals_topology_topology_test_h__
#define __test_internals_topology_topology_test_h__

#include "sys/xmi.h"
#include "Topology.h"
#include "Global.h"

const char *topo_types[] = {
	"EMPTY",
	"SINGLE",
	"RANGE",
	"LIST",
	"COORD",
};

void dump(XMI::Topology *topo, const char *title) {
	static char buf[4096];
	char *s = buf;
	size_t f = 0, l = 0;
	xmi_coord_t *ll, *ur;
	xmi_result_t rc;
	size_t x;

	size_t z = topo->size();
	s += sprintf(s, "%s: %s (%zu) {", title, topo_types[topo->type()], z);
	switch (topo->type()) {
	case XMI_EMPTY_TOPOLOGY:
		s += sprintf(s, "void");
		break;
	case XMI_SINGLE_TOPOLOGY:
	case XMI_LIST_TOPOLOGY:
		for (x = 0; x < z; ++x) {
			s += sprintf(s, " %zu", topo->index2Rank(x));
		}
		*s++ = ' ';
		break;
	case XMI_RANGE_TOPOLOGY:
		topo->rankRange(&f, &l);
		s += sprintf(s, " %zu..%zu ", f, l);
		break;
	case XMI_COORD_TOPOLOGY:
		rc = topo->rectSeg(&ll, &ur);
		if (rc) {
			// not possible? we already know its COORD...
		} else {
			s += sprintf(s, " (");
			for (x = 0; x < __global.mapping.globalDims(); ++x) {
				if (x) *s++ = ',';
				s += sprintf(s, "%zu", ll->u.n_torus.coords[x]);
			}
			s += sprintf(s, ")..(");
			for (x = 0; x < __global.mapping.globalDims(); ++x) {
				if (x) *s++ = ',';
				s += sprintf(s, "%zu", ur->u.n_torus.coords[x]);
			}
			s += sprintf(s, ") ");
		}
		break;
	default:
		break;
	}
	s += sprintf(s, "}\n");

	fprintf(stderr, buf);
}

#endif // __test_internals_topology_topology_test_h__
