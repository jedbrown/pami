///
/// \file test/internals/multisend/bgp/bgpmulticombine.cc
/// \brief ???
///

#include <stdio.h>
#include "sys/pami.h"

#include "components/devices/bgp/collective_network/CNAllreduceMsg.h"
#include "components/devices/bgp/collective_network/CNAllreducePPMsg.h"
#include "components/devices/bgp/collective_network/CNAllreduceSum2PMsg.h"
#include "test/internals/multisend/multicombine.h"

#ifndef TEST_BUF_SIZE
#define TEST_BUF_SIZE	1024
#endif // TEST_BUF_SIZE

int main(int argc, char ** argv) {
        pami_context_t context;
        size_t task_id;
        size_t num_tasks;

#if 0
        pami_client_t client;
        pami_result_t status = PAMI_ERROR;
        status = PAMI_Client_initialize("multicombine test", &client);
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
        initializeMemoryManager("bgp multicombine test", TEST_DEF_SHMEM_SIZE, mm);
#endif
        if (task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);

        if (__global.mapping.tSize() != 1) {
                fprintf(stderr, "this test requires SMP mode\n");
                exit(1);
        }

// END standard setup
// ------------------------------------------------------------------------

        // Register some multicombines, C++ style
        pami_result_t rc;

        pami_multicombine_t mcomb;

        // simple allreduce on the tree... SMP mode (todo: check and error)
        mcomb.client = 0;
        mcomb.context = 0;
        mcomb.roles = (unsigned)-1;
        mcomb.data_participants = NULL;
        mcomb.results_participants = NULL;
        mcomb.optor = PAMI_SUM;
        mcomb.dtype = PAMI_UNSIGNED_INT;
        mcomb.count = TEST_BUF_SIZE / sizeof(unsigned);

        const char *test = "PAMI::Device::BGP::CNAllreduceModel";
        if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);
        PAMI::Test::Multisend::Multicombine<PAMI::Device::BGP::CNAllreduceModel,PAMI::Device::BGP::CNAllreduceDevice,TEST_BUF_SIZE> test1(test, mm);
        rc = test1.perform_test(task_id, num_tasks, context, &mcomb);
        if (rc != PAMI_SUCCESS) {
                fprintf(stderr, "Failed %s test\n", test);
                exit(1);
        }
        fprintf(stderr, "PASS %s\n", test);

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
