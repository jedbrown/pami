/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file test/internals/topology/topology_test.cc
 * \brief Test the generic topology features
 */
/**
 * \file test/internals/topology/topology_test.cc
 * \brief ???
 */

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

int main(int argc, char **argv) {
	unsigned x;
	xmi_client_t client;
	xmi_context_t context;
	xmi_result_t status = XMI_ERROR;

	status = XMI_Client_initialize("progressfunc test", &client);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", status);
		return 1;
	}

	{ int _n = 1; status = XMI_Context_createv(client, NULL, 0, &context, &_n); }
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", status);
		return 1;
	}

	xmi_configuration_t configuration;

	configuration.name = XMI_TASK_ID;
	status = XMI_Configuration_query(client, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	size_t task_id = configuration.value.intval;
	//fprintf(stderr, "My task id = %zd\n", task_id);

	configuration.name = XMI_NUM_TASKS;
	status = XMI_Configuration_query(client, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	size_t num_tasks = configuration.value.intval;
	if (task_id == 0) fprintf(stderr, "Number of tasks = %zd\n", num_tasks);

// END standard setup
// ------------------------------------------------------------------------
	XMI::Topology topo, topo2, topo3;
	xmi_coord_t c0, c1;
	bool flag;
	size_t *ranks = (size_t *)malloc(num_tasks * sizeof(*ranks));
	size_t num;

	dump(&__global.topology_global, "world");

	__global.topology_global.subTopologyLocalToMe(&topo);
	dump(&topo, "local");

	__global.topology_global.subTopologyNthGlobal(&topo, 0);
	__global.mapping.task2network(task_id, &c0, XMI_N_TORUS_NETWORK);
	flag = topo.isCoordMember(&c0);
	dump(&topo, flag ? "global(0)*" : "global(0)");

	// SMP mode should return EMPTY topology here...
	__global.topology_global.subTopologyNthGlobal(&topo2, 1);
	flag = topo2.isRankMember(task_id);
	dump(&topo2, flag ? "global(1)*" : "global(1)");

	__global.topology_global.subtractTopology(&topo, &topo);
	dump(&topo, "world - global(0)");

	__global.topology_global.subtractTopology(&topo, &topo2);
	dump(&topo, "world - global(1)");

	__global.mapping.task2network(task_id, &c1, XMI_N_TORUS_NETWORK);
	// create a "format mask" for coords...
	for (x = 0; x < __global.mapping.globalDims(); ++x) {
		if (x < 2) {
			c0.u.n_torus.coords[x] = c1.u.n_torus.coords[x];
		} else {
			c0.u.n_torus.coords[x] = (size_t)-1;
		}
	}
	__global.topology_global.subTopologyReduceDims(&topo2, &c0);
	dump(&topo2, "reduce(x,y)");

	if (__global.topology_global.type() == XMI_COORD_TOPOLOGY) {
		xmi_coord_t *p0 = NULL, *p1 = NULL;
		__global.topology_global.rectSeg(&p0, &p1);
		c0 = *p0;
		c1 = *p1;
		if (c1.u.n_torus.coords[3] - c0.u.n_torus.coords[3] + 1 > 2) {
			++c0.u.n_torus.coords[3];
			new (&topo) XMI::Topology(&c0, &c1);
			--c0.u.n_torus.coords[3]; // undo previous incr
			--c1.u.n_torus.coords[3];
			new (&topo2) XMI::Topology(&c0, &c1);
			++c1.u.n_torus.coords[3]; // undo previous decr
			dump(&topo, "t-cube++");
			dump(&topo2, "t-cube--");
			topo.intersectTopology(&topo2, &topo2);
			dump(&topo2, "(t-cube++)/\\(t-cube--)");
			if (c1.u.n_torus.coords[0] - c0.u.n_torus.coords[0] + 1 > 2) {
				++c0.u.n_torus.coords[0];
				--c1.u.n_torus.coords[0];
				++c0.u.n_torus.coords[3];
				--c1.u.n_torus.coords[3];
				new (&topo2) XMI::Topology(&c0, &c1);
				dump(&topo2, "xt-cube--");
				// cut hole out of middle...
				__global.topology_global.subtractTopology(&topo, &topo2);
				dump(&topo, "(world) - (xt-cube--)");
			}
		}
	}

	if (__global.topology_global.type() != XMI_LIST_TOPOLOGY) {
		//-- ranklist-based topologies...
		__global.topology_global.getRankList(num_tasks, ranks, &num);
		if (num != num_tasks) {
			fprintf(stderr, "getRankList() did not return entire partition\n");
		}
		new (&topo) XMI::Topology(ranks, num_tasks);
		dump(&topo, "ranklist");

		topo.subTopologyLocalToMe(&topo2);
		dump(&topo2, "list-local");
	}
	if (__global.topology_global.type() != XMI_RANGE_TOPOLOGY) {
		//-- rankrange-based topologies...
		new (&topo) XMI::Topology((size_t)0, num_tasks - 1);
		dump(&topo, "rankrange");

		topo.subTopologyLocalToMe(&topo2);
		if (topo2.type() != XMI_RANGE_TOPOLOGY &&
				topo2.convertTopology(XMI_RANGE_TOPOLOGY)) {
			dump(&topo2, "range-local(cvt)");
		} else {
			dump(&topo2, "range-local");
		}
	}

// ------------------------------------------------------------------------

	status = XMI_Context_destroy(context);
	if (status != XMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to destroy xmi context. result = %d\n", status);
		return 1;
	}

	status = XMI_Client_finalize(client);
	if (status != XMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to finalize xmi client. result = %d\n", status);
		return 1;
	}

	return 0;
}
