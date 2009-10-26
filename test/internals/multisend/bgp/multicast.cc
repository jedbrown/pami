#include <stdio.h>
#include "sys/xmi.h"

#include "components/devices/bgp/collective_network/CNBroadcastMsg.h"
#include "test/internals/multisend/multicast.h"

#ifndef TEST_BUF_SIZE
#define TEST_BUF_SIZE	1024
#endif // TEST_BUF_SIZE

XMI_TOPOLOGY_CLASS itopo;
XMI_TOPOLOGY_CLASS otopo;

int main(int argc, char ** argv) {
	unsigned x;
	xmi_client_t client;
	xmi_context_t context;
	xmi_result_t status = XMI_ERROR;

	status = XMI_Client_initialize("multicast test", &client);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", status);
		return 1;
	}

	status = XMI_Context_create(client, NULL, 0, &context);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", status);
		return 1;
	}

	xmi_configuration_t configuration;

	configuration.name = XMI_TASK_ID;
	status = XMI_Configuration_query(context, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	size_t task_id = configuration.value.intval;
	//fprintf(stderr, "My task id = %zd\n", task_id);

	configuration.name = XMI_NUM_TASKS;
	status = XMI_Configuration_query(context, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	size_t num_tasks = configuration.value.intval;
	if (task_id == 0) fprintf(stderr, "Number of tasks = %zd\n", num_tasks);

// END standard setup
// ------------------------------------------------------------------------

	xmi_result_t rc;
	// Register some multicasts, C++ style

	size_t root = 0;

	new (&itopo) XMI_TOPOLOGY_CLASS(root);
	if (task_id != root) {
		new (&otopo) XMI_TOPOLOGY_CLASS(task_id);
	}

	xmi_multicast_t mcast;

	// simple allreduce on the tree... SMP mode (todo: check and error)
	mcast.context = context;
	mcast.roles = (unsigned)-1;
	mcast.src_participants = (xmi_topology_t *)&itopo;
	mcast.dst_participants = (xmi_topology_t *)&otopo;

	const char *test = "XMI::Device::BGP::CNBroadcastModel";
	if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);
	XMI::Test::Multisend::Multicast<XMI::Device::BGP::CNBroadcastModel, TEST_BUF_SIZE> test1(test);
	rc = test1.perform_test(task_id, num_tasks, &mcast);


	if (rc != XMI_SUCCESS) {
		fprintf(stderr, "Failed %s test\n", test);
		exit(1);
	}
	fprintf(stderr, "PASS\n");

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
