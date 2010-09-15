///
/// \file test/internals/multisend/multicast.cc
/// \brief ???
///

#include <stdio.h>
#include <pami.h>

#include "test/internals/multisend/multicast.h"
#include "components/devices/workqueue/WQRingBcastMsg.h"
#include "components/devices/workqueue/LocalBcastWQMessage.h"

#ifndef TEST_BUF_SIZE
#define TEST_BUF_SIZE	1024
#endif // TEST_BUF_SIZE

#define LOCAL_BCAST_NAME	"PAMI::Device::LocalBcastWQModel"
#define LOCAL_BCAST_MODEL	PAMI::Device::LocalBcastWQModel
#define LOCAL_BCAST_DEVICE	PAMI::Device::LocalBcastWQDevice
#define LOCAL_BCAST_NAME2	"PAMI::Device::WQRingBcastMdl"
#define LOCAL_BCAST_MODEL2	PAMI::Device::WQRingBcastMdl
#define LOCAL_BCAST_DEVICE2	PAMI::Device::WQRingBcastDev

PAMI::Topology itopo;
PAMI::Topology otopo;

int main(int argc, char ** argv) {
        pami_context_t context;
        size_t task_id;
        size_t num_tasks;

#if 0
        pami_client_t client;
        pami_result_t status = PAMI_ERROR;
        status = PAMI_Client_create("multicast test", &client, NULL, 0);
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

        configuration.name = PAMI_CLIENT_TASK_ID;
        status = PAMI_Client_query(client, &configuration, 1);
        if (status != PAMI_SUCCESS) {
                fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
                return 1;
        }
        task_id = configuration.value.intval;
        //fprintf(stderr, "My task id = %zu\n", task_id);

        configuration.name = PAMI_CLIENT_NUM_TASKS;
        status = PAMI_Client_query(client, &configuration, 1);
        if (status != PAMI_SUCCESS) {
                fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
                return 1;
        }
        num_tasks = configuration.value.intval;
#else
        task_id = __global.mapping.task();
        num_tasks = __global.mapping.size();
        context = (pami_context_t)1; // context must not be NULL
        PAMI::Memory::GenMemoryManager mm;
        initializeMemoryManager("multicast test", 512*1024, mm);
#endif
        if (task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);
        if (__global.topology_local.size() < 2) {
                fprintf(stderr, "requires at least 2 ranks to be local\n");
                exit(1);
        }

// END standard setup
// ------------------------------------------------------------------------

        pami_result_t rc;
        // Register some multicasts, C++ style

        size_t root = __global.topology_local.index2Rank(0);
        if (task_id == root) fprintf(stderr, "Number of local tasks = %zu\n", __global.topology_local.size());

        new (&itopo) PAMI::Topology(root);
        __global.topology_local.subtractTopology(&otopo, &itopo);
        PAMI_assertf(otopo.size() == num_tasks - 1, "Failed to create proper dest topology, size is %zu should be %zu\n", otopo.size(), num_tasks - 1);
        PAMI_assertf(itopo.size() == 1, "Failed to create proper root topology, size is %zu should be %u\n", otopo.size(), 1);

        pami_multicast_t mcast;
        memset(&mcast, 0x00, sizeof(mcast));

        // simple allreduce on the local ranks...
        mcast.client = 0;
        mcast.context = 0;
        mcast.roles = (unsigned)-1;
        mcast.src_participants = (pami_topology_t *)&itopo;
        mcast.dst_participants = (pami_topology_t *)&otopo;
        mcast.bytes = TEST_BUF_SIZE;

        const char *test = LOCAL_BCAST_NAME;
        if (task_id == root) fprintf(stderr, "=== Testing %s...\n", test);
        PAMI::Test::Multisend::Multicast<LOCAL_BCAST_MODEL, LOCAL_BCAST_DEVICE, TEST_BUF_SIZE> test1(test, mm);

        rc = test1.perform_test(task_id, num_tasks, context, &mcast);
        if (rc != PAMI_SUCCESS) {
                fprintf(stderr, "Failed %s test\n", test);
                exit(1);
        }
        fprintf(stderr, "PASS %s\n", test);

        initializeMemoryManager("multicast test", 512*1024, mm);
        test = LOCAL_BCAST_NAME2;
        if (task_id == root) fprintf(stderr, "=== Testing %s...\n", test);
        PAMI::Test::Multisend::Multicast<LOCAL_BCAST_MODEL2, LOCAL_BCAST_DEVICE2, TEST_BUF_SIZE> test2(test, mm);

        rc = test2.perform_test(task_id, num_tasks, context, &mcast);
        if (rc != PAMI_SUCCESS) {
                fprintf(stderr, "Failed %s test\n", test);
                exit(1);
        }
        fprintf(stderr, "PASS %s\n", test);

// ------------------------------------------------------------------------
#if 0
        status = PAMI_Client_destroy(&client);
        if (status != PAMI_SUCCESS) {
                fprintf(stderr, "Error. Unable to finalize pami client. result = %d\n", status);
                return 1;
        }
#endif

        return 0;
}
