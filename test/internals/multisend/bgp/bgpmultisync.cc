///
/// \file test/internals/multisend/bgp/bgpmultisync.cc
/// \brief ???
///

#include <stdio.h>
#include "sys/xmi.h"

#include "components/devices/bgp/global_interrupt/GIBarrierMsg.h"
#include "test/internals/multisend/multisync.h"


int main(int argc, char ** argv) {
	xmi_client_t client;
	xmi_context_t context;
	xmi_result_t status = XMI_ERROR;

	status = XMI_Client_initialize("multisync test", &client);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", status);
		return 1;
	}

	{ size_t _n = 1; status = XMI_Context_createv(client, NULL, 0, &context, _n); }
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

	if (__global.mapping.tSize() != 1) {
		fprintf(stderr, "This test requires SMP mode\n");
		exit(1);
	}

// END standard setup
// ------------------------------------------------------------------------

	// Register some multisyncs, C++ style
	xmi_result_t rc;

	xmi_multisync_t msync;

	// simple barrier on the GI network... SMP mode
	msync.client = 0;
	msync.context = 0;
	msync.roles = (unsigned)-1;
	msync.participants = (xmi_topology_t *)&__global.topology_global;

	const char *test = "XMI::Device::BGP::giModel";
	if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);
	XMI::Test::Multisend::Multisync<XMI::Device::BGP::giModel> test1(test);
	rc = test1.perform_test(task_id, num_tasks, context, &msync);
	if (rc != XMI_SUCCESS) {
		fprintf(stderr, "Failed %s test result = %d\n", test, rc);
		exit(1);
	}
	fprintf(stderr, "PASS? %5lld (%5lld) [delay: %lld, time: %lld]\n", test1.total_time, test1.barrier_time, test1.delay, test1.raw_time);

// ------------------------------------------------------------------------
	status = XMI_Client_finalize(client);
	if (status != XMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to finalize xmi client. result = %d\n", status);
		return 1;
	}

	return 0;
}
