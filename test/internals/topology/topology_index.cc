/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file test/internals/topology/topology_index.cc
 * \brief Test the generic topology features
 */

#include "topology_test.h"

void check_index2rank(XMI::Topology *topo, const char *str) {
	size_t x;
	for (x = 0; x < topo->size(); ++x) {
		xmi_task_t r = topo->index2Rank(x);
		size_t i = topo->rank2Index(r);
		xmi_coord_t c;
		__global.mapping.task2network(r, &c, XMI_N_TORUS_NETWORK);
		static char buf[1024];
		char *s = buf;
		char comma = '(';
		for (size_t y = 0; y < __global.mapping.globalDims(); ++y) {
			s += sprintf(s, "%c%zu", comma, c.u.n_torus.coords[y]);
			comma = ',';
		}
		*s++ = ')';
		*s++ = '\0';
		fprintf(stderr, "%s.index2Rank(%zu) => %d => %zu %s\n", str, x, r, i, buf);
	}
}

int main(int argc, char **argv) {
	unsigned x;
	xmi_client_t client;
	xmi_context_t context;
	xmi_result_t status = XMI_ERROR;

	status = XMI_Client_initialize("topology_index test", &client);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", status);
		return 1;
	}

	{ size_t _n = 1; status = XMI_Context_createv(client, NULL, 0, &context, &_n); }
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
	//fprintf(stderr, "My task id = %zu\n", task_id);

	configuration.name = XMI_NUM_TASKS;
	status = XMI_Configuration_query(client, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	size_t num_tasks = configuration.value.intval;
	if (task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);

// END standard setup
// ------------------------------------------------------------------------
	bool flag;
	size_t num;
	xmi_task_t *ranks = (xmi_task_t *)malloc(num_tasks * sizeof(*ranks));

	dump(&__global.topology_global, "world");
	check_index2rank(&__global.topology_global, "topology_global");

	for (x = 0; x < num_tasks; ++x) {
		ranks[x] = x;
	}
	XMI::Topology topo_list;
	new (&topo_list) XMI::Topology(ranks, num_tasks);
	if (topo_list.convertTopology(XMI_COORD_TOPOLOGY)) {
		dump(&topo_list, "topo_list(coords)");
		check_index2rank(&topo_list, "topo_list(coords)");
	} else {
		fprintf(stderr, "Failed to convert topo_list to coords.\n");
	}

	if (__global.topology_global.type() == XMI_COORD_TOPOLOGY) {
		memcpy(&topo_list, &__global.topology_global, sizeof(XMI::Topology));
		if (topo_list.convertTopology(XMI_LIST_TOPOLOGY)) {
			dump(&topo_list, "topo_list(list)");
			check_index2rank(&topo_list, "topo_list(list)");
		} else {
			fprintf(stderr, "Failed to convert topo_list to list.\n");
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
