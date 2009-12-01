/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/multisend/multicast_local_as.cc
 * \brief Simple all-sided multicast tests on local topology.
 */

#include "test/multisend/Buffer.h"

#include "Global.h"
#include <unistd.h>
#ifndef TEST_BUF_SIZE
  #define TEST_BUF_SIZE	32
#endif // TEST_BUF_SIZE


static XMI::Test::Buffer<TEST_BUF_SIZE> _buffer;

static int           _doneCountdown;
xmi_callback_t       _cb_done;
const xmi_quad_t     _msginfo = {0,1,2,3};

void _done_cb(xmi_context_t context, void *cookie, xmi_result_t err)
{
  XMI_assertf(_doneCountdown > 0,"doneCountdown %d\n",_doneCountdown);
  volatile int *doneCountdown = (volatile int*) cookie;
  DBG_FPRINTF((stderr, "%s:%s done %d/%d \n",__FILE__,__PRETTY_FUNCTION__, *doneCountdown,_doneCountdown));
  --*doneCountdown;
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
  DBG_FPRINTF((stderr,"Client %p\n",client));
  int n = 1;
  status = XMI_Context_createv(client, NULL, 0, &context, &n);
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
  DBG_FPRINTF((stderr, "My task id = %zd\n", task_id));

  configuration.name = XMI_NUM_TASKS;
  status = XMI_Configuration_query(client, &configuration);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  if(task_id == 0) fprintf(stderr, "Number of tasks = %zd\n", num_tasks);

// END standard setup
// ------------------------------------------------------------------------
  _cb_done.function   = &_done_cb;
  _cb_done.clientdata = &_doneCountdown;

  size_t ldispatch = 2, rdispatch=3;

  xmi_dispatch_callback_fn   fn;

  fn.multicast = NULL; // all-sided, no recv callback

  xmi_dispatch_hint_t        options;
  memset(&options, 0x00, sizeof(options));

  options.type = XMI_MULTICAST;

  options.config = NULL;

  options.hint.multicast.local     = 1;
  options.hint.multicast.all_sided = 1;
  options.hint.multicast.active_message = 0;
  // register local bcast 
  status = XMI_Dispatch_set_new(context,
                                ldispatch,
                                fn,
                                NULL,
                                options);



  options.hint.multicast.ring_wq   = 1;

  // register local ring bcast
  status = XMI_Dispatch_set_new(context,
                                rdispatch,
                                fn,
                                NULL,
                                options);

  //For testing ease, I'm assuming rank list topology, so convert them
  XMI::Topology topology_global = __global.topology_global; 
  topology_global.convertTopology(XMI_LIST_TOPOLOGY);

  XMI::Topology topology_local  = __global.topology_local; 
  topology_local.convertTopology(XMI_LIST_TOPOLOGY);

  // local topology variables
  size_t  lRoot    = topology_local.index2Rank(0);
  size_t *lRankList; topology_local.rankList(&lRankList);
  size_t  lSize   =  topology_local.size();

  XMI::Topology src_participants;
  XMI::Topology dst_participants;

  xmi_multicast_t mcast;
  memset(&mcast, 0x00, sizeof(mcast));

  mcast.connection_id = 0xB;
  mcast.msginfo = &_msginfo;
  mcast.msgcount = 1;
  mcast.src_participants = (xmi_topology_t *)&src_participants;
  mcast.dst_participants = (xmi_topology_t *)&dst_participants;

  mcast.src = (xmi_pipeworkqueue_t *)_buffer.srcPwq();
  mcast.dst = (xmi_pipeworkqueue_t *)_buffer.dstPwq();

  mcast.client = client;
  mcast.context = 0;
  mcast.roles = -1;
  mcast.bytes = TEST_BUF_SIZE;

  mcast.cb_done = _cb_done;

// ------------------------------------------------------------------------
// simple ring local mcast to all except root
// ------------------------------------------------------------------------
  {
    _doneCountdown = 1;
    //sleep(5); // instead of syncing

    new (&src_participants) XMI::Topology(lRoot); // local root
    new (&dst_participants) XMI::Topology(lRankList+1, (lSize-1)); // everyone except root in dst_participants

    DBG_FPRINTF((stderr,"lRoot %d, lSize %d\n",lRoot, lSize));
    for(size_t j=0;j<lSize;++j)
    {
      DBG_FPRINTF((stderr,"lRankList[%d] = %d\n",j, lRankList[j]));
    }

    if(lRoot == task_id)
      _buffer.reset(true); // isRoot = true
    else _buffer.reset(false);  // isRoot = false


    mcast.dispatch = rdispatch;

    status = XMI_Multicast(&mcast);

    while(_doneCountdown)
    {
      status = XMI_Context_advance (context, 10);
    }

    size_t
    bytesConsumed = 0,
    bytesProduced = 0;

    if(lRoot == task_id)
    {
      _buffer.validate(bytesConsumed,
                       bytesProduced,
                       true,   // isRoot = true
                       false); // isDest = false
      if((bytesConsumed != TEST_BUF_SIZE) ||
         (bytesProduced != 0))
      {
        fprintf(stderr, "FAIL bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
      }
      else
        fprintf(stderr, "PASS bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
    }
    else
    {
      _buffer.validate(bytesConsumed,
                       bytesProduced);
      if((bytesConsumed != 0) ||
         (bytesProduced != TEST_BUF_SIZE))
      {
        fprintf(stderr, "FAIL bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
      }
      else
        fprintf(stderr, "PASS bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
    }
  }
// ------------------------------------------------------------------------
// simple local mcast to all except root
// ------------------------------------------------------------------------
  {
    _doneCountdown = 1;
    //sleep(5); // instead of syncing

    new (&src_participants) XMI::Topology(lRoot); // local root
    new (&dst_participants) XMI::Topology(lRankList+1, (lSize-1)); // everyone except root in dst_participants

    DBG_FPRINTF((stderr,"lRoot %d, lSize %d\n",lRoot, lSize));
    for(size_t j=0;j<lSize;++j)
    {
      DBG_FPRINTF((stderr,"lRankList[%d] = %d\n",j, lRankList[j]));
    }

    if(lRoot == task_id)
      _buffer.reset(true); // isRoot = true
    else _buffer.reset(false);  // isRoot = false


    mcast.dispatch = ldispatch;

    status = XMI_Multicast(&mcast);

    while(_doneCountdown)
    {
      status = XMI_Context_advance (context, 10);
    }

    size_t
    bytesConsumed = 0,
    bytesProduced = 0;

    if(lRoot == task_id)
    {
      _buffer.validate(bytesConsumed,
                       bytesProduced,
                       true,   // isRoot = true
                       false); // isDest = false
      if((bytesConsumed != TEST_BUF_SIZE) ||
         (bytesProduced != 0))
      {
        fprintf(stderr, "FAIL bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
      }
      else
        fprintf(stderr, "PASS bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
    }
    else
    {
      _buffer.validate(bytesConsumed,
                       bytesProduced);
      if((bytesConsumed != 0) ||
         (bytesProduced != TEST_BUF_SIZE))
      {
        fprintf(stderr, "FAIL bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
      }
      else
        fprintf(stderr, "PASS bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
    }
  }
#if 0
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// simple local mcast to all including root
// ------------------------------------------------------------------------
  {
    _doneCountdown = 1;
    //sleep(5); // instead of syncing

    new (&src_participants) XMI::Topology(lRoot); // local root
    new (&dst_participants) XMI::Topology(lRankList, lSize); // include root in dst_participants
    if(lRoot == task_id)
    {
      _buffer.reset(true); // isRoot = true
      // need a non-null dst pwq since I'm now including myself as a dst
      mcast.dst = (xmi_pipeworkqueue_t *)_buffer.dstPwq();

      status = XMI_Multicast(&mcast);
    }

    while(_doneCountdown)
    {
      status = XMI_Context_advance (context, 10);
    }

    size_t
    bytesConsumed = 0,
    bytesProduced = 0;

    if(lRoot == task_id)
    {
      _buffer.validate(bytesConsumed,
                       bytesProduced,
                       true,   // isRoot = true
                       true);  // isDest = true
      if((bytesConsumed != TEST_BUF_SIZE) ||
         (bytesProduced != TEST_BUF_SIZE))
      {
        fprintf(stderr, "FAIL bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
      }
      else
        fprintf(stderr, "PASS bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
    }
    else
    {
      _buffer.validate(bytesConsumed,
                       bytesProduced);
      if((bytesConsumed != 0) ||
         (bytesProduced != TEST_BUF_SIZE))
      {
        fprintf(stderr, "FAIL bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
      }
      else
        fprintf(stderr, "PASS bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
    }
  }
// ------------------------------------------------------------------------

  //sleep(5);
#endif
// ------------------------------------------------------------------------
  DBG_FPRINTF((stderr, "XMI_Context_destroy(context);\n"));
  status = XMI_Context_destroy(context);
  if(status != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to destroy xmi context. result = %d\n", status);
    return 1;
  }

  DBG_FPRINTF((stderr, "XMI_Client_finalize(client);\n"));
  status = XMI_Client_finalize(client);
  if(status != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to finalize xmi client. result = %d\n", status);
    return 1;
  }

  DBG_FPRINTF((stderr, "return 0;\n"));
  return 0;
}
