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

typedef XMIQuad CCMI_Geometry_t [CCMI_GEOMETRY_SIZE];
typedef XMIQuad XMI_CollectiveProtocol_t [CCMI_PROTOCOL_SIZE];/// \todo XMI_Prototol_t?

// CCMI Collective request should be 32 CCMI_Requests. This is to store several
// algorithms/schedule/executor pairs and have several messages in flight  \todo XMI_?
typedef XMIQuad XMI_CollectiveRequest_t  [XMI_REQUEST_NQUADS*8*4];
typedef XMIQuad CCMI_Executor_t           [XMI_REQUEST_NQUADS*4];

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
