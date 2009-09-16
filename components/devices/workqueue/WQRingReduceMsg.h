/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file devices/prod/workqueue/WQRingReduceMsg.h
 * \brief ???
 */

#ifndef __generic_wq_reduce_h_
#define __generic_wq_reduce_h_

#include "generic/Device.h"
#include "generic/Message.h"
#include "generic/AdvanceThread.h"
#include "dcmf.h"
#include "PipeWorkQueue.h"
#include "prod/cdi/ReduceModel.h"
#include "math_coremath.h"

extern XMI::Topology *_g_topology_local;

namespace DCMF {
namespace CDI {

class WQRingReduceMdl;
class WQRingReduceMsg;
typedef DCMF::Queueing::Generic::SimpleAdvanceThread WQRingReduceThr;
typedef DCMF::Queueing::Generic::SimpleSubDevice<WQRingReduceMdl,WQRingReduceMsg,WQRingReduceThr> WQRingReduceDev;

}; //-- CDI
}; //-- DCMF

extern DCMF::CDI::WQRingReduceDev _g_wqreduce_dev;

namespace DCMF {
namespace CDI {
///
/// \brief A local barrier message that takes advantage of the
/// Load Linked and Store Conditional instructions
///
class WQRingReduceMsg : public DCMF::Queueing::Generic::GenericMessage {
private:
	enum roles {
		NO_ROLE = 0,
		NON_ROOT_ROLE = (1 << 0), // first role must be non-root(s)
		ROOT_ROLE = (1 << 1), // last role must be root
	};

public:
	WQRingReduceMsg(BaseDevice &Generic_QS,
		XMI::PipeWorkQueue *iwq,
		XMI::PipeWorkQueue *swq,
		XMI::PipeWorkQueue *rwq,
		XMI_Op op,
		XMI_Dt dt,
		size_t count,
		XMI_Callback_t cb) :
	DCMF::Queueing::Generic::GenericMessage(Generic_QS, cb),
	_iwq(iwq),
	_swq(swq), // might be NULL
	_rwq(rwq),
	_count(count),
	_shift(dcmf_dt_shift[dt])
	{
		if (_swq) {
			// full combine...
			_func = MATH_OP_FUNCS(dt, op, 2);
		} else {
			// copy only
			_func = NULL;
		}
	}

	// This is a virtual function, but declaring inline here avoids linker
	// complaints about multiple definitions.
	inline void complete();

	inline DCMF::Queueing::MessageStatus advanceThread(DCMF::Queueing::Generic::GenericAdvanceThread *t);

protected:
	//friend class WQRingReduceDev;
	friend class DCMF::Queueing::Generic::SimpleSubDevice<WQRingReduceMdl,WQRingReduceMsg,WQRingReduceThr>;

	inline int __setThreads(WQRingReduceThr *t, int n) {
		int nt = 0;
		// assert(nt < n);
		t[nt].setMsg(this);
		t[nt].setDone(false);
		t[nt]._bytesLeft = _count << _shift;
		++nt;
		// assert(nt > 0? && nt < n);
		_nThreads = nt;
		return nt;
	}

	inline DCMF::Queueing::MessageStatus __advanceThread(WQRingReduceThr *thr) {
		size_t min = thr->_bytesLeft;
		size_t wq = _rwq->bytesAvailableToProduce();
		if (wq < min) min = wq;
		wq = _iwq->bytesAvailableToConsume();
		if (wq < min) min = wq;
		// _swq != NULL iff _func != NULL...
		if (_swq) {
			wq = _swq->bytesAvailableToConsume();
			if (wq < min) min = wq;
			if (min == 0) {
				return DCMF::Queueing::Active;
			}
			void *buf[2] = { _iwq->bufferToConsume(), _swq->bufferToConsume() };
			_func(_rwq->bufferToProduce(), buf, 2, min >> _shift);
			_swq->consumeBytes(min);
		} else {
			if (min == 0) {
				return DCMF::Queueing::Active;
			}
			memcpy(_rwq->bufferToProduce(), _iwq->bufferToConsume(), min);
		}
		_iwq->consumeBytes(min);
		_rwq->produceBytes(min);
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
	/// \param[in] thr      The thread which wishes to be woken
	///
	inline void __setWakeup(WQRingReduceThr *thr) {
		void *v = thr->getWakeVec();
		_iwq->setConsumerWakeup(v);
		_rwq->setProducerWakeup(v);
		if (_swq) {
			_swq->setConsumerWakeup(v);
		}
	}

	/// \brief clear a previously set wakeup
	///
	/// \param[in] thr      The thread which no longer wishes to be woken
	///
	inline void __clearWakeup(WQRingReduceThr *thr) {
		_iwq->setConsumerWakeup(NULL);
		_rwq->setProducerWakeup(NULL);
		if (_swq) {
			_swq->setConsumerWakeup(NULL);
		}
	}

	unsigned _nThreads;
	XMI::PipeWorkQueue *_iwq;
	XMI::PipeWorkQueue *_swq;
	XMI::PipeWorkQueue *_rwq;
	size_t _count;
	int _shift;
	coremath _func;
}; //-- WQRingReduceMsg
//
class WQRingReduceMdl : public Reduce::Model<WQRingReduceMdl,WQRingReduceDev,WQRingReduceMsg> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = 1;

