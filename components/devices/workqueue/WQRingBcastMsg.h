/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/WQRingBcastMsg.h
 * \brief ???
 */

#ifndef __components_devices_workqueue_WQRingBcastMsg_h__
#define __components_devices_workqueue_WQRingBcastMsg_h__

#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/util/SubDeviceSuppt.h"
#include "sys/xmi.h"
#include "Global.h"
#include "PipeWorkQueue.h"
#include "Topology.h"
#include "components/devices/MulticastModel.h"

namespace XMI {
namespace Device {

class WQRingBcastMdl;
class WQRingBcastMsg;
typedef XMI::Device::Generic::SimpleAdvanceThread WQRingBcastThr;
typedef XMI::Device::Generic::SimpleSubDevice<WQRingBcastThr> WQRingBcastRealDev;

}; //-- Device
}; //-- XMI

extern XMI::Device::WQRingBcastRealDev _g_wqbcast_dev;

namespace XMI {
namespace Device {

class WQRingBcastDev : public XMI::Device::Generic::SimplePseudoDevice<WQRingBcastDev,WQRingBcastRealDev> {
public:
	static inline WQRingBcastDev *create(size_t client, size_t num_ctx, XMI::Device::Generic::Device *devices) {
		return __create(client, num_ctx, devices, &_g_wqbcast_dev);
	}

	inline WQRingBcastDev(size_t client, size_t num_ctx, XMI::Device::Generic::Device *devices, size_t ctx) :
	XMI::Device::Generic::SimplePseudoDevice<WQRingBcastDev,WQRingBcastRealDev>(client, num_ctx, devices, ctx)
	{
	}

	inline void init(SysDep *sd, size_t client, size_t num_ctx, xmi_context_t context, size_t contextid) {
		__init(sd, client, num_ctx, context, contextid, &_g_wqbcast_dev);
	}

	inline size_t advance_impl() {
		return _g_wqbcast_dev.advance(_clientid, _contextid);
	}
}; // class WQRingBcastDev

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
	WQRingBcastMsg(Generic::GenericSubDevice &Generic_QS,
		xmi_multicast_t *mcast,
		XMI::PipeWorkQueue *iwq,
		XMI::PipeWorkQueue *swq,
		XMI::PipeWorkQueue *rwq) :
	XMI::Device::Generic::GenericMessage(Generic_QS, mcast->cb_done,
				mcast->client, mcast->context),
	_iwq(iwq),
	_swq(swq), // might be NULL
	_rwq(rwq), // might be NULL (but not both)
	_bytes(mcast->bytes)
	{
	}

	STD_POSTNEXT(WQRingBcastDev,WQRingBcastThr,&_g_wqbcast_dev)

protected:
	//friend class WQRingBcastDev; // Until C++ catches up with real programming languages:
	friend class XMI::Device::Generic::SimpleSubDevice<WQRingBcastThr>;

	ADVANCE_ROUTINE(advanceThread,WQRingBcastMsg,WQRingBcastThr);
	inline int __setThreads(WQRingBcastThr *t, int n) {
		int nt = 0;
		// assert(nt < n);
		_nThreads = 1; // must predict total number of threads
		t[nt].setMsg(this);
		t[nt].setAdv(advanceThread);
		t[nt].setStatus(XMI::Device::Ready);
		t[nt]._bytesLeft = _bytes;
#ifdef USE_WAKEUP_VECTORS
		// not here - but somewhere/somehow...
		__setWakeup(thr);
#endif // USE_WAKEUP_VECTORS
		__advanceThread(&t[nt]);
		++nt;
		// assert(nt > 0? && nt < n);
		return nt;
	}

	friend class XMI::Device::Generic::GenericMessage;
	inline xmi_result_t __advanceThread(WQRingBcastThr *thr) {
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
			return XMI_EAGAIN;
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
			thr->setStatus(XMI::Device::Complete);
#ifdef USE_WAKEUP_VECTORS
			__clearWakeup(thr);
#endif // USE_WAKEUP_VECTORS
			setStatus(XMI::Device::Done);
			return XMI_SUCCESS;
		}
		return XMI_EAGAIN;
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
#ifdef USE_WAKEUP_VECTORS
		void *v = thr->getWakeVec();
		_iwq->setConsumerWakeup(v);
		if (_rwq) {
			_rwq->setProducerWakeup(v);
		}
		if (_swq) {
			_swq->setProducerWakeup(v);
		}
#endif // USE_WAKEUP_VECTORS
	}

	/// \brief clear a previously set wakeup
	///
	/// \param[in] thr	The thread which no longer wishes to be woken
	///
	inline void __clearWakeup(WQRingBcastThr *thr) {
#ifdef USE_WAKEUP_VECTORS
		_iwq->setConsumerWakeup(NULL);
		if (_rwq) {
			_rwq->setProducerWakeup(NULL);
		}
		if (_swq) {
			_swq->setProducerWakeup(NULL);
		}
#endif // USE_WAKEUP_VECTORS
	}

