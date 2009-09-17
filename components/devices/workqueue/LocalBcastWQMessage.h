/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/LocalBcastWQMessage.h
 * \brief ???
 */

#ifndef __dcmf_workqueue_localbcastwqmsg_h__
#define __dcmf_workqueue_localbcastwqmsg_h__

#include "workqueue/SharedWorkQueue.h"
#include "workqueue/MemoryWorkQueue.h"
#include "math_coremath.h"
#include "SysDep.h"
#include "generic/Device.h"
#include "generic/SubDevice.h"
#include "generic/Message.h"
#include "generic/AdvanceThread.h"
#include "prod/cdi/BroadcastModel.h"

extern XMI::Topology *_g_topology_local;

namespace DCMF {
namespace CDI {

class LocalBcastWQModel;
class LocalBcastWQMessage;
typedef DCMF::Queueing::Generic::GenericAdvanceThread LocalBcastWQThread;
typedef DCMF::Queueing::Generic::SimpleSubDevice<LocalBcastWQModel,LocalBcastWQMessage,LocalBcastWQThread> LocalBcastWQDevice;

}; // namespace CDI
}; // namespace DCMF

extern DCMF::CDI::LocalBcastWQDevice _g_l_bcastwq_dev;

namespace DCMF {
namespace CDI {

class LocalBcastWQMessage : public DCMF::Queueing::Generic::GenericMessage {
public:

          ///
          /// \brief Constructor to broadcast/produce a source buffer on the
          ///        root rank to a queue in a shared memory area.
          ///
          /// \param[in] device       Shared Memory device
          /// \param[in] cb           Callback to invoke when the broadcast is complete
          /// \param[in] sharedmemory Location of the shared memory
          /// \param[in] buffer       Location of the buffer to broadcast
          /// \param[in] nbytes       Number of bytes to broadcast
          /// \param[in] consumers    Number of consumers that will recieve the
          ///                         broadcast buffer
          ///
          inline LocalBcastWQMessage(BaseDevice &device,
                                      XMI_Callback_t   cb,
                                      DCMF::Device::WorkQueue::SharedWorkQueue & workqueue,
                                      bool              isrootrole,
                                      XMI::PipeWorkQueue   * sbuffer,
                                      XMI::PipeWorkQueue   * rbuffer,
                                      size_t            nbytes) :
            DCMF::Queueing::Generic::GenericMessage (device, cb),
            _isrootrole (isrootrole),
            _sbuffer (*sbuffer),
            _rbuffer (*rbuffer),
            _shared (workqueue)
          {
          }

	inline DCMF::Queueing::MessageStatus advanceThread(DCMF::Queueing::Generic::GenericAdvanceThread *t);

	inline void complete();

private:
	//friend class LocalBcastWQDevice;
	friend class DCMF::Queueing::Generic::SimpleSubDevice<LocalBcastWQModel,LocalBcastWQMessage,LocalBcastWQThread>;

	inline DCMF::Queueing::MessageStatus __advanceThread(LocalBcastWQThread *thr) {
		// This works around a bug with "g++ -fPIC -O3"...
		coremath1 func = (coremath1) DCMF::Device::WorkQueue::SharedWorkQueue::shmemcpy;
		if (_isrootrole) {
			// "broadcast" the source buffer into the shared queue.
			_shared.Q2Q(_sbuffer, func, 0);

			// If all bytes have been copied to the shared queue then the root is done.
			if (_sbuffer.bytesAvailableToConsume() == 0) setStatus(DCMF::Queueing::Done);
		} else {
			// read bytes from the shared queue into the local result buffer.
			_shared.Q2Qr(_rbuffer, func, 0);

			// If all bytes have been copied from the shared queue then the recv is done.
			if (_rbuffer.bytesAvailableToProduce() == 0) setStatus(DCMF::Queueing::Done);
		}

		return getStatus();
	}

	inline int __setThreads(LocalBcastWQThread *t, int n) {
		t[0].setMsg(this);
		t[0].setDone(false);
		return 1;
	}

private:
          bool              _isrootrole;
          XMI::PipeWorkQueue   &_sbuffer;
          XMI::PipeWorkQueue   &_rbuffer;
          DCMF::Device::WorkQueue::SharedWorkQueue & _shared;
}; // class LocalBcastWQMessage

class LocalBcastWQModel : public Broadcast::Model<LocalBcastWQModel,LocalBcastWQDevice,LocalBcastWQMessage> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = 1;

	LocalBcastWQModel(DCMF::SysDep *sysdep, XMI_Result &status) :
	Broadcast::Model<LocalBcastWQModel,LocalBcastWQDevice,LocalBcastWQMessage>(_g_l_bcastwq_dev, status),
	_shared(sysdep),
	_peer(_g_topology_local->rank2Index(sysdep->mapping().rank())),
	_npeers(_g_topology_local->size())
	{
		if (!_shared.available()) {
			status = XMI_ERROR;
			return;
		}
		_shared.setProducers(1, 0);
		_shared.setConsumers(_npeers - 1, 0);
		reset_impl();
	}

	inline void reset_impl() {
		if (_peer == 0) {
			_shared.reset();
		}
	}

	inline bool generateMessage_impl(XMI_Multicast_t *mcast);
private:
	DCMF::Device::WorkQueue::SharedWorkQueue _shared;
	unsigned _peer;
	unsigned _npeers;
}; // class LocalBcastWQModel

void LocalBcastWQMessage::complete() {
        ((LocalBcastWQDevice &)_QS).__complete(this);
        executeCallback();
}

inline DCMF::Queueing::MessageStatus LocalBcastWQMessage::advanceThread(DCMF::Queueing::Generic::GenericAdvanceThread *t) {
	return __advanceThread((LocalBcastWQThread *)t);
}

inline bool LocalBcastWQModel::generateMessage_impl(XMI_Multicast_t *mcast) {
	if (mcast->req_size < sizeof(LocalBcastWQMessage)) {
		return false;
	}
	// assert((src_topo .U. dst_topo).size() == _npeers);
	// use roles to determine root status
	XMI::Topology *src_topo = (XMI::Topology *)mcast->src_participants;
	unsigned rootpeer = _g_topology_local->rank2Index(src_topo->index2Rank(0));
	bool isrootrole = (_peer == rootpeer);
	unsigned consumer = (_peer - (_peer > rootpeer));
	if (isrootrole) consumer = 0; // hack!
	_shared.setConsumers(_npeers - 1, consumer);
	LocalBcastWQMessage *msg =
		new (mcast->request) LocalBcastWQMessage(_g_l_bcastwq_dev,
			mcast->cb_done, _shared, isrootrole,
			(XMI::PipeWorkQueue *)mcast->src, (XMI::PipeWorkQueue *)mcast->dst,
			mcast->bytes);
	_g_l_bcastwq_dev.__post(msg);
	return true;
}

}; // namespace CDI
}; // namespace DCMF

#endif // __dcmf_workqueue_localbcastwqmsg_h__
