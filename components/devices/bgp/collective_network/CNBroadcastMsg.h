/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file components/devices/bgp/collective_network/CNBroadcastMsg.h
 * \brief Default collective network broadcast interface.
 */
#ifndef __components_devices_bgp_collective_network_CNBroadcastMsg_h__
#define __components_devices_bgp_collective_network_CNBroadcastMsg_h__

#include "util/common.h"
#include "components/devices/bgp/collective_network/CNDevice.h" // externs for env vars
#include "components/devices/bgp/collective_network/CollectiveNetworkLib.h"
#include "components/devices/bgp/collective_network/CNPacket.h"
#include "components/devices/generic/Device.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/MulticastModel.h"

/**
 * \page cn_bcast Collective Network Broadcast
 *
 * Broadcast is implemented on the collective network using
 * an allreduce operation with an OR operator and all non-root
 * nodes injecting zero. Additionally, the root node discards
 * all received packets. Note, all nodes must inject and receive
 * in order to complete the operation.
 */

namespace XMI {
namespace Device {
namespace BGP {

class CNBroadcastModel;
class CNBroadcastMessage;
typedef XMI::Device::BGP::BaseGenericCNThread CNBroadcastThread;
typedef XMI::Device::Generic::SharedQueueSubDevice<CNDevice,CNBroadcastThread,2> CNBroadcastDevice;

};	// BGP
};	// Device
};	// XMI

extern XMI::Device::BGP::CNBroadcastDevice _g_cnbroadcast_dev;

namespace XMI {
namespace Device {
namespace BGP {

/**
 * \brief Collective Network Broadcast Send
 *
 */

class CNBroadcastMessage : public XMI::Device::BGP::BaseGenericCNMessage {
	enum roles {
		NO_ROLE = 0,
		INJECTION_ROLE = (1 << 0), // first role must be "injector"
		RECEPTION_ROLE = (1 << 1), // last role must be "receptor"
	};
public:
	CNBroadcastMessage(Generic::GenericSubDevice &qs,
			xmi_multicast_t *mcast,
			size_t bytes,
			bool doStore,
			bool doData,
			unsigned dispatch_id) :
	BaseGenericCNMessage(qs, mcast->client, mcast->context,
		(XMI::PipeWorkQueue *)mcast->src, (XMI::PipeWorkQueue *)mcast->dst,
		bytes, doStore, mcast->roles, mcast->cb_done,
		dispatch_id, XMI::Device::BGP::COMBINE_OP_OR, BGPCN_PKT_SIZE),
	_doData(doData),
	_roles(mcast->roles)
	{
	}

	STD_POSTNEXT(CNBroadcastDevice,CNBroadcastThread,&_g_cnbroadcast_dev)

protected:
	//friend class CNBroadcastDevice;
	friend class XMI::Device::Generic::SharedQueueSubDevice<CNDevice,CNBroadcastThread,2>;

	ADVANCE_ROUTINE(advanceInj,CNBroadcastMessage,CNBroadcastThread);
	ADVANCE_ROUTINE(advanceRcp,CNBroadcastMessage,CNBroadcastThread);
	inline int __setThreads(CNBroadcastThread *t, int n) {
		int nt = 0;
		_g_cnbroadcast_dev.common()->__resetThreads();
		_nThreads = ((_roles & INJECTION_ROLE) != 0) + ((_roles & RECEPTION_ROLE) != 0);
		if (_roles & INJECTION_ROLE) {
			t[nt].setMsg(this);
			t[nt].setAdv(advanceInj);
			t[nt].setStatus(XMI::Device::Ready);
			t[nt]._wq = _swq;
			t[nt]._bytesLeft = _bytes;
			t[nt]._cycles = 1;
			__advanceInj(&t[nt]);
			++nt;
		}
		if (_roles & RECEPTION_ROLE) {
			t[nt].setMsg(this);
			t[nt].setAdv(advanceRcp);
			t[nt].setStatus(XMI::Device::Ready);
			t[nt]._wq = _rwq;
			t[nt]._bytesLeft = _bytes;
			t[nt]._cycles = 3000; // DCMF_PERSISTENT_ADVANCE...
			__advanceRcp(&t[nt]);
			++nt;
		}
		// assert(nt > 0? && nt < n);
		return nt;
	}

	inline void __completeThread(CNBroadcastThread *thr);

