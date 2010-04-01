/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/queue/queue_stress1.cc
 * \brief ???
 */

#define _POSIX_C_SOURCE 199309
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "sys/pami.h"
#include "components/memory/MemoryManager.h"
#include "util/queue/GccThreadSafeMultiQueue.h"
#include "util/queue/GccThreadSafeQueue.h"
#include "util/queue/MutexedMultiQueue.h"
#include "util/queue/Queue.h" // includes MutexedQueue
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"

static inline pid_t gettid() {
	return syscall(SYS_gettid);
}

#ifndef MAX_PTHREADS
#define MAX_PTHREADS	8
#endif // MAX_PTHREADS

#define QUEUE1_NAME	"GccThreadSafeQueue"
typedef PAMI::GccThreadSafeQueue queue_1;


#define QUEUE2_NAME	"MutexedQueue<CounterMutex<GccProcCounter>>"
typedef PAMI::MutexedQueue<PAMI::Mutex::CounterMutex<PAMI::Counter::GccProcCounter> > queue_2;

#ifdef __bgp__
#include "components/atomic/bgp/LockBoxMutex.h"
#define QUEUE3_NAME	"MutexedQueue<LockBoxProcMutex>"
typedef PAMI::MutexedQueue<PAMI::Mutex::BGP::LockBoxProcMutex> queue_3;
#endif

#ifdef __bgp__
#include "components/atomic/bgq/L2Mutex.h"
#define QUEUE3_NAME	"MutexedQueue<L2ProcMutex>"
typedef PAMI::MutexedQueue<PAMI::Mutex::BGQ::L2ProcMutex> queue_3;
#endif

template <class T_Queue, int T_BackoffNS = 0>
class QueueTest {
public:

	struct element_t {
		typename T_Queue::Element elem;
		unsigned int pid;
		unsigned int seq;
		unsigned int val;
	};

	PAMI::Memory::MemoryManager mm;
	T_Queue queue;

	double base_t;
	const char *name;
	int pthreads;
	int elements;
	int seed;

	QueueTest(const char *n, int pth, int elem, int s) :
	name(n),
	pthreads(pth),
	elements(elem),
	seed(s)
	{
		size_t size = 32*1024;
		void *ptr = malloc(size);
		assert(ptr);
		mm.init(ptr, size);
		queue.init(&mm);

		int x;
		unsigned long long t1, t0 = PAMI_Wtimebase();
		for (x = 0; x < 10000; ++x) {
			t1 = PAMI_Wtimebase() - t0;
		}
		t1 = PAMI_Wtimebase() - t0;
		double d = t1;
		base_t = d / x;
	}

	static void *enqueuers(void *v) {
		QueueTest *thus = (QueueTest *)v;
		T_Queue *q = &thus->queue;
		int num = thus->elements;
		int x;
		element_t *e;
		unsigned long long t0, t = 0;
		timespec tv = {0, T_BackoffNS};

		fprintf(stderr, "%d: starting %d enqueues\n", gettid(), num);
		for (x = 0; x < num; ++x) {
			if (T_BackoffNS) nanosleep(&tv, NULL);
			e = (element_t *)malloc(sizeof(*e));
			assert(e);
			e->pid = gettid();
			e->seq = x;
			e->val = 1;	// debug
			t0 = PAMI_Wtimebase();
			q->enqueue(&e->elem);
			t += PAMI_Wtimebase() - t0;
		}
		double d = t;
		fprintf(stderr, "%d: finished %d enqueues (%g cycles each) %g\n",
				gettid(), num, d / num, thus->base_t);
		return NULL;
}

