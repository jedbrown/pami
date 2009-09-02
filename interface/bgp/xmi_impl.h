/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file interface/bgp/xmi_impl.h
 * \brief Common external collectives message layer types.
 */
#ifndef __interface_bgp_xmi_impl_h__
#define __interface_bgp_xmi_impl_h__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define DEPRECATED_MULTICAST
#define DEPRECATED_MANYTOMANY

#if defined(__bgp__)
#define XMI_MAX_DIMS  4
#define XMI_MAX_TORUS_DIMS  3
#elif defined(__bgq__)
#define XMI_MAX_DIMS  7
#define XMI_MAX_TORUS_DIMS  5
#else
#define XMI_MAX_DIMS  3		// ?
#define XMI_MAX_TORUS_DIMS  3	// ?
#endif

/**
 * \todo CNK's abort() does not core dump...
 *
 * \todo make the _debug versions do something only if DEBUG compiles
 */
#define XMI_abort()		abort()
/* These "f" versions support a printf format string and args. */
#define XMI_abortf(fmt...)	{ fprintf(stderr, __FILE__ ":%d: ", __LINE__); fprintf(stderr, fmt); abort(); }

#if ASSERT_LEVEL==0

/* No asserts at all */
#define XMI_assert(expr)
#define XMI_assertf(expr, fmt...)
#define XMI_assert_debug(expr)
#define XMI_assert_debugf(expr, fmt...)

#elif ASSERT_LEVEL==1

/* No debug asserts, only "normal" ones */
#define XMI_assert(expr)			assert(expr)
#define XMI_assertf(expr, fmt...)	{ if (!(expr)) XMI_abortf(fmt); }
#define XMI_assert_debug(expr)
#define XMI_assert_debugf(expr, fmt...)

#else /* ASSERT_LEVEL==2 */

/* all asserts */
#define XMI_assert(expr)			assert(expr)
#define XMI_assertf(expr, fmt...)	{ if (!(expr)) XMI_abortf(fmt); }
#define XMI_assert_debug(expr)		XMI_assert(expr)
#define XMI_assert_debugf(expr, fmt...)	XMI_assertf(expr, fmt)

#endif /* ASSERT_LEVEL */

#ifdef __bgp__
#define XMI_PROTOCOL_NQUADS	48
#define XMI_REQUEST_NQUADS	32
#define XMI_ERROR_NQUADS		2
#elif defined(__bgq__)
#define XMI_PROTOCOL_NQUADS	96
#define XMI_REQUEST_NQUADS	64
#define XMI_ERROR_NQUADS		2
#else
#define XMI_PROTOCOL_NQUADS	32
#define XMI_REQUEST_NQUADS	32
#define XMI_ERROR_NQUADS		2
#endif

#ifdef __cplusplus
};
#endif

#endif // __interface_bgp_xmi_impl_h__
