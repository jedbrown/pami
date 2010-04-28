#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "hwi/include/bqc/A2_inlines.h"

//#define DTYPE_DOUBLE	0 // not yet supporting longs...

#include "local_coll.h"

#if TEST_TYPE == PUSH
#error no PUSH reduce algorithm (yet?)
#endif // TEST_TYPE == PUSH

#if TEST_TYPE == PULL
#define FUNC	pull_reduce
#define STRUCT	pull
#define NAME	"pull"
#endif // TEST_TYPE == PULL

#if TEST_TYPE == PUSHPULL
#define FUNC	pushpull_reduce
#define STRUCT	pushpull
#define NAME	"pushpull"
#endif // TEST_TYPE == PUSHPULL

#if NTHREADS != 4 && NTHREADS != 8 && NTHREADS != 16 && NTHREADS != 64
#error Requires NTHREADS to be 4, 8, or 16
#endif
void qpx_memcomb(void *dst, void **srcs, size_t nsrc, size_t len
#if NTHREADS > 16
						,double *continuation
#endif // NTHREADS > 16
								) {
	register size_t z;
	// assert(nsrc == NTHREADS);
	register double *d = (double *)dst;
	register double *s0 = (double *)srcs[0];
	register double *s1 = (double *)srcs[1];
	register double *s2 = (double *)srcs[2];
	register double *s3 = (double *)srcs[3];
#if NTHREADS >= 8
	register double *s4 = (double *)srcs[4];
	register double *s5 = (double *)srcs[5];
	register double *s6 = (double *)srcs[6];
	register double *s7 = (double *)srcs[7];
#if NTHREADS >= 16
	register double *s8 = (double *)srcs[8];
	register double *s9 = (double *)srcs[9];
	register double *s10 = (double *)srcs[10];
	register double *s11 = (double *)srcs[11];
	register double *s12 = (double *)srcs[12];
	register double *s13 = (double *)srcs[13];
	register double *s14 = (double *)srcs[14];
	register double *s15 = (double *)srcs[15];
#if NTHREADS > 16
	z = 0;
	asm volatile(
		"qvlfdx 5, %0, %2;"
		"qvlfdx 6, %1, %2;"
		::  "b" (qpx_zero)
		  ,"b" (continuation)
		  ,"r" (z)
		);
#endif // NTHREADS > 16
#endif // NTHREADS >= 16
#endif // NTHREADS >= 8
	size_t idx = 0;
	z = len / sizeof(double);
	while (z) {
#if NTHREADS > 16
		asm volatile(
			"qvlfdx 7, %0, %1;"
			:  "+b" (d)
			  ,"+r" (idx)
			);
#endif // NTHREADS > 16
		asm volatile(
			"qvlfdx 0, %1, %0;"
#if NTHREADS > 16
			"qvfsel 4, 6, 7, 5;"
#endif // NTHREADS > 16
			"qvlfdx 1, %2, %0;"
#if NTHREADS > 16
			"qvfadd 0, 0, 4;"
#endif // NTHREADS > 16
			"qvlfdx 2, %3, %0;"
			"qvfadd 0, 0, 1;"
			"qvlfdx 4, %4, %0;"
			"qvfadd 0, 0, 2;"
			:  "+r" (idx)
			  ,"+b" (s0)
			  ,"+b" (s1)
			  ,"+b" (s2)
			  ,"+b" (s3)
			);
#if NTHREADS >= 8
		asm volatile(
			"qvlfdx 1, %1, %0;"
			"qvfadd 0, 0, 4;"
			"qvlfdx 2, %2, %0;"
			"qvfadd 0, 0, 1;"
			"qvlfdx 3, %3, %0;"
			"qvfadd 0, 0, 2;"
			"qvlfdx 4, %4, %0;"
			"qvfadd 0, 0, 3;"
			:  "+r" (idx)
			  ,"+b" (s4)
			  ,"+b" (s5)
			  ,"+b" (s6)
			  ,"+b" (s7)
			);
#if NTHREADS >= 16
		asm volatile(
			"qvlfdx 2, %1, %0;"
			"qvfadd 0, 0, 4;"
			"qvlfdx 3, %2, %0;"
			"qvfadd 0, 0, 2;"
			"qvlfdx 1, %3, %0;"
			"qvfadd 0, 0, 3;"
			"qvlfdx 2, %4, %0;"
			"qvfadd 0, 0, 1;"
			"qvlfdx 3, %5, %0;"
			"qvfadd 0, 0, 2;"
			"qvlfdx 1, %6, %0;"
			"qvfadd 0, 0, 3;"
			"qvlfdx 2, %7, %0;"
			"qvfadd 0, 0, 1;"
			"qvlfdx 4, %8, %0;"
			"qvfadd 0, 0, 2;"
			:  "+r" (idx)
			  ,"+b" (s8)
			  ,"+b" (s9)
			  ,"+b" (s10)
			  ,"+b" (s11)
			  ,"+b" (s12)
			  ,"+b" (s13)
			  ,"+b" (s14)
			  ,"+b" (s15)
			);
#endif // NTHREADS >= 16
#endif // NTHREADS >= 8
		asm volatile(
			"qvfadd 0, 0, 4;"
			"qvstfdx 0, %0, %1;"
			:  "+b" (d)
			  ,"+r" (idx)
			);
		idx += 4 * sizeof(double);
		z -= 4;
	}
}

