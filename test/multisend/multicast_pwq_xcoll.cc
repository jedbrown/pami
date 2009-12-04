/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/multisend/multicast_pwq_xcoll.cc
 * \brief Simple multicast test using pwq's to chain operations.
 */
#ifdef DISABLE_COLLDEVICE
  #warning generic device disabled
int main(int argc, char **argv) {
return 0;
}
#else

#include "test/multisend/Buffer.h"

#include "Global.h"
#include <unistd.h>
#ifndef TEST_BUF_SIZE
  #define TEST_BUF_SIZE	32
#endif // TEST_BUF_SIZE


static XMI::Test::Buffer<TEST_BUF_SIZE> _buffer1;
static XMI::Test::Buffer<TEST_BUF_SIZE> _buffer2;

static   void       *_cookie=(void*)"HI COOKIE";
static int           _doneCountdown;
xmi_callback_t       _cb_done;
const xmi_quad_t     _msginfo = {0,1,2,3};

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
  DBG_FPRINTF((stderr,"%s:%s msgcount %d, connection_id %d, root %d, sndlen %d, cookie %s\n",
               __FILE__,__PRETTY_FUNCTION__,msgcount, connection_id, root, sndlen, (char*) clientdata));
  XMI_assertf(_doneCountdown > 0,"doneCountdown %d\n",_doneCountdown);
  XMI_assertf(sndlen <= TEST_BUF_SIZE,"sndlen %zd\n",sndlen);
  XMI_assertf(msgcount == 1,"msgcount %d",msgcount);
  XMI_assertf(msginfo->w0 == _msginfo.w0,"msginfo->w0=%d\n",msginfo->w0);
  XMI_assertf(msginfo->w1 == _msginfo.w1,"msginfo->w1=%d\n",msginfo->w1);
  XMI_assertf(msginfo->w2 == _msginfo.w2,"msginfo->w2=%d\n",msginfo->w2);
  XMI_assertf(msginfo->w3 == _msginfo.w3,"msginfo->w3=%d\n",msginfo->w3);

  XMI::PipeWorkQueue * pwq;

  if(connection_id == 1)
    pwq = _buffer1.dstPwq();
  else if(connection_id == 2)
    pwq = _buffer2.dstPwq();
  else
    XMI_abortf("connection id %d",connection_id);

  DBG_FPRINTF((stderr,"%s:%s bytesAvailable (%p) %d, %d done out of %d\n",__FILE__,__PRETTY_FUNCTION__,
               pwq,pwq->bytesAvailableToProduce(),pwq->getBytesProduced(),sndlen));

  *rcvlen = sndlen;

  *rcvpwq = (xmi_pipeworkqueue_t*) pwq;

  *cb_done = _cb_done;

}

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

  size_t                     dispatch = 2;

  xmi_dispatch_callback_fn   fn;

  fn.multicast = &dispatch_multicast_fn;

  xmi_dispatch_hint_t        options;
  memset(&options, 0x00, sizeof(options));

  options.type = XMI_MULTICAST;

  options.config = NULL;

  options.hint.multicast.global = 1;
  options.hint.multicast.one_sided = 1;
  options.hint.multicast.collective = 1;
  options.hint.multicast.active_message = 1;

  status = XMI_Dispatch_set_new(context,
                                dispatch,
                                fn,
                                _cookie,
                                options);

  //For testing ease, I'm assuming rank list topology, so convert them
  XMI::Topology topology_global = __global.topology_global;
  topology_global.convertTopology(XMI_LIST_TOPOLOGY);

  XMI::Topology topology_local  = __global.topology_local;
  topology_local.convertTopology(XMI_LIST_TOPOLOGY);

  // global topology variables
  xmi_task_t  gRoot    = topology_global.index2Rank(0);
  xmi_task_t *gRankList; topology_global.rankList(&gRankList);
  size_t  gSize    = topology_global.size();

  DBG_FPRINTF((stderr,"gRoot %d, gSize %d\n",gRoot, gSize));
  for(size_t j=0;j<gSize;++j)
  {
    DBG_FPRINTF((stderr,"gRankList[%d] = %d\n",j, gRankList[j]));
  }
  XMI::Topology src_participants;
  XMI::Topology dst_participants;

  xmi_multicast_t mcast;
  memset(&mcast, 0x00, sizeof(mcast));
  if((gRoot == task_id) || (gRankList[gSize-1] == task_id))
  {

    mcast.dispatch = dispatch;

    mcast.msginfo = &_msginfo;
    mcast.msgcount = 1;
    mcast.src_participants = (xmi_topology_t *)&src_participants;
    mcast.dst_participants = (xmi_topology_t *)&dst_participants;


	mcast.client = client;
	mcast.context = 0;
    mcast.roles = -1;
    mcast.bytes = (TEST_BUF_SIZE/4)*4;

    mcast.cb_done = _cb_done;
  }

// ------------------------------------------------------------------------
// 1) Simple mcast to all except root using empty src pwq on root
// 2) Slowly produce into src on root.
// 3) Validate the buffers.
// ------------------------------------------------------------------------
  {
    _doneCountdown = 1;
    //sleep(5); // instead of syncing

    if(gRoot == task_id)
    {
      new (&src_participants) XMI::Topology(gRoot); // global root
      new (&dst_participants) XMI::Topology(gRankList+1, (gSize-1)); // everyone except root in dst_participants
      DBG_FPRINTF((stderr,"gRoot %d, gSize %d\n",gRoot, gSize));
      for(size_t j=0;j<gSize;++j)
      {
        DBG_FPRINTF((stderr,"gRankList[%d] = %d\n",j, gRankList[j]));
      }
      XMI_assert(!dst_participants.isRankMember(gRoot));

      _buffer1.reset(true); // isRoot = true

      mcast.connection_id = 1; // I'm going to use connection id to specify which buffer to receive into.

      XMI::PipeWorkQueue * srcPwq =_buffer1.srcPwq();
      srcPwq->configure(NULL, _buffer1.buffer(), TEST_BUF_SIZE, 0);
      srcPwq->reset();

      mcast.src = (xmi_pipeworkqueue_t *) srcPwq;
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
      while(_doneCountdown)
      {
        status = XMI_Context_advance (context, 10);
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

  //sleep(5);
// ------------------------------------------------------------------------
// 1) The last rank slowly mcasts to all
// 2) Global root pipelines/mcast's the output from 1) to all
// 3) Validate the buffers on root.
// ------------------------------------------------------------------------
  {
    _doneCountdown = 2;
    //sleep(5); // instead of syncing

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
#endif
