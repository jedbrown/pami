/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2005, 2007.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */

#ifndef __PGASRT_SMP_H_
#define __PGASRT_SMP_H_

#if defined(__cplusplus)
extern "C" {
#endif


#undef EXTERN
#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

#include "pgasrt_config.h"
#include "pgasrt_types.h"

#include <stdlib.h>         /* for size_t */
#include <limits.h>
#include <stddef.h>

/* ************************************************************************ */
/** @file pgasrt_smp.h                                                      */
/* ************************************************************************ */

/* ************************************************************************ */
/** 
 * \brief SMP entry point
 * \param argc                     command line arguments count
 * \param argv                     command line arguments
 * \param pmain                    function to run when all threads started
 * \param totalthreads             total # threads in program 
 * \param threadsPerNode           threads running in one node
 * \param lstack                   TL stack size (bytes)
 * \returns 0
 */
/* ************************************************************************ */

EXTERN int    __pgasrt_smp_start   (unsigned              argc,
				    char               ** argv,
				    int                (* pmain)(int, char**),
				    __pgasrt_thread_t     totalthreads,
				    __pgasrt_thread_t     threadsPerNode,
				    unsigned              lstack);

/* ************************************************************************ */
/** \brief maximum total threads per node                                   */
/* ************************************************************************ */

#define PGASRT_SMPTHREADS_MAX  128

/* ************************************************************************ */
/** \brief shorthand for \ref __pgasrt_smp_nthreads                         */
/* ************************************************************************ */

#define PGASRT_SMPTHREADS      __pgasrt_smp_nthreads

/* ************************************************************************ */
/** \brief shorthand for \ref __pgasrt_smp_log_nthreads                     */
/* ************************************************************************ */

#define PGASRT_LOGSMPTHREADS   __pgasrt_smp_log_nthreads

/* ************************************************************************ */
/** \brief shorthand for \ref __pgasrt_smp_myID                             */
/* ************************************************************************ */

#define PGASRT_MYSMPTHREAD     __pgasrt_smp_myID()

/* *********************************************************************** */
/** \brief return my local thread ID in current node                       */
/* *********************************************************************** */

EXTERN __pgasrt_thread_t __pgasrt_smp_myID          (void);

/* *********************************************************************** */
/** \brief return number of threads running on local node                  */
/* *********************************************************************** */

EXTERN __pgasrt_thread_t __pgasrt_smp_threads       (void);

/* *********************************************************************** */
/** \brief set the threads running in this node.                           */
/* *********************************************************************** */

EXTERN void              __pgasrt_smp_set_threads   (__pgasrt_thread_t tpn);

/* *********************************************************************** */
/** \brief public variable: total threads running in node                  */
/* *********************************************************************** */

extern __pgasrt_thread_t __pgasrt_smp_nthreads;

/* *********************************************************************** */
/** \brief public: floor(log(2)(threads running in node))                  */
/* *********************************************************************** */

extern int               __pgasrt_smp_log_nthreads;

/* *********************************************************************** */
/** \brief exit current thread with exit code                              */
/* *********************************************************************** */

EXTERN void              __pgasrt_smp_exit          (int exitcode);

/* *********************************************************************** */
/** \brief immediate exit: kill all other threads                          */
/* *********************************************************************** */

EXTERN void              __pgasrt_smp_exitNow       (int status);

/* *********************************************************************** */
/** \brief Initialize SMP barrier                                          */
/* *********************************************************************** */

EXTERN void             __pgasrt_smp_barrier_init   (int threadsPerNode);

/* *********************************************************************** */
/** \brief SMP barrier                                                     */
/* *********************************************************************** */

EXTERN void             __pgasrt_smp_barrier        (void);

/* *********************************************************************** */
/** \brief SMP fence                                                       */
/* *********************************************************************** */

EXTERN void             __pgasrt_smp_fence          (void);
						      
/* *********************************************************************** */
/** \brief      check the stack for overflows                              */
/* *********************************************************************** */

EXTERN int              __pgasrt_stackcheck         (void);

#if defined(__cplusplus)
}
#endif
/* **************************************************************************/
/** \page smp Threading subsystem

This API is provided in the file \ref pgasrt_smp.h and provides basic
functions for starting and collecting pthreads on a node, thread ID
inquiry functions, as well as basic barrier and fence operations.

*/
/* **************************************************************************/


#endif