	inline xmi_result_t __advanceInj(CNBroadcastThread *thr) {
		if (thr->_bytesLeft == 0) return XMI_SUCCESS;
		unsigned hcount = BGPCN_FIFO_SIZE, dcount = BGPCN_QUADS_PER_FIFO;
		// thr->_wq is not valid unless _doData...
		if (__wait_send_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return XMI_EAGAIN;
		}
		if (_doData) {
			size_t avail = thr->_wq->bytesAvailableToConsume();
			char *buf = thr->_wq->bufferToConsume();
			bool aligned = (((unsigned)buf & 0x0f) == 0);
			size_t did = 0;
			if (avail < BGPCN_PKT_SIZE && avail < thr->_bytesLeft) {
				return XMI_EAGAIN;
			}
			// is this possible??
			if (avail > thr->_bytesLeft) avail = thr->_bytesLeft;
			__send_whole_packets(thr, hcount, dcount, avail, did, buf, aligned);
			__send_last_packet(thr, hcount, dcount, avail, did, buf, aligned);
			thr->_wq->consumeBytes(did);
		} else {
			__send_null_packets(thr, hcount, dcount);
		}
		if (thr->_bytesLeft == 0) {
			thr->setStatus(XMI::Device::Complete);
			__completeThread(thr);
			return XMI_SUCCESS;
		}
		return XMI_EAGAIN;
	}

	inline xmi_result_t __advanceRcp(CNBroadcastThread *thr) {
		if (thr->_bytesLeft == 0) return XMI_SUCCESS;
		unsigned hcount = 0, dcount = 0;
		size_t did = 0;
		if (__wait_recv_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return XMI_EAGAIN;
		}
		// thr->_wq is not valid unless _doStore...
		if (_doStore) {
			unsigned toCopy = thr->_bytesLeft >= BGPCN_PKT_SIZE ? BGPCN_PKT_SIZE : thr->_bytesLeft;
			size_t avail = thr->_wq->bytesAvailableToProduce();
			char *buf = thr->_wq->bufferToProduce();
			bool aligned = (((unsigned)buf & 0x0f) == 0);
			if (avail < toCopy) {
				return XMI_EAGAIN;
			}
			__recv_whole_packets(thr, hcount, dcount, avail, did, buf, aligned);
			__recv_last_packet(thr, hcount, dcount, avail, did, buf, aligned);
			if (did) {
				thr->_wq->produceBytes(did);
			}
		} else {
			__recv_null_packets(thr, hcount, dcount, did);
		}
		if (thr->_bytesLeft == 0) {
			thr->setStatus(XMI::Device::Complete);
			__completeThread(thr);
			return XMI_SUCCESS;
		}
		return XMI_EAGAIN;
	}

	friend class XMI::Device::Generic::GenericMessage;

	bool _doData;
	unsigned _roles;
	unsigned _nThreads;
}; // class CNBroadcastMessage

class CNBroadcastModel : public XMI::Device::Interface::MulticastModel<CNBroadcastModel> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = -1;
	static const size_t sizeof_msg = sizeof(CNBroadcastMessage);

	CNBroadcastModel(xmi_result_t &status) :
	XMI::Device::Interface::MulticastModel<CNBroadcastModel>(status)
	{
		_dispatch_id = _g_cnbroadcast_dev.newDispID();
		_me = __global.mapping.task();
	}

	inline bool postMulticast_impl(xmi_multicast_t *mcast);

private:
	size_t _me;
	unsigned _dispatch_id;
}; // class CNBroadcastModel

inline void CNBroadcastMessage::__completeThread(CNBroadcastThread *thr) {
	unsigned c = _g_cnbroadcast_dev.common()->__completeThread(thr);
	if (c >= _nThreads) {
		setStatus(XMI::Device::Done);
	}
}

inline bool CNBroadcastModel::postMulticast_impl(xmi_multicast_t *mcast) {
	XMI::Topology *src_topo = (XMI::Topology *)mcast->src_participants;
	XMI::Topology *dst_topo = (XMI::Topology *)mcast->dst_participants;
	bool doData = (!src_topo || src_topo->isRankMember(_me));
	bool doStore = (!dst_topo || dst_topo->isRankMember(_me));

	// could try to complete broadcast before construction, but for now the code
	// is too dependent on having message and thread structures to get/keep context.
	// __post() will still try early advance... (after construction)
	CNBroadcastMessage *msg;
	msg = new (mcast->request) CNBroadcastMessage(*_g_cnbroadcast_dev.common(),
			mcast, mcast->bytes, doStore, doData, _dispatch_id);
	_g_cnbroadcast_dev.__post<CNBroadcastMessage>(msg);
	return true;
}

};	// BGP
};	// Device
};	// XMI

#endif // __components_devices_bgp_cnbroadcastmsg_h__