	static void *dequeuer(void *v) {
		QueueTest *thus = (QueueTest *)v;
		T_Queue *q = &thus->queue;
		int num = thus->elements * (thus->pthreads - 1);
		int x = 0, y = 0, z = 0;
		element_t *e;
		unsigned long long t0, t1, t = 0, tr = 0, tb = 0;

		fprintf(stderr, "%d: looking for %d dequeues\n", gettid(), num);
		typename T_Queue::Iterator qi;
		q->iter_init(&qi);
		while (x < num) {
			if (!q->head()) sched_yield();
			t0 = PAMI_Wtimebase();
			bool b = q->iter_begin(&qi);
			t1 = PAMI_Wtimebase();
			if (b) {
				tb += t1 - t0;
				++z;
			}
			t0 = PAMI_Wtimebase();
			for (; q->iter_check(&qi); q->iter_end(&qi)) {
				++y;
				e = (element_t *)q->iter_current(&qi);
				if (e->val == (unsigned)-1 || (rand() & 0x03) == 0) {
					e->val = -1;
					t += PAMI_Wtimebase() - t0;
					t0 = PAMI_Wtimebase();
					if (q->iter_remove(&qi) == PAMI_SUCCESS) {
						tr += PAMI_Wtimebase() - t0;
						free(e);
						e = NULL; // just in case we try to access it
						++x;
					} else {
						tr += PAMI_Wtimebase() - t0;
					}
					t0 = PAMI_Wtimebase();
				}
			}
			t += PAMI_Wtimebase() - t0;
		}
		double d = t;
		double dr = tr;
		double db = tb;
		fprintf(stderr, "%d: finished %d dequeues (%g cycles per iter-elem (%d), "
				"%g per remove, %g per merge (%d))\n",
				gettid(), num, d / y, y, dr / num, db / z, z);
		return NULL;
	}

	pthread_attr_t attr[MAX_PTHREADS];
	pthread_t thread[MAX_PTHREADS];

	int run_test(void) {
		int x;
		fprintf(stderr, "main: starting %s test with %d threads "
				"and %d elements per enqueuer (seed %d)\n",
				name, pthreads, elements, seed);

		int status;
		// thread "0" is the main thread - already running
		for (x = 1; x < pthreads; ++x) {
			pthread_attr_init(&attr[x]);
			pthread_attr_setscope(&attr[x], PTHREAD_SCOPE_SYSTEM);
			status = pthread_create(&thread[x], &attr[x],
						enqueuers, (void *)this);
			/* don't care about status? just reap threads below? */
		}
		// create args struct for main thread
		(void)dequeuer((void *)this);

		for (x = 1; x < pthreads; ++x) {
			pthread_join(thread[x], NULL);
		}
		fprintf(stderr, "main done. queue = { %p %p %zu }\n",
				queue.head(), queue.tail(), queue.size());

		return (queue.head() || queue.tail() || queue.size());
	}

}; // class QueueTest

int main(int argc, char **argv) {
	int x;
	int pthreads = 4;
	int elements = 1000;
	int seed = 1;
	int qtype = 0;

	//extern int optind;
	extern char *optarg;

	while ((x = getopt(argc, argv, "e:p:q:s:")) != EOF) {
		switch(x) {
		case 'e':
			elements = strtol(optarg, NULL, 0);
			break;
		case 'p':
			pthreads = strtol(optarg, NULL, 0);
			break;
		case 'q':
			qtype = strtol(optarg, NULL, 0);
			break;
		case 's':
			seed = strtol(optarg, NULL, 0);
			srand(seed);
			break;
		}
	}
	if (!qtype) qtype = 1;
	int ret = 0;
	switch(qtype) {
	case 1:
		{
		QueueTest<queue_1,0> test1(QUEUE1_NAME, pthreads, elements, seed);
		ret = test1.run_test();
		}
		break;
	case 2:
		{
		QueueTest<queue_2,0> test2(QUEUE2_NAME, pthreads, elements, seed);
		ret = test2.run_test();
		}
		break;
#ifdef QUEUE3_NAME
	case 3:
		{
		QueueTest<queue_3,0> test3(QUEUE3_NAME, pthreads, elements, seed);
		ret = test3.run_test();
		}
		break;
#endif // QUEUE3_NAME
	default:
		fprintf(stderr, "Invavlid queue type %d\n", qtype);
		ret = 1;
		break;
	}
	exit(ret);
}
