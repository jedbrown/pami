///
/// \file test/internals/multisend/mpi/mpimulticast.cc
/// \brief ???
///

#include <stdio.h>
#include "sys/xmi.h"

#include "components/devices/mpi/MPIBcastMsg.h"
#include "components/devices/mpi/mpimulticastprotocol.h"
#include "components/devices/mpi/mpidevice.h"
#include "components/devices/mpi/mpipacketmodel.h"
#include "components/devices/mpi/mpimessage.h"
#include "p2p/protocols/send/adaptive/Adaptive.h"
#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/eager/EagerSimple.h"
#include "p2p/protocols/send/eager/EagerImmediate.h"
#include "SysDep.h"

typedef XMI::Device::MPIDevice<XMI::SysDep> MPIDevice;
typedef XMI::Device::MPIMessage MPIMessage;
typedef XMI::Device::MPIPacketModel<MPIDevice,MPIMessage> MPIPacketModel;
typedef XMI::Protocol::Send::Eager <MPIPacketModel,MPIDevice> EagerMPI;
typedef XMI::Protocol::MPI::P2pDispatchMulticastProtocol<MPIDevice,EagerMPI,XMI::Device::MPIBcastMdl> P2pDispatchMulticastProtocol;

#ifndef TEST_BUF_SIZE
  #define TEST_BUF_SIZE	1024
#endif // TEST_BUF_SIZE

XMI::Topology itopo;
XMI::Topology otopo;
XMI::PipeWorkQueue _ipwq;
XMI::PipeWorkQueue _opwq;
xmi_callback_t       _cb_done;
const xmi_quad_t     _msginfo = {0,1,2,3};
static   void       *_cookie=(void*)"HI COOKIE";
int _done;

static void _done_cb(xmi_context_t context, void *cookie, xmi_result_t result)
{
  int* done = (int*)cookie;
  //fprintf(stderr, "... completion callback, done %d/%d ++\n", *done,_done);
  ++*done;
}

void dispatch_multicast_fn(const xmi_quad_t     *msginfo,
                           unsigned              msgcount,
                           unsigned              connection_id,
                           size_t                root,
                           size_t                sndlen,
                           void                 *clientdata,
                           size_t               *rcvlen,
                           xmi_pipeworkqueue_t **rcvpwq,
                           xmi_callback_t       *cb_done)
{
  //fprintf(stderr,"%s:%s msgcount %d, connection_id %d, root %d, sndlen %d, cookie %s\n",
  //        __FILE__,__PRETTY_FUNCTION__,msgcount, connection_id, root, sndlen, (char*) clientdata);

  XMI_assertf(strcmp((char*)clientdata,(char*)_cookie) == 0,"clientdata (%.8s) != cookie %s\n",(char*) clientdata,(char*)_cookie);
  XMI_assertf(_done == 0,"done %d\n",_done);
  XMI_assertf(sndlen <= TEST_BUF_SIZE,"sndlen %zd\n",sndlen);
  XMI_assertf(msgcount == 1,"msgcount %d",msgcount);
  XMI_assertf(msginfo->w0 == _msginfo.w0,"msginfo->w0=%d\n",msginfo->w0);
  XMI_assertf(msginfo->w1 == _msginfo.w1,"msginfo->w1=%d\n",msginfo->w1);
  XMI_assertf(msginfo->w2 == _msginfo.w2,"msginfo->w2=%d\n",msginfo->w2);
  XMI_assertf(msginfo->w3 == _msginfo.w3,"msginfo->w3=%d\n",msginfo->w3);

  if(connection_id == 1) // no data being sent
  {
    *rcvlen = 0;
    *rcvpwq = (xmi_pipeworkqueue_t*) NULL;
    if(sndlen == 0)
      fprintf(stderr, "PASS: msgdata received with no data\n");
    else
      fprintf(stderr, "FAIL: no data expected\n");
  }
  else
  {
    XMI::PipeWorkQueue * pwq;
    pwq = &_opwq;

//    fprintf(stderr,"%s:%s bytesAvailable (%p) %d, %d done out of %d\n",__FILE__,__PRETTY_FUNCTION__,
//            pwq,pwq->bytesAvailableToProduce(),pwq->getBytesProduced(),sndlen);

    *rcvlen = sndlen;
    *rcvpwq = (xmi_pipeworkqueue_t*) pwq;
  }

  *cb_done = _cb_done;

}

int main(int argc, char ** argv)
{
  unsigned x;
  xmi_client_t client;
  xmi_context_t context;
  xmi_result_t status = XMI_ERROR;

  status = XMI_Client_initialize("multicast test", &client);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", status);
    return 1;
  }

  { int _n = 1; status = XMI_Context_createv(client, NULL, 0, &context, &_n);}
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", status);
    return 1;
  }

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  status = XMI_Configuration_query(client, &configuration);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  //fprintf(stderr, "My task id = %zd\n", task_id);

  configuration.name = XMI_NUM_TASKS;
  status = XMI_Configuration_query(client, &configuration);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  if(task_id == 0) fprintf(stderr, "Number of tasks = %zd\n", num_tasks);
  if(num_tasks < 2)
  {
    fprintf(stderr, "requires at least 2 ranks\n");
    exit(1);
  }

// END standard setup
// ------------------------------------------------------------------------

  xmi_result_t rc;
  // Register some multicasts, C++ style

  size_t root = __global.topology_global.index2Rank(0);
