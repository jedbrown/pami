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

#ifndef __PGASRT_HYBRID_H_
#define __PGASRT_HYBRID_H_

#undef EXTERN
#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#include "pgasrt_config.h"
#include "pgasrt_types.h"

#include <stdlib.h>         /* for size_t */
#include <limits.h>
#include <stddef.h>

/* ************************************************************************ */
/** @file pgasrt_hbrd.h                                                     */
/* ************************************************************************ */

/* *********************************************************************** */
/** \brief runtime entry point: start hybrid execution
 *  \param argc            command line arguments count
 *  \param argv            command line arguments
 *  \param pmain           function to execute
 *  \param totalthreads    total threads executing in job
 *  \param lstack          size of thread-local stack
 */
/* *********************************************************************** */

EXTERN int __pgasrt_hbrd_start     (int                argc, 
				    char            ** argv, 
				    int             (* pmain)(int, char **), 
				    __pgasrt_thread_t  totalthreads, 
				    unsigned           lstack);

/* *********************************************************************** */
/** \brief short hand for \ref __pgasrt_hbrd_myID                          */
/* *********************************************************************** */

#define PGASRT_MYTHREAD            __pgasrt_hbrd_myID()

/* *********************************************************************** */
/** \brief short hand for \ref __pgasrt_hbrd_nthreads                      */
/* *********************************************************************** */

#define PGASRT_THREADS             __pgasrt_hbrd_nthreads

/* *********************************************************************** */
/** \brief conversion from hybrid thread ID to node ID                     */
/* *********************************************************************** */

#define PGASRT_NODE_ID(x)          (PGASRT_LOGSMPTHREADS>=0 ? \
                                   ((x)>>PGASRT_LOGSMPTHREADS) : \
                                   ((x)/PGASRT_SMPTHREADS))

/* *********************************************************************** */
/** \brief conversion from hybrid thread ID to node ID                     */
/* *********************************************************************** */

#define PGASRT_NODEOF(x)           PGASRT_NODE_ID(x)

/* *********************************************************************** */
/** \brief conversion from hybrid thread ID to SMP ID                      */
/* *********************************************************************** */

#define PGASRT_SMP_ID(x)           ((x)%PGASRT_SMPTHREADS)

/* *********************************************************************** */
/** \brief check whether a hybrid thread is running on my node             */
/* *********************************************************************** */

#define PGASRT_IS_LOCAL(x)         (((int)PGASRT_NODE_ID(x))==PGASRT_MYNODE)

/* *********************************************************************** */
/** \brief return my hybrid thread ID                                      */
/* *********************************************************************** */

EXTERN __pgasrt_thread_t       __pgasrt_hbrd_myID (void);

/* *********************************************************************** */
/** \brief number of total threads running in current job                  */
/* *********************************************************************** */

extern __pgasrt_thread_t       __pgasrt_hbrd_nthreads;

/* *********************************************************************** */
/** \brief number of total threads running in current job                  */
/* *********************************************************************** */

EXTERN __pgasrt_thread_t       __pgasrt_hbrd_threads     (void);

/* *********************************************************************** */
/** \brief set total number of threads running in job                      */
/* *********************************************************************** */

EXTERN  void                   __pgasrt_hbrd_set_threads (__pgasrt_thread_t);

/* *********************************************************************** */
/** \brief hybrid barrier                                                  */
/* *********************************************************************** */

EXTERN void                    __pgasrt_hbrd_barrier        (void);

/* *********************************************************************** */
/** \brief hybrid fence                                                    */
/* *********************************************************************** */

EXTERN void                    __pgasrt_hbrd_fence          (void);

/* *********************************************************************** */
/** \brief send an active message to other node. Sends header and data
 * to destination, and invokes header handler on destination. Header
 * handler will return an address to copy data to.
 * 
 * Local completion semantics (complete when outbound data has cleared
 * local buffers). Local completion results in invocation of send side
 * completion hander (if specified non-NULL). Otherwise user has to
 * wait on handle to ascertain local completion.
 *
 * \param destThread  destination thread
 * \param header      active message header
 * \param data        data buffer to be sent
 * \param datalen     length of send data buffer
 * \param comp_h      send side completion handler
 * \param info        send side completion handler argument
 * \returns           a handle to be waited on, or NULL if a send side
 *                    completion handler was specified.
 */
/* *********************************************************************** */

EXTERN void *
__pgasrt_hbrd_amsend       (__pgasrt_thread_t            destThread,
			    __pgasrt_AMHeader_t        * header,
			    const __pgasrt_local_addr_t  data,
			    unsigned                     datalen,
			    void                      (* comp_h) (void *),
			    void                       * info);


#if defined(__cplusplus)
}
#endif

/* **************************************************************************/
/** \page hbrd Hybrid subsystem

The hybrid subsystem combines the functions offered by the \ref tsp
"transport API" and the \ref smp "SMP thread API".

*/
/* **************************************************************************/

#endif
