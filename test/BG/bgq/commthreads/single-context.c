///
/// \file test/BG/bgq/commthreads/single-context.c
/// \brief Simple test for basic commthread functionality
///

#include "sys/pami.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

pami_result_t __add_context(pami_client_t client, pami_context_t context) {
	return PAMI_UNIMPL;
}

extern pami_result_t PAMI_Client_add_commthread_context(pami_client_t client, pami_context_t context) __attribute__((weak, alias("__add_context")));

typedef struct post_info {
	pami_work_t state;
	volatile unsigned value;
	unsigned seq;
} post_info_t;

post_info_t _info[2];

pami_result_t do_work(pami_context_t context, void *cookie) {
	post_info_t *info = (post_info_t *)cookie;
	fprintf(stderr, "do_work(%d) cookie = %p, %d -> %d\n", info->seq, cookie, info->value, info->value-1);
	info->value--;
	return PAMI_SUCCESS;
}

pami_result_t run_test(pami_context_t *ctx, size_t nctx) {
	pami_result_t result;

	_info[0].value = 1;
	_info[1].value = 1;

	/* Post some work to the contexts */
	result = PAMI_Context_post(ctx[0], &_info[0].state, do_work, (void *)&_info[0]);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to post work to the first pami context. result = %d (%d)\n", result, errno);
		return result;
	}
	result = PAMI_Context_post(ctx[1], &_info[1].state, do_work, (void *)&_info[1]);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to post work to the second pami context. result = %d (%d)\n", result, errno);
		return result;
	}

int count = 0;
	while (_info[0].value || _info[1].value) {
		// should complete without ever calling advance...
if (++count == 10000) fprintf(stderr, "stuck? %d %d\n", _info[0].value, _info[1].value);
	}
	return PAMI_SUCCESS;
}

int main(int argc, char ** argv) {
	pami_client_t client;
	pami_context_t context[2];
	pami_configuration_t *configuration = NULL;
	char cl_string[] = "TEST";
	pami_result_t result = PAMI_ERROR;

	_info[0].seq = 1;
	_info[1].seq = 2;

	result = PAMI_Client_initialize(cl_string, &client);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to initialize pami client. result = %d\n", result);
		return 1;
	}

	result = PAMI_Context_createv(client, configuration, 0, &context[0], 2);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to create two pami context. result = %d\n", result);
		return 1;
	}

	result = PAMI_Client_add_commthread_context(client, context[0]);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to add commthread to first pami context. result = %d (%d)\n", result, errno);
		return 1;
	}
	result = PAMI_Client_add_commthread_context(client, context[1]);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to add commthread to second pami context. result = %d (%d)\n", result, errno);
		return 1;
	}

	result = run_test(context, 2);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to run commthread test. result = %d\n", result);
		return 1;
	}

	fprintf(stderr, "Sleeping...\n");
unsigned long long t0 = PAMI_Wtimebase();
//	sleep(5);
//fprintf(stderr, "Woke up after %lld cycles\n", (PAMI_Wtimebase() - t0));
while (PAMI_Wtimebase() - t0 < 50000);
	_info[0].seq += 2;
	_info[1].seq += 2;

	result = run_test(context, 2);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to run commthread test. result = %d\n", result);
		return 1;
	}

	result = PAMI_Client_finalize(client);
	if (result != PAMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to finalize pami client. result = %d\n", result);
		return 1;
	}

	fprintf(stderr, "Success.\n");

	return 0;
};
