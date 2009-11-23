/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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

  int n = 1; 
  status = XMI_Context_createv(client, NULL, 0, &context, &n);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", status);
    return 1;
  }

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  status = XMI_Configuration_query(context, &configuration);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
    return 1;
  }
  size_t task_id = configuration.value.intval;   
  //DBG_FPRINTF((stderr, "My task id = %zd\n", task_id);

  configuration.name = XMI_NUM_TASKS;
  status = XMI_Configuration_query(context, &configuration);
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
  __global.topology_global.convertTopology(XMI_LIST_TOPOLOGY);
  __global.topology_local.convertTopology(XMI_LIST_TOPOLOGY);

  // global topology variables
  size_t  gRoot    = __global.topology_global.index2Rank(0);
  size_t *gRankList; __global.topology_global.rankList(&gRankList);
  size_t  gSize    = __global.topology_global.size();

  XMI::Topology src_participants;
  XMI::Topology dst_participants;

  xmi_multicast_t mcast;
  memset(&mcast, 0x00, sizeof(mcast));

  mcast.dispatch = dispatch;
  mcast.hints = options.hint.multicast; //?
  mcast.connection_id = 0xB;
  mcast.msginfo = &_msginfo;
  mcast.msgcount = 1;
  mcast.src_participants = (xmi_topology_t *)&src_participants;
  mcast.dst_participants = (xmi_topology_t *)&dst_participants;

  mcast.src = (xmi_pipeworkqueue_t *)_buffer1.srcPwq();
  mcast.dst = (xmi_pipeworkqueue_t *)_buffer1.dstPwq();

  mcast.client = client;
  mcast.context = 0;
  mcast.roles = -1;
  mcast.bytes = TEST_BUF_SIZE;

  mcast.cb_done = _cb_done;

// ------------------------------------------------------------------------
// 1) Simple mcast to all except root using empty src pwq on root
// 2) Slowly produce into src on root.  
// 3) Validate the buffers.
// ------------------------------------------------------------------------
  {
    _doneCountdown = 1;
    sleep(5); // instead of syncing

    new (&src_participants) XMI::Topology(gRoot); // global root
    new (&dst_participants) XMI::Topology(gRankList+1, (gSize-1)); // everyone except root in dst_participants
    DBG_FPRINTF((stderr,"gRoot %d, gSize %d\n",gRoot, gSize));
    for(size_t j=0;j<gSize;++j)
    {
      DBG_FPRINTF((stderr,"gRankList[%d] = %d\n",j, gRankList[j]));
    }
    XMI_assert(!dst_participants.isRankMember(task_id));

    XMI::PipeWorkQueue * srcPwq =_buffer1.srcPwq();
    if(gRoot == task_id)
    {
      _buffer1.reset(true); // isRoot = true
      srcPwq->configure(NULL, _buffer1.buffer(), TEST_BUF_SIZE, 0);
      srcPwq->reset();
    }
    else _buffer1.reset(false);  // isRoot = false

    mcast.connection_id = 0; 

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
        fprintf(stderr, "FAIL bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
      }
      else
        fprintf(stderr, "PASS bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
    }
    else
    {
      _buffer1.validate(bytesConsumed,
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
#if 0
  sleep(5);
// ------------------------------------------------------------------------
// 1) The last rank slowly mcasts to all 
// 2) Global root pipelines/mcast's the output from 1) to all
// 3) Validate the buffers on root.
// ------------------------------------------------------------------------
  {
    _doneCountdown = 2;
    sleep(5); // instead of syncing

    if(gRoot == task_id)
    {
      new (&src_participants) XMI::Topology(gRoot); // global root
      new (&dst_participants) XMI::Topology(gRankList+1, (gSize-1)); // everyone except root in dst_participants


      mcast.connection_id = 1; // I'm going to use connection id to specify which buffer to receive into.

      _buffer1.reset(true); // isRoot = true;

      // set buffer1 input from buffer2 output

      XMI::PipeWorkQueue * srcPwq =_buffer2.dstPwq();
//      srcPwq->configure(NULL, _buffer1.buffer(), TEST_BUF_SIZE, 0);
//      srcPwq->reset();
      _buffer1.set(srcPwq, _buffer1.dstPwq()); // isRoot = true

      mcast.src = (xmi_pipeworkqueue_t *)srcPwq;
      mcast.dst = (xmi_pipeworkqueue_t *)NULL;

      status = XMI_Multicast(&mcast);
    }
    else _buffer1.reset(); // non-root reset

    if(gRankList[gSize-1] == task_id) // last rank will mcast back
    {
      new (&src_participants) XMI::Topology(gRankList[gSize-1] ); // last rank in the list
      new (&dst_participants) XMI::Topology(gRankList, (gSize-1)); // everyone except last rank in the list

      _buffer2.reset(true ); // isRoot = true

      mcast.connection_id = 2;  // I'm going to use connection id to specify which buffer to receive into.

      XMI::PipeWorkQueue * srcPwq =_buffer2.srcPwq();
      srcPwq->configure(NULL, _buffer2.buffer(), TEST_BUF_SIZE, 0);
      srcPwq->reset();

      mcast.src = (xmi_pipeworkqueue_t *)srcPwq;
      mcast.dst = (xmi_pipeworkqueue_t *)NULL;

      status = XMI_Multicast(&mcast);

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
    else
    {
      _buffer2.reset();  // non-root reset
      while(_doneCountdown)
      {
        status = XMI_Context_advance (context, 10);
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
        fprintf(stderr, "FAIL bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
      }
      else
        fprintf(stderr, "PASS bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
    }
    else
    {
      _buffer1.validate(bytesConsumed,
                        bytesProduced);
      if((bytesConsumed != 0) || 
         (bytesProduced != TEST_BUF_SIZE))
      {
        fprintf(stderr, "FAIL bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
      }
      else
        fprintf(stderr, "PASS bytesConsumed = %zd, bytesProduced = %zd\n", bytesConsumed, bytesProduced);
    }
    if(gRankList[gSize-1] == task_id) // last rank will mcast back
    {
      _buffer2.validate(bytesConsumed,
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
      _buffer2.validate(bytesConsumed,
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
// ------------------------------------------------------------------------
#endif
  sleep(5);
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
