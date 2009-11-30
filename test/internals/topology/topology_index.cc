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

#include "topology_test.h"

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
	size_t num;

	dump(&__global.topology_global, "world");

	for (x = 0; x < __global.topology_global.size(); ++x) {
		size_t r = __global.topology_global.index2Rank(x);
		xmi_coord_t c;
		__global.mapping.task2network(r, &c, XMI_N_TORUS_NETWORK);
		static char buf[1024];
		char *s = buf;
		char comma = '(';
		for (int y = 0; y < __global.mapping.globalDims(); ++y) {
			s += sprintf(s, "%c%zd", comma, c.u.n_torus.coords[y]);
			comma = ',';
		}
		*s++ = ')';
		*s++ = '\0';
		fprintf(stderr, "__global.topology_global.index2Rank(%zd) => %zd %s\n", x, r, buf);
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
