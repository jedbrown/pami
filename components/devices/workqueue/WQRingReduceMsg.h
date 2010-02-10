/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/WQRingReduceMsg.h
 * \brief ???
 */

#ifndef __components_devices_workqueue_WQRingReduceMsg_h__
#define __components_devices_workqueue_WQRingReduceMsg_h__

#include "PipeWorkQueue.h"
#include "components/devices/util/SubDeviceSuppt.h"
#include "components/devices/generic/AdvanceThread.h"
#include "sys/xmi.h"
#include "components/devices/MulticombineModel.h"
#include "math/math_coremath.h"
#include "Global.h"

namespace XMI {
namespace Device {

class WQRingReduceMdl;
class WQRingReduceMsg;
typedef XMI::Device::Generic::SimpleAdvanceThread WQRingReduceThr;
typedef XMI::Device::Generic::MultiSendQSubDevice<WQRingReduceThr,1,1,true> WQRingReduceRealDev;

}; //-- Device
}; //-- XMI

extern XMI::Device::WQRingReduceRealDev _g_wqreduce_dev;

namespace XMI {
namespace Device {

class WQRingReduceDev : public XMI::Device::Generic::SimplePseudoDevice<WQRingReduceDev,WQRingReduceRealDev> {
public:
	static inline WQRingReduceDev *create(size_t client, size_t num_ctx, XMI::Device::Generic::Device *devices) {
		return __create(client, num_ctx, devices, &_g_wqreduce_dev);
	}

	inline WQRingReduceDev(size_t client, size_t num_ctx, XMI::Device::Generic::Device *devices, size_t ctx) :
	XMI::Device::Generic::SimplePseudoDevice<WQRingReduceDev,WQRingReduceRealDev>(client, num_ctx, devices, ctx)
	{
	}

	inline void init(SysDep *sd, size_t client, size_t num_ctx, xmi_context_t context, size_t contextid) {
		__init(sd, client, num_ctx, context, contextid, &_g_wqreduce_dev);
	}

	inline size_t advance_impl() {
		return _g_wqreduce_dev.advance(_clientid, _contextid);
	}
}; // class WQRingReduceDev

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
	WQRingReduceMsg(Generic::GenericSubDevice *Generic_QS,
		xmi_multicombine_t *mcomb,
		XMI::PipeWorkQueue *iwq,
		XMI::PipeWorkQueue *swq,
		XMI::PipeWorkQueue *rwq) :
	XMI::Device::Generic::GenericMessage(Generic_QS, mcomb->cb_done,
				mcomb->client, mcomb->context),
	_iwq(iwq),
	_swq(swq), // might be NULL
	_rwq(rwq),
	_count(mcomb->count),
	_shift(xmi_dt_shift[mcomb->dtype])
	{
		if (_swq) {
			// full combine...
			_func = MATH_OP_FUNCS(mcomb->dtype, mcomb->optor, 2);
		} else {
			// copy only
			_func = NULL;
		}
	}

	// virtual function
	xmi_context_t postNext(bool devPosted) {
		return _g_wqreduce_dev.__postNext<WQRingReduceMsg>(this, devPosted);
	}

