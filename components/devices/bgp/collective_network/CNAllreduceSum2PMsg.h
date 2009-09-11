/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file components/devices/bgp/collective_network/CNAllreduceSum2PMsg.h
 * \brief Default collective network allreduce interface
 * for DOUBLE-SUM 2-Pass algorithm
 */
#ifndef __components_devices_bgp_cnallreduce2psummsg_h__
#define __components_devices_bgp_cnallreduce2psummsg_h__

#include "Util.h"
#include "workqueue/SharedWorkQueue.h"
#include "components/devices/bgp/collective_network/CNDevice.h" // externs for env vars
#include "components/devices/bgp/collective_network/CNAllreduce.h"
#include "components/devices/bgp/collective_network/CollectiveNetworkLib.h"
#include "components/devices/bgp/collective_network/Packet.h"
#include "components/devices/generic/Device.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "math_coremath.h"
#include "components/devices/bgp/collective_network/DblUtils.h"

/**
 * \page twopass_dblsum The 2-Pass DOUBLE-SUM algorithm
 *
 * The 2-Pass DOUBLE-SUM algorithm is as follows. Each double
 * is split into a 16-bit exponent and a 96-bit mantissa. Exponents
 * must be processed before the corresponding mantissa can be sent.
 * Exponents are processed by the tree using a MAX operator. This
 * results in the largest exponent of a set of doubles being received.
 * This MAX exponent must be fed-back to the sending side and used
 * to adjust the mantissa so that all (integer) mantissas are aligned
 * decimally. Then the mantissa is processed by the tree using an ADD
 * operator. The received mantissa must be combined with the MAX exponent
 * to re-form a double which is the result.
 *
 * For efficiency sake, exponents and mantissas are buffered into
 * tree packets. 126 exponents are placed in a packet. This will
 * correspond to 6 packets of 21 mantissas each. Exponent packets are
 * sent up to some threshold intended to approximate the time at which
 * the first exponent (result) packet will be received. At this point,
 * the received exponents may be used to start sending the corresponding
 * mantissas. Note that all nodes must have a common understanding of
 * when to switch between exponent processing and mantissa processing,
 * otherwise the tree hardware will be proccessing disimilar packet
 * types resulting in garbage in the received packets.
 *
 * Because both the mantissa send and mantissa receive must use the
 * received exponents, there must be coordination between the sender
 * and receiver and sharing of this data. In the basic message type,
 * this is done with a static buffer. This will only work in SMP mode,
 * or at least a scheme where the sender and receiver are in the same
 * process image (have access to the same memory). An alternative
 * scheme uses the WorkQueue object, possibly backed by shared memory,
 * to create a single-producer dual-consumer FIFO for the exponent
 * packets. Receiving an exponent packets results in the packet being
 * produced into the WorkQueue FIFO. At this point, both the mantissa
 * sender and mantissa receiver have access to the data. The data will
 * not be overwritten until both have consumed it. Note, it is still
 * necessary for the exponent and mantissa senders to follow a strict
 * pattern, as the tree cannot process exponent and mantissa packets
 * simultaniously. It is expected that the static buffer scheme will
 * be obsoleted.
 *
 * This algorithm can be adapted to other operators.
 */

/** \brief The number of exponent packets to send before switching
 * to mantissa packets
 *
 * Ideally, this number would be computed based on tree depth
 * (partition size). If the exponent packets have not started
 * to arrive in the receive FIFO by the time we switch to mantissas,
 * we will wait for them to arrive and waste time. So the larger
 * the tree the more exponent packets we should send before switching.
 * This timing also affects the point at which 1-Pass becomes slower
 * than 2-pass.
 *
 * This value is used as the default for the _expcycle variable,
 * which is the actual limit/switch point. The current basic 2-pass
 * message class that uses a static buffer as the feedback mechanism
 * actually sizes the feedback buffer with this constant and in this
 * case _expcycle must be <= EXPCOUNT. In other message types _expcycle
 * may be set to any value.
 */

namespace XMI {
namespace Device {
namespace BGP {

class CNAllreduce2PModel;
class CNAllreduce2PMessage;
typedef XMI::Device::BGP::BaseGenericCNThread CNAllreduce2PThread;
typedef XMI::Device::Generic::SharedQueueSubDevice<CNAllreduce2PModel,CNDevice,CNAllreduce2PMessage,CNAllreduce2PThread,2> CNAllreduce2PDevice;

};	// BGP
};	// Device
};	// XMI

extern XMI::Device::BGP::CNAllreduce2PDevice _g_cnallreduce2p_dev;

