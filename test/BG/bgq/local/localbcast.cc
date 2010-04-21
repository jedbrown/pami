#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "hwi/include/bqc/A2_inlines.h"

#define Timebase()	GetTimeBase()

#define PUSH		1
#define PULL		2
#define PUSHPULL	3

#define DTYPE_DOUBLE	1

#if DTYPE_DOUBLE
typedef double data_t;
#else
typedef long data_t;
#endif

#ifndef NTHREADS
#define NTHREADS 8
#endif // ! NTHREADS

#ifndef BUFCNT
#define BUFCNT	(1024)
#endif // ! BUFCNT

#ifndef NITER
#define NITER	1000
#endif // ! NITER

#ifndef TEST_TYPE
#define TEST_TYPE	PUSH
#endif // ! TEST_TYPE

#define WARMUP 2

// use double / QPX regardless of datatype - assume size and alignment works...
void push_memcpy(void **dst, size_t ndst, void *src, size_t len) {
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
#if NTHREADS == 16
	register double *d7 = (double *)dst[7];
	register double *d8 = (double *)dst[8];
	register double *d9 = (double *)dst[9];
	register double *d10 = (double *)dst[10];
	register double *d11 = (double *)dst[11];
	register double *d12 = (double *)dst[12];
	register double *d13 = (double *)dst[13];
	register double *d14 = (double *)dst[14];
#endif // NTHREADS == 16
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
#if NTHREADS == 16
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
#endif // NTHREADS == 16
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
};

struct pushpull_bcast {
	volatile size_t done;
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
		size_t n = r->participant - (r->participant > r->root ? 1 : 0);
		pr->dests[n] = r->dest;
	}
	__sync_fetch_and_add(&pr->done, 1);
	while (pr->done < NTHREADS);
	if (r->root == r->participant) {
		push_memcpy((void **)pr->dests, NTHREADS - 1,
				r->source, r->count * sizeof(data_t));
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
	} else {
		size_t n = r->participant - (r->participant > r->root ? 1 : 0);
		pr->dests[n] = r->dest;
	}

	__sync_fetch_and_add(&pr->done, 1);
	while (pr->done < NTHREADS);

	pp.src = pr->src + off;
	for (x = 0; x < NTHREADS - 1; ++x) {
		pp.dests[x] = pr->dests[x] + off;
	}
	asm volatile ("msync" ::: "memory");
	push_memcpy((void **)pp.dests, NTHREADS - 1, pp.src, len * sizeof(data_t));

	if (r->root == r->participant) {
		pr->done = 0;
	} else {
		while (pr->done);
	}

	return NULL;
}

struct thread thr[NTHREADS];
unsigned long long min;
unsigned long long max;
double sum;
const char *name;

void *do_bcasts(void *v) {
	struct thread *t = (struct thread *)v;
	struct bcast b;
	int x;

	posix_memalign((void **)&b.source, 4 * sizeof(double), BUFCNT * sizeof(data_t));
	posix_memalign((void **)&b.dest, 4 * sizeof(double), BUFCNT * sizeof(data_t));
	b.count = BUFCNT;
	b.root = 0;
	b.participant = t->id;

	for (x = 0; x < BUFCNT * sizeof(data_t); ++x) {
		((char *)b.source)[x] = x & 0x0ff;
	}
	memset((void *)b.dest, -1, BUFCNT * sizeof(data_t));
	fprintf(stderr, "Thread %2d starting... %p %p\n", t->id, b.source, b.dest);

	b.cookie = &push;
	for (x = 0; x < WARMUP; ++x) {
		push_bcast(&b); // use specified test instead?
	}

#if TEST_TYPE == PUSH
	if (t->id == 0) name = "push";
	t->t = Timebase();
	b.cookie = &push;
	for (x = 0; x < NITER; ++x) {
		push_bcast(&b);
	}
	t->t = (Timebase() - t->t) / NITER;
#endif // TEST_TYPE == PUSH

#if TEST_TYPE == PULL
	if (t->id == 0) name = "pull";
	t->t = Timebase();
	b.cookie = &pull;
	for (x = 0; x < NITER; ++x) {
		pull_bcast(&b);
	}
	t->t = (Timebase() - t->t) / NITER;
#endif // TEST_TYPE == PULL

#if TEST_TYPE == PUSHPULL
	if (t->id == 0) name = "pushpull";
	t->t = Timebase();
	b.cookie = &pushpull;
	for (x = 0; x < NITER; ++x) {
		pushpull_bcast(&b);
	}
	t->t = (Timebase() - t->t) / NITER;
#endif // TEST_TYPE == PUSHPULL
	free(b.dest);
	free(b.source);
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
		sum += (double)thr[x].t;
		if (min > thr[x].t) min = thr[x].t;
		if (max < thr[x].t) max = thr[x].t;
	}
	printf("bcast %s algorithm, %d threads, %d bytes, %d iterations\n", name, NTHREADS, BUFCNT * sizeof(data_t), NITER);
	printf("%16u %16g %16u (min, avg, max)\n", min, sum / NTHREADS, max);
}
