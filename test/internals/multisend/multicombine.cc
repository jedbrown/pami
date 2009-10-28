#include <stdio.h>
#include "sys/xmi.h"

#include "components/devices/workqueue/WQRingReduceMsg.h"
#include "components/devices/workqueue/LocalReduceWQMessage.h"
#include "components/devices/workqueue/LocalAllreduceWQMessage.h"
#include "test/internals/multisend/multicombine.h"

#ifndef TEST_BUF_SIZE
#define TEST_BUF_SIZE	1024
#endif // TEST_BUF_SIZE

#define LOCAL_REDUCE_NAME	"XMI::Device::LocalReduceWQModel"
#define LOCAL_REDUCE_MODEL	XMI::Device::LocalReduceWQModel
#define LOCAL_REDUCE_NAME2	"XMI::Device::WQRingReduceMdl"
#define LOCAL_REDUCE_MODEL2	XMI::Device::WQRingReduceMdl

XMI::Topology itopo;
XMI::Topology otopo;

int main(int argc, char ** argv) {
	unsigned x;
	xmi_client_t client;
	xmi_context_t context;
	xmi_result_t status = XMI_ERROR;

	status = XMI_Client_initialize("multicombine test", &client);
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
	if (__global.topology_local.size() < 2) {
		fprintf(stderr, "requires at least 2 ranks to be local\n");
		exit(1);
	}

// END standard setup
// ------------------------------------------------------------------------

	// Register some multicombines, C++ style

	xmi_result_t rc;
	size_t root = 0;

	new (&otopo) XMI::Topology(root);
	new (&itopo) XMI::Topology(task_id);
	xmi_multicombine_t mcomb;

	// simple allreduce on the local ranks...
	mcomb.context = context;
	mcomb.roles = (unsigned)-1;
	mcomb.data_participants = (xmi_topology_t *)&itopo;
	mcomb.results_participants = (xmi_topology_t *)&otopo;
	mcomb.optor = XMI_SUM;
	mcomb.dtype = XMI_UNSIGNED_INT;
	mcomb.count = TEST_BUF_SIZE / sizeof(unsigned);

	const char *test = LOCAL_REDUCE_NAME;
	if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);
	XMI::Test::Multisend::Multicombine<LOCAL_REDUCE_MODEL,TEST_BUF_SIZE> test1(test);

	rc = test1.perform_test(task_id, num_tasks, &mcomb);

	if (rc != XMI_SUCCESS) {
		fprintf(stderr, "Failed %s test\n", test);
		exit(1);
	}
	fprintf(stderr, "PASS %s\n", test);

	test = LOCAL_REDUCE_NAME2;
	if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);
	XMI::Test::Multisend::Multicombine<LOCAL_REDUCE_MODEL2,TEST_BUF_SIZE> test2(test);

	rc = test2.perform_test(task_id, num_tasks, &mcomb);

	if (rc != XMI_SUCCESS) {
		fprintf(stderr, "Failed %s test\n", test);
		exit(1);
	}
	fprintf(stderr, "PASS %s\n", test);

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
