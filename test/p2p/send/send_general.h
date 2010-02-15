/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/p2p/send/send_general.h
 * \brief some generally-useful setups for send tests
 */

#ifndef __test_p2p_send_send_general_h__
#define __test_p2p_send_send_general_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <malloc.h>
#include <strings.h>

#include "xmi.h"
//#include "../util/barrier.h"

#ifndef ITERATIONS
#define ITERATIONS 1000
#elif ITERATIONS == -1
#undef ITERATIONS
extern "C" int ITERATIONS;
#endif

#ifndef MINBUFSIZE
#define MINBUFSIZE 0
#elif MINBUFSIZE == -1
#undef MINBUFSIZE
extern "C" size_t MINBUFSIZE;
#endif /* ! MINBUFSIZE */

#ifndef BUFSIZE
#define BUFSIZE 1024
#elif BUFSIZE == -1
#undef BUFSIZE
extern "C" size_t BUFSIZE;
#endif /* BUFSIZE == -1 */

char *rbuf, *sbuf;

void send_general_init(int bufsize) {
	if (bufsize <= 0) bufsize = 16;
	posix_memalign((void **)&rbuf, 16, bufsize);
	posix_memalign((void **)&sbuf, 16, bufsize);
	assert(rbuf && sbuf);
	assert(!(((size_t)rbuf | (size_t)sbuf) & 0x0f));
}

int setup_localpeers(xmi_task_t *ranks, size_t nranks, xmi_task_t *mine) {
	int i = 0;
#if 0
	size_t me = DCMF_Messager_rank();
	size_t t, r;
	*mine = -1;

	XMI_Coord_t addr;
	XMI_Network network;
	DCMF_Messager_rank2network(me, XMI_TORUS_NETWORK, &addr);
	for (t = 0; t < nranks; ++t) {
		addr.torus.t = t;
		if (DCMF_Messager_network2rank(&addr, &r, &network) == XMI_SUCCESS) {
			if (r == me) *mine = i;
			ranks[i++] = r;
		}
	}
#endif
	return i;
}

#ifndef NEXT_BUFSIZE
#define NEXT_BUFSIZE(sndlen)	((sndlen) * 3 / 2 + 1)
#endif /* ! NEXT_BUFSIZE */

#define TRACE_ERR(x...) // fprintf(x)

static struct networks {
	size_t netw;
	const char *name;
} networks[] = {
	{ 0,	"DEFAULT_NETWORK" },
};
#define NUM_NETWORKS	(sizeof(networks) / sizeof(networks[0]))

static struct protos {
	size_t prot;
	const char *name;
} protos[] = {
	{ 0,	"DEFAULT_SEND_PROTOCOL" },
};
#define NUM_PROTOCOLS	(sizeof(protos) / sizeof(protos[0]))

static int find_netw(const char *pro) {
	int x, y;
	const char *net;
	static int init = 0;
	net = strchr(pro, ':');
	if (net) {
		pro = strndup(pro, net - pro);
		net++;
	} else {
		net = pro;
		pro = "DEFAULT_SEND_PROTOCOL";
	}
	for (x = 0; x < (int)NUM_NETWORKS; ++x) {
		if (strcasecmp(net, networks[x].name) == 0) {
			break;
		}
	}
	if (x >= (int)NUM_NETWORKS) {
		return -1;
	}
	for (y = 0; y < (int)NUM_PROTOCOLS; ++y) {
		if (strcasecmp(pro, protos[y].name) == 0) {
			break;
		}
	}
	if (y >= (int)NUM_NETWORKS) {
		return -1;
	}
	return (y << 16) | x;
}

static char *name_netw(int netpro) {
	static char buf[512];
	int net = netpro & 0x0000ffff;
	int pro = netpro >> 16;
	sprintf(buf, "%s:%s", protos[pro].name, networks[net].name);
	return buf;
}

volatile int sendflag;
volatile int recvflag;

int check_buf(char *buf, size_t len, unsigned tag) {
	unsigned *u = (unsigned *)buf;
	size_t z = len / sizeof(*u);
	size_t x;
	for (x = 0; x < z; ++x) {
		if (u[x] != (tag + x)) {
			return (x * sizeof(*u)) + 1;
		}
	}
	for (x = z * sizeof(*u); x < len; ++x) {
		if (buf[x] != (x & (sizeof(*u) - 1))) {
			return x + 1;
		}
	}
	return 0;
}

void fill_buf(char *buf, size_t len, unsigned tag) {
	unsigned *u = (unsigned *)buf;
	size_t z = len / sizeof(*u);
	size_t x;
	for (x = 0; x < z; ++x) {
		u[x] = (tag + x);
	}
	for (x = z * sizeof(*u); x < len; ++x) {
		buf[x] = x & (sizeof(*u) - 1);
	}
#if 0
	if (check_buf(buf, len, tag)) {
		fprintf(stderr, "fill_buf/check_buf failed\n");
		abort();
	}
#endif
}

static unsigned curr_tag = 0;

/* --------------------------------------------------------------- */

