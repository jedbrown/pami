/**
 * \file test/internals/BG/bgq/local/local_coll.h
 * \brief ???
 */
#ifndef __test_internals_BG_bgq_local_local_coll_h__
#define __test_internals_BG_bgq_local_local_coll_h__

#include "spi/include/kernel/location.h"

#define Timebase()	GetTimeBase()

#define PUSH		1
#define PULL		2
#define PUSHPULL	3

#define QPX_ALIGN	(4 * sizeof(double))

#ifndef DTYPE_DOUBLE
// default to doubles... use "#define DTYPE_DOUBLE 0" before including this, to use long
#define DTYPE_DOUBLE	1
#endif // ! DTYPE_DOUBLE

#if DTYPE_DOUBLE
typedef double data_t;
typedef double qpx_vector_t[4] __attribute__((__aligned__(QPX_ALIGN)));
static qpx_vector_t qpx_true = { 1.0, 1.0, 1.0, 1.0 };
static qpx_vector_t qpx_false = { -1.0, -1.0, -1.0, -1.0 };
static qpx_vector_t qpx_zero = { 0.0, 0.0, 0.0, 0.0 };
#else
typedef long data_t;
#endif

#define L2SLICE_WIDTH	(128)
#define L2SLICE_COUNT	(16)
#define L2SLICE_SPAN	(L2SLICE_WIDTH * L2SLICE_COUNT)
#define L2SLICE(addr)	(((size_t)(addr) / L2SLICE_WIDTH) & (L2SLICE_COUNT - 1))

#if defined(BUF_L2ALIGN) || defined(BUF_L2STAGGER)

#define BUF_ALIGN	L2SLICE_SPAN
#define BUF_PAD		L2SLICE_SPAN
#ifdef BUF_L2STAGGER
#define BUF_STAGGER(buf,v)	{ buf = (data_t *)((char *)v + Kernel_PhysicalProcessorID() * L2SLICE_WIDTH); }
#else // ! BUF_L2STAGGER
#define BUF_STAGGER(buf,v)	{ buf = (data_t *)v; }
#endif // ! BUF_L2STAGGER

#else // ! BUF_L2ALIGN && ! BUF_L2STAGGER

#define BUF_STAGGER(buf,v)	{ buf = (data_t *)v; }
#define BUF_ALIGN	QPX_ALIGN
#define BUF_PAD		0

#endif // ! BUF_L2ALIGN && ! BUF_L2STAGGER

// currently, only 4, 8, or 16 threads
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

#ifndef VERBOSE
#define VERBOSE 0
#endif // !VERBOSE

#define WARMUP 2

#endif // __test_BG_bgq_local_local_coll_h__
