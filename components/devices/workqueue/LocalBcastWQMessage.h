/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/LocalBcastWQMessage.h
 * \brief ???
 */

#ifndef __components_devices_workqueue_LocalBcastWQMessage_h__
#define __components_devices_workqueue_LocalBcastWQMessage_h__

#include "components/devices/workqueue/SharedWorkQueue.h"
#include "components/devices/workqueue/MemoryWorkQueue.h"
#include "math/math_coremath.h"
#include "Global.h"
#include "components/devices/util/SubDeviceSuppt.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/MulticastModel.h"
#include "components/devices/FactoryInterface.h"

namespace XMI {
namespace Device {

class LocalBcastWQModel;
class LocalBcastWQMessage;
typedef XMI::Device::Generic::GenericAdvanceThread LocalBcastWQThread;
class LocalBcastWQDevice : public XMI::Device::Generic::MultiSendQSubDevice<LocalBcastWQThread,1,1,true> {
public:
	class Factory : public Interface::FactoryInterface<Factory,LocalBcastWQDevice,Generic::Device> {
	public:
		static inline LocalBcastWQDevice *generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager & mm);
		static inline xmi_result_t init_impl(LocalBcastWQDevice *devs, size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices);
		static inline size_t advance_impl(LocalBcastWQDevice *devs, size_t client, size_t context);
		static inline LocalBcastWQDevice & getDevice_impl(LocalBcastWQDevice *devs, size_t client, size_t context);
	}; // class Factory
}; // class LocalBcastWQDevice

}; // namespace Device
}; // namespace XMI

extern XMI::Device::LocalBcastWQDevice _g_l_bcastwq_dev;

namespace XMI {
namespace Device {

inline LocalBcastWQDevice *LocalBcastWQDevice::Factory::generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager &mm) {
	_g_l_bcastwq_dev.__create(client, num_ctx);
	return &_g_l_bcastwq_dev;
}

inline xmi_result_t LocalBcastWQDevice::Factory::init_impl(LocalBcastWQDevice *devs, size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices) {
	return _g_l_bcastwq_dev.__init(client, contextId, clt, ctx, sd, devices);
}

inline size_t LocalBcastWQDevice::Factory::advance_impl(LocalBcastWQDevice *devs, size_t client, size_t contextid) {
	return 0;
}

inline LocalBcastWQDevice & LocalBcastWQDevice::Factory::getDevice_impl(LocalBcastWQDevice *devs, size_t client, size_t contextid) {
	return _g_l_bcastwq_dev;
}

class LocalBcastWQMessage : public XMI::Device::Generic::GenericMessage {
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
          inline LocalBcastWQMessage(Generic::GenericSubDevice *device,
                                      xmi_multicast_t *mcast,
                                      XMI::Device::WorkQueue::SharedWorkQueue & workqueue,
                                      bool              isrootrole) :
            XMI::Device::Generic::GenericMessage (device, mcast->cb_done,
					mcast->client, mcast->context),
            _isrootrole (isrootrole),
            _sbuffer (*(XMI::PipeWorkQueue *)mcast->src),
            _rbuffer (*(XMI::PipeWorkQueue *)mcast->dst),
            _shared (workqueue)
          {
          }

protected:
	DECL_ADVANCE_ROUTINE(advanceThread,LocalBcastWQMessage,LocalBcastWQThread);
	inline xmi_result_t __advanceThread(LocalBcastWQThread *thr) {
		// This works around a bug with "g++ -fPIC -O3"...
		coremath1 func = (coremath1) XMI::Device::WorkQueue::SharedWorkQueue::shmemcpy;
		if (_isrootrole) {
			// "broadcast" the source buffer into the shared queue.
			_shared.Q2Q(_sbuffer, func, 0);

			// If all bytes have been copied to the shared queue then the root is done.
			if (_sbuffer.bytesAvailableToConsume() == 0) {
				thr->setStatus(XMI::Device::Complete);
				setStatus(XMI::Device::Done);
			}
		} else {
			// read bytes from the shared queue into the local result buffer.
			_shared.Q2Qr(_rbuffer, func, 0);

			// If all bytes have been copied from the shared queue then the recv is done.
			if (_rbuffer.bytesAvailableToProduce() == 0) {
				thr->setStatus(XMI::Device::Complete);
				setStatus(XMI::Device::Done);
			}
		}

		return getStatus() == XMI::Device::Done ? XMI_SUCCESS : XMI_EAGAIN;
	}

public:
	// virtual function
	xmi_context_t postNext(bool devPosted) {
		return _g_l_bcastwq_dev.__postNext<LocalBcastWQMessage>(this, devPosted);
	}

