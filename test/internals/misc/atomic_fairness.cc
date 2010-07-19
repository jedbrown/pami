/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file perf/spi/lockbox-fair.c
 * \brief ???
 */

/*
 * Program to experiment with atomic mutex fairness
 *
 * Runs a contention stress test of a mutex. Lock hold and unlock yield
 * times may be varied by commandline args (i.e. [<delay> [<yield>]] ) with defaults
 * being 100 cycles. These values are varied to change the timing characteristics,
 * as is needed to find a run that exposes unfairness.
 *
 * Each core starts, and ends, independently. So start and end times of each core
 * may not (will not) coincide.
 *
 * Runs the stress test for TEST_DURATION seconds. 
 *
 * May be run on any partition size, each node will independently test the lockbox.
 * In this way, a statistically more-interesting set of results may be seen (as exact
 * timing characteristics of the test will vary between runs, and/or nodes).
 */
#include <stdio.h>
#include <pthread.h>
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"
#include "Global.h"
#include "test/internals/multisend/memorymanager.h"

#ifdef __pami_target_bgq__
#include "components/atomic/bgq/L2Counter.h"
#define COUNTER_T	PAMI::Counter::BGQ::L2ProcCounter
#ifndef NUM_THREADS
#define NUM_THREADS	16
#endif // ! NUM_THREADS
#endif
#ifdef __pami_target_bgp__
#include "components/atomic/bgp/LockBoxCounter.h"
#define COUNTER_T	PAMI::Counter::BGP::LockBoxProcCounter
#endif

#ifndef COUNTER_T
#define COUNTER_T	PAMI::Counter::GccProcCounter
#endif // !COUNTER_T

typedef PAMI::Mutex::CounterMutex<COUNTER_T> Mutex_t;

#ifndef TEST_DURATION
#define TEST_DURATION	(1*60)
#endif /* ! TEST_DURATION */

#ifndef NUM_THREADS
#define NUM_THREADS	4
#endif // ! NUM_THREADS

Mutex_t thread_lock;

/*
 * Each pthread (core) has one of these. Its used to pass identity info to the thread
 * and to accumulate/pass statistics gathered to the main (reporting) thread.
 *
 * Statistics gathered:
 * + number of times a core acquired the lock.
 *
 * Statistics reported:
 * + node.
 * + number of times each core acquired the lock.
 * + total number of times the lock was acquired.
 * + "delay" and "yield" values.
 */
struct threader {
	pthread_attr_t attr;
	pthread_t thread;
	int rank;
	int core;
	unsigned long long counter;
};

int delay = 100;
int yield = 100;

struct threader tt[NUM_THREADS];

static inline void PRINT_STATS(pami_task_t rank) {
	static char buf[4096];
	char *s = buf;
	int x;
	unsigned long long t = 0;

	s += sprintf(s, "%3d:", rank);
	for (x = 0; x < NUM_THREADS; ++x) {
		s += sprintf(s, " %10lld", tt[x].counter);
		t += tt[x].counter;
	}
	s += sprintf(s, " (%lld) delay %d yield %d\n", t, delay, yield);
	printf(buf);
}

void *slave(void *v) {
	struct threader *t = (struct threader *)v;
	int c = t->core;
	int r = t->rank;
	size_t mhz = __global.time.clockMHz();
	size_t duration = TEST_DURATION * (mhz ? mhz : 1600) * 1000000;
#if defined(__pami_target_bgq__) || defined(__pami_target_bgp__)
	fprintf(stderr, "thread starting %d (%d %d)\n", Kernel_PhysicalProcessorID(), r, c);
#else
	fprintf(stderr, "thread starting (%d %d)\n", r, c);
#endif

	unsigned long long t0, t1;
	t1 = t0 = __global.time.timebase();
	thread_lock.acquire();
	while (t1 - t0 < duration) {
		++t->counter;
		if (delay)  {
			unsigned long long t00 = __global.time.timebase();
			while (__global.time.timebase() - t00 < (unsigned long long)delay);
		}
		t1 = __global.time.timebase();
		thread_lock.release();
		if (yield) {
			unsigned long long t00 = __global.time.timebase();
			while (__global.time.timebase() - t00 < (unsigned long long)yield);
		}
		thread_lock.acquire();
	}
	thread_lock.release();
	return NULL;
}

int main(int argc, char **argv) {
	if (argc > 1) {
		delay = strtoul(argv[1], NULL, 0);
		if (argc > 2) yield = strtoul(argv[2], NULL, 0);
	}

	pami_task_t task_id = __global.mapping.task();
	size_t num_tasks = __global.mapping.size();

	if (num_tasks != 1) {
		fprintf(stderr, "Must be run with 1 task per node\n");
		exit(1);
	}

	PAMI::Memory::MemoryManager mm;
	initializeMemoryManager("TEST", 512, mm, 1, true);
	thread_lock.init(&mm);

	memset(tt, 0, sizeof(tt));
	int n;
	for (n = 0; n < NUM_THREADS; ++n) {
		tt[n].rank = task_id;
		tt[n].core = n;
		if (n == 0) continue;
		pthread_attr_init(&tt[n].attr);
		pthread_attr_setscope(&tt[n].attr, PTHREAD_SCOPE_SYSTEM);
		int status = pthread_create(&tt[n].thread, &tt[n].attr, slave, (void *)&tt[n]);
		if (status < 0) {
			perror("pthread_create");
			exit(1);
		}
	}

	slave((void *)&tt[0]);
	for (n = 0; n < NUM_THREADS; ++n) {
		if (n == 0) continue;
		pthread_join(tt[n].thread, NULL);
	}
	PRINT_STATS(task_id);

        return 0;
}