void latency_senddone (xmi_context_t context, void *clientdata, xmi_result_t err)
{
	sendflag = 0;
	TRACE_ERR(stderr, "Send Done\n");
}

/* --------------------------------------------------------------- */

void latency_recvdone (xmi_context_t context, void *clientdata, xmi_result_t err)
{
	recvflag = 0;
	TRACE_ERR(stderr, "Recv Done\n");
}


/* --------------------------------------------------------------- */

void cb_recv_new(xmi_context_t context, void *clientdata,
			void *header_addr, size_t header_size,
			void *pipe_addr, size_t pipe_size,
			xmi_recv_t *recv) {
	if (pipe_addr != NULL) {
		memcpy(rbuf, pipe_addr, pipe_size);
		recvflag = 0;
	} else {
		//recv->
	}
}

int setup_netw(int netpro, xmi_context_t context, size_t *dispatch_id,
		xmi_dispatch_p2p_fn cb_recv = cb_recv_new,
		void *cb_recv_cd = NULL) {
	xmi_dispatch_callback_fn fn;
	fn.p2p = cb_recv;
	int net = netpro & 0x0000ffff;
	int pro = netpro >> 16;
	// TBD: figure out which type of send we want, and then tell XMI...
	*dispatch_id = 0;

	xmi_result_t e = XMI_Dispatch_set(context, *dispatch_id, fn, cb_recv_cd,
					(xmi_send_hint_t){0});
	return e;
}

#if 0 // finish porting later... if needed...
void send(XMI_Protocol_t *proto, char *buf, size_t sndlen, size_t targetrank, DCMF_Consistency consistency) {
	XMIQuad msginfo;
	XMI_Request_t sender;

	XMI_Callback_t cb_info = { latency_senddone, (void *)0 };

	CRITICAL_SECTION_ENTER;
	DCMF_Send(proto, &sender, cb_info, consistency, targetrank,
					sndlen, buf, &msginfo, 1);

	while (sendflag > 0) DCMF_Messager_advance();

	CRITICAL_SECTION_EXIT;
	sendflag = 1;
}

int recv_err = 0, send_err = 0;

void recv(xmi_context_t context, size_t dispatch_id, size_t bytes, bool verify) {
	XMI_Context_lock(context);
	while (recvflag > 0) XMI_Context_advance(context);
	TRACE_ERR(stderr, "After advance\n");
	XMI_Context_unlock(context);
	recvflag = 1;
	int x;
	if (verify && (x = check_buf(rbuf, bytes, curr_tag))) {
		if (++recv_err < 5) {
			printf("%zd: recv corrupted (%zd bytes) @%d [%08x : %08x]\n", DCMF_Messager_rank(), bytes, x, curr_tag, *((unsigned *)&rbuf[x]));
		}
	}
}

int test(xmi_context_t context, size_t dispatch_id, xmi_task_t rank,
		xmi_task_t originrank, xmi_task_t targetrank,
		unsigned *times, bool verify) {
	int i, niter = ITERATIONS, tt = 0;
	size_t sndlen;

	recv_err = send_err = 0;

	static bool init = true;
	if (init) {
		// Configure and register barriers
		barrier_init();
		init = false;
	}
	barrier();

	curr_tag = (((originrank << 8) | targetrank) << 16);
	sendflag = recvflag = 1;
	// Warm-up...
	if (rank == originrank) {
		send(context, dispatch_id, sbuf, 100, targetrank);
		recv(context, dispatch_id, 100, false);
	} else if (rank == targetrank) {
		recv(context, dispatch_id, 100, false);
		send(context, dispatch_id, sbuf, 100, originrank);
	} else {
		barrier();
		return 0;
	}

	for (sndlen = MINBUFSIZE; sndlen <= BUFSIZE; sndlen = NEXT_BUFSIZE(sndlen)) {
		++curr_tag;
		fill_buf(sbuf, sndlen, curr_tag);
		unsigned long long t1 = XMI_Wtimebase();
		for (i = 0; i < niter; i++) {
			if (rank == originrank) {
				TRACE_ERR(stderr, "Starting Iteration %d of size %d\n", i, sndlen);
			}

			if (rank == originrank) {
				send(context, dispatch_id, sbuf, sndlen, targetrank);
				recv(context, dispatch_id, sndlen, verify);
			} else if (rank == targetrank) {
				recv(context, dispatch_id, sndlen, verify);
				send(context, dispatch_id, sbuf, sndlen, originrank);
			}
		}
		unsigned long long t2 = XMI_Wtimebase();
		unsigned t = (unsigned)((t2 - t1) / 2 / niter);
		if (times) {
			times[tt++] = t;
		}
		if (verify && check_buf(sbuf, sndlen, curr_tag)) {
			if (++send_err < 5) {
				printf("%zd: send sbuf corrupted (%zd bytes)\n", rank, sndlen);
			}
		}
	}

	barrier();
	if (recv_err || send_err) {
		fprintf(stderr, "%zd saw %d recv buffer errors and %d send buffer errors\n",
			rank, recv_err, send_err);
	}
	return 0;
}
#endif

#endif
