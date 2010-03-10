/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/multisend/bgq/NativeInterface.cc
 * \brief Simple multicast tests.
 */

#include "Buffer.h"

#include "Global.h"
#include "components/devices/bgq/mu/MUCollDevice.h"
#include "components/devices/bgq/mu/MUMulticastModel.h"
#include "components/devices/bgq/mu/MUMultisyncModel.h"
#include "components/devices/bgq/mu/MUMulticombineModel.h"
#include "common/bgq/NativeInterface.h"
#include "../../../util.h"

#include <hwi/include/bqc/nd_500_dcr.h>

#ifndef TEST_BUF_SIZE
  #define TEST_BUF_SIZE	32
#endif // TEST_BUF_SIZE


static XMI::Test::Buffer<TEST_BUF_SIZE> _buffer;

static   void       *_cookie=(void*)"HI COOKIE";
static int           _doneCountdown, _countNoData=0;
xmi_callback_t       _cb_done;
const xmi_quad_t     _msginfo = {0,1,2,3};

static int          _failed = 0;

XMI::Topology dst_subtopology;

size_t task_id;


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
  DBG_FPRINTF((stderr,"%s:%s msgcount %d, connection_id %d, root %zd, sndlen %zd, cookie %s\n",
               __FILE__,__PRETTY_FUNCTION__,msgcount, connection_id, root, sndlen, (char*) clientdata));
  XMI_assertf(_doneCountdown > 0,"doneCountdown %d\n",_doneCountdown);
  XMI_assertf(sndlen <= TEST_BUF_SIZE,"sndlen %zu\n",sndlen);
  XMI_assertf(msgcount == 1,"msgcount %d",msgcount);
  XMI_assertf(msginfo->w0 == _msginfo.w0,"msginfo->w0=%d\n",msginfo->w0);
  XMI_assertf(msginfo->w1 == _msginfo.w1,"msginfo->w1=%d\n",msginfo->w1);
  XMI_assertf(msginfo->w2 == _msginfo.w2,"msginfo->w2=%d\n",msginfo->w2);
  XMI_assertf(msginfo->w3 == _msginfo.w3,"msginfo->w3=%d\n",msginfo->w3);
  XMI_assertf(dst_subtopology.isRankMember(task_id),"!isRankMember(%zd)\n",task_id);

  if (connection_id == 1) // no data being sent
  {
    ++_countNoData;
    *rcvlen = 0;
    *rcvpwq = (xmi_pipeworkqueue_t*) NULL;
    if (sndlen == 0)
      fprintf(stderr, "<%3.3d>PASS: msgdata received with no data\n",__LINE__);
    else
    {
      fprintf(stderr, "<%3.3d>FAIL: no data expected\n",__LINE__);
      _failed = 1;
    }
  }
  else
  {
    XMI::PipeWorkQueue * pwq;
    pwq = _buffer.dstPwq();
    DBG_FPRINTF((stderr,"%s:%s bytesAvailable (%p) %zd, %zd done out of %zd\n",__FILE__,__PRETTY_FUNCTION__,
                 pwq,pwq->bytesAvailableToProduce(),pwq->getBytesProduced(),sndlen));

    *rcvlen = sndlen;
    *rcvpwq = (xmi_pipeworkqueue_t*) pwq;
  }

  *cb_done = _cb_done;

}

void _done_cb(xmi_context_t context, void *cookie, xmi_result_t err)
{
  XMI_assertf(_doneCountdown > 0,"doneCountdown %d\n",_doneCountdown);
  volatile int *doneCountdown = (volatile int*) cookie;
  DBG_FPRINTF((stderr, "%s:%s doneCountdown %d/%d \n",__FILE__,__PRETTY_FUNCTION__, *doneCountdown,_doneCountdown));
  --*doneCountdown;
}

