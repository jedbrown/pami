/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/WQRingBcastMsg.h
 * \brief ???
 */

#ifndef __components_devices_workqueue_wqringbcastmsg_h__
#define __components_devices_workqueue_wqringbcastmsg_h__

#include "components/devices/generic/Device.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "sys/xmi.h"
#include "components/devices/MulticastModel.h"

namespace XMI {
namespace Device {

class WQRingBcastMdl;
class WQRingBcastMsg;
typedef XMI::Device::Generic::SimpleAdvanceThread WQRingBcastThr;
typedef XMI::Device::Generic::SimpleSubDevice<WQRingBcastThr> WQRingBcastDev;

}; //-- Device
}; //-- XMI

extern XMI::Device::WQRingBcastDev _g_wqbcast_dev;

namespace XMI {
namespace Device {

///
/// \brief A local bcast message that takes advantage of the
/// shared-memory workqueues
///
class WQRingBcastMsg : public XMI::Device::Generic::GenericMessage {
private:
	enum roles {
		NO_ROLE = 0,
		ROOT_ROLE = (1 << 0), // first role must be root
		NON_ROOT_ROLE = (1 << 1), // last role must be non-root(s)
	};
public:
	WQRingBcastMsg(Generic::BaseGenericDevice &Generic_QS,
		XMI_PIPEWORKQUEUE_CLASS *iwq,
		XMI_PIPEWORKQUEUE_CLASS *swq,
		XMI_PIPEWORKQUEUE_CLASS *rwq,
		size_t bytes,
		xmi_callback_t cb) :
	XMI::Device::Generic::GenericMessage(Generic_QS, cb),
	_iwq(iwq),
	_swq(swq), // might be NULL
	_rwq(rwq), // might be NULL (but not both)
	_bytes(bytes)
	{
	}

	// This is a virtual function, but declaring inline here avoids linker
	// complaints about multiple definitions.
	inline void complete();

	inline XMI::Device::MessageStatus advanceThread(XMI::Device::Generic::GenericAdvanceThread *t);

protected:
	//friend class WQRingBcastDev; // Until C++ catches up with real programming languages:
	friend class XMI::Device::Generic::SimpleSubDevice<WQRingBcastThr>;

	inline int __setThreads(WQRingBcastThr *t, int n) {
		int nt = 0;
		// assert(nt < n);
		t[nt].setMsg(this);
		t[nt].setDone(false);
		t[nt]._bytesLeft = _bytes;
		++nt;
		// assert(nt > 0? && nt < n);
		_nThreads = nt;
		return nt;
	}

	inline XMI::Device::MessageStatus __advanceThread(WQRingBcastThr *thr) {
		size_t min = thr->_bytesLeft;
		size_t wq = _iwq->bytesAvailableToConsume();
		if (wq < min) min = wq;
		if (_swq) {
			wq = _swq->bytesAvailableToProduce();
			if (wq < min) min = wq;
		}
		if (_rwq) {
			wq = _rwq->bytesAvailableToProduce();
			if (wq < min) min = wq;
		}
		if (min == 0) {
			return XMI::Device::Active;
		}
		if (_rwq) {
			memcpy(_rwq->bufferToProduce(), _iwq->bufferToConsume(), min);
			_rwq->produceBytes(min);
		}
		if (_swq) {
			memcpy(_swq->bufferToProduce(), _iwq->bufferToConsume(), min);
			_swq->produceBytes(min);
		}
		_iwq->consumeBytes(min);
		thr->_bytesLeft -= min;
		if (thr->_bytesLeft == 0) {
			// thread is Done, maybe not message
			thr->setDone(true);
#ifdef USE_WAKEUP_VECTORS
			__clearWakeup(thr);
#endif /* USE_WAKEUP_VECTORS */
			setStatus(XMI::Device::Done);
			return XMI::Device::Done;
		}
		return XMI::Device::Active;
	}

	/// \brief arrange to be woken up when inputs/outputs become "ready"
	///
	/// This is very simple - we wake up if any input/output becomes
	/// ready and it might be a false wakeup if any other input/output
	/// is not (yet) ready.
	///
	/// \param[in] thr	The thread which wishes to be woken
	///
	inline void __setWakeup(WQRingBcastThr *thr) {
		void *v = thr->getWakeVec();
		_iwq->setConsumerWakeup(v);
		if (_rwq) {
			_rwq->setProducerWakeup(v);
		}
		if (_swq) {
			_swq->setProducerWakeup(v);
		}
	}

