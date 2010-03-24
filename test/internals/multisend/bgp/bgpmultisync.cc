///
/// \file test/internals/multisend/bgp/bgpmultisync.cc
/// \brief ???
///

#include <stdio.h>
#include "sys/pami.h"

#include "components/devices/bgp/global_interrupt/GIBarrierMsg.h"
#include "test/internals/multisend/multisync.h"


int main(int argc, char ** argv) {
	pami_context_t context;
	size_t task_id;
	size_t num_tasks;

#if 0
	pami_client_t client;
	pami_result_t status = PAMI_ERROR;
	status = PAMI_Client_initialize("multisync test", &client);
	if (status != PAMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", status);
		return 1;
	}

	{ size_t _n = 1; status = PAMI_Context_createv(client, NULL, 0, &context, _n); }
	if (status != PAMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to create pami context. result = %d\n", status);
		return 1;
	}

	pami_configuration_t configuration;

	configuration.name = PAMI_TASK_ID;
	status = PAMI_Configuration_query(client, &configuration);
	if (status != PAMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	task_id = configuration.value.intval;
	//fprintf(stderr, "My task id = %zu\n", task_id);

	configuration.name = PAMI_NUM_TASKS;
	status = PAMI_Configuration_query(client, &configuration);
	if (status != PAMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	num_tasks = configuration.value.intval;
#else
	task_id = __global.mapping.task();
	num_tasks = __global.mapping.size();
	context = (pami_context_t)1; // context must not be NULL
	PAMI::Memory::MemoryManager mm;
	initializeMemoryManager("bgp multisync test", TEST_DEF_SHMEM_SIZE, mm);
#endif
	if (task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);

	if (__global.mapping.tSize() != 1) {
		fprintf(stderr, "This test requires SMP mode\n");
		exit(1);
	}

// END standard setup
// ------------------------------------------------------------------------

	// Register some multisyncs, C++ style
	pami_result_t rc;

	pami_multisync_t msync;

	// simple barrier on the GI network... SMP mode
	msync.client = 0;
	msync.context = 0;
	msync.roles = (unsigned)-1;
	msync.participants = (pami_topology_t *)&__global.topology_global;

	const char *test = "PAMI::Device::BGP::giModel";
	if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);
	PAMI::Test::Multisend::Multisync<PAMI::Device::BGP::giModel,PAMI::Device::BGP::giDevice> test1(test, mm);
	rc = test1.perform_test(task_id, num_tasks, context, &msync);
	if (rc != PAMI_SUCCESS) {
		fprintf(stderr, "Failed %s test result = %d\n", test, rc);
		exit(1);
	}
	fprintf(stderr, "PASS? %5lld (%5lld) [delay: %lld, time: %lld]\n", test1.total_time, test1.barrier_time, test1.delay, test1.raw_time);

// ------------------------------------------------------------------------
#if 0
	status = PAMI_Client_finalize(client);
	if (status != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to finalize pami client. result = %d\n", status);
		return 1;
	}
#endif

	return 0;
}