#warning Major hack: no call to client init or context create or official advance
int main(int argc, char ** argv)
{
  xmi_client_t client = NULL;
  xmi_context_t context = NULL;
  xmi_result_t status = XMI_ERROR;
  task_id = __global.mapping.task();
  DBG_FPRINTF((stderr, "%s:%s: task %zd starting\n",__FILE__,__PRETTY_FUNCTION__, task_id));
  size_t                     dispatch = 2;

  size_t  bytesConsumed = 0,  bytesProduced = 0;

  xmi_dispatch_callback_fn   fn;

  fn.multicast = &dispatch_multicast_fn;

  XMI::Device::MU::MUCollDevice mu (0,1,0);
  mu.init (0, 0, client, context, NULL /*sysdep*/, NULL /*progress*/);

  XMI::BGQNativeInterface<XMI::Device::MU::MUCollDevice,
  XMI::Device::MU::MUMulticastModel,
  XMI::Device::MU::MUMultisyncModel,
  XMI::Device::MU::MUMulticombineModel>  nativeInterface(mu, 0, context, 0);

  uint8_t mcast_state[XMI::Device::MU::MUMulticastModel::sizeof_msg];

  status = nativeInterface.setDispatch(fn, _cookie);


  _cb_done.function   = &_done_cb;
  _cb_done.clientdata = &_doneCountdown;

  //For testing ease, I'm assuming rank list topology, so convert them
  XMI::Topology topology_global_list = __global.topology_global;
  topology_global_list.convertTopology(XMI_LIST_TOPOLOGY);

  XMI::Topology topology_global_coord = __global.topology_global;
  topology_global_coord.convertTopology(XMI_COORD_TOPOLOGY);

  topology_global_coord.subTopologyNthGlobal(&dst_subtopology, 0); //0th rank on each node

  if (task_id == 0)
    DBG_FPRINTF((stderr, "%s: topology_global_list type %d and size %zd\n",__PRETTY_FUNCTION__, topology_global_list.type(), topology_global_list.size()));
    for (size_t i=0;i<topology_global_list.size();++i)
    {
      DBG_FPRINTF((stderr, "%s: topology_global_list index %zd is task %d\n",__PRETTY_FUNCTION__, i, topology_global_list.index2Rank(i)));
    }
    DBG_FPRINTF((stderr, "%s: topology_global_coord type %d and size %zd\n",__PRETTY_FUNCTION__, topology_global_coord.type(), topology_global_coord.size()));
    for (size_t i=0;i<topology_global_coord.size();++i)
    {
      DBG_FPRINTF((stderr, "%s: topology_global_coord index %zd is task %d\n",__PRETTY_FUNCTION__, i, topology_global_coord.index2Rank(i)));
    }
    DBG_FPRINTF((stderr, "%s: dst subtopology type %d and size %zd\n",__PRETTY_FUNCTION__, dst_subtopology.type(), dst_subtopology.size()));
    for (size_t i=0;i<dst_subtopology.size();++i)
    {
      DBG_FPRINTF((stderr, "%s: dst subtopology index %zd is task %d\n",__PRETTY_FUNCTION__, i, dst_subtopology.index2Rank(i)));
    }
  if (dst_subtopology.isRankMember(task_id))
    DBG_FPRINTF((stderr, "%s: task %zd is dst member\n",__PRETTY_FUNCTION__, task_id));
  else
    DBG_FPRINTF((stderr, "%s: task %zd is not dst member\n",__PRETTY_FUNCTION__, task_id));


  XMI::Topology topology_local  = __global.topology_local;
  topology_local.convertTopology(XMI_LIST_TOPOLOGY);

  // global topology variables
  xmi_task_t  gRoot    = topology_global_list.index2Rank(0);
  xmi_task_t *gRankList=NULL; topology_global_list.rankList(&gRankList);

  XMI::Topology src_participants;

  new (&src_participants) XMI::Topology(gRoot); // global root

  xmi_multicast_t mcast;
  memset(&mcast, 0x00, sizeof(mcast));
  if (gRoot == task_id)
  {

    mcast.dispatch = dispatch;
    mcast.connection_id = 0xB; // arbitrary
    mcast.msginfo = &_msginfo;
    mcast.msgcount = 1;
    mcast.src_participants = (xmi_topology_t *)&src_participants;
    mcast.dst_participants = (xmi_topology_t *)&dst_subtopology; // 0th rank on each node

    mcast.src = (xmi_pipeworkqueue_t *)_buffer.srcPwq();
    mcast.dst = (xmi_pipeworkqueue_t *) _buffer.dstPwq();

    mcast.client = 0;  // client ID
    mcast.context = 0;  // context ID
    mcast.roles = -1;
    mcast.bytes = TEST_BUF_SIZE;

    mcast.cb_done = _cb_done;
  }
// ------------------------------------------------------------------------
// simple mcast root to all
// ------------------------------------------------------------------------

  if (dst_subtopology.isRankMember(task_id))
    _doneCountdown = 1;  // advance until we receive the mcast
  else
    _doneCountdown = 10; // advances an arbitray N times - not participating

  //sleep(5); // instead of syncing

  if (gRoot == task_id)
  {
    _buffer.reset(true); // isRoot = true

    DBG_FPRINTF((stderr, "%s:%s root mcast\n",__FILE__,__PRETTY_FUNCTION__));
    status = nativeInterface.multicast(&mcast); // this version of ni allocates/frees our request storage for us.
  }

  if (dst_subtopology.isRankMember(task_id)) ;
  else
  {
#ifdef ENABLE_MAMBO_WORKAROUNDS
    mamboSleep(10);
#endif // ENABLE_MAMBO_WORKAROUNDS
  }

  DBG_FPRINTF((stderr,"%s:before advance\n",__PRETTY_FUNCTION__));
  while (_doneCountdown)
  {
    mu.advance();
    if (dst_subtopology.isRankMember(task_id)) ;
    else --_doneCountdown; 
  }

  bytesConsumed = 0,
  bytesProduced = 0;

  if (gRoot == task_id)
  {
    _buffer.validate(bytesConsumed,
                     bytesProduced,
                     true,   // isRoot = true
                     true);  // isDest = true
    if ((bytesConsumed != TEST_BUF_SIZE) ||
        (bytesProduced != TEST_BUF_SIZE))
    {
      fprintf(stderr, "<%3.3d>FAIL bytesConsumed = %zu, bytesProduced = %zu\n",__LINE__, bytesConsumed, bytesProduced);
      _failed = 1;
    }
    else
      fprintf(stderr, "<%3.3d>PASS bytesConsumed = %zu, bytesProduced = %zu\n",__LINE__, bytesConsumed, bytesProduced);
  }
  else if (dst_subtopology.isRankMember(task_id))
  {
    _buffer.validate(bytesConsumed,
                     bytesProduced);
    if ((bytesConsumed != 0) ||
        (bytesProduced != TEST_BUF_SIZE))
    {
      fprintf(stderr, "<%3.3d>FAIL bytesConsumed = %zu, bytesProduced = %zu\n",__LINE__, bytesConsumed, bytesProduced);
      _failed = 1;
    }
    else
      fprintf(stderr, "<%3.3d>PASS bytesConsumed = %zu, bytesProduced = %zu\n",__LINE__, bytesConsumed, bytesProduced);
  }

#ifdef ENABLE_MAMBO_WORKAROUNDS
  mamboSleep(5);
#endif // ENABLE_MAMBO_WORKAROUNDS
  DBG_FPRINTF((stderr, "%s:%s: task %zd exiting\n",__FILE__,__PRETTY_FUNCTION__, task_id));
  return 0;
// ------------------------------------------------------------------------
// simple mcast root to all
// ------------------------------------------------------------------------

  _doneCountdown = 1;
  //sleep(5); // instead of syncing

  if (gRoot == task_id)
  {
    _buffer.reset(true); // isRoot = true

    status = nativeInterface.multicast(mcast_state, &mcast); // this version uses our request storage directly.
  }

  while (_doneCountdown)
  {
    mu.advance();
  }


  bytesConsumed = 0,
  bytesProduced = 0;

  if (gRoot == task_id)
  {
    _buffer.validate(bytesConsumed,
                     bytesProduced,
                     true,   // isRoot = true
                     true);  // isDest = true
    if ((bytesConsumed != TEST_BUF_SIZE) ||
        (bytesProduced != TEST_BUF_SIZE))
    {
      fprintf(stderr, "<%3.3d>FAIL bytesConsumed = %zu, bytesProduced = %zu\n",__LINE__, bytesConsumed, bytesProduced);
      _failed = 1;
    }
    else
      fprintf(stderr, "<%3.3d>PASS bytesConsumed = %zu, bytesProduced = %zu\n",__LINE__, bytesConsumed, bytesProduced);
  }
  else
  {
    _buffer.validate(bytesConsumed,
                     bytesProduced);
    if ((bytesConsumed != 0) ||
        (bytesProduced != TEST_BUF_SIZE))
    {
      fprintf(stderr, "<%3.3d>FAIL bytesConsumed = %zu, bytesProduced = %zu\n",__LINE__, bytesConsumed, bytesProduced);
      _failed = 1;
    }
    else
      fprintf(stderr, "<%3.3d>PASS bytesConsumed = %zu, bytesProduced = %zu\n",__LINE__, bytesConsumed, bytesProduced);
  }

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// simple mcast metadata only
// ------------------------------------------------------------------------


  _doneCountdown = 1;
  _countNoData=0;
  if (gRoot == task_id)
  {
    mcast.connection_id = 1; // arbitrary - dispatch knows this means no data

    mcast.src = (xmi_pipeworkqueue_t *)NULL;
    mcast.dst = (xmi_pipeworkqueue_t *)NULL;

    mcast.bytes = 0;

    status = nativeInterface.multicast(&mcast); // this version of ni allocates/frees our request storage for us.
  }

  while (_doneCountdown)
  {
    mu.advance();
  }
  if (_countNoData != 1)
  {
    fprintf(stderr,"<%3.3d>FAIL didn't receive expected metadata - received %d\n",__LINE__, _countNoData);
    _failed = 1;
  }
  else fprintf(stderr,"<%3.3d>PASS received %d expected metadata\n",__LINE__,_countNoData);

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// simple mcast metadata only
// ------------------------------------------------------------------------


  _doneCountdown = 1;
  _countNoData=0;
  if (gRoot == task_id)
  {
    mcast.connection_id = 1; // arbitrary - dispatch knows this means no data

    mcast.src = (xmi_pipeworkqueue_t *)NULL;
    mcast.dst = (xmi_pipeworkqueue_t *)NULL;

    mcast.bytes = 0;

    status = nativeInterface.multicast(mcast_state, &mcast); // this version uses our request storage directly.
  }

  while (_doneCountdown)
  {
    mu.advance();
  }
  if (_countNoData != 1)
  {
    fprintf(stderr,"<%3.3d>FAIL didn't receive expected metadata - received %d\n",__LINE__, _countNoData);
    _failed = 1;
  }
  else fprintf(stderr,"<%3.3d>PASS received %d expected metadata\n",__LINE__,_countNoData);


// ------------------------------------------------------------------------
// Test multisync
// ------------------------------------------------------------------------
  xmi_multisync_t multisync;
  memset(&multisync, 0x00, sizeof(multisync));

  uint8_t msync_state[XMI::Device::MU::MUMultisyncModel::sizeof_msg];

  multisync.connection_id = 0xB; // arbitrary
  multisync.participants = (xmi_topology_t *)&topology_global_list;

  multisync.client = 0;	// client ID
  multisync.context = 0;	// context ID
  multisync.roles = -1;

  multisync.cb_done = _cb_done;

// ------------------------------------------------------------------------
// simple multisync root to all
// ------------------------------------------------------------------------

  _doneCountdown = 1;

  status = nativeInterface.multisync(msync_state, &multisync); // this version uses our request storage directly.

  while (_doneCountdown)
  {
    mu.advance();
  }

  /// \todo Validate the msync test passed?

// ------------------------------------------------------------------------
// Test multicombine
// ------------------------------------------------------------------------

  xmi_multicombine_t multicombine;
  memset(&multicombine, 0x00, sizeof(multicombine));

  multicombine.connection_id = 0xB0BC; // arbitrary
  multicombine.data_participants = (xmi_topology_t *)&topology_global_list;
  multicombine.results_participants = (xmi_topology_t *)&topology_global_list;
  multicombine.count = TEST_BUF_SIZE/sizeof(unsigned);
  multicombine.data = (xmi_pipeworkqueue_t*) _buffer.srcPwq();
  multicombine.dtype = XMI_UNSIGNED_INT;
  multicombine.optor = XMI_MIN;
  multicombine.results = (xmi_pipeworkqueue_t*) _buffer.dstPwq();
  multicombine.client = 0;	// client ID
  multicombine.context = 0;	// context ID
  multicombine.roles = -1;

  multicombine.cb_done = _cb_done;

// ------------------------------------------------------------------------
// simple multicombine
// ------------------------------------------------------------------------

  uint8_t mcomb_state[XMI::Device::MU::MUMulticombineModel::sizeof_msg];

  _buffer.reset(true); // isRoot = true


  _doneCountdown = 1;
  //sleep(5); // instead of combineing

  status = nativeInterface.multicombine(mcomb_state, &multicombine); // this version uses our request storage directly.

  while (_doneCountdown)
  {
    mu.advance();
  }

  bytesConsumed = 0,
  bytesProduced = 0;

  _buffer.validate(bytesConsumed,
                   bytesProduced,
                   true,   // isRoot = true
                   true);  // isDest = true
  if ((bytesConsumed != TEST_BUF_SIZE) ||
      (bytesProduced != TEST_BUF_SIZE))
  {
    fprintf(stderr, "<%3.3d>FAIL bytesConsumed = %zu, bytesProduced = %zu\n",__LINE__, bytesConsumed, bytesProduced);
    _failed = 1;
  }
  else
    fprintf(stderr, "<%3.3d>PASS bytesConsumed = %zu, bytesProduced = %zu\n",__LINE__, bytesConsumed, bytesProduced);

// ------------------------------------------------------------------------
// simple multicombine root to all with one contributing '0' to the MIN allreduce
// ------------------------------------------------------------------------
  if (task_id == topology_global_list.index2Rank(0))
  {
    _buffer.resetMIN0(true); // isRoot = true so set to 0's
  }
  else
  {
    _buffer.resetMIN0();
  }


  _doneCountdown = 1;

  status = nativeInterface.multicombine(mcomb_state, &multicombine); // this version uses our request storage directly.

  while (_doneCountdown)
  {
    mu.advance();
  }

  bytesConsumed = 0,
  bytesProduced = 0;

  _buffer.validateMIN0(bytesConsumed,
                       bytesProduced,
                       true,   // isRoot = true
                       true);  // isDest = true
  if ((bytesConsumed != TEST_BUF_SIZE) ||
      (bytesProduced != TEST_BUF_SIZE))
  {
    fprintf(stderr, "<%3.3d>FAIL bytesConsumed = %zu, bytesProduced = %zu\n",__LINE__, bytesConsumed, bytesProduced);
    _failed = 1;
  }
  else
    fprintf(stderr, "<%3.3d>PASS bytesConsumed = %zu, bytesProduced = %zu\n",__LINE__, bytesConsumed, bytesProduced);

  if (_failed)
    fprintf(stderr,"FAIL\n");
  else fprintf(stderr,"PASS\n");
  return _failed;
  ;
}
