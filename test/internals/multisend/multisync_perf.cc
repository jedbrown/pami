///
/// \file test/internals/multisend/multisync.cc
/// \brief ???
///

#include <stdio.h>
#include <pami.h>

#include "test/internals/multisend/multisync.h"
#include "components/devices/misc/AtomicBarrierMsg.h"

#define BARRIER1_NAME	"PAMI::Barrier::Indirect< PAMI::Barrier::Counter<PAMI::Counter::Native> >"
#define BARRIER1_MM	(&__global.mm)
#define BARRIER1_ISLOCAL	1
#include "components/atomic/native/NativeCounter.h"
#include "components/atomic/counter/CounterBarrier.h"
#include "components/atomic/indirect/IndirectCounter.h"
typedef PAMI::Barrier::IndirectCounter< PAMI::Counter::Indirect<PAMI::Counter::Native> > Barrier_Type1;

typedef PAMI::Device::AtomicBarrierMdl<Barrier_Type1> Barrier_Model1;
typedef PAMI::Device::AtomicBarrierDev Barrier_Device1;

#undef BARRIER2_NAME

int main(int argc, char ** argv) {
        pami_context_t context;
        pami_task_t task_id;
        size_t num_tasks;

#if 0
        pami_client_t client;
        pami_result_t status = PAMI_ERROR;
        status = PAMI_Client_create("multisync test", &client, NULL, 0);
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
#endif
        if (task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);


// END standard setup
// ------------------------------------------------------------------------

        // Register some multisyncs, C++ style
#ifdef BARRIER1_NAME
	DO_BARRIER_PERF_TEST(BARRIER1_NAME, Barrier_Model1, Barrier_Device1, BARRIER1_ISLOCAL,
		BARRIER1_MM, task_id, num_tasks, context);
#endif // BARRIER1_NAME;

#ifdef BARRIER2_NAME
	DO_BARRIER_PERF_TEST(BARRIER2_NAME, Barrier_Model2, Barrier_Device2, BARRIER2_ISLOCAL,
		BARRIER2_MM, task_id, num_tasks, context);
#endif // BARRIER2_NAME

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
