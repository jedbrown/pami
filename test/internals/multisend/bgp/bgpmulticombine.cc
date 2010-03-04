///
/// \file test/internals/multisend/bgp/bgpmulticombine.cc
/// \brief ???
///

#include <stdio.h>
#include "sys/xmi.h"

#include "components/devices/bgp/collective_network/CNAllreduceMsg.h"
#include "components/devices/bgp/collective_network/CNAllreducePPMsg.h"
#include "components/devices/bgp/collective_network/CNAllreduceSum2PMsg.h"
#include "test/internals/multisend/multicombine.h"

#ifndef TEST_BUF_SIZE
#define TEST_BUF_SIZE	1024
#endif // TEST_BUF_SIZE

int main(int argc, char ** argv) {
	xmi_context_t context;
	size_t task_id;
	size_t num_tasks;

#if 0
	xmi_client_t client;
	xmi_result_t status = XMI_ERROR;
	status = XMI_Client_initialize("multicombine test", &client);
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
	task_id = configuration.value.intval;
	//fprintf(stderr, "My task id = %zu\n", task_id);

	configuration.name = XMI_NUM_TASKS;
	status = XMI_Configuration_query(client, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	num_tasks = configuration.value.intval;
#else
	task_id = __global.mapping.task();
	num_tasks = __global.mapping.size();
	context = (xmi_context_t)1; // context must not be NULL
	XMI::Memory::MemoryManager mm;
	initializeMemoryManager("bgp multicombine test", 1024*1024, mm);
#endif
	if (task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);

	if (__global.mapping.tSize() != 1) {
		fprintf(stderr, "this test requires SMP mode\n");
		exit(1);
	}

// END standard setup
// ------------------------------------------------------------------------

	// Register some multicombines, C++ style
	xmi_result_t rc;

	xmi_multicombine_t mcomb;

	// simple allreduce on the tree... SMP mode (todo: check and error)
	mcomb.client = 0;
	mcomb.context = 0;
	mcomb.roles = (unsigned)-1;
	mcomb.data_participants = NULL;
	mcomb.results_participants = NULL;
	mcomb.optor = XMI_SUM;
	mcomb.dtype = XMI_UNSIGNED_INT;
	mcomb.count = TEST_BUF_SIZE / sizeof(unsigned);

	const char *test = "XMI::Device::BGP::CNAllreduceModel";
	if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);
	XMI::Test::Multisend::Multicombine<XMI::Device::BGP::CNAllreduceModel,XMI::Device::BGP::CNAllreduceDevice,TEST_BUF_SIZE> test1(test, mm);
	rc = test1.perform_test(task_id, num_tasks, context, &mcomb);
	if (rc != XMI_SUCCESS) {
		fprintf(stderr, "Failed %s test\n", test);
		exit(1);
	}
	fprintf(stderr, "PASS %s\n", test);

// ------------------------------------------------------------------------
#if 0
	status = XMI_Client_finalize(client);
	if (status != XMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to finalize xmi client. result = %d\n", status);
		return 1;
	}
#endif

	return 0;
}