namespace XMI {
namespace Device {
namespace BGP {

/**
 * \brief collective Network Allreduce DOUBLE-SUM 2-Pass Send
 *
 * A static buffer is used for exponent feedback.
 *
 */

class CNAllreduce2PMessage : public XMI::Device::BGP::BaseGenericCN2PMessage {
	enum roles {
		NO_ROLE = 0,
		INJECTION_ROLE = (1 << 0), // first role must be "injector"
		RECEPTION_ROLE = (1 << 1), // last role must be "receptor"
	};
public:
	CNAllreduce2PMessage(BaseDevice &qs,
		XMI::Device::WorkQueue::WorkQueue &ewq,
		XMI::Device::WorkQueue::WorkQueue &mwq,
		XMI::Device::WorkQueue::WorkQueue &xwq,
		XMI::PipeWorkQueue *swq,
		XMI::PipeWorkQueue *rwq,
		size_t bytes,
		bool doStore,
		unsigned roles,
		const XMI_Callback_t cb,
		unsigned dispatch_id_e,
		unsigned dispatch_id_m) :
	BaseGenericCN2PMessage(qs, ewq, mwq, xwq, swq, rwq, bytes, doStore,
				roles, cb, dispatch_id_e, dispatch_id_m),
	_roles(roles),
	_offx(0)
	{
	}

	inline XMI::Device::MessageStatus advanceThread(XMI::Device::Generic::GenericAdvanceThread *t);
	inline void complete();

protected:
	//friend class CNAllreduce2PDevice;
	friend class XMI::Device::Generic::SharedQueueSubDevice<CNAllreduce2PModel,CNDevice,CNAllreduce2PMessage,CNAllreduce2PThread,2>;

	inline int __setThreads(CNAllreduceThread *t, int n) {
		int nt = 0;
		int maxnt = ((CNAllreduce2PDevice &)_QS).common()->getMaxThreads();
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
		if (_bytes >= 16384 /*DCMF_TREE_HELPER_THRESH*/) {
			setThreadsWanted(MIN(nt, maxnt));
		}
		return nt;
	}

	inline XMI::Device::MessageStatus __advanceInj(CNAllreduce2PThread *thr) {
		XMI::Device::MessageStatus rc = XMI::Device::Active;
		unsigned hcount = BGPCN_FIFO_SIZE, dcount = BGPCN_QUADS_PER_FIFO;
		size_t avail = thr->_wq->bytesAvailableToConsume();
		char *buf = thr->_wq->bufferToConsume();
		unsigned expRemain = _expcount - _expsent;
		unsigned manRemain = _expsent - _mansent;
		size_t didx = 0;
		size_t did = 0;
		if (__wait_send_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return rc;
		}
		char *bufx = buf + _offx;
		__send_expo_packets(thr, hcount, dcount, expRemain, avail, didx, bufx);
		_offx += didx;
		// mantissas dont use original data buffer, partial mantissas in _mwq.
		__send_mant_packets(thr, hcount, dcount, manRemain, did);
		if (did) {
			thr->_bytesLeft -= did;
			thr->_wq->consumeBytes(did);
			_offx -= did;
		}
		if (thr->_bytesLeft == 0) {
			rc = XMI::Device::Done;
			thr->setDone(true);
		}
		return rc;
	}

	inline XMI::Device::MessageStatus __advanceRcp(CNAllreduceThread *thr) {
		XMI::Device::MessageStatus rc = XMI::Device::Active;
		register unsigned hcount = 0, dcount = 0;
		if (__wait_recv_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return rc;
		}
		_BGP_TreeHwHdr hdr;
		size_t avail = thr->_wq->bytesAvailableToProduce();
		char *buf = thr->_wq->bufferToProduce();
		size_t total = 0;
		size_t left = thr->_bytesLeft;
		size_t dstBytes = (left < MANT_PER_PKT * sizeof(double) ?
				left : MANT_PER_PKT * sizeof(double));
		// we have to check avail before reading header, otherwise we
		// might get committed to reading payload without any place to
		// store it. This means we must also have space available in order
		// to process an exponent packet - even though it doesnt need it.
		while (left > 0 && avail >= dstBytes &&
				hcount > 0 && dcount > 0) {
			CollectiveRawReceiveHeader(VIRTUAL_CHANNEL, &hdr);
			--hcount;
			if (hdr.CtvHdr.Tag == _expo_disp_id) {
				__recv_expo_packet(thr);
				// recv expo can never complete message
			} else if (hdr.CtvHdr.Tag == _mant_disp_id) {
				size_t did = 0;
				__recv_mant_packet(thr, avail, did, buf + total, dstBytes);
				if (did) {
					left -= did;
					total += did;
					avail -= did;
				}
			} else {
fprintf(stderr, "bad packet header 0x%08x\n", hdr.CtvHdr.word);
CollectiveRawReceivePacketNoHdrNoStore(VIRTUAL_CHANNEL);
//				XMI_abort();
			}
			dcount -= BGPCN_QUADS_PER_PKT;
		}
		if (total > 0) {
			thr->_bytesLeft -= total;
			thr->_wq->produceBytes(total);
			if (thr->_bytesLeft == 0) {
				rc = XMI::Device::Done;
				thr->setDone(true);
			}
		}
		return rc;
	}

