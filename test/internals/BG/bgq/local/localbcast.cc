/**
 * \file test/internals/BG/bgq/local/localbcast.cc
 * \brief ???
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "hwi/include/bqc/A2_inlines.h"

//#define DTYPE_DOUBLE  0 // not yet supporting longs...

#include "local_coll.h"

#if TEST_TYPE == PUSH
#define FUNC    push_bcast
#define STRUCT  push
#define NAME    "push"
#endif // TEST_TYPE == PUSH

#if TEST_TYPE == PULL
#define FUNC    pull_bcast
#define STRUCT  pull
#define NAME    "pull"
#endif // TEST_TYPE == PULL

#if TEST_TYPE == PUSHPULL
#define FUNC    pushpull_bcast
#define STRUCT  pushpull
#define NAME    "pushpull"
#endif // TEST_TYPE == PUSHPULL

#if NTHREADS != 4 && NTHREADS != 8 && NTHREADS != 16 && NTHREADS != 64
#error Requires NTHREADS to be 4, 8, or 16
#endif
// use double / QPX regardless of datatype - assume size and alignment works...
void qpx_memcast(void **dst, size_t ndst, void *src, size_t len) {
	// assert(ndst == NTHREADS - 1);
	register double *s = (double *)src;
	register double *d0 = (double *)dst[0];
	register double *d1 = (double *)dst[1];
	register double *d2 = (double *)dst[2];
#if NTHREADS >= 8
	register double *d3 = (double *)dst[3];
	register double *d4 = (double *)dst[4];
	register double *d5 = (double *)dst[5];
	register double *d6 = (double *)dst[6];
#if NTHREADS >= 16
	register double *d7 = (double *)dst[7];
	register double *d8 = (double *)dst[8];
	register double *d9 = (double *)dst[9];
	register double *d10 = (double *)dst[10];
	register double *d11 = (double *)dst[11];
	register double *d12 = (double *)dst[12];
	register double *d13 = (double *)dst[13];
	register double *d14 = (double *)dst[14];
#if NTHREADS > 16
	register double *d15 = (double *)dst[15];
#endif // NTHREADS > 16
#endif // NTHREADS >= 16
#endif // NTHREADS >= 8
	size_t idx = 0;
	size_t z = len / sizeof(double);
	while (z) {
		asm volatile(
			"qvlfdx 0, %0, %1;"
			"qvstfdx 0, %2, %1;"
			"qvstfdx 0, %3, %1;"
			"qvstfdx 0, %4, %1;"
#if NTHREADS >= 8
			"qvstfdx 0, %5, %1;"
			"qvstfdx 0, %6, %1;"
			"qvstfdx 0, %7, %1;"
			"qvstfdx 0, %8, %1;"
#endif // NTHREADS >= 8
			:  "+b" (s)
			  ,"+r" (idx)
			  ,"+b" (d0)
			  ,"+b" (d1)
			  ,"+b" (d2)
#if NTHREADS >= 8
			  ,"+b" (d3)
			  ,"+b" (d4)
			  ,"+b" (d5)
			  ,"+b" (d6)
			);
#if NTHREADS >= 16
		asm volatile(
			"qvstfdx 0, %1, %0;"
			"qvstfdx 0, %2, %0;"
			"qvstfdx 0, %3, %0;"
			"qvstfdx 0, %4, %0;"
			"qvstfdx 0, %5, %0;"
			"qvstfdx 0, %6, %0;"
			"qvstfdx 0, %7, %0;"
			"qvstfdx 0, %8, %0;"
			:  "+r" (idx)
			  ,"+b" (d7)
			  ,"+b" (d8)
			  ,"+b" (d9)
			  ,"+b" (d10)
			  ,"+b" (d11)
			  ,"+b" (d12)
			  ,"+b" (d13)
			  ,"+b" (d14)
			);
#if NTHREADS > 16
		asm volatile(
			"qvstfdx 0, %1, %0;"
			:  "+r" (idx)
			  ,"+b" (d15)
			);
#endif // NTHREADS > 16
#endif // NTHREADS >= 16
#endif // NTHREADS >= 8
		idx += 4 * sizeof(double);
		z -= 4;
	}
}

struct thread {
	size_t id;
	pthread_t thr;
	unsigned long long t;
};

struct bcast {
	data_t *source;
	data_t *dest;
	size_t count;
	size_t root;
	size_t participant;
	void *cookie;
	size_t iter;
};

struct pushpull_bcast {
	volatile size_t done[2];
	data_t *src;
	data_t *dests[NTHREADS];
};

struct push_bcast {
	volatile size_t done;
	data_t *dests[NTHREADS];
};

struct pull_bcast {
	volatile size_t done;
	data_t *src;
};

// a la shmem...
struct push_bcast push;
struct pull_bcast pull;
struct pushpull_bcast pushpull;

// only root copies, to all buffers at once
void *push_bcast(void *v) {
	struct bcast *r = (struct bcast *)v;
	struct push_bcast *pr = (struct push_bcast *)r->cookie;
	if (r->root != r->participant) {
#if NTHREADS > 16
		size_t n = r->participant;
#else // !(NTHREADS > 16)
		size_t n = r->participant - (r->participant > r->root ? 1 : 0);
#endif // !(NTHREADS > 16)
		pr->dests[n] = r->dest;
#if NTHREADS > 16
	} else {
		pr->dests[r->participant] = r->dest; // need some dummy destination
#endif // NTHREADS > 16
	}
	__sync_fetch_and_add(&pr->done, 1);
	while (pr->done < NTHREADS);
	if (r->root == r->participant) {
#if NTHREADS > 16
		size_t x;
		for (x = 0; x < NTHREADS; x += 16) {
			qpx_memcast((void **)&pr->dests[x], 16, r->source, r->count * sizeof(data_t));
		}
#else // !(NTHREADS > 16)
		qpx_memcast((void **)pr->dests, NTHREADS - 1,
				r->source, r->count * sizeof(data_t));
#endif // !(NTHREADS > 16)
		pr->done = 0;
	} else {
		while (pr->done);
	}
	return NULL;
}

// everybody (except root) copies to their own buffer
void *pull_bcast(void *v) {
	struct bcast *r = (struct bcast *)v;
	struct pull_bcast *pr = (struct pull_bcast *)r->cookie;
	if (r->root == r->participant) {
		pr->src = r->source;
		pr->done = NTHREADS - 1;
		while (pr->done > 0);
	} else {
		while (!pr->done);
		memcpy(r->dest, pr->src, r->count * sizeof(data_t));
		__sync_fetch_and_add(&pr->done, -1);
		while (pr->done > 0);
	}
	return NULL;
}

// all copy a slice, each slice to all destinations at once
void *pushpull_bcast(void *v) {
	struct bcast *r = (struct bcast *)v;
	struct pushpull_bcast *pr = (struct pushpull_bcast *)r->cookie;
	struct pushpull_bcast pp;
	size_t x, off, len;
	len = r->count / NTHREADS;
	off = r->participant * len;
	if (r->root == r->participant) {
		pr->src = r->source;
#if NTHREADS > 16
		pr->dests[r->participant] = r->dest; // need some dummy destination
#endif // NTHREADS > 16
	} else {
#if NTHREADS > 16
		size_t n = r->participant;
#else // !(NTHREADS > 16)
		size_t n = r->participant - (r->participant > r->root ? 1 : 0);
#endif // !(NTHREADS > 16)
		pr->dests[n] = r->dest;
	}

	int p = r->iter & 1;
	__sync_fetch_and_add(&pr->done[p], 1);
	while (pr->done[p] < NTHREADS);

	pp.src = pr->src + off;
#if NTHREADS > 16
	for (x = 0; x < NTHREADS; ++x) {
		pp.dests[x] = pr->dests[x] + off;
	}
	asm volatile ("msync" ::: "memory");
	for (x = 0; x < NTHREADS; x += 16) {
		qpx_memcast((void **)&pp.dests[x], 16, pp.src, len * sizeof(data_t));
	}
#else // !(NTHREADS > 16)
	for (x = 0; x < NTHREADS - 1; ++x) {
		pp.dests[x] = pr->dests[x] + off;
	}
	asm volatile ("msync" ::: "memory");
	qpx_memcast((void **)pp.dests, NTHREADS - 1, pp.src, len * sizeof(data_t));
#endif // !(NTHREADS > 16)

	if (r->root == r->participant) {
		pr->done[1 - p] = 0;
	}
	__sync_fetch_and_add(&pr->done[p], 1);
	while (pr->done[p] < 2 * NTHREADS);

	return NULL;
}

struct thread thr[NTHREADS];
unsigned long long min;
unsigned long long max;
double sum;
const char *name = NAME;
volatile size_t barrier1 = 0;

void *do_bcasts(void *v) {
	struct thread *t = (struct thread *)v;
	struct bcast b;
	void *buf1, *buf2;
	int x;

	posix_memalign(&buf1, BUF_ALIGN, BUFCNT * sizeof(data_t) + BUF_PAD);
	posix_memalign(&buf2, BUF_ALIGN, BUFCNT * sizeof(data_t) + BUF_PAD);
	BUF_STAGGER(b.source, buf1);
	BUF_STAGGER(b.dest, buf2);
	b.count = BUFCNT;
	b.root = 0;
	b.participant = t->id;

	for (x = 0; x < BUFCNT * sizeof(data_t); ++x) {
		((unsigned char *)b.source)[x] = x & 0x0ff;
	}
	memset((void *)b.dest, -1, BUFCNT * sizeof(data_t));
	fprintf(stderr, "Thread %2d starting... %p %p (%zd %zd)\n", t->id,
		b.source, b.dest,
		L2SLICE(b.source), L2SLICE(b.dest));
	__sync_fetch_and_add(&barrier1, 1);
	while (barrier1 < NTHREADS);

	b.cookie = &STRUCT;
#ifndef VERIFY
	for (b.iter = 0; b.iter < WARMUP; ++b.iter) {
		FUNC(&b);
	}

	t->t = Timebase();
	for (; b.iter < WARMUP + NITER; ++b.iter) {
		FUNC(&b);
	}
	t->t = (Timebase() - t->t) / NITER;
#else // VERIFY
	size_t e = 0;
	FUNC(&b);
	if (b.participant == b.root) {
#if !(NTHREADS > 16)
		for (x = 0; x < BUFCNT * sizeof(data_t); ++x) {
			if (((unsigned char *)b.dest)[x] != 0x0ff) {
				++e;
#if VERBOSE
				if (e <= (unsigned)VERBOSE) fprintf(stderr, "Error at byte dest[%d]: %u expected %u\n", x, ((unsigned char *)b.dest)[x], 0x0ff);
#endif // VERBOSE
			}
		}
#endif // !(NTHREADS > 16)
	} else {
		for (x = 0; x < BUFCNT * sizeof(data_t); ++x) {
			if (((unsigned char *)b.dest)[x] != (x & 0x0ff)) {
				++e;
#if VERBOSE
				if (e <= (unsigned)VERBOSE) fprintf(stderr, "Error at byte dest[%d]: %u expected %u\n", x, ((unsigned char *)b.dest)[x], x & 0x0ff);
#endif // VERBOSE
			}
		}
	}
	for (x = 0; x < BUFCNT * sizeof(data_t); ++x) {
		if (((unsigned char *)b.source)[x] != (x & 0x0ff)) {
			++e;
#if VERBOSE
			if (e <= (unsigned)VERBOSE) fprintf(stderr, "Error at byte source[%d]: %u expected %u\n", x, ((unsigned char *)b.source)[x], x & 0x0ff);
#endif // VERBOSE
		}
	}
	t->t = e;
#endif // VERIFY

	free(buf1);
	free(buf2);
	return NULL;
}

int main(int argc, char **argv) {
	int x;

	memset(&push, 0, sizeof(push));
	memset(&pull, 0, sizeof(pull));
	memset(&pushpull, 0, sizeof(pushpull));
	memset(thr, 0, sizeof(thr));
	min = (unsigned)-1;
	max = 0;
	sum = 0.0;
	fprintf(stderr, "Main starting threads...\n");
	asm volatile ("msync" ::: "memory");

	for (x = 0; x < NTHREADS; ++x) {
		thr[x].id = x;
		if (x > 0) {
			pthread_create(&thr[x].thr, NULL, do_bcasts, &thr[x]);
		}
	}
	do_bcasts(&thr[0]);

	for (x = 0; x < NTHREADS; ++x) {
		void *status;
		if (x > 0) {
			pthread_join(thr[x].thr, &status);
		}
#ifndef VERIFY
		sum += (double)thr[x].t;
		if (min > thr[x].t) min = thr[x].t;
		if (max < thr[x].t) max = thr[x].t;
#else // VERIFY
		max += thr[x].t;
#endif // VERIFY
	}
	printf("bcast %s algorithm, %d threads, %d bytes, %d iterations\n", name, NTHREADS, BUFCNT * sizeof(data_t), NITER);
#ifndef VERIFY
	double b = BUFCNT * sizeof(data_t) * (NTHREADS - 1);
	double d = sum / NTHREADS;
	printf("%16u %16g %16u (min, avg, max) (%g b/c)\n", min, d, max, b / d);
#else // VERIFY
	if (!max) printf("No errors\n");
	else printf("%d errors over all threads\n", max);
#endif // VERIFY
}
