/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file components/devices/bgp/collective_network/CNAllreducePPMsg.h
 * \brief Default collective network allreduce interface.
 */
#ifndef __component_devices_bgp_cnallreduceppmsg_h__
#define __component_devices_bgp_cnallreduceppmsg_h__

#include "util/common.h"
#include "components/devices/bgp/collective_network/CNDevice.h" // externs for env vars
#include "components/devices/bgp/collective_network/CNAllreduce.h"
#include "components/devices/bgp/collective_network/CollectiveNetworkLib.h"
#include "components/devices/bgp/collective_network/CNPacket.h"
#include "components/devices/generic/Device.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "math/bgp/collective_network/xmi_optibgmath.h"

namespace XMI {
namespace Device {
namespace BGP {

/**
 * \brief Collective Network Allreduce Send Message base class
 *
 * Collective Network Allreduce with source/dest as pipe WQ.
 */

class CNAllreducePPModel;
class CNAllreducePPMessage;
typedef XMI::Device::BGP::BaseGenericCNThread CNAllreducePPThread;
typedef XMI::Device::Generic::SharedQueueSubDevice<CNDevice,CNAllreducePPThread,2> CNAllreducePPDevice;

};	// BGP
};	// Device
};	// XMI

extern XMI::Device::BGP::CNAllreducePPDevice _g_cnallreducepp_dev;

namespace XMI {
namespace Device {
namespace BGP {

class CNAllreducePPMessage : public XMI::Device::BGP::BaseGenericCNPPMessage {
	enum roles {
		NO_ROLE = 0,
		INJECTION_ROLE = (1 << 0), // first role must be "injector"
		RECEPTION_ROLE = (1 << 1), // last role must be "receptor"
	};
public:
	CNAllreducePPMessage(Generic::BaseGenericDevice &qs,
			XMI_PIPEWORKQUEUE_CLASS *swq,
			XMI_PIPEWORKQUEUE_CLASS *rwq,
			size_t bytes,
			bool doStore,
			unsigned roles,
			const xmi_callback_t cb,
			unsigned dispatch_id,
			XMI::Device::BGP::CNAllreduceSetup tas) :
	BaseGenericCNPPMessage(qs, swq, rwq, bytes, doStore, roles, cb,
				dispatch_id, tas),
	_roles(roles)
	{
	}

	inline XMI::Device::MessageStatus advanceThread(XMI::Device::Generic::GenericAdvanceThread *t);
	inline void complete();
protected:
	//friend class CNAllreducePPDevice;
	friend class XMI::Device::Generic::SharedQueueSubDevice<CNDevice,CNAllreducePPThread,2>;

	// _bytesLeft == bytes on network!
	inline int __setThreads(CNAllreduceThread *t, int n) {
		int nt = 0;
		int maxnt = ((CNAllreducePPDevice &)_QS).common()->getMaxThreads();
		if (_roles & INJECTION_ROLE) {
			t[nt].setMsg(this);
			t[nt].setDone(false);
			t[nt]._sender = true;
			t[nt]._wq = _swq;
			t[nt]._bytesLeft = _bytes << _allreduceSetup._logbytemult;
			t[nt]._cycles = 1;
			++nt;
		}
		if (_roles & RECEPTION_ROLE) {
			t[nt].setMsg(this);
			t[nt].setDone(false);
			t[nt]._sender = false;
			t[nt]._wq = _rwq;
			t[nt]._bytesLeft = _bytes << _allreduceSetup._logbytemult;
			t[nt]._cycles = 3000; // DCMF_PERSISTENT_ADVANCE;
			++nt;
		}
		// assert(nt > 0? && nt < n);
		_nThreads = nt;
		if (_bytes >= 16384 /* DCMF_TREE_HELPER_THRESH */) {
			setThreadsWanted(MIN(nt, maxnt));
		}
		return nt;
	}

	inline void __completeThread(CNAllreduceThread *thr);

	inline XMI::Device::MessageStatus __advanceInj(CNAllreducePPThread *thr) {
		if (thr->_bytesLeft == 0) return XMI::Device::Done;
		unsigned hcount = BGPCN_FIFO_SIZE, dcount = BGPCN_QUADS_PER_FIFO;
		size_t avail = thr->_wq->bytesAvailableToConsume();
		char *buf = thr->_wq->bufferToConsume();
		bool aligned = (((unsigned)buf & 0x0f) == 0);
		size_t did = 0;
		if (__wait_send_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return XMI::Device::Active;
		}
		__send_whole_packets(thr, hcount, dcount, avail, did, buf, aligned);
		__send_last_packet(thr, hcount, dcount, avail, did, buf, aligned);
		if (did) {
			thr->_wq->consumeBytes(did);
			if (thr->_bytesLeft == 0) {
				thr->setDone(true);
				return XMI::Device::Done;
			}
		}
		return XMI::Device::Active;
	}