	inline int setThreads(LocalBcastWQThread **th) {
		LocalBcastWQThread *t;
		int n;
		_g_l_bcastwq_dev.__getThreads(&t, &n);
		t[0].setMsg(this);
		t[0].setAdv(advanceThread);
		t[0].setStatus(XMI::Device::Ready);
		__advanceThread(t);
		*th = t;
		return 1;
	}

private:
          bool              _isrootrole;
          XMI::PipeWorkQueue   &_sbuffer;
          XMI::PipeWorkQueue   &_rbuffer;
          XMI::Device::WorkQueue::SharedWorkQueue & _shared;
}; // class LocalBcastWQMessage

class LocalBcastWQModel : public XMI::Device::Interface::MulticastModel<LocalBcastWQModel,LocalBcastWQDevice,sizeof(LocalBcastWQMessage)> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = 1;
	static const size_t sizeof_msg = sizeof(LocalBcastWQMessage);

	LocalBcastWQModel(LocalBcastWQDevice &device, xmi_result_t &status) :
        XMI::Device::Interface::MulticastModel<LocalBcastWQModel,LocalBcastWQDevice,sizeof(LocalBcastWQMessage)>(device, status),
	_shared(_g_l_bcastwq_dev.getSysdep()),
	_peer(__global.topology_local.rank2Index(__global.mapping.task())),
	_npeers(__global.topology_local.size())
	{
		// assert(device == _g_l_bcastwq_dev);
		if (!_shared.available()) {
			status = XMI_ERROR;
			return;
		}
		_shared.setProducers(1, 0);
		_shared.setConsumers(_npeers - 1, 0);
		// since we hard-code topology_local, we know _peer==0 exists...
		_shared.barrier_reset(_npeers, (_peer == 0));
	}

	inline void reset_impl() {
		if (_peer == 0) {
			_shared.reset();
		}
	}

	inline xmi_result_t postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                               xmi_multicast_t *mcast);
private:
	XMI::Device::WorkQueue::SharedWorkQueue _shared;
	unsigned _peer;
	unsigned _npeers;
}; // class LocalBcastWQModel

inline xmi_result_t LocalBcastWQModel::postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                                  xmi_multicast_t *mcast) {
	// assert((src_topo .U. dst_topo).size() == _npeers);
	// use roles to determine root status
	XMI::Topology *src_topo = (XMI::Topology *)mcast->src_participants;
	unsigned rootpeer = __global.topology_local.rank2Index(src_topo->index2Rank(0));
	bool isrootrole = (_peer == rootpeer);
	unsigned consumer = (_peer - (_peer > rootpeer));
	if (isrootrole) consumer = 0; // hack!
	_shared.setConsumers(_npeers - 1, consumer);
	LocalBcastWQMessage *msg =
		new (&state) LocalBcastWQMessage(_g_l_bcastwq_dev.getQS(),
			mcast, _shared, isrootrole);
	_g_l_bcastwq_dev.__post<LocalBcastWQMessage>(msg);
	return XMI_SUCCESS;
}

}; // namespace Device
}; // namespace XMI

#endif // __xmi_components_devices_workqueue_localbcastwqmsg_h__
