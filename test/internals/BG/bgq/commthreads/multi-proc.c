///
/// \file test/internals/BG/bgq/commthreads/multi-proc.c
/// \brief Simple test for basic commthread functionality using multiple processes per node
///

// override the normal defaults
#ifndef NUM_CONTEXTS
#define NUM_CONTEXTS	2
#endif // NUM_CONTEXTS

#include "commthread_test.h"

int main(int argc, char ** argv) {
	pami_client_t client;
	char cl_string[] = "TEST";
	pami_result_t result = PAMI_ERROR;
	pami_context_t context[NUM_CONTEXTS];
	size_t num_contexts;
	int x, y;
	size_t meix = TEST_Local_myindex();
	char buf[64];

	result = PAMI_Client_create(cl_string, &client);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to initialize pami client. "
						"result = %d\n", result);
		return 1;
	}

	pami_configuration_t configuration;
	configuration.name = PAMI_TASK_ID;
	result = PAMI_Configuration_query(client, &configuration);
	pami_task_t task = configuration.value.intval;

	configuration.name = PAMI_NUM_TASKS;
	result = PAMI_Configuration_query(client, &configuration);
	size_t ntasks = configuration.value.intval;
	sprintf(buf, "St %ld %d %zd\n", pthread_self(), task, ntasks);
	if (ntasks & 1) {
		fprintf(stderr, "Must be run with an even number of tasks\n");
		exit(1);
	}
	int bufl = strlen(buf);

	num_contexts = TEST_Local_size();
	if (num_contexts > NUM_CONTEXTS) num_contexts = NUM_CONTEXTS;
	result = PAMI_Context_createv(client, NULL, 0, &context[0], num_contexts);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to create %zd pami context. "
						"result = %d\n", num_contexts, result);
		return 1;
	}

	result = init_test_send(client, context, num_contexts);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to init test %zd pami contexts. "
						"result = %d\n", num_contexts, result);
		return 1;
	}

	write(2, buf, bufl);
	for (x = 0; x < num_contexts; ++x) {
		result = PAMI_Client_add_commthread_context(client, context[x]);
		if (result != PAMI_SUCCESS) {
			fprintf(stderr, "Error. Unable to add commthread to context[%d]. "
							"result = %d (%d)\n", x, result, errno);
			return 1;
		}
	}
	for (y = 0; y < NUM_TESTRUNS; ++y) {
		for (x = 0; x < num_contexts; ++x) {
			_info[x].seq = (task * 1000) + y * num_contexts + x + 1;
		}

		result = run_test_send(client, context, num_contexts, (meix ^ y) & 1);
		if (result != PAMI_SUCCESS) {
			fprintf(stderr, "Error. Unable to run commthread test. "
							"result = %d\n", result);
			return 1;
		}

		if (y + 1 < NUM_TESTRUNS) {
			buf[0] = 'S'; buf[1] = 'p';
			write(2, buf, bufl);
			fprintf(stderr, "Sleeping...\n");
			/* sleep(5); */
			unsigned long long t0 = PAMI_Wtimebase();
			//fprintf(stderr, "Woke up after %lld cycles\n", (PAMI_Wtimebase() - t0));
			while (PAMI_Wtimebase() - t0 < 500000);
			buf[0] = 'W'; buf[1] = 'a';
			write(2, buf, bufl);
		}
	}

	buf[0] = 'S'; buf[1] = 'p';
	write(2, buf, bufl);
	{unsigned long long t0 = PAMI_Wtimebase();
		while (PAMI_Wtimebase() - t0 < 500000);}
	buf[0] = 'F'; buf[1] = 'i';
	write(2, buf, bufl);
	result = PAMI_Context_destroyv(context, num_contexts);
	result = PAMI_Client_destroy(&client);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to finalize pami client. "
						"result = %d\n", result);
		return 1;
	}

	buf[0] = 'S'; buf[1] = 'u';
	write(2, buf, bufl);
	fprintf(stderr, "Success.\n");

	return 0;
}
