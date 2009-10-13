/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
////////////////////////////////////////////////////////////////////////////////
///
/// \file components/devices/bgq/mu/MUChannel.h
///
/// \brief Channel definitions.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __components_devices_bgq_mu_channel_h__
#define __components_devices_bgq_mu_channel_h__

#include "components/devices/bgq/mu/MUBaseMessage.h"
#include "components/devices/bgq/mu/ResourceManager.h"
#include "components/devices/bgq/mu/Dispatch.h"

#include "SysDep.h"

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x

namespace XMI
{
  namespace Device
  {
    namespace MU
    {
      class Channel
      {
          const static int MAX_INJ_FIFOS = 10;

        public:

          ///
          /// \brief Channel is the unit of parallelism in the MU device
          ///

          Channel();

          ~Channel() {}

          virtual int init( SysDep * sd, dispatch_t *dispatch ) = 0;

          void * operator new ( size_t nbytes, void * addr) { return addr; }

          ///
          /// \brief Advance the channel
          ///
          int advanceSend();
          int advanceRecv();


          void postToFifo( MUBaseMessage & msg,
                           unsigned        fifoIndex,
                           bool            hiprio = false );



          /// ????
          void getSubGroupAndRelativeFifoNum ( uint32_t          fifoNum,
                                               InjFifoSubGroup **injFifoSubGroup,
                                               uint32_t         *relativeFifoNum )
          {
            _resMgr.getSubGroupAndRelativeFifoNum ( fifoNum,
                                                    injFifoSubGroup,
                                                    relativeFifoNum );
          }

          /// ?????
          uint32_t getRecFifoIdForDescriptor ( int t )
          {
            return _resMgr.getRecFifoIdForDescriptor ( t );
          }

          uint32_t pinFifo (size_t target_rank)
          {
            return _resMgr.pinFifo (target_rank);
          }

          bool isEmptyMsgQ (uint32_t fnum)
          {
            return _msgQ[fnum].isEmpty();
          }

          uint16_t getRgetInjFifoId (size_t target_rank)
          {
            //fprintf (stderr, "Channel::getRgetInjFifoId(%d) .. BGQ_MU_NUM_FIFO_SUBGROUPS_PER_NODE = %d, BGQ_MU_NUM_INJ_FIFOS_PER_GROUP = %d\n", target_rank, BGQ_MU_NUM_FIFO_SUBGROUPS_PER_NODE, BGQ_MU_NUM_INJ_FIFOS_PER_GROUP);
            return (BGQ_MU_NUM_FIFO_SUBGROUPS_PER_NODE - 1) * 8; // change this later!
          }

        protected:

          ResourceManager           _resMgr;   /**<  HW resources : MU fifos and bats */

          ///
          /// \brief Ordered per-injfifo send queues. Messages are queued
          /// here when the injection fifo fills up. The advance routine
          /// then makes progress on these messages.
          ///
          Queue      _msgQ [MAX_INJ_FIFOS];

          bool                       _pollQueues;  /**< flag to record
						  that messages need
						  to be advanced.*/

          ///
          /// \brief Process queued messages
          ///
          uint32_t  pollQueuedMessages ();


      }; // class Channel


      class P2PChannel : public Channel
      {
        public:
          P2PChannel() : Channel() { }

          int init( SysDep * sd, dispatch_t *dispatch )
          {
            return _resMgr.init( ResourceManager::P2P_TYPE, sd, dispatch );
          }

          inline void post( MUBaseMessage & msg,
                            int             dstrank,
                            bool            hiprio = false )
          {
            unsigned fnum = _resMgr.pinFifo (dstrank);

            postToFifo( msg, fnum, hiprio );
          }

      };


      class ColChannel : public Channel
      {
        public:
          ColChannel() : Channel() { }

          int init( SysDep * sd, dispatch_t * dispatch )
          {
            return _resMgr.init( ResourceManager::COLL_TYPE, sd, dispatch );
          }

          void post( MUBaseMessage & msg )
          {
            unsigned fnum = _resMgr.pinFifo ();

            postToFifo( msg, fnum );
          }

      };

    };   // XMI::Device::MU namespace
  };     // XMI::Device namespace
};       // XMI namespace



///
/// \brief post a message to the MU Fifos. The message is short
/// circuited if the software injection queue is empty. Hence a
/// send/post will immediately be posted on the MU.
///  \param msg : the message to post
///  \param fnum : relative fifo number
///  \param lifo : Post message at the front of queue or back of
///  queue. Its posted at the back by default. This is required
///  in the RZV protocol to avoid deadlocks. The Remote Get ACK
///  can be stuck behind a Remote Get message to the same
///  destination, allowing the two processors to deadlock each
///  other.
///

inline void XMI::Device::MU::Channel::postToFifo( MUBaseMessage   & msg,
                                                  unsigned          fnum,
                                                  bool              hiprio)
{
  //TRACE_ERR ((stderr, "%d: Posting message to fifo %d\n", _mapping->rank(),
  //          fnum));

  //Short circuit and go straight to waitForDoneQ
  if (_msgQ[fnum].isEmpty() && msg.advance(&fnum, 1))
    { }
  else
    {
      if (hiprio)
        _msgQ[fnum].pushHead (&msg);
      else
        _msgQ[fnum].pushTail (&msg);

      _pollQueues = true;
    }
}



inline int XMI::Device::MU::Channel::advanceSend()
{
  int events = 0;

  if (_pollQueues)
    {
      events += pollQueuedMessages ();
    }

  events += _resMgr.advanceInjectionFifos();

  return events;
}


inline int XMI::Device::MU::Channel::advanceRecv()
{
  int events = 0;

  TRACE((stderr, ">> MU::Channel::advanceRecv()\n"));
  events += _resMgr.advanceReceptionFifos();
  TRACE((stderr, "<< MU::Channel::advanceRecv() .. events = %d\n", events));

  //Counter advance for rzv messages
  return events;
}
#undef TRACE
#endif // __components_devices_bgq_mu_channel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
