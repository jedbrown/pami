/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file interface/ccmi_internal.h
 * \brief ???
 */

#ifndef   __ccmi_internal_h__
#define   __ccmi_internal_h__

#include <stdlib.h>
#include "sys/xmi.h"


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

typedef xmi_dispatch_multicast_fn CCMI_RecvMulticast_t;
#define __ccmi_recv_multicast_callback_defined__

/* These are defined in ccmi_collectives.h */
//#define __ccmi_consistency_defined__
#define __ccmi_subtask_defined__
#define __ccmi_recvasynccallback_defined__

/* These are defined in xmi_types.h */
#define __ccmi_pipeworkqueue_defined__
#define __ccmi_topology_defined__
#define __ccmi_op_defined__
#define __ccmi_dt_defined__
#define __ccmi_quad_defined__
#define __ccmi_error_defined__
#define __ccmi_callback_defined__


typedef xmi_quad_t XMI_Request_t[XMI_REQUEST_NQUADS];
typedef xmi_event_function XMI_Callback_t;

///
///\brief CCMI definitions
///

#ifndef __ccmi_consistency_defined__
#define __ccmi_consistency_defined__
typedef enum
{
  CCMI_UNDEFINED_CONSISTENCY = -1,
  CCMI_RELAXED_CONSISTENCY,
  CCMI_MATCH_CONSISTENCY,
  CCMI_WEAK_CONSISTENCY,
  CCMI_CONSISTENCY_COUNT
}
  CCMI_Consistency;
#endif

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

#define CCMI_UNDEFINED_PHASE ((unsigned)-1)

#define  CCMI_UNDEFINED_RANK ((unsigned)-1)

#ifndef CCMI_PROTOCOL_SIZE
#define CCMI_PROTOCOL_SIZE	64
#endif

#ifndef CCMI_GEOMETRY_SIZE
#define CCMI_GEOMETRY_SIZE	32
#endif

typedef xmi_quad_t CCMI_Geometry_t [CCMI_GEOMETRY_SIZE];
typedef xmi_quad_t XMI_CollectiveProtocol_t [CCMI_PROTOCOL_SIZE];/// \todo XMI_Prototol_t?

// CCMI Collective request should be 32 CCMI_Requests. This is to store several
// algorithms/schedule/executor pairs and have several messages in flight  \todo XMI_?
typedef xmi_quad_t XMI_CollectiveRequest_t  [XMI_REQUEST_NQUADS*8*4];
typedef xmi_quad_t CCMI_Executor_t           [XMI_REQUEST_NQUADS*4];

#ifndef __ccmi_recvasynccallback_defined__
typedef void * (*CCMI_RecvAsyncBroadcast) (unsigned           root,
                                           unsigned           comm,
                                           const unsigned     sndlen,
                                           unsigned         * rcvlen,
                                           char            ** rcvbuf,
                                           XMI_Callback_t  * const cb_info);
#define __ccmi_recvasynccallback_defined__
#endif


/**
 *  \brief A callback to map the geometry id to the geometry
 *  structure. The CCMI runtime has to be able to freely translate
 *  between geometry id's and geometry structures.
 */

typedef CCMI_Geometry_t * (*CCMI_mapIdToGeometry) (int comm);


#endif
