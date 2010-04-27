/**
 * \file test/internals/multisend/bgp/bgpmulticast.cc
 * \brief ???
 */

#include <stdio.h>
#include "sys/pami.h"

#include "components/devices/bgp/collective_network/CNBroadcastMsg.h"
#include "test/internals/multisend/multicast.h"

#ifndef TEST_BUF_SIZE
#define TEST_BUF_SIZE	1024
#endif // TEST_BUF_SIZE

PAMI::Topology itopo;
PAMI::Topology otopo;

int main(int argc, char ** argv) {
        pami_context_t context;
        size_t task_id;
        size_t num_tasks;

#if 0
        pami_client_t client;
        pami_result_t status = PAMI_ERROR;
        status = PAMI_Client_create("multicast test", &client);
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
        initializeMemoryManager("bgp multicast test", TEST_DEF_SHMEM_SIZE, mm);
#endif
        if (task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);

// END standard setup
// ------------------------------------------------------------------------

        pami_result_t rc;
        // Register some multicasts, C++ style

        size_t root = 0;

        new (&itopo) PAMI::Topology(root);
        if (task_id != root) {
                new (&otopo) PAMI::Topology(task_id);
        }

        pami_multicast_t mcast;

        // simple allreduce on the tree... SMP mode (todo: check and error)
        mcast.client = 0;
        mcast.context = 0;
        mcast.roles = (unsigned)-1;
        mcast.src_participants = (pami_topology_t *)&itopo;
        mcast.dst_participants = (pami_topology_t *)&otopo;
        mcast.bytes = TEST_BUF_SIZE;

        const char *test = "PAMI::Device::BGP::CNBroadcastModel";
        if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);
        PAMI::Test::Multisend::Multicast<PAMI::Device::BGP::CNBroadcastModel,PAMI::Device::BGP::CNBroadcastDevice, TEST_BUF_SIZE> test1(test, mm);
        rc = test1.perform_test(task_id, num_tasks, context, &mcast);


        if (rc != PAMI_SUCCESS) {
                fprintf(stderr, "Failed %s test\n", test);
                exit(1);
        }
        fprintf(stderr, "PASS %s\n", test);

// ------------------------------------------------------------------------
#if 0
        status = PAMI_Client_destroy(client);
        if (status != PAMI_SUCCESS) {
                fprintf(stderr, "Error. Unable to finalize pami client. result = %d\n", status);
                return 1;
        }
#endif

        return 0;
}