	inline int setThreads(WQRingReduceThr **th) {
		WQRingReduceThr *t;
		int n;
		_g_wqreduce_dev.__getThreads(&t, &n);
		int nt = 0;
		// assert(nt < n);
		_nThreads = 1; // must predict total number of threads
		t[nt].setMsg(this);
		t[nt].setAdv(advanceThread);
		t[nt].setStatus(XMI::Device::Ready);
		t[nt]._bytesLeft = _count << _shift;
#ifdef USE_WAKEUP_VECTORS
		// not here - but somewhere/somehow...
		__setWakeup(thr);
#endif // USE_WAKEUP_VECTORS
		__advanceThread(&t[nt]);
		++nt;
		*th = t;
		// assert(nt > 0? && nt < n);
		return nt;
	}

protected:
	DECL_ADVANCE_ROUTINE(advanceThread,WQRingReduceMsg,WQRingReduceThr);
	inline xmi_result_t __advanceThread(WQRingReduceThr *thr) {
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
				return XMI_EAGAIN;
			}
			void *buf[2] = { _iwq->bufferToConsume(), _swq->bufferToConsume() };
			_func(_rwq->bufferToProduce(), buf, 2, min >> _shift);
			_swq->consumeBytes(min);
		} else {
			if (min == 0) {
				return XMI_EAGAIN;
			}
			memcpy(_rwq->bufferToProduce(), _iwq->bufferToConsume(), min);
		}
		_iwq->consumeBytes(min);
		_rwq->produceBytes(min);
		thr->_bytesLeft -= min;
		if (thr->_bytesLeft == 0) {
			// thread is Done, maybe not message
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
	/// \param[in] thr      The thread which wishes to be woken
	///
	inline void __setWakeup(WQRingReduceThr *thr) {
#ifdef USE_WAKEUP_VECTORS
		void *v = thr->getWakeVec();
		_iwq->setConsumerWakeup(v);
		_rwq->setProducerWakeup(v);
		if (_swq) {
			_swq->setConsumerWakeup(v);
		}
#endif // USE_WAKEUP_VECTORS
	}

	/// \brief clear a previously set wakeup
	///
	/// \param[in] thr      The thread which no longer wishes to be woken
	///
	inline void __clearWakeup(WQRingReduceThr *thr) {
#ifdef USE_WAKEUP_VECTORS
		_iwq->setConsumerWakeup(NULL);
		_rwq->setProducerWakeup(NULL);
		if (_swq) {
			_swq->setConsumerWakeup(NULL);
		}
#endif // USE_WAKEUP_VECTORS
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
class WQRingReduceMdl : public XMI::Device::Interface::MulticombineModel<WQRingReduceMdl,sizeof(WQRingReduceMsg)> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = 1;
	static const size_t sizeof_msg = sizeof(WQRingReduceMsg);

	WQRingReduceMdl(xmi_result_t &status) :
	XMI::Device::Interface::MulticombineModel<WQRingReduceMdl,sizeof(WQRingReduceMsg)>(status)
	{
		XMI::SysDep *sd = _g_wqreduce_dev.getSysdep();
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

	inline void reset_impl() {}

	inline xmi_result_t postMulticombine_impl(uint8_t (&state)[sizeof_msg], xmi_multicombine_t *mcomb);

private:
	size_t _me;
	XMI::PipeWorkQueue _wq[XMI_MAX_PROC_PER_NODE];
}; // class WQRingReduceMdl

inline xmi_result_t WQRingReduceMdl::postMulticombine_impl(uint8_t (&state)[sizeof_msg], xmi_multicombine_t *mcomb) {
	XMI::Topology *data_topo = (XMI::Topology *)mcomb->data_participants;
	XMI::Topology *results_topo = (XMI::Topology *)mcomb->results_participants;
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
		msg = new (&state) WQRingReduceMsg(_g_wqreduce_dev.getQS(), mcomb,
					(XMI::PipeWorkQueue *)mcomb->data, &_wq[meix_1], (XMI::PipeWorkQueue *)mcomb->results);
	} else if (results_topo->isRankMember(me_1)) {
		// I am head of stream.
		// XMI_assert(roles == NON_ROOT_ROLE);
#ifdef USE_FLAT_BUFFER
		_wq[meix].reset();
#endif /* USE_FLAT_BUFFER */
		msg = new (&state) WQRingReduceMsg(_g_wqreduce_dev.getQS(), mcomb,
					(XMI::PipeWorkQueue *)mcomb->data, NULL, &_wq[meix]);
	} else {
		// I am upstream of root, but not head.
		// XMI_assert(roles == NON_ROOT_ROLE);
#ifdef USE_FLAT_BUFFER
		_wq[meix].reset();
#endif /* USE_FLAT_BUFFER */
		msg = new (&state) WQRingReduceMsg(_g_wqreduce_dev.getQS(), mcomb,
					(XMI::PipeWorkQueue *)mcomb->data, &_wq[meix_1], &_wq[meix]);
	}
	_g_wqreduce_dev.__post<WQRingReduceMsg>(msg);
	return XMI_SUCCESS;
}

}; //-- Device
}; //-- XMI

#endif // __components_devices_workqueue_wqringreducemsg_h__
