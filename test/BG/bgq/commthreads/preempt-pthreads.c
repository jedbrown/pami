///
/// \file test/BG/bgq/commthreads/single-context.c
/// \brief Simple test for basic commthread functionality
///

#include <pami.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

/* dummy routine in case PAMI target doesn't provide real one */
pami_result_t __add_context(pami_client_t client, pami_context_t context) {
	return PAMI_UNIMPL;
}

extern pami_result_t PAMI_Client_add_commthread_context(pami_client_t client,
		pami_context_t context) __attribute__((weak, alias("__add_context")));

typedef struct post_info {
	pami_work_t state;
	volatile unsigned value;
	unsigned ctx;
	unsigned seq;
} post_info_t;

#ifndef NUM_CONTEXTS
#define NUM_CONTEXTS	2
#endif /* ! NUM_CONTEXTS */

#ifndef NUM_TESTRUNS
#define NUM_TESTRUNS	3
#endif /* ! NUM_TESTRUNS */

int run = 0;
struct thread_data {
	pami_context_t context;
	pthread_mutex_t mutex;
	pthread_t thread;
};

void *user_pthread(void *cookie) {
	struct thread_data *dat = (struct thread_data *)cookie;
	pami_result_t rc;
	int lrc;
	size_t events;
	fprintf(stderr, "Ready...\n");
	while (run) {
		rc = PAMI_Context_trylock(dat->context); // should cause wakeups...
		if (rc == PAMI_SUCCESS) {
			fprintf(stderr, "Taking over...\n");
			do {
				events = PAMI_Context_advance(dat->context, 100);
			} while (events == 0);
			rc = PAMI_Context_unlock(dat->context);
			fprintf(stderr, "Giving back...\n");
			lrc = pthread_mutex_lock(&dat->mutex);
			pthread_mutex_unlock(&dat->mutex);
		}
	}
	fprintf(stderr, "All done...\n");
	return NULL;
}

post_info_t _info[NUM_CONTEXTS];
struct thread_data thr_data[NUM_CONTEXTS];

pami_result_t do_work(pami_context_t context, void *cookie) {
	post_info_t *info = (post_info_t *)cookie;
	fprintf(stderr, "do_work(%d) on context %d: cookie = %p, %d -> %d\n",
				info->seq, info->ctx, cookie, info->value, info->value - 1);
	--info->value;
	return PAMI_SUCCESS;
}

pami_result_t run_test(pami_client_t client, pami_context_t *ctx, size_t nctx) {
	pami_result_t result;
	int x;

	for (x = 0; x < nctx; ++x) {
		_info[x].value = 1;
		_info[x].ctx = x;

		/* Post some work to the contexts */
		result = PAMI_Context_post(ctx[x], &_info[x].state, do_work, (void *)&_info[x]);
		if (result != PAMI_SUCCESS) {
			fprintf(stderr, "Error. Unable to post work to pami context[%d]. "
					"result = %d (%d)\n", x, result, errno);
			return result;
		}
	}

	const unsigned long long timeout = 500000;
	unsigned long long t0, t1;
	t0 = PAMI_Wtimebase();
	int busy;
	int stuck = 0;
	do {
		for (busy = 0, x = 0; x < nctx; ++x) busy += _info[x].value;
		if (!busy) break;

		// should complete without ever calling advance...
		t1 = PAMI_Wtimebase();
		if (t1 - t0 >= timeout) {
			static char buf[1024];
			char *s = buf;
			for (x = 0; x < nctx; ++x) {
				s += sprintf(s, " [%d]=%d", _info[x].seq, _info[x].value);
			}
			fprintf(stderr, "No progress after %lld cycles...? %s\n",
									timeout, buf);
			// abort... ?
			if (++stuck > 10) return PAMI_ERROR;
			t0 = t1;
		}
	} while (busy);
	return PAMI_SUCCESS;
}

int main(int argc, char ** argv) {
	pami_client_t client;
	pami_configuration_t *configuration = NULL;
	char cl_string[] = "TEST";
	pami_result_t result = PAMI_ERROR;
	pami_context_t context[NUM_CONTEXTS];
	int x, y;

	result = PAMI_Client_create(cl_string, &client);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to initialize pami client. "
							"result = %d\n", result);
		return 1;
	}

	result = PAMI_Context_createv(client, configuration, 0, &context[0], NUM_CONTEXTS);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to create %d pami context. "
						"result = %d\n", NUM_CONTEXTS, result);
		return 1;
	}
fprintf(stderr, "Starting...\n");
	for (x = 0; x < NUM_CONTEXTS; ++x) {
		result = PAMI_Client_add_commthread_context(client, context[x]);
		if (result != PAMI_SUCCESS) {
			fprintf(stderr, "Error. Unable to add commthread to context[%d]. "
					"result = %d (%d)\n", x, result, errno);
			return 1;
		}
	}
	for (y = 0; y < NUM_TESTRUNS; ++y) {
		for (x = 0; x < NUM_CONTEXTS; ++x) {
			_info[x].seq = y * NUM_CONTEXTS + x + 1;
		}

		if (y & 1) {
			if (run) {
				for (x = 0; x < NUM_CONTEXTS; ++x) {
					pthread_mutex_unlock(&thr_data[x].mutex);
					// we shouldn't wait at all...
					pthread_mutex_lock(&thr_data[x].mutex);
				}
			} else {
				run = 1;
				for (x = 0; x < NUM_CONTEXTS; ++x) {
					pthread_attr_t attr;
					thr_data[x].context = context[x];
					pthread_mutex_init(&thr_data[x].mutex, NULL);
					pthread_mutex_lock(&thr_data[x].mutex);
					pthread_attr_init(&attr);
#ifdef __pami_target_bgq__
					cpu_set_t cpu_mask;
					CPU_ZERO(&cpu_mask);
					CPU_SET((x + 1) * 4, &cpu_mask);
					pthread_attr_setaffinity_np(&attr,
						sizeof(cpu_set_t),
						&cpu_mask);
#endif // __pami_target_bgq__
					pthread_create(&thr_data[x].thread, &attr,
						user_pthread, (void *)&thr_data[x]);
					pthread_attr_destroy(&attr);
				}
			}
		}

		result = run_test(client, context, NUM_CONTEXTS);
		if (result != PAMI_SUCCESS) {
			fprintf(stderr, "Error. Unable to run commthread test. "
						"result = %d\n", result);
			return 1;
		}

		if (y + 1 < NUM_TESTRUNS) {
			fprintf(stderr, "Sleeping...\n");
//			sleep(5);
unsigned long long t0 = PAMI_Wtimebase();
//fprintf(stderr, "Woke up after %lld cycles\n", (PAMI_Wtimebase() - t0));
while (PAMI_Wtimebase() - t0 < 500000);
fprintf(stderr, "Waking...\n");
		}
	}
	run = 0;

fprintf(stderr, "Sleeping...\n");
{unsigned long long t0 = PAMI_Wtimebase();
while (PAMI_Wtimebase() - t0 < 500000);}
fprintf(stderr, "Finishing...\n");
	for (x = 0; x < NUM_CONTEXTS; ++x) {
		result = PAMI_Context_destroy(context[x]);
	}
	result = PAMI_Client_destroy(client);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to finalize pami client. "
						"result = %d\n", result);
		return 1;
	}

	fprintf(stderr, "Success.\n");

	return 0;
}
