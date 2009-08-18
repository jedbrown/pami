/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file interface/mpi/adaptor_pre.h
 * \brief ???
 */


#ifndef   __adaptor_pre_h__
#define   __adaptor_pre_h__
#warning mpi pre adaptor
#include "interface/cm_types.h"
#include "interface/ll_multisend.h"
#include "interface/mpi/ccmi_collectives.h"

#ifdef __PGASRT_DISTRIBUTED /* pure distributed - no pthreads library */
#define DECL_STATIC_MUTEX(x)    int x = 0
#define DECL_MUTEX(x)           int x
#define MUTEX_INIT(x)           { (*(x)) = 0; }
#define MUTEX_TRYLOCK(x)        ((*(x)) > 0 ? 1 : (((*x)=1),0))
#define MUTEX_LOCK(x) { while ((*(x)) > 0) __pgasrt_tsp_wait(NULL); (*(x))=1; }
#define MUTEX_UNLOCK(x)         { (*(x))=0; }
#define MUTEX_DESTROY(x)        { }
#define DECL_RW_MUTEX(x)
#define RDLOCK(x)
#define WRLOCK(x)
#define RWUNLOCK(x)

#else /* shared memory & hybrid */

#include <pthread.h>

#define DECL_STATIC_MUTEX(x) \
static pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER
#define DECL_MUTEX(x)    pthread_mutex_t x
#define MUTEX_INIT(x)    pthread_mutex_init(x,NULL)
#define MUTEX_TRYLOCK(x) pthread_mutex_trylock(x)
#define MUTEX_LOCK(x)    pthread_mutex_lock(x)
#define MUTEX_UNLOCK(x)  pthread_mutex_unlock(x)
#define MUTEX_DESTROY(x) pthread_mutex_destroy(x)

#if (_XOPEN_SOURCE - 0) >= 500
#define DECL_RW_MUTEX(x) \
static pthread_rwlock_t x = PTHREAD_RWLOCK_INITIALIZER
#define RDLOCK(x)        pthread_rwlock_rdlock(x)
#define WRLOCK(x)        pthread_rwlock_wrlock(x)
#define RWUNLOCK(x)      pthread_rwlock_unlock(x)
#else
#define DECL_RW_MUTEX(x) \
static pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER
#define RDLOCK(x)        pthread_mutex_lock(x)
#define WRLOCK(x)        pthread_mutex_lock(x)
#define RWUNLOCK(x)      pthread_mutex_unlock(x)
#endif

#endif


// A debug thread id
#define ThreadID() 0

    /// The pipeline width must be a multiple of 240 (DMA) and 256 (Tree)
    /// 240 = 15 * 16
    /// 256 =      16 * 16
    /// So, width = 15 * 16 * 16 * x = 3840 * x
    /// Currently x = 4 for a width of 15360.  Adjust as desired.
    const unsigned MIN_PIPELINE_WIDTH = 3840;

    /// FP collectives on Blue Gene/L and Blue Gene/P use a double
    /// pass scheme that processes exponents and mantissas. The
    /// minimum transfer unit for this scheme is 1008 bytes (126
    /// doubles). The LCM of 240 and 1008 is 5040.
    const unsigned MIN_PIPELINE_WIDTH_SUM2P = 5040;

    /// On a torus we can use a smaller pipeline width that can allow
    /// better performance for shorter messages. We use 480 bytes
    /// which is 2 packets 
    const unsigned MIN_PIPELINE_WIDTH_TORUS = 480;

     /// This is the default allreduce min torus pipeline width, set to 1920
     const unsigned ALLREDUCE_MIN_PIPELINE_WIDTH_TORUS = 1920;    

typedef XMI_RecvMulticast CCMI_RecvMulticast_t;
#define __ccmi_recv_multicast_callback_defined__

/* These are defined in ccmi_collectives.h */
#define __ccmi_consistency_defined__
#define __ccmi_subtask_defined__
#define __ccmi_recvasynccallback_defined__

/* These are defined in cm_types.h */
#define __ccmi_pipeworkqueue_defined__
#define __ccmi_topology_defined__
#define __ccmi_op_defined__
#define __ccmi_dt_defined__
#define __ccmi_quad_defined__
#define __ccmi_error_defined__
#define __ccmi_callback_defined__

#endif
