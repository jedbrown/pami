///
/// \file test/internals/multisend/mpi/P2pDispatchMulticastProtocol.cc
/// \brief ???
///

#include <stdio.h>
#include "sys/xmi.h"

#include "components/devices/mpi/MPIBcastMsg.h"
#include "components/devices/mpi/mpimulticastprotocol.h"
#include "components/devices/mpi/mpidevice.h"
#include "components/devices/mpi/mpipacketmodel.h"
#include "components/devices/mpi/mpimessage.h"
//#include "p2p/protocols/send/adaptive/Adaptive.h"
#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/eager/EagerSimple.h"
#include "p2p/protocols/send/eager/EagerImmediate.h"
#include "SysDep.h"

typedef XMI::Device::MPIDevice MPIDevice;
typedef XMI::Device::MPIMessage MPIMessage;
typedef XMI::Device::MPIPacketModel<MPIDevice,MPIMessage> MPIPacketModel;
typedef XMI::Protocol::Send::Eager <MPIPacketModel,MPIDevice> EagerMPI;
typedef XMI::Protocol::MPI::P2PMcastProto<MPIDevice,EagerMPI,XMI::Device::MPIBcastMdl,XMI::Device::MPIBcastDev> P2PMcastProto;

#include "test/internals/multisend/multicast.h"

#ifndef TEST_BUF_SIZE
#define TEST_BUF_SIZE	1024
#endif // TEST_BUF_SIZE

#define GLOBAL_BCAST_NAME	"P2PMcastProto"
#define GLOBAL_BCAST_MODEL	P2PMcastProto
#define GLOBAL_BCAST_DEVICE	MPIDevice

XMI::Topology itopo;
XMI::Topology otopo;

int main(int argc, char ** argv) {
	xmi_context_t context;
	xmi_result_t status = XMI_ERROR;
	size_t task_id;
	size_t num_tasks;

#ifdef ALL_BROKEN
#if 0
	unsigned x;
	xmi_client_t client;
	status = XMI_Client_initialize("multicast test", &client);
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
#else
	task_id = __global.mapping.task();
	num_tasks = __global.mapping.size();
	context = NULL;
	XMI::Memory::MemoryManager mm;
	initializeMemoryManager("bgp multicast test", 1024*1024, mm);
#endif
	if (task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);
	if (num_tasks < 2) {
		fprintf(stderr, "requires at least 2 ranks\n");
		exit(1);
	}

// END standard setup
// ------------------------------------------------------------------------

	xmi_result_t rc;
	// Register some multicasts, C++ style

	size_t root = __global.topology_global.index2Rank(0);
	if (task_id == root) fprintf(stderr, "Number of tasks = %zu\n", __global.topology_global.size());

	new (&itopo) XMI::Topology(root);
	__global.topology_global.subtractTopology(&otopo, &itopo);

	xmi_multicast_t mcast;
	memset(&mcast, 0x00, sizeof(mcast));
	// simple allreduce on the local ranks...
	mcast.client = 0;
	mcast.context = 0;
	mcast.roles = (unsigned)-1;
	mcast.src_participants = (xmi_topology_t *)&itopo;
	mcast.dst_participants = (xmi_topology_t *)&otopo;
	mcast.bytes = TEST_BUF_SIZE;
    mcast.connection_id = 5;

	const char *test = GLOBAL_BCAST_NAME;
	if (task_id == root) fprintf(stderr, "=== Testing %s...\n", test);
	XMI::Test::Multisend::Multicast<GLOBAL_BCAST_MODEL, GLOBAL_BCAST_DEVICE, TEST_BUF_SIZE> test1(test, 1, mm);

	rc = test1.perform_test_active_message(task_id, num_tasks, context, &mcast);
	if (rc != XMI_SUCCESS) {
		fprintf(stderr, "Failed %s test\n", test);
		exit(1);
	}
	fprintf(stderr, "PASS %s\n", test);

	mcast.bytes = 0;
    mcast.connection_id = 0;
	rc = test1.perform_test_active_message(task_id, num_tasks, context, &mcast);
	if (rc != XMI_SUCCESS) {
		fprintf(stderr, "Failed %s test\n", test);
		exit(1);
	}
	fprintf(stderr, "PASS %s\n", test);
// ------------------------------------------------------------------------
#if 0
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
#endif
#warning fix this test! the model does not conform to multicast.h
#endif // ALL_BROKEN

	return 0;
}
