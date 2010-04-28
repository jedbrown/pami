///
/// \file test/internals/multisend/bgq/bgqmultisync.cc
/// \brief ???
///

#include <stdio.h>
#include <pami.h>

#include "components/devices/misc/AtomicBarrierMsg.h"
#include "components/atomic/bgq/L2Barrier.h"
#include "test/internals/multisend/multisync.h"

#define BARRIER1_NAME	"PAMI::Barrier::BGQ::L2NodeProcBarrier"
#define	BARRIER1_ISLOCAL	1
typedef PAMI::Barrier::BGQ::L2NodeProcBarrier Barrier_Type1;
typedef PAMI::Device::AtomicBarrierMdl<Barrier_Type1> Barrier_Model1;
typedef PAMI::Device::AtomicBarrierDev Barrier_Device1;

#if 0 // future...
#include "components/devices/.../GIBarrierMsg.h"
#define BARRIER2_NAME	"PAMI::Device::BGQ::GIBarrierMdl"
#define BARRIER2_ISLOCAL	0
typedef PAMI::Device::BGQ::GIBarrierMdl Barrier_Model2;
typedef PAMI::Device::BGQ::GIBarrierDev Barrier_Device2;
#endif

int main(int argc, char ** argv) {
        pami_context_t context;
        size_t task_id;
        size_t num_tasks;

#if 0
        pami_client_t client;
        pami_result_t status = PAMI_ERROR;
        status = PAMI_Client_create("multisync test", &client);
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

// END standard setup
// ------------------------------------------------------------------------

        // Register some multisyncs, C++ style
#ifdef BARRIER1_NAME
	DO_BARRIER_TEST(BARRIER1_NAME, Barrier_Model1, Barrier_Device1, BARRIER1_ISLOCAL,
			mm, task_id, num_tasks, context);
#endif // BARRIER1_NAME

#ifdef BARRIER2_NAME
	DO_BARRIER_TEST(BARRIER2_NAME, Barrier_Model2, Barrier_Device2, BARRIER2_ISLOCAL,
			mm, task_id, num_tasks, context);
#endif // BARRIER2_NAME

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