#if DTYPE_DOUBLE
#define memcomb		qpx_memcomb
#else
#define memcomb		long_memcomb
#endif

struct thread {
	size_t id;
	pthread_t thr;
	unsigned long long t;
};

struct reduce {
	data_t *source;
	data_t *dest;
	size_t count;
	size_t root;
	size_t participant;
	void *cookie;
	size_t iter;
};

struct pushpull_reduce {
	volatile size_t done[2];
	data_t *dst;
	data_t *srcs[NTHREADS];
};

struct push_reduce {
	volatile size_t done;
	data_t *dst;
};

struct pull_reduce {
	volatile size_t done;
	data_t *srcs[NTHREADS];
};

// a la shmem...
struct push_reduce push;
struct pull_reduce pull;
struct pushpull_reduce pushpull;

// only root does combine from all buffers
void *pull_reduce(void *v) {
	struct reduce *r = (struct reduce *)v;
	struct pull_reduce *pr = (struct pull_reduce *)r->cookie;
	if (r->root == r->participant) {
		pr->srcs[r->participant] = r->source;
		__sync_fetch_and_add(&pr->done, 1);
		while (pr->done < NTHREADS);

#if NTHREADS > 16
		size_t x;
		for (x = 0; x < NTHREADS; x += 16) {
			memcomb(r->dest, (void **)&pr->srcs[x], 16, r->count * sizeof(data_t),
					(x != 0 ? qpx_true : qpx_false));
		}
#else // !(NTHREADS > 16)
		memcomb(r->dest, (void **)pr->srcs, NTHREADS, r->count * sizeof(data_t));
#endif // !(NTHREADS > 16)

		pr->done = 0;
	} else {
		pr->srcs[r->participant] = r->source;
		__sync_fetch_and_add(&pr->done, 1);
		while (pr->done);
	}
	return NULL;
}

