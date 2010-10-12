///
/// \file test/internals/multisend/bgq/local_multicombine.cc
/// \brief ???
///

#include <stdio.h>
#include <pami.h>

#include "test/internals/multisend/local_multicombine.h"
#include "components/devices/shmemcoll/ShmemCollDevice.h"
#include "components/devices/shmemcoll/ShmemCollDesc.h"
#include "components/devices/shmemcoll/ShmemMcombModelWorld.h"
#include "components/atomic/bgq/L2Counter.h"

//#include "quad_sum.h"
//#include "16way_sum.h"
//#include "8way_sum.h"

#ifndef TEST_BUF_SIZE
//#define TEST_BUF_SIZE	8
//#define TEST_BUF_SIZE	4104
//#define TEST_BUF_SIZE	8192
//#define TEST_BUF_SIZE	65536
#define TEST_BUF_SIZE	524288
//#define TEST_BUF_SIZE	1024
#endif // TEST_BUF_SIZE
#define ITER 100

#define SHORTITER 100
#define LONGITER	10
typedef PAMI::Device::Shmem::ShmemCollDesc <PAMI::Atomic::GccBuiltin> ShmemCollDesc;
//typedef PAMI::Device::Shmem::ShmemCollDesc <PAMI::Counter::BGQ::L2NodeCounter> ShmemCollDesc;
typedef PAMI::Device::ShmemCollDevice <ShmemCollDesc> ShmemCollDevice;
typedef PAMI::Device::Shmem::ShmemMcombModelWorld <ShmemCollDevice, ShmemCollDesc> ShmemMcombModel;

#define LOCAL_REDUCE_NAME	"PAMI::Device::ShmemMcombModel"
#define LOCAL_REDUCE_MODEL ShmemMcombModel
#define LOCAL_REDUCE_DEVICE ShmemCollDevice

//#define SUM_PROCESS_SINGLE

#define MEM_ALLOC_ALIGNED(x) 	{  myMemory = malloc ( TEST_BUF_SIZE + my_alignment );\
									if ( !myMemory ) printf("malloc failed\n");\
									x  = (double*)( ((uint64_t)myMemory + my_alignment)  & ~(my_alignment-1) );\
								}
#define LOCAL_BARRIER	{ counter.fetch_and_inc();\
							while (counter.fetch() < total_count+num_tasks){};\
							total_count+=num_tasks;\
						}

PAMI::Topology otopo;

int main(int argc, char ** argv) {
        pami_context_t context;
        size_t task_id;
        size_t num_tasks;

		PAMI::Counter::BGQ::L2NodeCounter counter;


        task_id = __global.mapping.task();
        num_tasks = __global.mapping.size();
        context = (pami_context_t)1; // context must not be NULL
        PAMI::Memory::MemoryManager mm;
        initializeMemoryManager("multicombine test", 8192*1024, mm);

        if (task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);
        if (__global.topology_local.size() < 2) {
                fprintf(stderr, "requires at least 2 ranks to be local\n");
                exit(1);
        }


        //pami_result_t rc;
        size_t root = __global.topology_local.index2Rank(0);
        if (task_id == root) fprintf(stderr, "Number of local tasks = %zu\n", __global.topology_local.size());

        new (&otopo) PAMI::Topology(root);

        pami_multicombine_t mcomb;

        // simple allreduce on the local ranks...
        mcomb.client = 0;
        mcomb.context = 0;
        mcomb.roles = (unsigned)-1;
        mcomb.data_participants = (pami_topology_t *)&__global.topology_local;
        mcomb.results_participants = (pami_topology_t *)&otopo;
        mcomb.optor = PAMI_SUM;
        mcomb.dtype = PAMI_DOUBLE;
        mcomb.count = TEST_BUF_SIZE / sizeof(double);

        const char *test = LOCAL_REDUCE_NAME;
        if (task_id == root) fprintf(stderr, "=== Testing %s...\n", test);
        PAMI::Test::Multisend::Multicombine<LOCAL_REDUCE_MODEL,LOCAL_REDUCE_DEVICE,TEST_BUF_SIZE> test1(test, mm);
#if 0
		unsigned long long diff = 0, sum=0;
		for (unsigned iter =0 ; iter < ITER+2; iter++)
        {

        diff = test1.perform_test(task_id, num_tasks, context, &mcomb);

		if (iter > 1) sum+=diff;
		}

		if (task_id == root) fprintf(stderr,"cycles:%lld bytes:%d\n", sum/ITER, TEST_BUF_SIZE);
#endif
		unsigned long long diff = 0, sum=0;
		unsigned num_iter;

		for (unsigned msg = 128; msg <= TEST_BUF_SIZE; msg*=2)
		{
			diff =0;
			sum = 0;
        	mcomb.count = msg / sizeof(double);

			if (msg < 16384) num_iter = SHORTITER;
			else num_iter = LONGITER;
			for (unsigned iter =0 ; iter < num_iter+2; iter++)
			{
				//rc = test1.perform_test(task_id, num_tasks, context, &mcast);
				diff = test1.perform_test(task_id, num_tasks, context, &mcomb);

				//printf("passed iter:%d\n", iter);
				if (iter > 1) sum+=diff;

			}
			if (task_id == root) fprintf(stderr,"cycles:%lld bytes:%d \n", sum/num_iter, msg);
		}
        return 0;
}
