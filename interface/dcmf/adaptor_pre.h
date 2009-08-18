/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file collectives/bgp/adaptor_pre.h
 * \brief ???
 */

/* This block is about the minimum DCMF needs to define in the adaptor.  
Mostly it's compatible definitions for quads and errors so that callbacks can 
work without another layer of calls between the adaptor and ccmi.  Alternately,
we probably could redefine ccmi to use more void*'s.  Just here for an 
example/info.

#ifndef   __adaptor_pre_h__
#define   __adaptor_pre_h__


#include "dcmf.h"

#define __ccmi_quad_defined__
#define __ccmi_error_defined__
#define __ccmi_callback_defined__

#endif
*/


#ifndef   __adaptor_pre_h__
#define   __adaptor_pre_h__

#include "dcmf.h"
/* dcmf_collectives.h might not be installed yet so reference relative dir */
#include "../../include/dcmf_collectives.h"
#include "ll_multisend.h"

/* all these are defined in cm_types.h, dcmf.h et al. */
#define __ccmi_pipeworkqueue_defined__
#define __ccmi_topology_defined__
#define __ccmi_op_defined__
#define __ccmi_dt_defined__
#define __ccmi_quad_defined__
#define __ccmi_error_defined__
#define __ccmi_callback_defined__

#include <spi/bgp_SPI.h>

// Debug thread id
#define ThreadID Kernel_PhysicalProcessorID 

#define __ccmi_consistency_defined__
typedef enum
{
  CCMI_UNDEFINED_CONSISTENCY = -1,
  CCMI_RELAXED_CONSISTENCY   = DCMF_RELAXED_CONSISTENCY, 
  CCMI_MATCH_CONSISTENCY     = DCMF_MATCH_CONSISTENCY,   
  CCMI_WEAK_CONSISTENCY      = DCMF_WEAK_CONSISTENCY,    
  CCMI_CONSISTENCY_COUNT
}
CCMI_Consistency;

//--------------------------------------------------
//----- Communication Subtasks ---------------------
//----- Each communication operation is split ------
//----- into phases of one or more sub tasks -------
//--------------------------------------------------

#define __ccmi_subtask_defined__
#define  LINE_BCAST_MASK    (CCMI_LINE_BCAST_XP|CCMI_LINE_BCAST_XM|\
                                 CCMI_LINE_BCAST_YP|CCMI_LINE_BCAST_YM|\
                                 CCMI_LINE_BCAST_ZP|CCMI_LINE_BCAST_ZM)
// Warning. This enum must "follow" XMI_Opcode_t in sys/include/dcmf.h.
// Both enums must define the same values.
//
typedef enum
{
  CCMI_PT_TO_PT_SUBTASK           = DCMF_PT_TO_PT_SEND,    
  CCMI_LINE_BCAST_XP              = DCMF_LINE_BCAST_XP,       
  CCMI_LINE_BCAST_XM              = DCMF_LINE_BCAST_XM,       
  CCMI_LINE_BCAST_YP              = DCMF_LINE_BCAST_YP,       
  CCMI_LINE_BCAST_YM              = DCMF_LINE_BCAST_YM,       
  CCMI_LINE_BCAST_ZP              = DCMF_LINE_BCAST_ZP,       
  CCMI_LINE_BCAST_ZM              = DCMF_LINE_BCAST_ZM,       
  CCMI_COMBINE_SUBTASK            = DCMF_COMBINE,     
  CCMI_GI_BARRIER                 = DCMF_GI_BARRIER,          
  CCMI_LOCKBOX_BARRIER            = DCMF_LOCKBOX_BARRIER,     
  CCMI_TREE_BARRIER               = DCMF_TREE_BARRIER,        
  CCMI_TREE_BCAST                 = DCMF_TREE_BCAST,          
  CCMI_TREE_ALLREDUCE             = DCMF_TREE_ALLREDUCE,      
  CCMI_REDUCE_RECV_STORE          = DCMF_REDUCE_RECV_STORE,   
  CCMI_REDUCE_RECV_NOSTORE        = DCMF_REDUCE_RECV_NOSTORE, 
  CCMI_BCAST_RECV_STORE           = DCMF_BCAST_RECV_STORE,    
  CCMI_BCAST_RECV_NOSTORE         = DCMF_BCAST_RECV_NOSTORE,  
  CCMI_LOCALCOPY                  = DCMF_LOCALCOPY,

  CCMI_UNDEFINED_SUBTASK          = (~LINE_BCAST_MASK),
} CCMI_Subtask;

