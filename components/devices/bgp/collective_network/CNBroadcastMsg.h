/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file components/devices/bgp/collective_network/CNBroadcastMsg.h
 * \brief Default collective network broadcast interface.
 */
#ifndef __components_devices_bgp_cnbroadcastmsg_h__
#define __components_devices_bgp_cnbroadcastmsg_h__

#include "Util.h"
#include "components/devices/common/MulticastModelImpl.h"
#include "components/devices/bgp/collective_network/CollectiveNetworkLib.h"
#include "components/devices/bgp/collective_network/Packet.h"
#include "PipeWorkQueue.h"

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
typedef XMI::Device::Generic::SharedQueueSubDevice<CNBroadcastModel,CNDevice,CNBroadcastMessage,CNBroadcastThread,2> CNBroadcastDevice;

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
	CNBroadcastMessage(BaseDevice &qs,
			XMI::PipeWorkQueue *swq,
			XMI::PipeWorkQueue *rwq,
			size_t bytes,
			bool doStore,
			bool doData,
			unsigned roles,
			const XMI_Callback_t cb,
			unsigned dispatch_id) :
	BaseGenericCNMessage(qs, swq, rwq, bytes, doStore, roles, cb,
		dispatch_id, XMI::Device::BGP::COMBINE_OP_OR, BGPCN_PKT_SIZE),
	_doData(doData),
	_roles(roles)
	{
	}

	inline XMI::Device::MessageStatus advanceThread(XMI::Device::Generic::GenericAdvanceThread *t);
	inline void complete();
protected:
	//friend class CNBroadcastDevice;
	friend class XMI::Device::Generic::SharedQueueSubDevice<CNBroadcastModel,CNDevice,CNBroadcastMessage,CNBroadcastThread,2>;

	inline int __setThreads(CNBroadcastThread *t, int n) {
		int nt = 0;
		if (_roles & INJECTION_ROLE) {
			t[nt].setMsg(this);
			t[nt].setDone(false);
			t[nt]._sender = true;
			t[nt]._wq = _swq;
			t[nt]._bytesLeft = _bytes;
			t[nt]._cycles = 1;
			++nt;
		}
		if (_roles & RECEPTION_ROLE) {
			t[nt].setMsg(this);
			t[nt].setDone(false);
			t[nt]._sender = false;
			t[nt]._wq = _rwq;
			t[nt]._bytesLeft = _bytes;
			t[nt]._cycles = 3000; // DCMF_PERSISTENT_ADVANCE...
			++nt;
		}
		// assert(nt > 0? && nt < n);
		_nThreads = nt;
		if (_bytes > 4096) {
			setThreadsWanted(nt);
		}
		return nt;
	}

	inline void __completeThread(CNBroadcastThread *thr);

	inline XMI::Device::MessageStatus __advanceInj(CNBroadcastThread *thr) {
		if (thr->_bytesLeft == 0) return XMI::Device::Done;
		unsigned hcount = BGPCN_FIFO_SIZE, dcount = BGPCN_QUADS_PER_FIFO;
		// thr->_wq is not valid unless _doData...
		if (__wait_send_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return XMI::Device::Active;
		}
		if (_doData) {
			size_t avail = thr->_wq->bytesAvailableToConsume();
			char *buf = thr->_wq->bufferToConsume();
			bool aligned = (((unsigned)buf & 0x0f) == 0);
			size_t did = 0;
			if (avail < BGPCN_PKT_SIZE && avail < thr->_bytesLeft) {
				return XMI::Device::Active;
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
			thr->setDone(true);
			return XMI::Device::Done;
		}
		return XMI::Device::Active;
	}

	inline XMI::Device::MessageStatus __advanceRcp(CNBroadcastThread *thr) {
		if (thr->_bytesLeft == 0) return XMI::Device::Done;
		unsigned hcount = 0, dcount = 0;
		size_t did = 0;
		if (__wait_recv_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return XMI::Device::Active;
		}
		// thr->_wq is not valid unless _doStore...
		if (_doStore) {
			unsigned toCopy = thr->_bytesLeft >= BGPCN_PKT_SIZE ? BGPCN_PKT_SIZE : thr->_bytesLeft;
			size_t avail = thr->_wq->bytesAvailableToProduce();
			char *buf = thr->_wq->bufferToProduce();
			bool aligned = (((unsigned)buf & 0x0f) == 0);
			if (avail < toCopy) {
				return XMI::Device::Active;
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
			thr->setDone(true);
			return XMI::Device::Done;
		}
		return XMI::Device::Active;
	}

	inline XMI::Device::MessageStatus __advanceThread(CNBroadcastThread *thr) {
		XMI::Device::MessageStatus ms;
		if (thr->_sender) {
			ms = __advanceInj(thr);
		} else {
			ms = __advanceRcp(thr);
		}
		if (ms == XMI::Device::Done) {
			// thread is Done, maybe not message
			__completeThread(thr);
			return ms;
		}
		return ms;
	}

	bool _doData;
	unsigned _roles;
	unsigned _nThreads;
}; // class CNBroadcastMessage

class CNBroadcastModel : public XMI::Device::Interface::MulticastModel<CNBroadcastModel> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = -1;

	CNBroadcastModel(XMI_Result &status) :
	XMI::Device::Interface::MulticastModel<CNBroadcastModel>(status)
	{
		_dispatch_id = _g_cnbroadcast_dev.newDispID();
		_me = _g_cnbroadcast_dev.getSysdep()->mapping().rank();
	}

	inline bool postMulticast_impl(xmi_multicast_t *mcast);

private:
	size_t _me;
	unsigned _dispatch_id;
	static inline void compile_time_assert () {
		COMPILE_TIME_ASSERT(sizeof(XMI_Request_t) >= sizeof(CNBroadcastMessage));
	}
}; // class CNBroadcastModel

inline void CNBroadcastMessage::__completeThread(CNBroadcastThread *thr) {
	unsigned c = ((CNBroadcastDevice &)_QS).__completeThread(thr);
	if (c >= _nThreads) {
		setStatus(XMI::Device::Done);
	}
}

void CNBroadcastMessage::complete() {
	((CNBroadcastDevice &)_QS).__complete(this);
	executeCallback();
}

XMI::Device::MessageStatus CNBroadcastMessage::advanceThread(XMI::Device::Generic::GenericAdvanceThread *t) {
	return __advanceThread((CNBroadcastThread *)t);
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
	msg = new (mcast->request) CNBroadcastMessage(_g_cnbroadcast_dev,
			(XMI::PipeWorkQueue *)mcast->src;
			(XMI::PipeWorkQueue *)mcast->dst, mcast->bytes,
			doStore, doData, mcast->roles, mcast->cb_done, _dispatch_id);
	_g_cnbroadcast_dev.__post(msg);
	return true;
}

};	// BGP
};	// Device
};	// XMI

#endif // __components_devices_bgp_cnbroadcastmsg_h__