	inline XMI::Device::MessageStatus __advanceRcp(CNAllreducePPThread *thr) {
		if (thr->_bytesLeft == 0) return XMI::Device::Done;
		unsigned hcount = 0, dcount = 0;
		size_t avail = thr->_wq->bytesAvailableToProduce();
		char *buf = thr->_wq->bufferToProduce();
		bool aligned = (((unsigned)buf & 0x0f) == 0);
		size_t did = 0;
		if (__wait_recv_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return XMI::Device::Active;
		}
		__recv_whole_packets(thr, hcount, dcount, avail, did, buf, aligned);
		__recv_last_packet(thr, hcount, dcount, avail, did, buf, aligned);
		if (did) {
			thr->_wq->produceBytes(did);
			if (thr->_bytesLeft == 0) {
				thr->setDone(true);
				return XMI::Device::Done;
			}
		}
		return XMI::Device::Active;
	}

	inline XMI::Device::MessageStatus __advanceThread(CNAllreducePPThread *thr) {
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

	unsigned _roles;
	unsigned _nThreads;
}; // class CNAllreducePPMessage

class CNAllreducePPModel : public XMI::Device::Interface::MulticombineModel<CNAllreducePPModel> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = -1;
	static const size_t sizeof_msg = sizeof(CNAllreducePPMessage);

	CNAllreducePPModel(xmi_result_t &status) :
	XMI::Device::Interface::MulticombineModel<CNAllreducePPModel>(status)
	{
		_dispatch_id = _g_cnallreducepp_dev.newDispID();
		_me = _g_cnallreducepp_dev.common()->getSysdep()->mapping.task();
	}

	inline bool postMulticombine_impl(xmi_multicombine_t *mcomb);

private:
	size_t _me;
	unsigned _dispatch_id;
}; // class CNAllreducePPModel

inline void CNAllreducePPMessage::__completeThread(CNAllreducePPThread *thr) {
	unsigned c = ((CNAllreducePPDevice &)_QS).__completeThread(thr);
	if (c >= _nThreads) {
		setStatus(XMI::Device::Done);
	}
}

void CNAllreducePPMessage::complete() {
	((CNAllreducePPDevice &)_QS).__complete<CNAllreducePPMessage>(this);
	executeCallback();
}

XMI::Device::MessageStatus CNAllreducePPMessage::advanceThread(XMI::Device::Generic::GenericAdvanceThread *t) {
	return __advanceThread((CNAllreducePPThread *)t);
}

// Permit a NULL results_topo to mean "everyone" (i.e. "root == -1")
inline bool CNAllreducePPModel::postMulticombine_impl(xmi_multicombine_t *mcomb) {
	XMI::Device::BGP::CNAllreduceSetup &tas =
		XMI::Device::BGP::CNAllreduceSetup::getCNAS(mcomb->dtype, mcomb->optor);
	// assert(tas._pre != NULL);
	if (!tas._pre || !tas._post) {
XMI_abort();
		return false;
	}
	XMI_TOPOLOGY_CLASS *result_topo = (XMI_TOPOLOGY_CLASS *)mcomb->results_participants;
	bool doStore = (!result_topo || result_topo->isRankMember(_me));
	size_t bytes = mcomb->count << xmi_dt_shift[mcomb->dtype];

	// could try to complete allreduce before construction, but for now the code
	// is too dependent on having message and thread structures to get/keep context.
	// __post() will still try early advance... (after construction)
	CNAllreducePPMessage *msg;
	msg = new (mcomb->request) CNAllreducePPMessage(_g_cnallreducepp_dev,
			(XMI_PIPEWORKQUEUE_CLASS *)mcomb->data,
			(XMI_PIPEWORKQUEUE_CLASS *)mcomb->results,
			bytes, doStore, mcomb->roles, mcomb->cb_done, _dispatch_id, tas);
	_g_cnallreducepp_dev.__post<CNAllreducePPMessage>(msg);
	return true;
}

};	// BGP
};	// Device
};	// XMI

#endif // __component_devices_bgp_cnallreduceppmsg_h__
