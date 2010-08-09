/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/ccmi.h
 * \brief ???
 */

#ifndef __algorithms_ccmi_h__
#define __algorithms_ccmi_h__

#include <stdlib.h>
#include <pami.h>


// A debug thread id
#define ThreadID() 0

/// The pipeline width must be a multiple of 240 (DMA) and 256 (Tree)
/// 240 = 15 * 16
/// 256 =      16 * 16
/// So, width = 15 * 16 * 16 * x = 3840 * x
/// Currently x = 4 for a width of 15360.  Adjust as desired.
const unsigned MIN_PIPELINE_WIDTH = 3840;
//const unsigned MIN_PIPELINE_WIDTH = 2147483647;

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

//typedef pami_dispatch_multicast_fn CCMI_RecvMulticast_t;
#define __ccmi_recv_multicast_callback_defined__

/* These are defined in ccmi_collectives.h */
//#define __ccmi_consistency_defined__
#define __ccmi_subtask_defined__
#define __ccmi_recvasynccallback_defined__

/* These are defined in pami_types.h */
#define __ccmi_pipeworkqueue_defined__
#define __ccmi_topology_defined__
#define __ccmi_op_defined__
#define __ccmi_dt_defined__
#define __ccmi_quad_defined__
#define __ccmi_error_defined__
#define __ccmi_callback_defined__


/** \todo fix NDIMS */
#define PAMI_X_DIM 0
#define PAMI_Y_DIM 0//1
#define PAMI_Z_DIM 0//2
#define PAMI_T_DIM 0//3

// TODO:  generalize this using mappings

#define PAMI_TORUS_NDIMS 1

#define PAMI_COPY_COORDS(a,b)               \
{                                           \
  int _i;                                   \
  for (_i = 0; _i < PAMI_TORUS_NDIMS; _i++) \
    a[_i] = b[_i];                          \
}





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


typedef pami_quad_t PAMI_Request_t[PAMI_REQUEST_NQUADS];
typedef pami_callback_t PAMI_Callback_t;


//Descriptive quad of data that identifies each collective
typedef struct _cheader_data
{
  unsigned        _root;         //root of the collective
  unsigned        _comm;         //communicator
  unsigned        _count;        //count for this collective operation
  unsigned        _phase:12;     //phase of the collective
  unsigned        _iteration:4;  //iteration of the allreduce, otherwise unused
  unsigned        _op:8;         //op code of the allreduce, otherwise unused
  unsigned        _dt:8;         //datatype of the allreduce, otherwise unused
} CollHeaderData  __attribute__((__aligned__(16)));


#ifndef CCMI_Alloc
#define CCMI_Alloc(x)  malloc(x)
#endif

#ifndef CCMI_Free
#define CCMI_Free(x)   free(x)
#endif

#define PAMI_UNDEFINED_PHASE ((unsigned)-1)

#define  PAMI_UNDEFINED_RANK ((unsigned)-1)

#ifndef CCMI_PROTOCOL_SIZE
#define CCMI_PROTOCOL_SIZE	64
#endif

#ifndef CCMI_GEOMETRY_SIZE
#define CCMI_GEOMETRY_SIZE	32
#endif

typedef enum
{
  CCMI_FLAGS_UNSET                =  0,
  CCMI_PERSISTENT_MESSAGE         =  1,
} CCMI_MulticastFlags_t;



typedef pami_quad_t CCMI_Geometry_t [CCMI_GEOMETRY_SIZE];
typedef pami_quad_t PAMI_CollectiveProtocol_t [CCMI_PROTOCOL_SIZE];/// \todo PAMI_Prototol_t?

// CCMI Collective request should be 32 CCMI_Requests. This is to store several
// algorithms/schedule/executor pairs and have several messages in flight  \todo PAMI_?
typedef pami_quad_t PAMI_CollectiveRequest_t  [PAMI_REQUEST_NQUADS*8*4];
typedef pami_quad_t CCMI_Executor_t           [PAMI_REQUEST_NQUADS*4];

typedef void * (*CCMI_RecvAsyncBroadcast) (unsigned           root,
                                           unsigned           comm,
                                           const unsigned     sndlen,
                                           unsigned         * rcvlen,
                                           char            ** rcvbuf,
                                           PAMI_Callback_t  * const cb_info);


/**
 *  \brief A callback to map the geometry id to the geometry
 *  structure. The CCMI runtime has to be able to freely translate
 *  between geometry id's and geometry structures.
 */


#endif