	/// \brief clear a previously set wakeup
	///
	/// \param[in] thr	The thread which no longer wishes to be woken
	///
	inline void __clearWakeup(WQRingBcastThr *thr) {
		_iwq->setConsumerWakeup(NULL);
		if (_rwq) {
			_rwq->setProducerWakeup(NULL);
		}
		if (_swq) {
			_swq->setProducerWakeup(NULL);
		}
	}

	unsigned _nThreads;
	XMI_PIPEWORKQUEUE_CLASS *_iwq;
	XMI_PIPEWORKQUEUE_CLASS *_swq;
	XMI_PIPEWORKQUEUE_CLASS *_rwq;
	size_t _bytes;
}; //-- WQRingBcastMsg

class WQRingBcastMdl : public XMI::Device::Interface::MulticastModel<WQRingBcastMdl> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = 1;
	static const size_t sizeof_msg = sizeof(WQRingBcastMsg);

	WQRingBcastMdl(xmi_result_t &status) :
	XMI::Device::Interface::MulticastModel<WQRingBcastMdl>(status)
	{
		XMI_SYSDEP_CLASS *sd = _g_wqbcast_dev.getSysdep();
		_me = __global.mapping.task();
		size_t tz;
		__global.mapping.nodePeers(tz);
		for (size_t x = 0; x < tz; ++x) {
#ifdef USE_FLAT_BUFFER
			_wq[x].configure(sd, USE_FLAT_BUFFER, 0);
#else /* ! USE_FLAT_BUFFER */
			_wq[x].configure(sd, 8192);
#endif /* ! USE_FLAT_BUFFER */
			_wq[x].reset();
		}
	}

	inline bool postMulticast_impl(xmi_multicast_t *mcast);

private:
	size_t _me;
	XMI_PIPEWORKQUEUE_CLASS _wq[XMI_MAX_PROC_PER_NODE];
}; // class WQRingBcastMdl

void WQRingBcastMsg::complete() {
	((WQRingBcastDev &)_QS).__complete<WQRingBcastMsg>(this);
	executeCallback();
}

inline XMI::Device::MessageStatus WQRingBcastMsg::advanceThread(XMI::Device::Generic::GenericAdvanceThread *t) {
	return __advanceThread((WQRingBcastThr *)t);
}

inline bool WQRingBcastMdl::postMulticast_impl(xmi_multicast_t *mcast) {
	XMI_TOPOLOGY_CLASS *dst_topo = (XMI_TOPOLOGY_CLASS *)mcast->dst_participants;
	XMI_TOPOLOGY_CLASS *src_topo = (XMI_TOPOLOGY_CLASS *)mcast->src_participants;
	size_t meix = dst_topo->rank2Index(_me);
	size_t meix_1 = meix + 1;
	if (meix_1 >= dst_topo->size()) {
		meix_1 -= dst_topo->size();
	}
	size_t me_1 = dst_topo->index2Rank(meix_1);
	WQRingBcastMsg *msg;
	if (src_topo->isRankMember(_me)) {      // I am root
		// I am root - at head of stream
		// XMI_assert(roles == ROOT_ROLE);
		// _input ===> _wq[meix]
#ifdef USE_FLAT_BUFFER
		_wq[meix_1].reset();
#endif /* USE_FLAT_BUFFER */
		msg = new (mcast->request) WQRingBcastMsg(_g_wqbcast_dev,
					(XMI_PIPEWORKQUEUE_CLASS *)mcast->src, &_wq[meix_1], (XMI_PIPEWORKQUEUE_CLASS *)mcast->dst, mcast->bytes, mcast->cb_done);
	} else if (src_topo->isRankMember(me_1)) {
		// I am tail of stream - no one is downstream from me.
		// XMI_assert(roles == NON_ROOT_ROLE);
		// _wq[meix_1] ===> results
		msg = new (mcast->request) WQRingBcastMsg(_g_wqbcast_dev,
					&_wq[meix], NULL, (XMI_PIPEWORKQUEUE_CLASS *)mcast->dst, mcast->bytes, mcast->cb_done);
	} else {
		// XMI_assert(roles == NON_ROOT_ROLE);
		// _wq[meix_1] =+=> results
		//              +=> _wq[meix]
#ifdef USE_FLAT_BUFFER
		_wq[meix_1].reset();
#endif /* USE_FLAT_BUFFER */
		msg = new (mcast->request) WQRingBcastMsg(_g_wqbcast_dev,
					&_wq[meix], &_wq[meix_1], (XMI_PIPEWORKQUEUE_CLASS *)mcast->dst, mcast->bytes, mcast->cb_done);
	}
	_g_wqbcast_dev.__post<WQRingBcastMsg>(msg);
	return true;
}

}; //-- Device
}; //-- XMI

#endif // __components_devices_workqueue_wqringbcastmsg_h__
