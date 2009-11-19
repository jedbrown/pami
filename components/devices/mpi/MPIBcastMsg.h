/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/MPIBcastMsg.h
 * \brief ???
 */

#ifndef __components_devices_workqueue_MPIBcastMsg_h__
#define __components_devices_workqueue_MPIBcastMsg_h__

#include "components/devices/generic/Device.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/generic/SubDevice.h"
#include "sys/xmi.h"
#include "Global.h"
#include "PipeWorkQueue.h"
#include "Topology.h"
#include "components/devices/MulticastModel.h"

namespace XMI {
namespace Device {

class MPIBcastMdl;
class MPIBcastMsg;
typedef XMI::Device::Generic::SimpleAdvanceThread MPIBcastThr;
typedef XMI::Device::Generic::SimpleSubDevice<MPIBcastThr> MPIBcastDev;

}; //-- Device
}; //-- XMI

extern XMI::Device::MPIBcastDev _g_mpibcast_dev;

namespace XMI {
namespace Device {

///
/// \brief A local bcast message that takes advantage of the
/// shared-memory workqueues
///
class MPIBcastMsg : public XMI::Device::Generic::GenericMessage {
private:
	enum roles {
		NO_ROLE = 0,
		ROOT_ROLE = (1 << 0), // first role must be root
		NON_ROOT_ROLE = (1 << 1), // last role must be non-root(s)
	};
public:
	MPIBcastMsg(Generic::BaseGenericDevice &Generic_QS,
		xmi_multicast_t *mcast) :
	XMI::Device::Generic::GenericMessage(Generic_QS, mcast->cb_done,
					mcast->client, mcast->context),
	_dst((XMI::Topology *)mcast->dst_participants),
	_iwq((XMI::PipeWorkQueue *)mcast->src),
	_rwq((XMI::PipeWorkQueue *)mcast->dst),
	_bytes(mcast->bytes),
	_tag(mcast->connection_id),
	_idx(0),
	_currBytes(0),
	_currBuf(NULL),
	_req(MPI_REQUEST_NULL)
	{
		XMI::Topology *src_topo = (XMI::Topology *)mcast->src_participants;
		size_t root = src_topo->index2Rank(0); // assert size(0 == 1...
		bool iamroot = (root == __global.mapping.task());
		if (iamroot) {
			_rwq = NULL;
		} else {
			_iwq = NULL;
		}
		XMI_assertf(_rwq || _iwq, "MPIBcastMsg has neither input or output data\n");
	}

	// This is a virtual function, but declaring inline here avoids linker
	// complaints about multiple definitions.
	inline void complete();

	inline XMI::Device::MessageStatus advanceThread(XMI::Device::Generic::GenericAdvanceThread *t);

protected:
	//friend class MPIBcastDev; // Until C++ catches up with real programming languages:
	friend class XMI::Device::Generic::SimpleSubDevice<MPIBcastThr>;

	inline int __setThreads(MPIBcastThr *t, int n) {
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

	inline XMI::Device::MessageStatus __advanceThread(MPIBcastThr *thr) {
		if (getStatus() == XMI::Device::Done) {
			fprintf(stderr, "Warning: message/thread advanced after Done\n");
			return XMI::Device::Done;
		}
		int flag = 0;
		MPI_Status status;
		if (_req != MPI_REQUEST_NULL) {
			MPI_Test(&_req, &flag, &status);
			if (flag) {
				_req = MPI_REQUEST_NULL; // redundant?
			} else {
				return XMI::Device::Active;
			}
			// current message was completed...
		}
		if (_iwq) { // root - keep send until all ranks have all bytes...
			if (_idx >= _dst->size()) {
				thr->_bytesLeft -= _currBytes;
				_iwq->consumeBytes(_currBytes);
				if (thr->_bytesLeft == 0) {
					thr->setDone(true);
					setStatus(XMI::Device::Done);
					return XMI::Device::Done;
				}
				_currBytes = 0;
			}
			if (_currBytes == 0) {
				_currBytes = _iwq->bytesAvailableToConsume();
				_currBuf = _iwq->bufferToConsume();
				_idx = 0;
			}
			if (_currBytes == 0) {
				return XMI::Device::Active;
			}
			MPI_Isend(_currBuf, _currBytes, MPI_BYTE,
					_dst->index2Rank(_idx), _tag,
					MPI_COMM_WORLD, &_req);
			// error checking?
			++_idx;
		} else { // receive data until done...
			XMI_assert(_rwq);
			if (flag) {
				if ((size_t)status.count < _currBytes) {
					// make sure we only count what was recv'ed
					_currBytes = status.count;
				} else {
					// how does MPI_Status.count work?
					XMI_assertf((size_t)status.count <= _currBytes,
						"MPIBcastMsg recv overrun (got %d, kept %d)\n",
						status.count, _currBytes);
				}
			}

			// if we are here, we completed a chunk of recv data...
			thr->_bytesLeft -= _currBytes;
			_rwq->produceBytes(_currBytes);
			if (thr->_bytesLeft == 0) {
				thr->setDone(true);
				setStatus(XMI::Device::Done);
				return XMI::Device::Done;
			}
			_currBytes = _rwq->bytesAvailableToProduce();
			_currBuf = _rwq->bufferToProduce();
			if (_currBytes == 0) {
				return XMI::Device::Active;
			}
			MPI_Irecv(_currBuf, _currBytes, MPI_BYTE,
					_dst->index2Rank(0), _tag,
					MPI_COMM_WORLD, &_req);
			// error checking?
		}

		return XMI::Device::Active;
	}

	unsigned _nThreads;
	XMI::Topology *_dst;
	XMI::PipeWorkQueue *_iwq;
	XMI::PipeWorkQueue *_rwq;
	size_t _bytes;
	int _tag;
	size_t _idx;
	size_t _currBytes;
	char *_currBuf;
	MPI_Request _req;
}; //-- MPIBcastMsg

class MPIBcastMdl : public XMI::Device::Interface::MulticastModel<MPIBcastMdl> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = 1;
	static const size_t sizeof_msg = sizeof(MPIBcastMsg);

	MPIBcastMdl(xmi_result_t &status) :
	XMI::Device::Interface::MulticastModel<MPIBcastMdl>(status)
	{
		//XMI::SysDep *sd = _g_mpibcast_dev.getSysdep();
	}

	inline bool postMulticast_impl(xmi_multicast_t *mcast);

private:
}; // class MPIBcastMdl

void MPIBcastMsg::complete() {
	((MPIBcastDev &)_QS).__complete<MPIBcastMsg>(this);
	executeCallback();
}

inline XMI::Device::MessageStatus MPIBcastMsg::advanceThread(XMI::Device::Generic::GenericAdvanceThread *t) {
	return __advanceThread((MPIBcastThr *)t);
}

inline bool MPIBcastMdl::postMulticast_impl(xmi_multicast_t *mcast) {
	MPIBcastMsg *msg = new (mcast->request) MPIBcastMsg(_g_mpibcast_dev, mcast);
	_g_mpibcast_dev.__post<MPIBcastMsg>(msg);
	return true;
}

}; //-- Device
}; //-- XMI

#endif // __components_devices_workqueue_MPIBcastMsg_h__
