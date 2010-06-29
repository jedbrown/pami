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
#include <string.h>
#include <errno.h>
#include "misc-util.h"

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
	pami_send_t send; // not used by do_work()
} post_info_t;

#ifndef NUM_CONTEXTS
#define NUM_CONTEXTS	2
#endif /* ! NUM_CONTEXTS */

#ifndef NUM_TESTRUNS
#define NUM_TESTRUNS	3
#endif /* ! NUM_TESTRUNS */

#ifndef NUM_TESTPROCS
#define NUM_TESTPROCS	2
#endif /* ! NUM_TESTPROCS */

#if NUM_CONTEXTS < NUM_TESTPROCS
#error Must have at least as many contexts as local processes
#endif

post_info_t _info[NUM_CONTEXTS];

pami_result_t do_work(pami_context_t context, void *cookie) {
	post_info_t *info = (post_info_t *)cookie;
	char buf[128];
	sprintf(buf, "do_work(%d) by %ld on context %d: cookie = %p, %d -> %d\n",
				info->seq, pthread_self(), info->ctx, cookie, info->value, info->value - 1);
	write(2, buf, strlen(buf));
	// [f]printf() is susceptible to context switches...
	//fprintf(stderr, "do_work(%d) by %ld on context %d: cookie = %p, %d -> %d\n",
	//			info->seq, pthread_self(), info->ctx, cookie, info->value, info->value - 1);
	--info->value;
	return PAMI_SUCCESS;
}

pami_result_t do_send(pami_context_t context, void *cookie) {
	post_info_t *info = (post_info_t *)cookie;
	char buf[128];
	sprintf(buf, "do_send(%d) by %ld on context %d: cookie = %p, %d -> %d\n",
				info->seq, pthread_self(), info->ctx, cookie, info->value, info->value - 1);
	write(2, buf, strlen(buf));
	pami_result_t rc = PAMI_Send(context, &info->send);
	if (rc != PAMI_SUCCESS) {
fprintf(stderr, "failed sending %d\n", info->seq);
	}

	--info->value;
	return PAMI_SUCCESS;
}

void do_recv(pami_context_t context,
             void *cookie,
             const void *hdr,
             size_t hdrlen,
             const void *pipe,
             size_t datlen,
             pami_endpoint_t origin,
             pami_recv_t *recv) {
	// assert that it is immediate recv...
	post_info_t *info = (post_info_t *)cookie;
	//assert(pipe == NULL && datlen == 0 && hdrlen == sizeof(info->seq));
	char buf[128];
	sprintf(buf, "do_recv(%d) by %ld on context %d: cookie = %p, %d\n",
				info->seq, pthread_self(), info->ctx, cookie,
				*((typeof(info->seq) *)hdr));
	write(2, buf, strlen(buf));
	--info->value;
if (info->value == (unsigned)-1) fprintf(stderr, "fault\n");
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

static size_t disp_id[NUM_CONTEXTS];

pami_result_t init_test_send(pami_client_t client, pami_context_t *ctx, size_t nctx) {
	int x;
	pami_send_hint_t h = {0};
	for (x = 0; x < nctx; ++x) {
		disp_id[x] = x;
		pami_result_t rc = PAMI_Dispatch_set(ctx[x], disp_id[x], (pami_dispatch_callback_fn){do_recv}, (void *)&_info[x], h);
		if (rc != PAMI_SUCCESS) {
			fprintf(stderr, "Failed to set dispatch for context %d\n", x);
		}
	}
	return PAMI_SUCCESS;
}

pami_result_t run_test_send(pami_client_t client, pami_context_t *ctx, size_t nctx, int role) {
	pami_result_t result;
	int x;
	size_t ix;
	pami_send_hint_t h = {0};
	size_t meix = TEST_Local_myindex();

	// even index sends, odd receives...
	if (role) { // receiver...
		for (x = 0; x < nctx; ++x) {
			++_info[x].value; // expecting a receive ...
		}

	} else { // sender...

		for (x = 0; x < nctx; ++x) {

			// assert(_info[x].value == 0)
			_info[x].ctx = x;
			ix = meix ^ 1;
			pami_task_t task = TEST_Local_index2task(ix);
			if (task == (pami_task_t)-1) continue; // never?

			size_t targ = x;

			++_info[x].value; // expecting call to posted func
			_info[x].send.send.header.iov_base = &_info[x].seq;
			_info[x].send.send.header.iov_len = sizeof(_info[x].seq);
			_info[x].send.send.data.iov_base = NULL;
			_info[x].send.send.data.iov_len = 0;
			_info[x].send.send.dispatch = disp_id[x];
			_info[x].send.send.hints = h;
			_info[x].send.events.cookie = NULL;
			_info[x].send.events.local_fn = NULL;
			_info[x].send.events.remote_fn = NULL;
			(void)PAMI_Endpoint_create(client, task, targ, &_info[x].send.send.dest);

//			if (PAMI_Context_trylock(ctx[x])) {
//				result = PAMI_Send(ctx[x], &_info[x].send);
//			} else {
				result = PAMI_Context_post(ctx[x], &_info[x].state, do_send, (void *)&_info[x]);
//			}
			if (result != PAMI_SUCCESS) {
				fprintf(stderr, "Error. Unable to post work to pami context[%d]. "
						"result = %d (%d)\n", x, result, errno);
				return result;
			}
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
