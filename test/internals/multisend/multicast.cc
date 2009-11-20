///
/// \file test/internals/multisend/multicast.cc
/// \brief ???
///

#include <stdio.h>
#include "sys/xmi.h"

#include "components/devices/workqueue/WQRingBcastMsg.h"
#include "components/devices/workqueue/LocalBcastWQMessage.h"
#include "test/internals/multisend/multicast.h"

#ifndef TEST_BUF_SIZE
#define TEST_BUF_SIZE	1024
#endif // TEST_BUF_SIZE

#define LOCAL_BCAST_NAME	"XMI::Device::LocalBcastWQModel"
#define LOCAL_BCAST_MODEL	XMI::Device::LocalBcastWQModel
#define LOCAL_BCAST_NAME2	"XMI::Device::WQRingBcastMdl"
#define LOCAL_BCAST_MODEL2	XMI::Device::WQRingBcastMdl

XMI::Topology itopo;
XMI::Topology otopo;

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
	if (__global.topology_local.size() < 2) {
		fprintf(stderr, "requires at least 2 ranks to be local\n");
		exit(1);
	}

// END standard setup
// ------------------------------------------------------------------------

	xmi_result_t rc;
	// Register some multicasts, C++ style

	size_t root = __global.topology_local.index2Rank(0);
	if (task_id == root) fprintf(stderr, "Number of local tasks = %zd\n", __global.topology_local.size());

	new (&itopo) XMI::Topology(root);
	__global.topology_local.subtractTopology(&otopo, &itopo);

	xmi_multicast_t mcast;

	// simple allreduce on the local ranks...
	mcast.client = client;
	mcast.context = 0;
	mcast.roles = (unsigned)-1;
	mcast.src_participants = (xmi_topology_t *)&itopo;
	mcast.dst_participants = (xmi_topology_t *)&otopo;
	mcast.bytes = TEST_BUF_SIZE;

	const char *test = LOCAL_BCAST_NAME;
	if (task_id == root) fprintf(stderr, "=== Testing %s...\n", test);
	XMI::Test::Multisend::Multicast<LOCAL_BCAST_MODEL, TEST_BUF_SIZE> test1(test);

	rc = test1.perform_test(task_id, num_tasks, &mcast);
	if (rc != XMI_SUCCESS) {
		fprintf(stderr, "Failed %s test\n", test);
		exit(1);
	}
	fprintf(stderr, "PASS %s\n", test);

	test = LOCAL_BCAST_NAME2;
	if (task_id == root) fprintf(stderr, "=== Testing %s...\n", test);
	XMI::Test::Multisend::Multicast<LOCAL_BCAST_MODEL2, TEST_BUF_SIZE> test2(test);

	rc = test2.perform_test(task_id, num_tasks, &mcast);
	if (rc != XMI_SUCCESS) {
		fprintf(stderr, "Failed %s test\n", test);
		exit(1);
	}
	fprintf(stderr, "PASS %s\n", test);

// ------------------------------------------------------------------------
	status = XMI_Client_finalize(client);
	if (status != XMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to finalize xmi client. result = %d\n", status);
		return 1;
	}

	return 0;
}
