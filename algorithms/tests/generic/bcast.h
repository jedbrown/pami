/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/tests/generic/bcast.h
 * \brief ???
 */

#ifndef __algorithms_tests_generic_bcast_h__
#define __algorithms_tests_generic_bcast_h__

#include "common.h"
#include "barrier.h"

XMI_CollectiveProtocol_t              bcast_reg;
CCMI_Broadcast_Configuration_t         bcast_conf;
XMI_CollectiveRequest_t               bcast_request;

inline void bcast_advance (unsigned * srcbuf, unsigned src_count, unsigned root)
{
  TRACE_TEST_VERBOSE((stderr, "%s:%s   src %p, count %d, root %d\n", argv0,__PRETTY_FUNCTION__, srcbuf, src_count, root));
  done = 0;
  CCMI_Broadcast(&bcast_reg,
                 &bcast_request,
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
                CCMI_Broadcast_Protocol bcast_protocol)
{

  initialize_common(barrier_protocol, lbarrier_protocol);

  common_done.function  = done_callback;
  common_done.clientdata= NULL;

  bcast_conf.protocol = bcast_protocol;

  CCMI_Result dcmfResult;

  if((dcmfResult = (CCMI_Result) CCMI_Broadcast_register (&bcast_reg, &bcast_conf)) != XMI_SUCCESS)
    if(rank == 0) fprintf(stderr,"CCMI_Bcast_register failed %d\n",dcmfResult);

  if(!CCMI_Geometry_analyze(&geometry, &bcast_reg))
  {
    no_op("Not a supported geometry - NO OP\n");
    exit(0);
  }


  return;
}

#endif
