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

///
/// \brief CCMI Adaptor dependent definitions
///
#include "adaptor_pre.h" // build needs to select this

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

#ifndef __ccmi_op_defined__
  #define __ccmi_op_defined__
#endif

#ifndef __ccmi_dt_defined__
  #error define __ccmi_dt_defined__
#endif

//--------------------------------------------------
//----- Communication CCMI_Subtasks ---------------------
//----- Each communication operation is split ------
//----- into phases of one or more sub tasks -------
//--------------------------------------------------

#ifndef __ccmi_subtask_defined__
  #define __ccmi_subtask_defined__
  #define  LINE_BCAST_MASK    (CCMI_LINE_BCAST_XP|CCMI_LINE_BCAST_XM|\
                                 CCMI_LINE_BCAST_YP|CCMI_LINE_BCAST_YM|\
                                 CCMI_LINE_BCAST_ZP|CCMI_LINE_BCAST_ZM)

typedef enum
{
  CCMI_PT_TO_PT_SUBTASK           =  0,      //Send a pt-to-point message
  CCMI_LINE_BCAST_XP              =  0x20,   //Bcast along x+
  CCMI_LINE_BCAST_XM              =  0x10,   //Bcast along x-
  CCMI_LINE_BCAST_YP              =  0x08,   //Bcast along y+
  CCMI_LINE_BCAST_YM              =  0x04,   //Bcast along y-
  CCMI_LINE_BCAST_ZP              =  0x02,   //Bcast along z+
  CCMI_LINE_BCAST_ZM              =  0x01,   //Bcast along z-
  CCMI_COMBINE_SUBTASK            =  0x0100,   //Combine the incoming message
  //with the local state
  CCMI_GI_BARRIER                 =  0x0200,
  CCMI_LOCKBOX_BARRIER            =  0x0300,
  CCMI_TREE_BARRIER               =  0x0400,
  CCMI_TREE_BCAST                 =  0x0500,
  CCMI_TREE_ALLREDUCE             =  0x0600,
  CCMI_REDUCE_RECV_STORE          =  0x0700,
  CCMI_REDUCE_RECV_NOSTORE        =  0x0800,
  CCMI_BCAST_RECV_STORE           =  0x0900,
  CCMI_BCAST_RECV_NOSTORE         =  0x0a00,
  CCMI_LOCALCOPY                  =  0x0b00,

  CCMI_UNDEFINED_SUBTASK          =  (~LINE_BCAST_MASK),
} CCMI_Subtask;
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

#ifndef __ccmi_quad_defined__
  #error Adaptor did not declare CMQuad!
  #define __ccmi_quad_defined__
#endif

#ifndef __ccmi_topology_defined__
#error typedef CMQuad LL_Topology_t[2]; /// \todo resolve to ll interface
#endif

#ifndef __ccmi_pipeworkqueue_defined__
#error typedef CMQuad LL_PipeWorkQueue_t[4]; /// \todo resolve to ll interface
#endif

#ifndef CCMI_REQUEST_SIZE
#define CCMI_REQUEST_SIZE	32
#endif

#ifndef CCMI_PROTOCOL_SIZE
#define CCMI_PROTOCOL_SIZE	64
#endif

#ifndef CCMI_GEOMETRY_SIZE
#define CCMI_GEOMETRY_SIZE	32
#endif

typedef CMQuad CCMI_Geometry_t [CCMI_GEOMETRY_SIZE];
typedef CMQuad CCMI_CollectiveProtocol_t [CCMI_PROTOCOL_SIZE];/// \todo CM_Prototol_t?
typedef CMQuad CCMI_Request_t            [CCMI_REQUEST_SIZE]; /// \todo CM_Request_t?

// CCMI Collective request should be 32 CCMI_Requests. This is to store several
// algorithms/schedule/executor pairs and have several messages in flight  \todo CM_?
typedef CMQuad CCMI_CollectiveRequest_t  [CCMI_REQUEST_SIZE*8*4];
typedef CMQuad CCMI_Executor_t           [CCMI_REQUEST_SIZE*4];

#ifndef __ccmi_error_defined__
  #define __ccmi_error_defined__
#endif


#ifndef __ccmi_callback_defined__
  #define __ccmi_callback_defined__
#endif


#ifndef __ccmi_recvasynccallback_defined__
typedef void * (*CCMI_RecvAsyncBroadcast) (unsigned           root,
                                           unsigned           comm,
                                           const unsigned     sndlen,
                                           unsigned         * rcvlen,
                                           char            ** rcvbuf,
                                           CM_Callback_t  * const cb_info);
  #define __ccmi_recvasynccallback_defined__
#endif


/**
 *  \brief A callback to map the geometry id to the geometry
 *  structure. The CCMI runtime has to be able to freely translate
 *  between geometry id's and geometry structures.
 */

typedef CCMI_Geometry_t * (*CCMI_mapIdToGeometry) (int comm);

typedef CCMI_Request_t * (*CCMI_OldRecvMulticast) (const CMQuad    * info,
                                                   unsigned          count,
                                                   unsigned          peer,
                                                   unsigned          sndlen,
                                                   unsigned          conn_id,
                                                   void            * arg,
                                                   unsigned        * rcvlen,
                                                   char           ** rcvbuf,
                                                   unsigned        * pipewidth,
                                                   CM_Callback_t * cb_done);

#endif
