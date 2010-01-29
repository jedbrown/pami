/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/multisend/multicast_pwq_as.cc
 * \brief Simple all-sided multicast test using pwq's to chain operations.
 */

#include "test/multisend/Buffer.h"

#include "Global.h"
#include <unistd.h>
#ifndef TEST_BUF_SIZE
  #define TEST_BUF_SIZE	32
#endif // TEST_BUF_SIZE


static XMI::Test::Buffer<TEST_BUF_SIZE> _buffer1;
static XMI::Test::Buffer<TEST_BUF_SIZE> _buffer2;

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
  size_t n = 1;
  status = XMI_Context_createv(client, NULL, 0, &context, n);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", status);
    return 1;
  }
  DBG_FPRINTF((stderr,"Context %p\n",context));

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  status = XMI_Configuration_query(client, &configuration);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  DBG_FPRINTF((stderr, "My task id = %zu\n", task_id));

  configuration.name = XMI_NUM_TASKS;
  status = XMI_Configuration_query(client, &configuration);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  if(task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);

// END standard setup
// ------------------------------------------------------------------------
  _cb_done.function   = &_done_cb;
  _cb_done.clientdata = &_doneCountdown;

  size_t                     dispatch = 2;

  xmi_dispatch_callback_fn   fn;

  fn.multicast = NULL; // all-sided, no recv callback

  xmi_dispatch_hint_t        options;
  memset(&options, 0x00, sizeof(options));

  options.type = XMI_MULTICAST;

  options.config = NULL;

  options.hint.multicast.global = 1;
  options.hint.multicast.all_sided = 1;
  options.hint.multicast.active_message = 0;

  status = XMI_Dispatch_set_new(context,
                                dispatch,
                                fn,
                                NULL,
                                options);

  //For testing ease, I'm assuming rank list topology, so convert them
  XMI::Topology topology_global = __global.topology_global;
  topology_global.convertTopology(XMI_LIST_TOPOLOGY);

  XMI::Topology topology_local  = __global.topology_local;
  topology_local.convertTopology(XMI_LIST_TOPOLOGY);

  // global topology variables
  xmi_task_t  gRoot    = topology_global.index2Rank(0);
  xmi_task_t *gRankList=NULL; topology_global.rankList(&gRankList);
  size_t  gSize    = topology_global.size();

  DBG_FPRINTF((stderr,"gRoot %d, gSize %zd\n",gRoot, gSize));
  for(size_t j=0;j<gSize;++j)
  {
    DBG_FPRINTF((stderr,"gRankList[%zd] = %d\n",j, gRankList[j]));
  }

  xmi_multicast_t mcast;
  memset(&mcast, 0x00, sizeof(mcast));

  mcast.dispatch = dispatch;

  mcast.msginfo = &_msginfo;
  mcast.msgcount = 1;
  mcast.src_participants = (xmi_topology_t *)NULL;
  mcast.dst_participants = (xmi_topology_t *)NULL;


  mcast.client = 0;
  mcast.context = 0;
  mcast.roles = -1;
  mcast.bytes = (TEST_BUF_SIZE/4)*4;

  mcast.cb_done = _cb_done;

