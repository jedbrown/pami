/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/WQRingBcastMsg.h
 * \brief ???
 */

#ifndef __generic_wq_bcast_h_
#define __generic_wq_bcast_h_

#include "generic/Device.h"
#include "generic/Message.h"
#include "generic/AdvanceThread.h"
#include "dcmf.h"
#include "PipeWorkQueue.h"
#include "prod/cdi/BroadcastModel.h"

extern XMI::Topology *_g_topology_local;

namespace DCMF {
namespace CDI {

class WQRingBcastMdl;
class WQRingBcastMsg;
typedef DCMF::Queueing::Generic::SimpleAdvanceThread WQRingBcastThr;
typedef DCMF::Queueing::Generic::SimpleSubDevice<WQRingBcastMdl,WQRingBcastMsg,WQRingBcastThr> WQRingBcastDev;

}; //-- CDI
}; //-- DCMF

extern DCMF::CDI::WQRingBcastDev _g_wqbcast_dev;

namespace DCMF {
namespace CDI {

///
/// \brief A local bcast message that takes advantage of the
/// shared-memory workqueues
///
class WQRingBcastMsg : public DCMF::Queueing::Generic::GenericMessage {
private:
	enum roles {
		NO_ROLE = 0,
		ROOT_ROLE = (1 << 0), // first role must be root
		NON_ROOT_ROLE = (1 << 1), // last role must be non-root(s)
	};
public:
	WQRingBcastMsg(BaseDevice &Generic_QS,
		XMI::PipeWorkQueue *iwq,
		XMI::PipeWorkQueue *swq,
		XMI::PipeWorkQueue *rwq,
		size_t bytes,
		XMI_Callback_t cb) :
	DCMF::Queueing::Generic::GenericMessage(Generic_QS, cb),
	_iwq(iwq),
	_swq(swq), // might be NULL
	_rwq(rwq), // might be NULL (but not both)
	_bytes(bytes)
	{
	}

	// This is a virtual function, but declaring inline here avoids linker
	// complaints about multiple definitions.
	inline void complete();

	inline DCMF::Queueing::MessageStatus advanceThread(DCMF::Queueing::Generic::GenericAdvanceThread *t);

protected:
	//friend class WQRingBcastDev; // Until C++ catches up with real programming languages:
	friend class DCMF::Queueing::Generic::SimpleSubDevice<WQRingBcastMdl,WQRingBcastMsg,WQRingBcastThr>;

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

	inline DCMF::Queueing::MessageStatus __advanceThread(WQRingBcastThr *thr) {
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
			return DCMF::Queueing::Active;
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
			setStatus(DCMF::Queueing::Done);
			return DCMF::Queueing::Done;
		}
		return DCMF::Queueing::Active;
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
	XMI::PipeWorkQueue *_iwq;
	XMI::PipeWorkQueue *_swq;
	XMI::PipeWorkQueue *_rwq;
	size_t _bytes;
}; //-- WQRingBcastMsg

class WQRingBcastMdl : public Broadcast::Model<WQRingBcastMdl,WQRingBcastDev,WQRingBcastMsg> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = 1;

	WQRingBcastMdl(DCMF::SysDep *sysdep, XMI_Result &status) :
	Broadcast::Model<WQRingBcastMdl,WQRingBcastDev,WQRingBcastMsg>(_g_wqbcast_dev, status)
	{
		_me = DCMF_Messager_rank();
		size_t tz = _g_topology_local->size();
		for (size_t x = 0; x < tz; ++x) {
#ifdef USE_FLAT_BUFFER
			_wq[x].configure(sysdep, USE_FLAT_BUFFER, 0);
#else /* ! USE_FLAT_BUFFER */
			_wq[x].configure(sysdep, 8192);
#endif /* ! USE_FLAT_BUFFER */
			_wq[x].reset();
		}
	}

	inline void reset_impl() {}

	inline bool generateMessage_impl(XMI_Multicast_t *mcast);

private:
	size_t _me;
	XMI::PipeWorkQueue _wq[NUM_CORES /* * NUM_THREADS */];

	static inline void compile_time_assert () {
		COMPILE_TIME_ASSERT(sizeof(XMI_Request_t) >= sizeof(WQRingBcastMsg));
	}
}; // class WQRingBcastMdl

void WQRingBcastMsg::complete() {
	((WQRingBcastDev &)_QS).__complete(this);
	executeCallback();
}

inline DCMF::Queueing::MessageStatus WQRingBcastMsg::advanceThread(DCMF::Queueing::Generic::GenericAdvanceThread *t) {
	return __advanceThread((WQRingBcastThr *)t);
}

inline bool WQRingBcastMdl::generateMessage_impl(XMI_Multicast_t *mcast) {
	if (mcast->req_size < sizeof(WQRingBcastMsg)) {
		return false;
	}
	XMI::Topology *dst_topo = (XMI::Topology *)mcast->dst_participants;
	XMI::Topology *src_topo = (XMI::Topology *)mcast->src_participants;
	size_t meix = dst_topo->rank2Index(_me);
	size_t meix_1 = meix + 1;
	if (meix_1 >= dst_topo->size()) {
		meix_1 -= dst_topo->size();
	}
	size_t me_1 = dst_topo->index2Rank(meix_1);
	WQRingBcastMsg *msg;
	if (src_topo->isRankMember(_me)) {      // I am root
		// I am root - at head of stream
		// DCMF_assert(roles == ROOT_ROLE);
		// _input ===> _wq[meix]
#ifdef USE_FLAT_BUFFER
		_wq[meix_1].reset();
#endif /* USE_FLAT_BUFFER */
		msg = new (mcast->request) WQRingBcastMsg(_g_wqbcast_dev,
					(XMI::PipeWorkQueue *)mcast->src, &_wq[meix_1], (XMI::PipeWorkQueue *)mcast->dst, mcast->bytes, mcast->cb_done);
	} else if (src_topo->isRankMember(me_1)) {
		// I am tail of stream - no one is downstream from me.
		// DCMF_assert(roles == NON_ROOT_ROLE);
		// _wq[meix_1] ===> results
		msg = new (mcast->request) WQRingBcastMsg(_g_wqbcast_dev,
					&_wq[meix], NULL, (XMI::PipeWorkQueue *)mcast->dst, mcast->bytes, mcast->cb_done);
	} else {
		// DCMF_assert(roles == NON_ROOT_ROLE);
		// _wq[meix_1] =+=> results
		//              +=> _wq[meix]
#ifdef USE_FLAT_BUFFER
		_wq[meix_1].reset();
#endif /* USE_FLAT_BUFFER */
		msg = new (mcast->request) WQRingBcastMsg(_g_wqbcast_dev,
					&_wq[meix], &_wq[meix_1], (XMI::PipeWorkQueue *)mcast->dst, mcast->bytes, mcast->cb_done);
	}
	_g_wqbcast_dev.__post(msg);
	return true;
}

}; //-- CDI
}; //-- DCMF

#endif /* __generic_wq_bcast_h_ */
