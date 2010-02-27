/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file components/devices/bgp/collective_network/CNAllreduceMsg.h
 * \brief Default collective network allreduce interface.
 */
#ifndef __components_devices_bgp_collective_network_CNAllreduceMsg_h__
#define __components_devices_bgp_collective_network_CNAllreduceMsg_h__

#include "util/common.h"
#include "components/devices/bgp/collective_network/CNDevice.h" // externs for env vars
#include "components/devices/bgp/collective_network/CNAllreduce.h"
#include "components/devices/bgp/collective_network/CollectiveNetworkLib.h"
#include "components/devices/bgp/collective_network/CNPacket.h"
#include "components/devices/generic/Device.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/MulticombineModel.h"
#include "components/devices/FactoryInterface.h"

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
class CNAllreduceDevice : public XMI::Device::Generic::SharedQueueSubDevice<CNDevice,CNAllreduceThread,2> {
public:
	inline CNAllreduceDevice(CNDevice *common) :
	XMI::Device::Generic::SharedQueueSubDevice<CNDevice,CNAllreduceThread,2>(common) {
	}
	class Factory : public Interface::FactoryInterface<Factory,CNAllreduceDevice,Generic::Device> {
	public:
		static inline CNAllreduceDevice *generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager & mm);
		static inline xmi_result_t init_impl(CNAllreduceDevice *devs, size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices);
		static inline size_t advance_impl(CNAllreduceDevice *devs, size_t client, size_t context);
	}; // class Factory
}; // class CNAllreduceDevice

};	// BGP
};	// Device
};	// XMI

extern XMI::Device::BGP::CNAllreduceDevice _g_cnallreduce_dev;

namespace XMI {
namespace Device {
namespace BGP {

inline CNAllreduceDevice *CNAllreduceDevice::Factory::generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager &mm) {
	_g_cnallreduce_dev.__create(client, num_ctx);
	return &_g_cnallreduce_dev;
}

inline xmi_result_t CNAllreduceDevice::Factory::init_impl(CNAllreduceDevice *devs, size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices) {
	return _g_cnallreduce_dev.__init(client, contextId, clt, ctx, sd, devices);
}

inline size_t CNAllreduceDevice::Factory::advance_impl(CNAllreduceDevice *devs, size_t client, size_t contextId) {
	return 0;
}

class CNAllreduceMessage : public XMI::Device::BGP::BaseGenericCNMessage {
	enum roles {
		NO_ROLE = 0,
		INJECTION_ROLE = (1 << 0), // first role must be "injector"
		RECEPTION_ROLE = (1 << 1), // last role must be "receptor"
	};
public:
	CNAllreduceMessage(Generic::GenericSubDevice *qs,
			xmi_multicombine_t *mcomb,
			size_t bytes,
			bool doStore,
			unsigned dispatch_id,
			XMI::Device::BGP::CNAllreduceSetup &tas) :
	BaseGenericCNMessage(qs, mcomb->client, mcomb->context,
			(XMI::PipeWorkQueue *)mcomb->data,
			(XMI::PipeWorkQueue *)mcomb->results,
			bytes, doStore, mcomb->roles, mcomb->cb_done,
				dispatch_id, tas._hhfunc, tas._opsize),
	_roles(mcomb->roles)
	{
	}

	// virtual function
	xmi_context_t postNext(bool devPosted) {
		return _g_cnallreduce_dev.common()->__postNext<CNAllreduceMessage,CNAllreduceThread>(this, devPosted);
	}

	inline int setThreads(CNAllreduceThread **th) {
		CNAllreduceThread *t;
		int n;
		_g_cnallreduce_dev.__getThreads(&t, &n);
		int nt = 0;
		_g_cnallreduce_dev.common()->__resetThreads();
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
			t[nt]._cycles = 3000; // DCMF_PERSISTENT_ADVANCE;
			// maybe not inject reception here?
			__advanceRcp(&t[nt]);
			++nt;
		}
		// assert(nt > 0? && nt < n);
		*th = t;
		return nt;
	}

protected:
	inline void __completeThread(CNAllreduceThread *thr);

