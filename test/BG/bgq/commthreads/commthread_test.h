///
/// \file test/BG/bgq/commthreads/commthread_test.h
/// \brief Simple test for basic commthread functionality
///

#ifndef __test_BG_bgq_commthreads_commthread_test_h__
#define __test_BG_bgq_commthreads_commthread_test_h__

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

post_info_t _info[NUM_CONTEXTS];

pami_result_t do_work(pami_context_t context, void *cookie) {
	post_info_t *info = (post_info_t *)cookie;
	fprintf(stderr, "do_work(%d) by %ld on context %d: cookie = %p, %d -> %d\n",
				info->seq, pthread_self(), info->ctx, cookie, info->value, info->value - 1);
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

#endif // __test_BG_bgq_commthreads_commthread_test_h__
