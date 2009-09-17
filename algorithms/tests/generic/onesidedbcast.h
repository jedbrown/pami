/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/tests/generic/onesidedbcast.h
 * \brief ???
 */

#include "common.h"
#include "barrier.h"

XMI_CollectiveProtocol_t              onesidedbcast_reg;
CCMI_Broadcast_Configuration_t         onesidedbcast_conf;
XMI_CollectiveRequest_t               onesidedbcast_request;
int                                    nreceived_countdown=0;
unsigned                               narrived=0;
XMI_CollectiveRequest_t              *recvRequests=NULL;

void recv_callback(void* cd, XMI_Error_t *err)
{
  TRACE_TEST_VERBOSE((stderr,"%s:%s nreceived_countdown %d\n", argv0,__PRETTY_FUNCTION__,nreceived_countdown));
  assert(nreceived_countdown);
  nreceived_countdown--;
}
void * recvBcast (unsigned          root,
                  unsigned          comm,
                  unsigned          sndlen,
                  unsigned        * rcvlen,
                  char           ** rcvbuf,
                  XMI_Callback_t * const cb_info)
{
  //  printf ("Received broadcast message \n");
  TRACE_TEST_VERBOSE((stderr,"%s:%s narrived %d\n", argv0,__PRETTY_FUNCTION__, narrived));
  *rcvlen  = sndlen;
  *rcvbuf  = (char*)dstbuf;
  cb_info->function = recv_callback;
  cb_info->clientdata = NULL;

  assert (narrived < (unsigned)repetitions);
  return recvRequests[narrived ++];
}



inline void onesidedbcast_advance (unsigned * srcbuf, unsigned src_count, unsigned root)
{
  TRACE_TEST_VERBOSE((stderr, "%s:%s   src %p, count %d, root %d\n", argv0,__PRETTY_FUNCTION__, srcbuf, src_count, root));

  done = 0; nreceived_countdown = src_count;

    CCMI_Broadcast(&onesidedbcast_reg,
		   &onesidedbcast_request,
		   common_done,
		   consistency,
		   &geometry,
		   root,
		   (char *)srcbuf,
		   src_count*sizeof(unsigned));
    while(!done)
    {
      CCMI_Generic_adaptor_advance();
      //TRACE_TEST_VERBOSE((stderr,"%s:%s advancing done %d\n", argv0,__PRETTY_FUNCTION__, done));
    }
  return;
}

inline void onesidedbcast_advance_repetitions(unsigned * srcbuf, unsigned src_count, unsigned root, unsigned onesided_repetitions)
{
  TRACE_TEST_VERBOSE((stderr, "%s:%s   src %p, count %d, root %d repetitions %d\n", argv0,__PRETTY_FUNCTION__, srcbuf, src_count, root, onesided_repetitions));
  assert(nreceived_countdown);
  if(rank == root)
  {
    for(unsigned i = 0; i < onesided_repetitions; i++) onesidedbcast_advance (srcbuf,src_count,root);
  }
  else
  {
    while(nreceived_countdown)
    {
      CCMI_Generic_adaptor_advance();
      //TRACE_TEST_VERBOSE((stderr,"%s:%s advancing nreceived_countdown %d\n", argv0,__PRETTY_FUNCTION__, nreceived_countdown));
    }
    assert(narrived == (unsigned)onesided_repetitions); narrived = 0; // reset narrived onesided bcasts
  }
}

void initialize(CCMI_Barrier_Protocol barrier_protocol,
                CCMI_Barrier_Protocol lbarrier_protocol,
                CCMI_Broadcast_Protocol onesidedbcast_protocol)
{

  initialize_common(barrier_protocol, lbarrier_protocol);

  common_done.function  = done_callback;
  common_done.clientdata= NULL;

  onesidedbcast_conf.protocol = onesidedbcast_protocol;
  onesidedbcast_conf.cb_geometry = getGeometry;
  onesidedbcast_conf.cb_recv = recvBcast;
  onesidedbcast_conf.isBuffered = 0;

  CCMI_Result ccmiResult;

  if((ccmiResult = (CCMI_Result) CCMI_Broadcast_register (&onesidedbcast_reg, &onesidedbcast_conf)) != XMI_SUCCESS)
    if(rank == 0) fprintf(stderr,"CCMI_OnesidedBcast_register failed %d\n",ccmiResult);

  if(!CCMI_Geometry_analyze(&geometry, &onesidedbcast_reg))
  {
    no_op("Not a supported geometry - NO OP\n");
    exit(0);
  }

  assert((unsigned)(repetitions * sizeof(XMI_CollectiveRequest_t)) > 0);
  recvRequests = (XMI_CollectiveRequest_t *)malloc(repetitions * sizeof(XMI_CollectiveRequest_t));
  assert(recvRequests);
  return;
}