//  if(task_id == root) fprintf(stderr, "Number of tasks = %zd\n", __global.topology_global.size());

  new (&itopo) XMI::Topology(root);
  __global.topology_global.subtractTopology(&otopo, &itopo);

// not necessary right now, but will be later	char _msgbuf[T_MulticastModel::sizeof_msg];

  char _source[TEST_BUF_SIZE];
  char _result[TEST_BUF_SIZE];

  _cb_done.function   = &_done_cb;
  _cb_done.clientdata = &_done;

  _ipwq.configure(NULL, _source, sizeof(_source), sizeof(_source));
  _ipwq.reset();
  _opwq.configure(NULL, _result, sizeof(_result), 0);
  _opwq.reset();

  xmi_multicast_t mcast;
  memset(&mcast, 0x00, sizeof(mcast));
  // simple allreduce 
  mcast.client = (xmi_client_t*) NULL; // temporary test for sameer without client
  mcast.context = 0;
  mcast.roles = (unsigned)-1;
  mcast.src_participants = (xmi_topology_t *)&itopo;
  mcast.dst_participants = (xmi_topology_t *)&otopo;
  mcast.msginfo = &_msginfo;
  mcast.msgcount = 1;
 
  // simple allreduce on the local ranks...
  mcast.request = NULL;//&_msgbuf; temporary
  mcast.cb_done = (xmi_callback_t) _cb_done;

  mcast.connection_id = 5;  
  mcast.src = (xmi_pipeworkqueue_t *)&_ipwq;
  mcast.dst = (xmi_pipeworkqueue_t *)NULL;
  mcast.bytes = TEST_BUF_SIZE;

  size_t count = mcast.bytes / sizeof(unsigned);
  for(x = 0; x < count; ++x)
  {
    ((unsigned *)_source)[x] = (task_id == root)?1:-1;
    ((unsigned *)_result)[x] = -1;
  }
  _done = 0;

  status = XMI_SUCCESS;
  P2pDispatchMulticastProtocol * multicast = new P2pDispatchMulticastProtocol(status);
//P2pDispatchMulticastProtocol * multicast = new P2pDispatchMulticastProtocol(1, &dispatch_multicast_fn, _cookie,
//                                                                            __global.mpi_device,
//                                                                            NULL,//this->_client,
//                                                                            NULL,//this->_context,
//                                                                            0,//this->_contextid,
//                                                                            status);
  multicast->registerMcastRecvFunction (1,&dispatch_multicast_fn, _cookie);

  if(status != XMI_SUCCESS)
  {
    fprintf(stderr, "Failed to ctor multicast \n");
    return XMI_ERROR;
  }

  if(task_id == root) fprintf(stderr,"Variation 1: root multicasts to other ranks\n");

  if(task_id == root)
  {
    multicast->postMulticast(&mcast);
//    if(status != XMI_SUCCESS)
//    {
//      fprintf(stderr, "Failed to post multicast \n");
//      return XMI_ERROR;
//    }

  }

  //fprintf(stderr, "... before advance loop for %s.postMulticast\n", _name);
  while(!_done)
  {
    rc = XMI_Context_advance(context, 100);

    if(rc != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", rc);
      return rc;
    }
  }
  if(task_id == root)
  {
    for(x = 0; x < count; ++x)
    {
      if(((unsigned *)_source)[x] != 1)
      {
        fprintf(stderr, "Corrupted source buffer at index %zd.\n", x);
        break;
      }
      if(((unsigned *)_result)[x] != (unsigned)-1)
      {
        fprintf(stderr, "Corrupted result buffer at index %zd.\n", x);
        break;
      }
    }
  }
  else
  {
    for(x = 0; x < count; ++x)
    {
      if(((unsigned *)_source)[x] != (unsigned)-1)
      {
        fprintf(stderr, "Corrupted source buffer at index %zd.\n", x);
        break;
      }
      if(((unsigned *)_result)[x] != 1)
      {
        fprintf(stderr, "Incorrect result at index %zd.\n", x);
        break;
      }
    }
  }
  if(x < count)
    fprintf(stderr, "Failed validation\n");
  else  fprintf(stderr, "PASS validation\n");

  sleep(2);
  if(task_id == root) fprintf(stderr,"Variation 2: root multicasts no data - only msgdata - to other ranks \n");

  mcast.connection_id = 1;  // no data connection id (see dispatch_multicast_fn())
  mcast.src = (xmi_pipeworkqueue_t *)NULL;
  mcast.dst = (xmi_pipeworkqueue_t *)NULL;
  mcast.bytes = 0;

  _done = 0;

  if(task_id == root)
  {
    multicast->postMulticast(&mcast);
//    if(status != XMI_SUCCESS)
//    {
//      fprintf(stderr, "Failed to post multicast \n");
//      return XMI_ERROR;
//    }

  }

  //fprintf(stderr, "... before advance loop for %s.postMulticast\n", _name);
  while(!_done)
  {
    rc = XMI_Context_advance(context, 100);

    if(rc != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", rc);
      return rc;
    }
  }

// ------------------------------------------------------------------------
  status = XMI_Context_destroy(context);
  if(status != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to destroy xmi context. result = %d\n", status);
    return 1;
  }

  status = XMI_Client_finalize(client);
  if(status != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to finalize xmi client. result = %d\n", status);
    return 1;
  }

  return 0;
}
