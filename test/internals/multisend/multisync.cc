///
/// \file test/internals/multisend/multisync.cc
/// \brief ???
///

#include <stdio.h>
#include "sys/pami.h"

#include "components/devices/misc/AtomicBarrierMsg.h"
#include "test/internals/multisend/multisync.h"

#define BARRIER_NAME	"PAMI::Barrier::CounterBarrier<PAMI::Counter::GccNodeCounter>"
#include "components/atomic/gcc/GccCounter.h"
#include "components/atomic/counter/CounterBarrier.h"
typedef PAMI::Barrier::CounterBarrier<PAMI::Counter::GccNodeCounter> Barrier_Type;

typedef PAMI::Device::AtomicBarrierMdl<Barrier_Type> Barrier_Model;
typedef PAMI::Device::AtomicBarrierDev Barrier_Device;

#undef BARRIER_NAME2
#ifdef __bgp__

#define BARRIER_NAME2	"PAMI::Barrier::BGP::LockBoxNodeProcBarrier"
#include "components/atomic/bgp/LockBoxBarrier.h"
typedef PAMI::Barrier::BGP::LockBoxNodeProcBarrier Barrier_Type2;
typedef PAMI::Device::AtomicBarrierMdl<Barrier_Type2> Barrier_Model2;
typedef PAMI::Device::AtomicBarrierDev Barrier_Device2;

#endif // __bgp__
#ifdef __bgq__
#if 0
#define BARRIER_NAME2	"PAMI::Barrier::BGQ::L2NodeProcBarrier"
#include "components/atomic/bgq/L2Barrier.h"
typedef PAMI::Barrier::BGQ::L2NodeProcBarrier Barrier_Type2;
typedef PAMI::Device::AtomicBarrierMdl<Barrier_Type2> Barrier_Model2;
typedef PAMI::Device::AtomicBarrierDev Barrier_Device2;
#endif
#endif // __bgq__

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
        initializeMemoryManager("multisync test", 128*1024, mm);
#endif
        if (task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);

        if (__global.topology_local.size() < 2) {
                fprintf(stderr, "This test requires more than one \"local\" rank.\n");
                exit(1);
        }

// END standard setup
// ------------------------------------------------------------------------

        // Register some multisyncs, C++ style
        pami_result_t rc;

        pami_multisync_t msync;

        // simple allreduce on the tree... SMP mode (todo: check and error)
        msync.client = 0;
        msync.context = 0;
        msync.roles = (unsigned)-1;
        msync.participants = (pami_topology_t *)&__global.topology_local;

        const char *test = BARRIER_NAME;
        if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);
        PAMI::Test::Multisend::Multisync<Barrier_Model,Barrier_Device> test1(test, mm);
        rc = test1.perform_test(task_id, num_tasks, context, &msync);
        if (rc != PAMI_SUCCESS) {
                fprintf(stderr, "Failed %s test result = %d\n", test, rc);
                exit(1);
        }

        // first number is total time for test barrier, second number is time for *this*
        // rank's barrier. The second numbers should show a significant variation
        // between ranks, while the first number should be more uniform.
        fprintf(stderr, "PASS? %5lld (%5lld) [delay: %lld, time: %lld]\n", test1.total_time, test1.barrier_time, test1.delay, test1.raw_time);

#ifdef BARRIER_NAME2

        initializeMemoryManager("multisync test", 128*1024, mm);
        test = BARRIER_NAME2;
        if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);
        PAMI::Test::Multisend::Multisync<Barrier_Model2,Barrier_Device2> test2(test, mm);
        rc = test2.perform_test(task_id, num_tasks, context, &msync);
        if (rc != PAMI_SUCCESS) {
                fprintf(stderr, "Failed %s test result = %d\n", test, rc);
                exit(1);
        }

        // first number is total time for test barrier, second number is time for *this*
        // rank's barrier. The second numbers should show a significant variation
        // between ranks, while the first number should be more uniform.
        fprintf(stderr, "PASS2? %5lld (%5lld) [delay: %lld, time: %lld]\n", test2.total_time, test2.barrier_time, test2.delay, test2.raw_time);

#endif // BARRIER_NAME2

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
