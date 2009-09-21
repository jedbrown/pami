/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file components/devices/bgp/collective_network/CNAllreduceMsg.h
 * \brief Default collective network allreduce interface.
 */
#ifndef __components_devices_bgp_cnallreducemsg_h__
#define __components_devices_bgp_cnallreducemsg_h__

#include "Util.h"
#include "components/devices/bgp/collective_network/CNDevice.h" // externs for env vars
#include "components/devices/bgp/collective_network/CNAllreduce.h"
#include "components/devices/bgp/collective_network/CollectiveNetworkLib.h"
#include "components/devices/bgp/collective_network/Packet.h"
#include "components/devices/generic/Device.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/common/MulticombineModelImpl.h"
#include "PipeWorkQueue.h"

/// \page xmi_multicombine_examples
///
/// #include "components/devices/bgp/collective_network/CNAllreduceMsg.h"
/// typedef XMI::Device::BGP::CNAllreduceModel   MY_ALLREDUCE_MODEL;
/// typedef XMI::Device::BGP::CNAllreduceMessage MY_ALLREDUCE_MEGSSAGE;
///
/// xmi_result_t status;
/// MY_ALLREDUCE_MODEL _allreduce(status);
/// XMI_assert(status == XMI_SUCCESS);
///
/// xmi_multicombine_t _mcomb;
/// MY_ALLREDUCE_MEGSSAGE _msg;
/// _mcomb.request = &_msg;
/// _mcomb.cb_done = ...;
/// _mcomb.roles = ...;
/// _mcomb.data = ...;
/// ...
/// _allreduce.postMulticombine(&_mcomb);
///

namespace XMI {
namespace Device {
namespace BGP {

/**
 * \brief Tree Allreduce Send Message base class
 *
 * Tree Allreduce with source/dest as pipeworkqueue.
 */

class CNAllreduceModel;
class CNAllreduceMessage;
typedef XMI::Device::BGP::BaseGenericCNThread CNAllreduceThread;
typedef XMI::Device::Generic::SharedQueueSubDevice<CNAllreduceModel,CNDevice,CNAllreduceMessage,CNAllreduceThread,2> CNAllreduceDevice;

};	// BGP
};	// Device
};	// XMI

extern XMI::Device::BGP::CNAllreduceDevice _g_cnallreduce_dev;

namespace XMI {
namespace Device {
namespace BGP {

class CNAllreduceMessage : public XMI::Device::BGP::BaseGenericCNMessage {
	enum roles {
		NO_ROLE = 0,
		INJECTION_ROLE = (1 << 0), // first role must be "injector"
		RECEPTION_ROLE = (1 << 1), // last role must be "receptor"
	};
public:
	CNAllreduceMessage(BaseDevice &qs,
			XMI::PipeWorkQueue *swq,
			XMI::PipeWorkQueue *rwq,
			size_t bytes,
			bool doStore,
			unsigned roles,
			const XMI_Callback_t cb,
			unsigned dispatch_id,
			XMI::Device::BGP::CNAllreduceSetup &tas) :
	BaseGenericCNMessage(qs, swq, rwq, bytes, doStore, roles, cb,
				dispatch_id, tas._hhfunc, tas._opsize),
	_roles(roles)
	{
	}

	inline XMI::Device::MessageStatus advanceThread(XMI::Device::Generic::GenericAdvanceThread *t);
	inline void complete();
protected:
	//friend class CNAllreduceDevice;
	friend class XMI::Device::Generic::SharedQueueSubDevice<CNAllreduceModel,CNDevice,CNAllreduceMessage,CNAllreduceThread,2>;

	inline int __setThreads(CNAllreduceThread *t, int n) {
		int nt = 0;
		int maxnt = ((CNAllreduceDevice &)_QS).common()->getMaxThreads();
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
			t[nt]._cycles = 3000; // DCMF_PERSISTENT_ADVANCE;
			++nt;
		}
		// assert(nt > 0? && nt < n);
		_nThreads = nt;
		if (_bytes >= 16384; // DCMF_TREE_HELPER_THRESH) {
			setThreadsWanted(MIN(nt, maxnt));
		}
		return nt;
	}

	inline void __completeThread(CNAllreduceThread *thr);