	inline void __completeThread(CNAllreduce2PThread *thr);

	inline XMI::Device::MessageStatus __advanceThread(CNAllreduce2PThread *thr) {
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
private:
	unsigned _roles;
	unsigned _offx;
	unsigned _nThreads;
}; // class CNAllreduce2PMessage

class CNAllreduce2PModel : public Impl::MulticombineModelImpl {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = -1;

	CNAllreduce2PModel(XMI::SysDep *sysdep, XMI_Result &status) :
	Impl::MulticombineModelImpl(status),
	// we depend on doing consumeBytes(bytesAvailableToConsume()) in order
	// to "jump" to next "boundary" so we maintain alignment for each cycle.
	// this requires the WQ behavior based on workunits and worksize that
	// creates artificial "boundaries" at those points.
	_ewq(sysdep, EXPO_WQ_SIZE, BGPCN_PKT_SIZE),
	_mwq(sysdep, EXPO_WQ_SIZE, MANT_WQ_FACT * BGPCN_PKT_SIZE),
	_xwq(sysdep, EXPO_WQ_SIZE, BGPCN_PKT_SIZE),
	_dispatch_id_e(_g_cnallreduce2p_dev.newDispID()),
	_dispatch_id_m(_g_cnallreduce2p_dev.newDispID())
	{
		_me = sysdep->mapping().rank();
		_ewq.setConsumers(1, 0);
		_ewq.setProducers(1, 0);
		_mwq.setConsumers(1, 0);
		_mwq.setProducers(1, 0);
		_xwq.setConsumers(2, 0);
		_xwq.setProducers(1, 0);
		reset_impl();
	}

	inline void reset_impl() {
		_ewq.reset();
		_mwq.reset();
		_xwq.reset();
	}

	inline bool postMulticombine_impl(CNAllreduce2PMessage *msg);

private:
	XMI::Device::WorkQueue::SharedWorkQueue _ewq;
	XMI::Device::WorkQueue::SharedWorkQueue _mwq;
	XMI::Device::WorkQueue::SharedWorkQueue _xwq;
	unsigned _dispatch_id_e;
	unsigned _dispatch_id_m;
	size_t _me;

	static inline void compile_time_assert () {
		COMPILE_TIME_ASSERT(sizeof(XMI_Request_t) >= sizeof(CNAllreduce2PMessage));
		COMPILE_TIME_ASSERT(EXPO_WQ_SIZE >= EXPCOUNT);
		COMPILE_TIME_ASSERT((EXPO_WQ_SIZE & (EXPO_WQ_SIZE - 1)) == 0);
		COMPILE_TIME_ASSERT(MANT_WQ_FACT * EXPO_WQ_SIZE >= EXPO_MANT_FACTOR * EXPCOUNT);
		COMPILE_TIME_ASSERT((MANT_WQ_FACT & (MANT_WQ_FACT - 1)) == 0);
	}
}; // class CNAllreduce2PModel

inline void CNAllreduce2PMessage::__completeThread(CNAllreduce2PThread *thr) {
	unsigned c = ((CNAllreduce2PDevice &)_QS).__completeThread(thr);
	if (c >= _nThreads) {
		setStatus(XMI::Device::Done);
	}
}

void CNAllreduce2PMessage::complete() {
	((CNAllreduce2PDevice &)_QS).__complete(this);
	executeCallback();
}

XMI::Device::MessageStatus CNAllreduce2PMessage::advanceThread(XMI::Device::Generic::GenericAdvanceThread *t) {
	return __advanceThread((CNAllreduce2PThread *)t);
}

inline bool CNAllreduce2PModel::postMulticombine_impl(CNAllreduce2PMessage *msg) {
	// we don't need CNAllreduceSetup since we know this is DOUBLE-SUM
	XMI::Topology *results_topo = (XMI::Topology *)_getResultsRanks();
	bool doStore = (!results_topo || results_topo->isRankMember(_me));
	size_t bytes = _getcount() << xmi_dt_shift[_getDt()];
	// could try to complete allreduce before construction, but for now the code
	// is too dependent on having message and thread structures to get/keep context.
	// __post() will still try early advance... (after construction)
	new (msg) CNAllreduce2PMessage(_g_cnallreduce2p_dev,
			_ewq, _mwq, _xwq,
			(XMI::PipeWorkQueue *)_getData(),
			(XMI::PipeWorkQueue *)_getResults(),
			bytes, doStore, _getRoles(), _getCallback(),
			_dispatch_id_e, _dispatch_id_m);
	_g_cnallreduce2p_dev.__post(msg);
	return true;
}

};	// BGP
};	// Device
};	// XMI

#endif // __components_devices_bgp_cnallreduce2psummsg_h__
