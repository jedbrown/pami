/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file cm_impl.h
 * \brief Common external collectives message layer types.
 */
#ifndef __cm_impl_h__
#define __cm_impl_h__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define DEPRECATED_MULTICAST
#define DEPRECATED_MANYTOMANY

/**
 * \todo CNK's abort() does not core dump...
 *
 * \todo make the _debug versions do something only if DEBUG compiles
 */
#define CM_abort()		abort()
/* These "f" versions support a printf format string and args. */
#define CM_abortf(fmt...)	{ fprintf(stderr, __FILE__ ":%d: ", __LINE__); fprintf(stderr, fmt); abort(); }

#if ASSERT_LEVEL==0

/* No asserts at all */
#define CM_assert(expr)
#define CM_assertf(expr, fmt...)
#define CM_assert_debug(expr)
#define CM_assert_debugf(expr, fmt...)

#elif ASSERT_LEVEL==1

/* No debug asserts, only "normal" ones */
#define CM_assert(expr)			assert(expr)
#define CM_assertf(expr, fmt...)	{ if (!(expr)) CM_abortf(fmt); }
#define CM_assert_debug(expr)
#define CM_assert_debugf(expr, fmt...)

#else /* ASSERT_LEVEL==2 */

/* all asserts */
#define CM_assert(expr)			assert(expr)
#define CM_assertf(expr, fmt...)	{ if (!(expr)) CM_abortf(fmt); }
#define CM_assert_debug(expr)		CM_assert(expr)
#define CM_assert_debugf(expr, fmt...)	CM_assertf(expr, fmt)

#endif /* ASSERT_LEVEL */

#ifdef __bgp__
#define CM_PROTOCOL_NQUADS	48
#define CM_REQUEST_NQUADS	32
#define CM_ERROR_NQUADS		2
#elif defined(__bgq__)
#define CM_PROTOCOL_NQUADS	96
#define CM_REQUEST_NQUADS	64
#define CM_ERROR_NQUADS		2
#else
#define CM_PROTOCOL_NQUADS	32
#define CM_REQUEST_NQUADS	32
#define CM_ERROR_NQUADS		2
#endif

#ifdef __cplusplus
};
#endif

#endif // __cm_impl_h__
