/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/tests/generic/asyncbcast.h
 * \brief ???
 */

#include "common.h"
#include "barrier.h"

XMI_CollectiveProtocol_t              asyncbcast_reg;
CCMI_Broadcast_Configuration_t         asyncbcast_conf;
XMI_CollectiveRequest_t               asyncbcast_request;

inline void asyncbcast_advance (unsigned * srcbuf, unsigned src_count, unsigned root)
{
  TRACE_TEST_VERBOSE((stderr, "%s:%s   src %p, count %d, root %d\n", argv0,__PRETTY_FUNCTION__, srcbuf, src_count, root));
  done = 0;
  CCMI_Broadcast(&asyncbcast_reg,
		 &asyncbcast_request,
		 common_done,
		 consistency,
		 &geometry,
		 root,
		 (char *)srcbuf,
		 src_count*sizeof(unsigned));
  while(!done)
  {
    CCMI_Generic_adaptor_advance();
  }
  return;
}


void initialize(CCMI_Barrier_Protocol barrier_protocol,
                CCMI_Barrier_Protocol lbarrier_protocol,
                CCMI_Broadcast_Protocol asyncbcast_protocol)
{

  initialize_common(barrier_protocol, lbarrier_protocol);

  common_done.function  = done_callback;
  common_done.clientdata= NULL;

  asyncbcast_conf.protocol = asyncbcast_protocol;
  asyncbcast_conf.cb_geometry = getGeometry;
  asyncbcast_conf.cb_recv = NULL;
  asyncbcast_conf.isBuffered = 1;

  CCMI_Result ccmiResult;

  if((ccmiResult = (CCMI_Result) CCMI_Broadcast_register (&asyncbcast_reg, &asyncbcast_conf)) != XMI_SUCCESS)
    if(rank == 0) fprintf(stderr,"CCMI_AsyncBcast_register failed %d\n",ccmiResult);

  if(!CCMI_Geometry_analyze(&geometry, &asyncbcast_reg))
  {
    no_op("Not a supported geometry - NO OP\n");
    exit(0);
  }


  return;
}