	unsigned _nThreads;
	XMI::PipeWorkQueue *_iwq;
	XMI::PipeWorkQueue *_swq;
	XMI::PipeWorkQueue *_rwq;
	size_t _bytes;
}; //-- WQRingBcastMsg

class WQRingBcastMdl : public XMI::Device::Interface::MulticastModel<WQRingBcastMdl,sizeof(WQRingBcastMsg)> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = 1;
	static const size_t sizeof_msg = sizeof(WQRingBcastMsg);

	WQRingBcastMdl(xmi_result_t &status) :
        XMI::Device::Interface::MulticastModel<WQRingBcastMdl,sizeof(WQRingBcastMsg)>(status)
	{
		XMI::SysDep *sd = _g_wqbcast_dev.getSysdep();
		_me = __global.mapping.task();
		size_t t0 = __global.topology_local.index2Rank(0);
		size_t tz;
		__global.mapping.nodePeers(tz);
		for (size_t x = 0; x < tz; ++x) {
#ifdef USE_FLAT_BUFFER
			_wq[x].configure(sd, USE_FLAT_BUFFER, 0);
#else /* ! USE_FLAT_BUFFER */
			_wq[x].configure(sd, 8192);
#endif /* ! USE_FLAT_BUFFER */
			_wq[x].barrier_reset(tz, _me == t0);
		}
	}

	inline xmi_result_t postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                               xmi_multicast_t *mcast);

private:
	size_t _me;
	XMI::PipeWorkQueue _wq[XMI_MAX_PROC_PER_NODE];
}; // class WQRingBcastMdl

inline xmi_result_t WQRingBcastMdl::postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                               xmi_multicast_t *mcast) {
	XMI::Topology *dst_topo = (XMI::Topology *)mcast->dst_participants;
	XMI::Topology *src_topo = (XMI::Topology *)mcast->src_participants;
	size_t root = src_topo->index2Rank(0); // assert size(0 == 1...
	bool iamroot = (root == _me);
	bool iamlast = false;
	// root may not be member of dst_topo!
	size_t meix_1;
	size_t me_1;
	size_t meix = dst_topo->rank2Index(_me);
	if (meix == (size_t)-1) {
		XMI_assertf(iamroot, "WQRingBcastMdl::postMulticast called by non-participant\n");
		// meix = 0; // not used
		meix_1 = 0; // output to _wq[0]
	} else {
		// input from _wq[meix], output to _wq[meix+1]...
		// this only gets funky if the root is also a member of dst_topo...
		meix_1 = meix + 1;
		if (meix_1 >= dst_topo->size()) {
			meix_1 -= dst_topo->size();
		}
		me_1 = dst_topo->index2Rank(meix_1);
		// if my downstream is "root", or if root is not in dst_topo and
		if (dst_topo->isRankMember(root)) {
			iamlast = (me_1 == root);
		} else {
			iamlast = (meix_1 == 0);
		}
	}
	WQRingBcastMsg *msg;
	if (iamroot) {      // I am root
		// I am root - at head of stream
		// XMI_assert(roles == ROOT_ROLE);
		// _input ===> _wq[meix]
#ifdef USE_FLAT_BUFFER
		_wq[meix_1].reset();
#endif /* USE_FLAT_BUFFER */
		msg = new (&state) WQRingBcastMsg(_g_wqbcast_dev, mcast,
					(XMI::PipeWorkQueue *)mcast->src, &_wq[meix_1], NULL);
	} else if (iamlast) {
		// I am tail of stream - no one is downstream from me.
		// XMI_assert(roles == NON_ROOT_ROLE);
		// _wq[meix_1] ===> results
		msg = new (&state) WQRingBcastMsg(_g_wqbcast_dev, mcast,
					&_wq[meix], NULL, (XMI::PipeWorkQueue *)mcast->dst);
	} else {
		// XMI_assert(roles == NON_ROOT_ROLE);
		// _wq[meix_1] =+=> results
		//              +=> _wq[meix]
#ifdef USE_FLAT_BUFFER
		_wq[meix_1].reset();
#endif /* USE_FLAT_BUFFER */
		msg = new (&state) WQRingBcastMsg(_g_wqbcast_dev, mcast,
					&_wq[meix], &_wq[meix_1], (XMI::PipeWorkQueue *)mcast->dst);
	}
	_g_wqbcast_dev.__post<WQRingBcastMsg>(msg);
	return XMI_SUCCESS;
}

}; //-- Device
}; //-- XMI

#endif // __components_devices_workqueue_wqringbcastmsg_h__