// all combine a slice, each slice from all sources at once
void *pushpull_reduce(void *v) {
	struct reduce *r = (struct reduce *)v;
	struct pushpull_reduce *pr = (struct pushpull_reduce *)r->cookie;
	struct pushpull_reduce pp;
	size_t x, off, len;
#if NTHREADS > 16
	// divide into groups of 16, each group must do (NTHREADS / 16) passes,
	// each pass with a different set of 16 sources but the same chunk of
	// the destination. Within the group, the work is parallelized by 16.
	// This means each thread is still doing 1/NTHREADS of the result, but
	// it must split up the work into separate calls to qpx_memcomb(),
	// to cover the NTHREADS/16 sets of 16 sources.
	// On all except the first pass, the intermediate value must be loaded
	// from the destination buffer.
#endif // NTHREADS > 16
	len = r->count / NTHREADS;
	off = r->participant * len;
	if (r->root == r->participant) {
		pr->dst = r->dest;
	}
	size_t n = r->participant;
	pr->srcs[n] = r->source;

	int p = r->iter & 1;
	__sync_fetch_and_add(&pr->done[p], 1);
	while (pr->done[p] < NTHREADS);

	pp.dst = pr->dst + off;
	for (x = 0; x < NTHREADS; ++x) {
		pp.srcs[x] = pr->srcs[x] + off;
	}
	asm volatile ("msync" ::: "memory"); // needed on every iter?
#if NTHREADS > 16
	for (x = 0; x < NTHREADS; x += 16) {
		memcomb(pp.dst, (void **)&pp.srcs[x], 16, len * sizeof(data_t),
				(x != 0 ? qpx_true : qpx_false));
	}
#else // !(NTHREADS > 16)
	memcomb(pp.dst, (void **)pp.srcs, NTHREADS, len * sizeof(data_t));
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

void *do_reduces(void *v) {
	struct thread *t = (struct thread *)v;
	struct reduce r;
	void *buf1, *buf2;
	int x;

	posix_memalign(&buf1, BUF_ALIGN, BUFCNT * sizeof(data_t) + BUF_PAD);
	posix_memalign(&buf2, BUF_ALIGN, BUFCNT * sizeof(data_t) + BUF_PAD);
	BUF_STAGGER(r.source, buf1);
	BUF_STAGGER(r.dest, buf2);
	r.count = BUFCNT;
	r.root = 0;
	r.participant = t->id;

	for (x = 0; x < BUFCNT; ++x) {
		r.source[x] = (data_t)x + 1;
#if DTYPE_DOUBLE
		r.dest[x] = -99999.99999;
#endif // DTYPE_DOUBLE
	}
#if ! DTYPE_DOUBLE
	memset((void *)r.dest, -1, BUFCNT * sizeof(data_t));
#endif // ! DTYPE_DOUBLE
	fprintf(stderr, "Thread %2d starting... %p %p (%zd %zd)\n", t->id,
		r.source, r.dest,
		L2SLICE(r.source), L2SLICE(r.dest));
	__sync_fetch_and_add(&barrier1, 1);
	while (barrier1 < NTHREADS);

	r.cookie = &STRUCT;
#ifndef VERIFY
	for (r.iter = 0; r.iter < WARMUP; ++r.iter) {
		FUNC(&r);
	}

	t->t = Timebase();
	for (; r.iter < WARMUP + NITER; ++r.iter) {
		FUNC(&r);
	}
	t->t = (Timebase() - t->t) / NITER;
#else // VERIFY
	size_t e = 0;
	FUNC(&r);
	if (r.participant == r.root) {
		for (x = 0; x < BUFCNT; ++x) {
			if (r.dest[x] != ((data_t)x + 1) * NTHREADS) {
				++e;
#if VERBOSE
				if (e <= (unsigned)VERBOSE) fprintf(stderr, "Error at dest[%d]: %g expected %g\n", x, r.dest[x], ((data_t)x + 1) * NTHREADS);
#endif // VERBOSE
			}
		}
	}
	for (x = 0; x < BUFCNT; ++x) {
		if (r.source[x] != (data_t)x + 1) {
			++e;
#if VERBOSE
			if (e <= (unsigned)VERBOSE) fprintf(stderr, "Error at source[%d]: %g expected %g\n", x, r.source[x], ((data_t)x + 1));
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
			pthread_create(&thr[x].thr, NULL, do_reduces, &thr[x]);
		}
	}
	do_reduces(&thr[0]);

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
	printf("reduce %s algorithm, %d threads, %d bytes, %d iterations\n", name, NTHREADS, BUFCNT * sizeof(data_t), NITER);
#ifndef VERIFY
	double b = BUFCNT * sizeof(data_t) * NTHREADS;
	double d = sum / NTHREADS;
	printf("%16u %16g %16u (min, avg, max) (%g b/c)\n", min, d, max, b / d);
#else // VERIFY
	if (!max) printf("No errors\n");
	else printf("%d errors over all threads\n", max);
#endif // VERIFY
}