	DECL_ADVANCE_ROUTINE(advanceInj,CNAllreduceMessage,CNAllreduceThread);
	DECL_ADVANCE_ROUTINE(advanceRcp,CNAllreduceMessage,CNAllreduceThread);
	inline xmi_result_t __advanceInj(CNAllreduceThread *thr) {
		if (thr->_bytesLeft == 0) return XMI_SUCCESS;
		unsigned hcount = BGPCN_FIFO_SIZE, dcount = BGPCN_QUADS_PER_FIFO;
		size_t avail = thr->_wq->bytesAvailableToConsume();
		char *buf = thr->_wq->bufferToConsume();
		bool aligned = (((unsigned)buf & 0x0f) == 0);
		size_t did = 0;
		if (avail < BGPCN_PKT_SIZE && avail < thr->_bytesLeft) {
			return XMI_EAGAIN;
		}
		if (__wait_send_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return XMI_EAGAIN;
		}
		__send_whole_packets(thr, hcount, dcount, avail, did, buf, aligned);
		__send_last_packet(thr, hcount, dcount, avail, did, buf, aligned);
		if (did) {
			thr->_wq->consumeBytes(did);
		}
		if (thr->_bytesLeft == 0) {
			thr->setStatus(XMI::Device::Complete);
			__completeThread(thr);
			return XMI_SUCCESS;
		}
		return XMI_EAGAIN;
	}


	inline xmi_result_t __advanceRcp(CNAllreduceThread *thr) {
		if (thr->_bytesLeft == 0) return XMI_SUCCESS;
		unsigned hcount = 0, dcount = 0;
		unsigned toCopy = thr->_bytesLeft >= BGPCN_PKT_SIZE ? BGPCN_PKT_SIZE : thr->_bytesLeft;
		size_t avail = thr->_wq->bytesAvailableToProduce();
		char *buf = thr->_wq->bufferToProduce();
		bool aligned = (((unsigned)buf & 0x0f) == 0);
		size_t did = 0;
		if (avail < toCopy) {
			return XMI_EAGAIN;
		}
		if (__wait_recv_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return XMI_EAGAIN;
		}
		__recv_whole_packets(thr, hcount, dcount, avail, did, buf, aligned);
		__recv_last_packet(thr, hcount, dcount, avail, did, buf, aligned);
		if (did) {
			thr->_wq->produceBytes(did);
		}
		if (thr->_bytesLeft == 0) {
			thr->setStatus(XMI::Device::Complete);
			__completeThread(thr);
			return XMI_SUCCESS;
		}
		return XMI_EAGAIN;
	}

	friend class XMI::Device::Generic::GenericMessage;

	unsigned _roles;
	unsigned _nThreads;
}; // class CNAllreduceMessage

class CNAllreduceModel : public XMI::Device::Interface::MulticombineModel<CNAllreduceModel,sizeof(CNAllreduceMessage)> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = -1;
	static const size_t sizeof_msg = sizeof(CNAllreduceMessage);

	CNAllreduceModel(xmi_result_t &status) :
	XMI::Device::Interface::MulticombineModel<CNAllreduceModel,sizeof(CNAllreduceMessage)>(status)
	{
		_dispatch_id = _g_cnallreduce_dev.newDispID();
		_me = __global.mapping.task();
		// at least one must do this
		XMI::Device::BGP::CNAllreduceSetup::initCNAS();
	}

	inline xmi_result_t postMulticombine_impl(uint8_t (&state)[sizeof_msg], xmi_multicombine_t *mcomb);

private:
	size_t _me;
	unsigned _dispatch_id;
}; // class CNAllreduceModel

inline void CNAllreduceMessage::__completeThread(CNAllreduceThread *thr) {
	unsigned c = _g_cnallreduce_dev.common()->__completeThread(thr);
	if (c >= _nThreads) {
		setStatus(XMI::Device::Done);
	}
}

// Permit a NULL results_topo to mean "everyone" (i.e. "root == -1")
inline xmi_result_t CNAllreduceModel::postMulticombine_impl(uint8_t (&state)[sizeof_msg], xmi_multicombine_t *mcomb) {
	XMI::Device::BGP::CNAllreduceSetup &tas = XMI::Device::BGP::CNAllreduceSetup::getCNAS(mcomb->dtype, mcomb->optor);
	// assert(tas._pre == NULL);
	if (tas._pre) {
		return XMI_ERROR;
	}
	XMI::Topology *result_topo = (XMI::Topology *)mcomb->results_participants;
	bool doStore = (!result_topo || result_topo->isRankMember(_me));
	size_t bytes = mcomb->count << xmi_dt_shift[mcomb->dtype];

	// could try to complete allreduce before construction, but for now the code
	// is too dependent on having message and thread structures to get/keep context.
	// __post() will still try early advance... (after construction)
	CNAllreduceMessage *msg;
	msg = new (&state) CNAllreduceMessage(_g_cnallreduce_dev.common(),
			mcomb, bytes, doStore, _dispatch_id, tas);
	_g_cnallreduce_dev.__post<CNAllreduceMessage>(msg);
	return XMI_SUCCESS;
}


};	// BGP
};	// Device
};	// XMI

#endif // __components_devices_bgp_cnallreducemsg_h__
