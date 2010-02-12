///
/// \file test/internals/multisend/multisync.cc
/// \brief ???
///

#include <stdio.h>
#include "sys/xmi.h"

#include "components/devices/generic/AtomicBarrierMsg.h"
#include "test/internals/multisend/multisync.h"

#define BARRIER_NAME	"XMI::Barrier::CounterBarrier<XMI::Counter::GccNodeCounter>"
#include "components/atomic/gcc/GccCounter.h"
#include "components/atomic/counter/CounterBarrier.h"
typedef XMI::Barrier::CounterBarrier<XMI::Counter::GccNodeCounter> Barrier_Type;

typedef XMI::Device::AtomicBarrierMdl<Barrier_Type> Barrier_Model;

#ifdef __bgp__

#define BARRIER_NAME2	"XMI::Barrier::BGP::LockBoxNodeProcBarrier"
#include "components/atomic/bgp/LockBoxBarrier.h"
typedef XMI::Barrier::BGP::LockBoxNodeProcBarrier Barrier_Type2;
typedef XMI::Device::AtomicBarrierMdl<Barrier_Type2> Barrier_Model2;

#endif // __bgp__

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

	if (__global.topology_local.size() < 2) {
		fprintf(stderr, "This test requires more than one \"local\" rank.\n");
		exit(1);
	}

// END standard setup
// ------------------------------------------------------------------------

	// Register some multisyncs, C++ style
	xmi_result_t rc;

	xmi_multisync_t msync;

	// simple allreduce on the tree... SMP mode (todo: check and error)
	msync.client = client;
	msync.context = 0;
	msync.roles = (unsigned)-1;
	msync.participants = (xmi_topology_t *)&__global.topology_local;

	const char *test = BARRIER_NAME;
	if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);
	XMI::Test::Multisend::Multisync<Barrier_Model> test1(test);
	rc = test1.perform_test(task_id, num_tasks, context, &msync);
	if (rc != XMI_SUCCESS) {
		fprintf(stderr, "Failed %s test result = %d\n", test, rc);
		exit(1);
	}

	// first number is total time for test barrier, second number is time for *this*
	// rank's barrier. The second numbers should show a significant variation
	// between ranks, while the first number should be more uniform.
	fprintf(stderr, "PASS? %5lld (%5lld) [delay: %lld, time: %lld]\n", test1.total_time, test1.barrier_time, test1.delay, test1.raw_time);

#ifdef __bgp__

	test = BARRIER_NAME2;
	if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);
	XMI::Test::Multisend::Multisync<Barrier_Model2> test2(test);
	rc = test2.perform_test(task_id, num_tasks, context, &msync);
	if (rc != XMI_SUCCESS) {
		fprintf(stderr, "Failed %s test result = %d\n", test, rc);
		exit(1);
	}

	// first number is total time for test barrier, second number is time for *this*
	// rank's barrier. The second numbers should show a significant variation
	// between ranks, while the first number should be more uniform.
	fprintf(stderr, "PASS2? %5lld (%5lld) [delay: %lld, time: %lld]\n", test2.total_time, test2.barrier_time, test2.delay, test2.raw_time);

#endif // __bgp__

// ------------------------------------------------------------------------
	status = XMI_Client_finalize(client);
	if (status != XMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to finalize xmi client. result = %d\n", status);
		return 1;
	}

	return 0;
}
