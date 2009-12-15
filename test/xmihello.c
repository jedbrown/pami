/**
 * \file test/xmihello.c
 * \brief ???
 */

#include <stdio.h>
#include "sys/xmi.h"
#include "Global.h"

int main(int argc, char ** argv) {
	unsigned x;
	xmi_client_t client;
	xmi_context_t context;
	xmi_result_t status = XMI_ERROR;
	const char *name = "multicombine test";

	status = XMI_Client_initialize(name, &client);
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

	configuration.name = XMI_NUM_TASKS;
	status = XMI_Configuration_query(client, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	size_t num_tasks = configuration.value.intval;

	char buf[256];
	char *s = buf;
	char p = '(';
	size_t d;
	xmi_coord_t c;
	status = __global.mapping.task2network(task_id, &c, XMI_N_TORUS_NETWORK);
	if (status == XMI_SUCCESS) {
		for (d = 0; d < __global.mapping.globalDims(); ++d) {
			s += sprintf(s, "%c%ld", p, c.u.n_torus.coords[d]);
			p = ',';
		}
		*s++ = ')';
	}
	*s++ = '\0';
	fprintf(stderr, "Hello world from XMI rank %zd of %zd %s\n", task_id, num_tasks, buf);

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
