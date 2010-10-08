///
/// \file test/internals/multisend/multicast.cc
/// \brief ???
///

#include <stdio.h>
#include <pami.h>

#include "test/internals/multisend/local_multicast.h"

#include "components/devices/shmemcoll/ShmemCollDevice.h"
#include "components/devices/shmemcoll/ShmemCollDesc.h"
#include "components/devices/shmemcoll/ShmemMcstModelWorld.h"
#include "components/atomic/bgq/L2Counter.h"


#ifndef TEST_BUF_SIZE
//#define TEST_BUF_SIZE	65536
#define TEST_BUF_SIZE	131072
//#define TEST_BUF_SIZE	8
#endif // TEST_BUF_SIZE
#define SHORTITER 100
#define LONGITER	10
typedef PAMI::Device::Shmem::ShmemCollDesc <PAMI::Atomic::GccBuiltin> ShmemCollDesc;
//typedef PAMI::Device::Shmem::ShmemCollDesc <PAMI::Counter::BGQ::L2NodeCounter> ShmemCollDesc;
typedef PAMI::Device::ShmemCollDevice <ShmemCollDesc> ShmemCollDevice;
typedef PAMI::Device::Shmem::ShmemMcstModelWorld <ShmemCollDevice, ShmemCollDesc> ShmemMcstModel;

#define LOCAL_BCAST_NAME	"PAMI::Device::ShmemMcstModel"
#define LOCAL_BCAST_MODEL 	ShmemMcstModel	
#define LOCAL_BCAST_DEVICE 	ShmemCollDevice

PAMI::Topology itopo;
PAMI::Topology otopo;

int main(int argc, char ** argv) {
        pami_context_t context;
        size_t task_id;
        size_t num_tasks;

        task_id = __global.mapping.task();
        num_tasks = __global.mapping.size();
        context = (pami_context_t)1; // context must not be NULL
        PAMI::Memory::MemoryManager mm;
        initializeMemoryManager("multicast test", 8192*1024, mm);

        if (task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);
        if (__global.topology_local.size() < 2) {
                fprintf(stderr, "requires at least 2 ranks to be local\n");
                exit(1);
        }


        //pami_result_t rc;
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

		unsigned long long diff = 0, sum=0;
		unsigned num_iter;

		for (unsigned msg = 4; msg <= TEST_BUF_SIZE; msg*=2)
		{
			diff =0;
			sum = 0;	
			mcast.bytes = msg;
		
			if (msg < 16384) num_iter = SHORTITER;
			else num_iter = LONGITER;	
			for (unsigned iter =0 ; iter < num_iter+2; iter++)
			{
				//rc = test1.perform_test(task_id, num_tasks, context, &mcast);
				diff = test1.perform_test(task_id, num_tasks, context, &mcast);

				//printf("passed iter:%d\n", iter);
				if (iter > 1) sum+=diff;

			}
			if (task_id == root) fprintf(stderr,"cycles:%lld bytes:%d \n", sum/num_iter, msg); 
		}
      //  fprintf(stderr, "PASS %s\n", test);

        return 0;
}