	WQRingReduceMdl(DCMF::SysDep *sysdep, XMI_Result &status) :
	Reduce::Model<WQRingReduceMdl,WQRingReduceDev,WQRingReduceMsg>(_g_wqreduce_dev, status)
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

	inline bool generateMessage_impl(XMI_Multicombine_t *mcomb);

private:
	size_t _me;
	XMI::PipeWorkQueue _wq[NUM_CORES /* * NUM_THREADS */];

	static inline void compile_time_assert () {
		COMPILE_TIME_ASSERT(sizeof(XMI_Request_t) >= sizeof(WQRingReduceMsg));
	}
}; // class WQRingReduceMdl

void WQRingReduceMsg::complete() {
	((WQRingReduceDev &)_QS).__complete(this);
	executeCallback();
}

inline DCMF::Queueing::MessageStatus WQRingReduceMsg::advanceThread(DCMF::Queueing::Generic::GenericAdvanceThread *t) {
	return __advanceThread((WQRingReduceThr *)t);
}

inline bool WQRingReduceMdl::generateMessage_impl(XMI_Multicombine_t *mcomb) {
	if (mcomb->req_size < sizeof(WQRingReduceMsg)) {
		return false;
	}
	XMI::Topology *data_topo = (XMI::Topology *)mcomb->data_participants;
	XMI::Topology *results_topo = (XMI::Topology *)mcomb->results_participants;
	// data_participants will be all local nodes...
	// results_participants should be one only.
	// both MUST be local-only topologies. we don't verify.
	// we keep WQs for all local ranks so that we can adapt
	// to whatever those others are.
	//
	// DCMF_assert_debug(mcomb_info->results_participants->size() == 1);
	//
	// Simple "ring" reduce... .e.g:
	//
	//              (A) ---------> (B)
	//               ^              |
	//               |              |
	//               |              v
	//  "head" ===> (D) <--------- (C) <== "root"
	//
	// ("head" is always the next rank on the ring from "root")
	//
	//  copy     combine   combine   combine
	//   (D) ----> (A) ----> (B) ----> (C) ----> output
	//    ^         ^         ^         ^
	//    |         |         |         |
	//  input     input     input     input
	//
	size_t meix = data_topo->rank2Index(_me);
	size_t meix_1 = meix + 1;
	if (meix_1 >= data_topo->size()) {
		meix_1 -= data_topo->size();
	}
	size_t me_1 = data_topo->index2Rank(meix_1);
	WQRingReduceMsg *msg;
	// this had better match roles...
	if (results_topo->isRankMember(_me)) {
		// I am root - downstream from eveyone.
		// _input (op) _wq[meix_1] => _output
		// DCMF_assert(roles == ROOT_ROLE);
		msg = new (mcomb->request) WQRingReduceMsg(_g_wqreduce_dev,
					(XMI::PipeWorkQueue *)mcomb->data, &_wq[meix_1], (XMI::PipeWorkQueue *)mcomb->results,
					mcomb->optor, mcomb->dtype, mcomb->count, mcomb->cb_done);
	} else if (results_topo->isRankMember(me_1)) {
		// I am head of stream.
		// DCMF_assert(roles == NON_ROOT_ROLE);
#ifdef USE_FLAT_BUFFER
		_wq[meix].reset();
#endif /* USE_FLAT_BUFFER */
		msg = new (mcomb->request) WQRingReduceMsg(_g_wqreduce_dev,
					(XMI::PipeWorkQueue *)mcomb->data, NULL, &_wq[meix],
					mcomb->optor, mcomb->dtype, mcomb->count, mcomb->cb_done);
	} else {
		// I am upstream of root, but not head.
		// DCMF_assert(roles == NON_ROOT_ROLE);
#ifdef USE_FLAT_BUFFER
		_wq[meix].reset();
#endif /* USE_FLAT_BUFFER */
		msg = new (mcomb->request) WQRingReduceMsg(_g_wqreduce_dev,
					(XMI::PipeWorkQueue *)mcomb->data, &_wq[meix_1], &_wq[meix],
					mcomb->optor, mcomb->dtype, mcomb->count, mcomb->cb_done);
	}
	_g_wqreduce_dev.__post(msg);
	return true;
}

}; //-- CDI
}; //-- DCMF

#endif /* __generic_wq_reduce_h_ */
