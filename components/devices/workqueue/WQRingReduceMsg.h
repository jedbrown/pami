/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/WQRingReduceMsg.h
 * \brief ???
 */

#ifndef __components_devices_workqueue_wqringreducemsg_h__
#define __components_devices_workqueue_wqringreducemsg_h__

#include "PipeWorkQueue.h"
#include "components/devices/generic/Device.h"
#include "components/devices/generic/SubDevice.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "sys/xmi.h"
#include "components/devices/MulticombineModel.h"
#include "math/math_coremath.h"

namespace XMI {
namespace Device {

class WQRingReduceMdl;
class WQRingReduceMsg;
typedef XMI::Device::Generic::SimpleAdvanceThread WQRingReduceThr;
typedef XMI::Device::Generic::SimpleSubDevice<WQRingReduceThr> WQRingReduceDev;

}; //-- Device
}; //-- XMI

extern XMI::Device::WQRingReduceDev _g_wqreduce_dev;

namespace XMI {
namespace Device {
///
/// \brief A local barrier message that takes advantage of the
/// Load Linked and Store Conditional instructions
///
class WQRingReduceMsg : public XMI::Device::Generic::GenericMessage {
private:
	enum roles {
		NO_ROLE = 0,
		NON_ROOT_ROLE = (1 << 0), // first role must be non-root(s)
		ROOT_ROLE = (1 << 1), // last role must be root
	};

public:
	WQRingReduceMsg(Generic::BaseGenericDevice &Generic_QS,
		XMI_PIPEWORKQUEUE_CLASS *iwq,
		XMI_PIPEWORKQUEUE_CLASS *swq,
		XMI_PIPEWORKQUEUE_CLASS *rwq,
		xmi_op op,
		xmi_dt dt,
		size_t count,
		xmi_callback_t cb) :
	XMI::Device::Generic::GenericMessage(Generic_QS, cb),
	_iwq(iwq),
	_swq(swq), // might be NULL
	_rwq(rwq),
	_count(count),
	_shift(xmi_dt_shift[dt])
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

	inline XMI::Device::MessageStatus advanceThread(XMI::Device::Generic::GenericAdvanceThread *t);

protected:
	//friend class WQRingReduceDev;
	friend class XMI::Device::Generic::SimpleSubDevice<WQRingReduceThr>;

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

	inline XMI::Device::MessageStatus __advanceThread(WQRingReduceThr *thr) {
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
				return XMI::Device::Active;
			}
			void *buf[2] = { _iwq->bufferToConsume(), _swq->bufferToConsume() };
			_func(_rwq->bufferToProduce(), buf, 2, min >> _shift);
			_swq->consumeBytes(min);
		} else {
			if (min == 0) {
				return XMI::Device::Active;
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
	XMI_PIPEWORKQUEUE_CLASS *_iwq;
	XMI_PIPEWORKQUEUE_CLASS *_swq;
	XMI_PIPEWORKQUEUE_CLASS *_rwq;
	size_t _count;
	int _shift;
	coremath _func;
}; //-- WQRingReduceMsg
//
class WQRingReduceMdl : public XMI::Device::Interface::MulticombineModel<WQRingReduceMdl> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = 1;
	static const size_t sizeof_msg = sizeof(WQRingReduceMsg);

	WQRingReduceMdl(xmi_result_t &status) :
	XMI::Device::Interface::MulticombineModel<WQRingReduceMdl>(status)
	{
		XMI_SYSDEP_CLASS *sd = _g_wqreduce_dev.getSysdep();
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

	inline void reset_impl() {}

	inline bool postMulticombine_impl(xmi_multicombine_t *mcomb);

private:
	size_t _me;
	XMI_PIPEWORKQUEUE_CLASS _wq[NUM_CORES /* * NUM_THREADS */];
}; // class WQRingReduceMdl

void WQRingReduceMsg::complete() {
	((WQRingReduceDev &)_QS).__complete<WQRingReduceMsg>(this);
	executeCallback();
}

inline XMI::Device::MessageStatus WQRingReduceMsg::advanceThread(XMI::Device::Generic::GenericAdvanceThread *t) {
	return __advanceThread((WQRingReduceThr *)t);
}

inline bool WQRingReduceMdl::postMulticombine_impl(xmi_multicombine_t *mcomb) {
	XMI_TOPOLOGY_CLASS *data_topo = (XMI_TOPOLOGY_CLASS *)mcomb->data_participants;
	XMI_TOPOLOGY_CLASS *results_topo = (XMI_TOPOLOGY_CLASS *)mcomb->results_participants;
	// data_participants will be all local nodes...
	// results_participants should be one only.
	// both MUST be local-only topologies. we don't verify.
	// we keep WQs for all local ranks so that we can adapt
	// to whatever those others are.
	//
	// XMI_assert_debug(mcomb_info->results_participants->size() == 1);
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
		// XMI_assert(roles == ROOT_ROLE);
		msg = new (mcomb->request) WQRingReduceMsg(_g_wqreduce_dev,
					(XMI_PIPEWORKQUEUE_CLASS *)mcomb->data, &_wq[meix_1], (XMI_PIPEWORKQUEUE_CLASS *)mcomb->results,
					mcomb->optor, mcomb->dtype, mcomb->count, mcomb->cb_done);
	} else if (results_topo->isRankMember(me_1)) {
		// I am head of stream.
		// XMI_assert(roles == NON_ROOT_ROLE);
#ifdef USE_FLAT_BUFFER
		_wq[meix].reset();
#endif /* USE_FLAT_BUFFER */
		msg = new (mcomb->request) WQRingReduceMsg(_g_wqreduce_dev,
					(XMI_PIPEWORKQUEUE_CLASS *)mcomb->data, NULL, &_wq[meix],
					mcomb->optor, mcomb->dtype, mcomb->count, mcomb->cb_done);
	} else {
		// I am upstream of root, but not head.
		// XMI_assert(roles == NON_ROOT_ROLE);
#ifdef USE_FLAT_BUFFER
		_wq[meix].reset();
#endif /* USE_FLAT_BUFFER */
		msg = new (mcomb->request) WQRingReduceMsg(_g_wqreduce_dev,
					(XMI_PIPEWORKQUEUE_CLASS *)mcomb->data, &_wq[meix_1], &_wq[meix],
					mcomb->optor, mcomb->dtype, mcomb->count, mcomb->cb_done);
	}
	_g_wqreduce_dev.__post<WQRingReduceMsg>(msg);
	return true;
}

}; //-- Device
}; //-- XMI

#endif // __components_devices_workqueue_wqringreducemsg_h__
