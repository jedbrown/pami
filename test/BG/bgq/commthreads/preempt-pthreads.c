///
/// \file test/BG/bgq/commthreads/preempt-pthreads.c
/// \brief Simple test for basic commthread functionality
///

#include "commthread_test.h"

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
	fprintf(stderr, "Ready as %ld ...\n", pthread_self());
	while (run) {
		rc = PAMI_Context_trylock(dat->context); // should cause wakeups...
		if (rc == PAMI_SUCCESS) {
			fprintf(stderr, "Taking over...\n");
			do {
				// why doesn't advance return "num events" anymore?
				rc = PAMI_Context_advance(dat->context, 100);
				lrc = pthread_mutex_trylock(&dat->mutex);
				if (lrc == 0) {
					pthread_mutex_unlock(&dat->mutex);
				}
			} while (run && lrc == 0);
			rc = PAMI_Context_unlock(dat->context);
			fprintf(stderr, "Giving back...\n");
			lrc = pthread_mutex_lock(&dat->mutex);
			pthread_mutex_unlock(&dat->mutex);
		}
	}
	fprintf(stderr, "All done...\n");
	return NULL;
}

struct thread_data thr_data[NUM_CONTEXTS];

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
				if (y & 2) {
					for (x = 0; x < NUM_CONTEXTS; ++x) {
						pthread_mutex_unlock(&thr_data[x].mutex);
					}
				} else {
					for (x = 0; x < NUM_CONTEXTS; ++x) {
						// we shouldn't wait at all...
						pthread_mutex_lock(&thr_data[x].mutex);
					}
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
					int rc = pthread_create(&thr_data[x].thread, &attr,
						user_pthread, (void *)&thr_data[x]);
					if (rc == -1) perror("pthread_create");
					pthread_attr_destroy(&attr);
				}
			}
		}

		result = run_test(client, context, NUM_CONTEXTS);
		if (result != PAMI_SUCCESS) {
			fprintf(stderr, "Error. Unable to run commthread test. "
						"result = %d\n", result);
			//return 1;
			break;
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
		pthread_mutex_unlock(&thr_data[x].mutex);
	}
	for (x = 0; x < NUM_CONTEXTS; ++x) {
		result = PAMI_Context_destroy(context[x]);
	}
	for (x = 0; x < NUM_CONTEXTS; ++x) {
		pthread_join(thr_data[x].thread, NULL);
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