// ------------------------------------------------------------------------
// 1) Simple mcast to all except root using empty src pwq on root
// 2) Slowly produce into src on root.
// 3) Validate the buffers.
// ------------------------------------------------------------------------
  {
    _doneCountdown = 1;
    //sleep(5); // instead of syncing

    mcast.src_participants = (xmi_topology_t *)new XMI::Topology(gRoot); // global root (mem leak)
    mcast.dst_participants = (xmi_topology_t *)new XMI::Topology(gRankList+1, (gSize-1)); // everyone except root in dst_participants (mem leak)
    DBG_FPRINTF((stderr,"gRoot %d, gSize %zd\n",gRoot, gSize));
    for(size_t j=0;j<gSize;++j)
    {
      DBG_FPRINTF((stderr,"gRankList[%zd] = %d\n",j, gRankList[j]));
    }

    mcast.connection_id = 1;
    XMI::PipeWorkQueue * srcPwq =_buffer1.srcPwq();
    if(gRoot == task_id)
    {
      _buffer1.reset(true); // isRoot = true
      srcPwq->configure(NULL, _buffer1.buffer(), TEST_BUF_SIZE, 0);
      srcPwq->reset();
      mcast.src = (xmi_pipeworkqueue_t *) srcPwq;
      mcast.dst = (xmi_pipeworkqueue_t *) NULL;
    }
    else
    {
      _buffer1.reset(false);  // isRoot = false
      mcast.src = (xmi_pipeworkqueue_t *) NULL;
      mcast.dst = (xmi_pipeworkqueue_t *) _buffer1.dstPwq();
    }

    status = XMI_Multicast(&mcast);
    sleep(1);
    unsigned dataCountDown = 17;
    while(_doneCountdown)
    {
      status = XMI_Context_advance (context, 10);
      if(gRoot == task_id)
        if(dataCountDown && !(--dataCountDown % 4)) // slowly feed the src pwq
        {
          sleep(1);
          DBG_FPRINTF((stderr,"pwq %p, produce %d\n",srcPwq, TEST_BUF_SIZE/4));
          srcPwq->produceBytes(TEST_BUF_SIZE/4);
        }
    }
    size_t
    bytesConsumed = 0,
    bytesProduced = 0;

    if(gRoot == task_id)
    {
      _buffer1.validate(bytesConsumed,
                        bytesProduced,
                        true,   // isRoot = true
                        false); // isDest = false
      if((bytesConsumed != TEST_BUF_SIZE) ||
         (bytesProduced != 0))
      {
        fprintf(stderr, "FAIL bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
      }
      else
        fprintf(stderr, "PASS bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
    }
    else
    {
      _buffer1.validate(bytesConsumed,
                        bytesProduced);
      if((bytesConsumed != 0) ||
         (bytesProduced != TEST_BUF_SIZE))
      {
        fprintf(stderr, "FAIL bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
      }
      else
        fprintf(stderr, "PASS bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
    }
  }
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// 1) Root mcast to "spanning" topology (0th rank in each local)
// 2) 0th rank in each locale mcast's the output from 1) to locale
// ------------------------------------------------------------------------
  {
    // local topology variables
    xmi_task_t  lRoot    = topology_local.index2Rank(0);
    xmi_task_t *lRankList=NULL; topology_local.rankList(&lRankList);
    size_t  lSize   =  topology_local.size();

    options.type = XMI_MULTICAST;

    options.config = NULL;

    options.hint.multicast.spanning = 1;
    options.hint.multicast.one_sided = 1;
    options.hint.multicast.active_message = 1;

    size_t                     spanning_dispatch = dispatch+1;
    status = XMI_Dispatch_set_new(context,
                                  spanning_dispatch,
                                  fn,
                                  NULL,
                                  options);

    options.hint.multicast.local = 1;
    options.hint.multicast.one_sided = 1;
    options.hint.multicast.active_message = 1;

    size_t                     local_dispatch = spanning_dispatch+1;
    status = XMI_Dispatch_set_new(context,
                                  local_dispatch,
                                  fn,
                                  NULL,
                                  options);

    _doneCountdown = 1;
    //sleep(5); // instead of syncing

    {
      DBG_FPRINTF((stderr,"task_id %u -> 1st ranks\n", gRoot));
      mcast.src_participants = (xmi_topology_t *)new XMI::Topology(gRoot); // global root (mem leak)
      // This isn't working correctly, so do it the hard way
      //topology_global.subTopologyNthGlobal(&dst_participants, 0); //0th rank on each locale

      XMI::Topology topology(gRankList, (gSize)), subtopology;
      topology.convertTopology(XMI_COORD_TOPOLOGY);
      topology.subTopologyNthGlobal(&subtopology, 0);

      xmi_task_t *ranklist = new xmi_task_t[subtopology.size()];

      // loop all global ranks, if they're in the subtopology, put then in a ranklist.
      for(size_t i = 0, j = 0; i < gSize; ++i)
      {
        if(subtopology.isRankMember(i)) ranklist[j++] = i;
        XMI_assert(j<=subtopology.size());
      }

      mcast.dst_participants = (xmi_topology_t *)new XMI::Topology(ranklist, subtopology.size());// (mem leak)

      mcast.dispatch = spanning_dispatch;
      mcast.connection_id = 1; // I'm going to use connection id to specify which buffer to receive into.

      if(gRoot == task_id)
      {
        _buffer1.reset(true); // isRoot = true;

        XMI::PipeWorkQueue * srcPwq =_buffer1.srcPwq();
        srcPwq->configure(NULL, _buffer1.buffer(), (TEST_BUF_SIZE/4)*4, 0);
        srcPwq->reset();


        mcast.src = (xmi_pipeworkqueue_t *)srcPwq;
        mcast.dst = (xmi_pipeworkqueue_t *)_buffer1.dstPwq();
        status = XMI_Multicast(&mcast);
      }
      else
      {
        _buffer1.reset();
        mcast.src = (xmi_pipeworkqueue_t *)NULL;
        mcast.dst = (xmi_pipeworkqueue_t *)_buffer1.dstPwq();

        if(((XMI::Topology*)mcast.dst_participants)->isRankMember(task_id))
          status = XMI_Multicast(&mcast);
      }
    }

    {
      mcast.src_participants = (xmi_topology_t *)new XMI::Topology(lRoot); // root (mem leak)
      mcast.dst_participants = (xmi_topology_t *)new XMI::Topology(lRankList+1, (lSize-1)); // everyone except root in dst_participants (mem leak)

      if(lRoot == task_id) // I am 0th rank on this local topology
      {
        DBG_FPRINTF((stderr,"task_id %zu -> local ranks\n",task_id));
        ++_doneCountdown;  // I'm doing another multicast so another cb_done is expected
        _buffer2.reset(true ); // isRoot = true

        // set buffer2 input from buffer1 output (for validation)
        XMI::PipeWorkQueue * srcPwq =_buffer1.dstPwq();
        _buffer2.set(srcPwq, _buffer2.dstPwq());

        mcast.src = (xmi_pipeworkqueue_t *)srcPwq;
        mcast.dst = (xmi_pipeworkqueue_t *)NULL;
      }
      else
      {
        _buffer2.reset();
        mcast.src = (xmi_pipeworkqueue_t *)NULL;
        mcast.dst = (xmi_pipeworkqueue_t *)_buffer2.dstPwq();
      }
      mcast.dispatch = local_dispatch;
      mcast.connection_id = 2;


      DBG_FPRINTF((stderr,"Context %p\n",context));
      status = XMI_Multicast(&mcast);


    }

    if(gRoot == task_id)
    {
      XMI::PipeWorkQueue * srcPwq =_buffer1.srcPwq();
      sleep(1);
      unsigned dataCountDown = 17;
      while(_doneCountdown)
      {
        status = XMI_Context_advance (context, 10);
        if(dataCountDown && !(--dataCountDown % 4)) // slowly feed the src pwq
        {
          sleep(1);
          DBG_FPRINTF((stderr,"pwq %p, produce %d\n",srcPwq, TEST_BUF_SIZE/4));
          srcPwq->produceBytes(TEST_BUF_SIZE/4);
        }
      }
    }
    while(_doneCountdown)
    {
      status = XMI_Context_advance (context, 10);
    }
    size_t
    bytesConsumed = 0,
    bytesProduced = 0;

    if(gRoot == task_id)
    {
      bytesConsumed = _buffer2.srcPwq()->getBytesConsumed();
      if(bytesConsumed != (TEST_BUF_SIZE/4)*4)
        fprintf(stderr, "FAIL bytesConsumed = %zu\n", bytesConsumed);
      else
        fprintf(stderr, "PASS bytesConsumed = %zu\n", bytesConsumed);
      _buffer1.validate(bytesConsumed,
                        bytesProduced,
                        true,   // isRoot = true
                        true); // isDest = false
      if((bytesConsumed != (TEST_BUF_SIZE/4)*4) ||
         (bytesProduced != (TEST_BUF_SIZE/4)*4))
        fprintf(stderr, "FAIL bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
      else
        fprintf(stderr, "PASS bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
      _buffer2.validate(bytesConsumed,
                        bytesProduced,
                        true,   // isRoot = true
                        false); // isDest = false
    }
    else if(topology_local.index2Rank(0) == task_id) // I am 0th rank on this local topology
    {
      bytesConsumed = _buffer1.srcPwq()->getBytesConsumed();
      bytesProduced = _buffer1.dstPwq()->getBytesProduced();
      if((bytesProduced != (TEST_BUF_SIZE/4)*4) ||
         (bytesConsumed != 0))
        fprintf(stderr, "FAIL bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
      else
        fprintf(stderr, "PASS bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
      _buffer2.validate(bytesConsumed,
                        bytesProduced,
                        true,   // isRoot = true
                        false); // isDest = false
      if((bytesConsumed != (TEST_BUF_SIZE/4)*4) ||
         (bytesProduced != 0))
        fprintf(stderr, "FAIL bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
      else
        fprintf(stderr, "PASS bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
      _buffer1.validate(bytesConsumed,
                        bytesProduced);
    }
    else
    {
      _buffer2.validate(bytesConsumed,
                        bytesProduced);
      if((bytesConsumed != 0) ||
         (bytesProduced != (TEST_BUF_SIZE/4)*4))
        fprintf(stderr, "FAIL bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
      else
        fprintf(stderr, "PASS bytesConsumed = %zu, bytesProduced = %zu\n", bytesConsumed, bytesProduced);
    }
  }
// ------------------------------------------------------------------------

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
