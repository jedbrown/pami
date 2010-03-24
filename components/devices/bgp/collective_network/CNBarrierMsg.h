/* OCO Source Materials                                             */
/*                                                                  */
/* Product(s):                                                      */
/*     5733-BG1                                                     */
/*                                                                  */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/*                                                                  */
/* The Source code for this program is not published  or otherwise  */
/* divested of its trade secrets,  irrespective of what has been    */
/* deposited with the U.S. Copyright Office.                        */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgp/collective_network/CNBarrierMsg.h
 * \brief Currently, Collective Network Barrier is not used.
 */
#ifndef __components_devices_bgp_collective_network_CNBarrierMsg_h__
#define __components_devices_bgp_collective_network_CNBarrierMsg_h__

#if NOT_YET // or ever?
#include "bgp/tree/Device.h"
#include "bgp/tree/Message.h"

namespace DCMF
{
  namespace Protocol
    {
      namespace MultiSend
        {
          class TreeBarrierMessage: public DCMF::Queueing::Tree::TreeSendMessage
            {
            public:
              ///
              /// \brief Non Blocking Tree Barrier Message
              ///
              /// \param[in]  Tree Device to send the data
              /// \param[in]  Classroute to send the data on
              /// \param[in]  DispatchID to put in the header
              ///
              TreeBarrierMessage(DCMF::Queueing::Tree::Device    &TreeQS,
                                 unsigned                         classroute,
                                 unsigned                         dispatch_id,
                                 const PAMI_Callback_t            cb_done):
                TreeSendMessage(TreeQS, cb_done, 1),
                _classroute(classroute),
                _modelPkt(classroute,
                          DCMF::Tree::COMBINE_OP_OR,
                          7,
                          dispatch_id)
                {
                }
		int advance(unsigned cycles, DCMF::Queueing::Tree::TreeMsgContext ctx);
            private:
              unsigned           _classroute;
              DCMF::Tree::Packet _modelPkt;
            };

          ///
          /// \brief Non Blocking Tree Barrier Receive Message
          ///
          /// \param[in]  Tree Device to send the data
          /// \param[in]  Callback to dispatch
          ///
          class TreeBarrierRecvMessage: public DCMF::Queueing::Tree::TreeRecvMessage
            {
            public:
              TreeBarrierRecvMessage(DCMF::Queueing::Tree::Device    &TreeQS,
                                     const PAMI_Callback_t            cb):
                TreeRecvMessage(TreeQS, cb, NULL, 1, 1, DCMF_BCAST_RECV_NOSTORE)
                {
                }
	      int advance(unsigned cycles, DCMF::Queueing::Tree::TreeMsgContext ctx);
            private:
            };
        }
    }
}

#endif // __dcmf_cdi_bgp_cnbarriermsg_h__

/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/devices/bgp/collective_network/CNBarrierMsg.h
/// \brief Tree barrier implementation.
///
#define __INLINE__ inline
#include <Util.h>
#include "multisend/bgp/tree/TreeBarrier.h"
#include "spi/bgp_SPI.h"

namespace DCMF
{
  namespace Protocol
  {
    namespace MultiSend
    {
      //  Barrier Code
      int TreeBarrierMessage::advance(unsigned cycles, DCMF::Queueing::Tree::TreeMsgContext ctx)
      {
	register unsigned rechcount, recdcount, injhcount, injdcount;
	CollectiveFifoStatus(VIRTUAL_CHANNEL,
		&rechcount, &recdcount,
		&injhcount, &injdcount);
	if (injhcount >= TREE_FIFO_SIZE || injdcount >= TREE_QUADS_PER_FIFO) {
		return DCMF::Queueing::Tree::Working;
	}
        CollectiveRawSendPacket0(VIRTUAL_CHANNEL,&_modelPkt._hh);
        return DCMF::Queueing::Tree::Done;
      }
      int TreeBarrierRecvMessage::advance(unsigned cycles, DCMF::Queueing::Tree::TreeMsgContext ctx)
      {
	register unsigned rechcount, recdcount, injhcount, injdcount;
	CollectiveFifoStatus(VIRTUAL_CHANNEL,
		&rechcount, &recdcount,
		&injhcount, &injdcount);
	if (rechcount == 0 || recdcount == 0) {
		return DCMF::Queueing::Tree::Working;
	}
        CollectiveRawReceivePacketNoHdrNoStore(VIRTUAL_CHANNEL);
        return DCMF::Queueing::Tree::Done;
      }
    };
  };
};
#endif // NOT_YET
#endif // __components_devices_bgp_cnbarriermsg_h__