	inline XMI::Device::MessageStatus __advanceInj(CNAllreduceThread *thr) {
		if (thr->_bytesLeft == 0) return XMI::Device::Done;
		unsigned hcount = BGPCN_FIFO_SIZE, dcount = BGPCN_QUADS_PER_FIFO;
		size_t avail = thr->_wq->bytesAvailableToConsume();
		char *buf = thr->_wq->bufferToConsume();
		bool aligned = (((unsigned)buf & 0x0f) == 0);
		size_t did = 0;
		if (avail < BGPCN_PKT_SIZE && avail < thr->_bytesLeft) {
			return XMI::Device::Active;
		}
		if (__wait_send_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return XMI::Device::Active;
		}
		__send_whole_packets(thr, hcount, dcount, avail, did, buf, aligned);
		__send_last_packet(thr, hcount, dcount, avail, did, buf, aligned);
		if (did) {
			thr->_wq->consumeBytes(did);
		}
		if (thr->_bytesLeft == 0) {
			thr->setDone(true);
			return XMI::Device::Done;
		}
		return XMI::Device::Active;
	}


	inline XMI::Device::MessageStatus __advanceRcp(CNAllreduceThread *thr) {
		if (thr->_bytesLeft == 0) return XMI::Device::Done;
		unsigned hcount = 0, dcount = 0;
		unsigned toCopy = thr->_bytesLeft >= BGPCN_PKT_SIZE ? BGPCN_PKT_SIZE : thr->_bytesLeft;
		size_t avail = thr->_wq->bytesAvailableToProduce();
		char *buf = thr->_wq->bufferToProduce();
		bool aligned = (((unsigned)buf & 0x0f) == 0);
		size_t did = 0;
		if (avail < toCopy) {
			return XMI::Device::Active;
		}
		if (__wait_recv_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return XMI::Device::Active;
		}
		__recv_whole_packets(thr, hcount, dcount, avail, did, buf, aligned);
		__recv_last_packet(thr, hcount, dcount, avail, did, buf, aligned);
		if (did) {
			thr->_wq->produceBytes(did);
		}
		if (thr->_bytesLeft == 0) {
			thr->setDone(true);
			return XMI::Device::Done;
		}
		return XMI::Device::Active;
	}

	inline XMI::Device::MessageStatus __advanceThread(CNAllreduceThread *thr) {
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
}; // class CNAllreduceMessage

class CNAllreduceModel : public XMI::Device::Interface::MulticombineModel<CNAllreduceModel> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = -1;

	CNAllreduceModel(XMI_Result &status) :
	XMI::Device::Interface::MulticombineModel<CNAllreduceModel>(status)
	{
		_dispatch_id = _g_cnallreduce_dev.newDispID();
		_me = sysdep->mapping().rank();
		// at least one must do this
		XMI::Device::BGP::CNAllreduceSetup::initCNAS();
		// if we need sysdep, use _g_cnallreduce_dev.getSysdep()...
	}

	inline bool postMulticombine_impl(xmi_multicombine_t *mcomb);

private:
	size_t _me;
	unsigned _dispatch_id;
	static inline void compile_time_assert () {
		COMPILE_TIME_ASSERT(sizeof(XMI_Request_t) >= sizeof(CNAllreduceMessage));
	}
}; // class CNAllreduceModel

inline void CNAllreduceMessage::__completeThread(CNAllreduceThread *thr) {
	unsigned c = ((CNAllreduceDevice &)_QS).__completeThread(thr);
	if (c >= _nThreads) {
		setStatus(XMI::Device::Done);
	}
}

void CNAllreduceMessage::complete() {
	((CNAllreduceDevice &)_QS).__complete(this);
	executeCallback();
}

XMI::Device::MessageStatus CNAllreduceMessage::advanceThread(XMI::Device::Generic::GenericAdvanceThread *t) {
	return __advanceThread((CNAllreduceThread *)t);
}

// Permit a NULL results_topo to mean "everyone" (i.e. "root == -1")
inline bool CNAllreduceModel::postMulticombine_impl(xmi_multicombine_t *mcomb) {
	XMI::Device::BGP::CNAllreduceSetup &tas = XMI::Device::BGP::CNAllreduceSetup::getCNAS(mcomb->dtype, mcomb->optor);
	// assert(tas._pre == NULL);
	if (tas._pre) {
		return false;
	}
	XMI::Topology *result_topo = (XMI::Topology *)mcomb->results_participants();
	bool doStore = (!result_topo || result_topo->isRankMember(_me));
	size_t bytes = mcomb->count << xmi_dt_shift[mcomb->dtype];

	// could try to complete allreduce before construction, but for now the code
	// is too dependent on having message and thread structures to get/keep context.
	// __post() will still try early advance... (after construction)
	CNAllreduceMessage *msg;
	msg = new (mcomb->request) CNAllreduceMessage(_g_cnallreduce_dev,
			(XMI::PipeWorkQueue *)mcomb->data,
			(XMI::PipeWorkQueue *)mcomb->results,
			bytes, doStore, mcomb->roles, mcomb->cb_done, _dispatch_id, tas);
	_g_cnallreduce_dev.__post(msg);
	return true;
}


};	// BGP
};	// Device
};	// XMI

#endif // __components_devices_bgp_cnallreducemsg_h__
