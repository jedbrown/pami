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
#define TEST_TYPE	PULL
#endif // ! TEST_TYPE

#define WARMUP 2

void qpx_memcomb(void *dst, void **srcs, size_t nsrc, size_t len) {
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
#if NTHREADS == 16
	register double *s8 = (double *)srcs[8];
	register double *s9 = (double *)srcs[9];
	register double *s10 = (double *)srcs[10];
	register double *s11 = (double *)srcs[11];
	register double *s12 = (double *)srcs[12];
	register double *s13 = (double *)srcs[13];
	register double *s14 = (double *)srcs[14];
	register double *s15 = (double *)srcs[15];
#endif // NTHREADS == 16
#endif // NTHREADS >= 8
	size_t idx = 0;
	size_t z = len / sizeof(double);
	while (z) {
		asm volatile(
			"qvlfdx 0, %1, %0;"
			"qvlfdx 1, %2, %0;"
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
#if NTHREADS == 16
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
#endif // NTHREADS == 16
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
};

struct pushpull_reduce {
	volatile size_t done;
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

		memcomb(r->dest, (void **)pr->srcs, NTHREADS, r->count * sizeof(data_t));

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
	len = r->count / NTHREADS;
	off = r->participant * len;
	if (r->root == r->participant) {
		pr->dst = r->dest;
	}
	size_t n = r->participant;
	pr->srcs[n] = r->source;

	__sync_fetch_and_add(&pr->done, 1);
	while (pr->done < NTHREADS);

	pp.dst = pr->dst + off;
	for (x = 0; x < NTHREADS; ++x) {
		pp.srcs[x] = pr->srcs[x] + off;
	}
	asm volatile ("msync" ::: "memory");
	memcomb(pp.dst, (void **)pp.srcs, NTHREADS, len);

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

void *do_reduces(void *v) {
	struct thread *t = (struct thread *)v;
	struct reduce r;
	int x;

	posix_memalign((void **)&r.source, 4 * sizeof(double), BUFCNT * sizeof(data_t));
	posix_memalign((void **)&r.dest, 4 * sizeof(double), BUFCNT * sizeof(data_t));
	r.count = BUFCNT;
	r.root = 0;
	r.participant = t->id;

	for (x = 0; x < BUFCNT; ++x) {
		r.source[x] = (data_t)x + 1;
	}
	memset((void *)r.dest, 0, BUFCNT * sizeof(data_t));
	fprintf(stderr, "Thread %2d starting... %p %p\n", t->id, r.source, r.dest);

	r.cookie = &pull;
	for (x = 0; x < WARMUP; ++x) {
		pull_reduce(&r); // use specified test instead?
	}

#if TEST_TYPE == PUSH
#error no PUSH algorithm (yet?)
#endif // TEST_TYPE == PUSH

#if TEST_TYPE == PULL
	if (t->id == 0) name = "pull";
	t->t = Timebase();
	r.cookie = &pull;
	for (x = 0; x < NITER; ++x) {
		pull_reduce(&r);
	}
	t->t = (Timebase() - t->t) / NITER;
#endif // TEST_TYPE == PULL

#if TEST_TYPE == PUSHPULL
	if (t->id == 0) name = "pushpull";
	t->t = Timebase();
	r.cookie = &pushpull;
	for (x = 0; x < NITER; ++x) {
		pushpull_reduce(&r);
	}
	t->t = (Timebase() - t->t) / NITER;
#endif // TEST_TYPE == PUSHPULL
	free(r.dest);
	free(r.source);
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
		sum += (double)thr[x].t;
		if (min > thr[x].t) min = thr[x].t;
		if (max < thr[x].t) max = thr[x].t;
	}
	printf("reduce %s algorithm, %d threads, %d bytes, %d iterations\n", name, NTHREADS, BUFCNT * sizeof(data_t), NITER);
	printf("%16u %16g %16u (min, avg, max)\n", min, sum / NTHREADS, max);
}