#define CCMI_REQUEST_SIZE	XMI_REQUEST_NQUADS
// CCMI Protocol may contain two DCMF Protocols
#define CCMI_PROTOCOL_SIZE	(XMI_PROTOCOL_NQUADS*2)
#define CCMI_GEOMETRY_SIZE	32


#define __ccmi_recv_multicast_callback_defined__
#ifdef DEPRECATED_MULTICAST
typedef DCMF_OldRecvMulticast CCMI_OldRecvMulticast_t;
#endif /*  DEPRECATED_MULTICAST */

typedef XMI_RecvMulticast CCMI_RecvMulticast_t;

#define __ccmi_recvasynccallback_defined__
typedef DCMF_RecvAsyncBroadcast CCMI_RecvAsyncBroadcast;

//#include <assert.h>

//static inline void _adaptor_pre_h_compile_time_assert_ ()
//{
// Just trying to get rid of " warning: #XMI_Op_to_CM_Op# defined but not used"
//  assert(XMI_Dt_to_CM_Dt[XMI_DT_COUNT]==(XMI_Dt)XMI_DT_COUNT);
//  assert(XMI_Op_to_CM_Op[XMI_OP_COUNT]==(XMI_Op)XMI_OP_COUNT);
//}

#if 0
class CCMI_OldMulticast_t :  public DCMF_OldMulticast_t
{
public:

  CCMI_OldMulticast_t ()
  {
  }

  void setRequestBuffer (XMI_Request_t *request)
  {
    this->request = request;
  }

  void setConnectionId (unsigned conn)
  {
    this->connection_id = conn;
  }

  void setSendData (const char *src, unsigned bytes)
  {
    this->src = src;
    this->bytes = bytes;
  }

  void setCallback (void (*fn) (void *),  void *cd)
  {
    cb_done.function = fn;
    cb_done.clientdata = cd;
  }

  void setReduceInfo (XMI_Op op,  XMI_Dt dt)
  {
    this->op = XMI_Op_to_CM_Op[op];
    this->dt = XMI_Dt_to_CM_Dt[dt];
  }

  void setConsistency (CCMI_Consistency c)
  {
    this->consistency = (DCMF_Consistency) c;
  }

  void setOpcodes (CCMI_Subtask *op)
  {
    this->opcodes = (XMI_Opcode_t *) op;
  }

  void setInfo (CMQuad *info, int count)
  {
    this->msginfo = info;
    this->count   = count;
  }

  void setRanks (unsigned *ranks, unsigned nranks)
  {
    this->rank = ranks;
    this->nranks = nranks;
  }
};
#endif

///Define pipeline constants to be used by the different
///protocols. These are for Blue Gene/P and may need to be changed for
///other architectures

//namespace DCMF
//{
//  namespace Collectives
//  {
    
    const unsigned NUM_TORUS_DIMENSIONS = 3;

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

    /// The packet size on the torus network
    const unsigned TORUS_PACKET_SIZE = 240;

    /// A standard recommended pipelinewidth that works for integer
    /// allreduce and torus operations. Protocols should use multiple
    /// of the MIN_* if the would like to override this value
    const unsigned PIPELINE_WIDTH = 3840 * 4;
//  };
//};


#if 0  // malloc/free overrides
#include <stdio.h>
  #include <stdlib.h>

inline void * CCMI_dcmf_alloc(const char * file,const char* function,const int line, unsigned size)
{
  void * p= malloc(size);
  fprintf(stderr,"adaptor::dcmf::CCMI_Alloc<%#X/%#X> in %s<%d>/%s\n",size,(int)p,file,line,function);
  return p;
}
  #define CCMI_Alloc(x) CCMI_dcmf_alloc(__FILE__,__FUNCTION__,__LINE__,x)

inline void CCMI_dcmf_free(const char * file,const char* function,const int line, void * p)
{
  fprintf(stderr,"adaptor::dcmf::CCMI_Free<%#X> in %s<%d>/%s\n",(int)p,file,line,function);
  free(p);
}
  #define CCMI_Free(x) CCMI_dcmf_free(__FILE__,__FUNCTION__,__LINE__,x)

#endif


#endif
