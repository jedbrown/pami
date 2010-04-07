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
#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

namespace PAMI {
namespace Device {

class LocalBcastWQModel;
class LocalBcastWQMessage;
typedef PAMI::Device::Generic::GenericAdvanceThread LocalBcastWQThread;
typedef PAMI::Device::Generic::MultiSendQSubDevice<LocalBcastWQThread,1,true> LocalBcastWQPendQ;
typedef PAMI::Device::Generic::NillSubDevice LocalBcastWQDevice;

class LocalBcastWQMessage : public PAMI::Device::Generic::GenericMessage {
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
          inline LocalBcastWQMessage(GenericDeviceMessageQueue *device,
                                      pami_multicast_t *mcast,
                                      PAMI::Device::WorkQueue::SharedWorkQueue & workqueue,
                                      bool              isrootrole) :
            PAMI::Device::Generic::GenericMessage (device, mcast->cb_done,
                                        mcast->client, mcast->context),
            _isrootrole (isrootrole),
            _sbuffer (*(PAMI::PipeWorkQueue *)mcast->src),
            _rbuffer (*(PAMI::PipeWorkQueue *)mcast->dst),
            _shared (workqueue)
          {
            TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
          }

protected:
        DECL_ADVANCE_ROUTINE(advanceThread,LocalBcastWQMessage,LocalBcastWQThread);
        inline pami_result_t __advanceThread(LocalBcastWQThread *thr) {
                // This works around a bug with "g++ -fPIC -O3"...
                coremath1 func = (coremath1) PAMI::Device::WorkQueue::SharedWorkQueue::shmemcpy;
                if (_isrootrole) {
                        // "broadcast" the source buffer into the shared queue.
                        _shared.Q2Q(_sbuffer, func, 0);

                        // If all bytes have been copied to the shared queue then the root is done.
                        if (_sbuffer.bytesAvailableToConsume() == 0) {
                                thr->setStatus(PAMI::Device::Complete);
                                setStatus(PAMI::Device::Done);
                        }
                } else {
                        // read bytes from the shared queue into the local result buffer.
                        _shared.Q2Qr(_rbuffer, func, 0);

                        // If all bytes have been copied from the shared queue then the recv is done.
                        if (_rbuffer.bytesAvailableToProduce() == 0) {
                                thr->setStatus(PAMI::Device::Complete);
                                setStatus(PAMI::Device::Done);
                        }
                }

                return getStatus() == PAMI::Device::Done ? PAMI_SUCCESS : PAMI_EAGAIN;
        }

public:
        // virtual function
        pami_context_t postNext(bool devQueued) {
		LocalBcastWQPendQ *qs = (LocalBcastWQPendQ *)getQS();
                return qs->__postNext<LocalBcastWQMessage>(this, devQueued);
        }

        inline int setThreads(LocalBcastWQThread **th) {
		LocalBcastWQPendQ *qs = (LocalBcastWQPendQ *)getQS();
                LocalBcastWQThread *t;
                int n;
                qs->__getThreads(&t, &n);
                t[0].setMsg(this);
                t[0].setAdv(advanceThread);
                t[0].setStatus(PAMI::Device::Ready);
                __advanceThread(t);
                *th = t;
                return 1;
        }

private:
          bool              _isrootrole;
          PAMI::PipeWorkQueue   &_sbuffer;
          PAMI::PipeWorkQueue   &_rbuffer;
          PAMI::Device::WorkQueue::SharedWorkQueue & _shared;
}; // class LocalBcastWQMessage

class LocalBcastWQModel : public PAMI::Device::Interface::MulticastModel<LocalBcastWQModel,LocalBcastWQDevice,sizeof(LocalBcastWQMessage)> {
public:
        static const int NUM_ROLES = 2;
        static const int REPL_ROLE = 1;
        static const size_t sizeof_msg = sizeof(LocalBcastWQMessage);

        LocalBcastWQModel(LocalBcastWQDevice &device, pami_result_t &status) :
        PAMI::Device::Interface::MulticastModel<LocalBcastWQModel,LocalBcastWQDevice,sizeof(LocalBcastWQMessage)>(device, status),
	_gd(&device),
        _shared(_gd->getMM()),
        _peer(__global.topology_local.rank2Index(__global.mapping.task())),
        _npeers(__global.topology_local.size())
        {
            TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
                if (!_shared.available()) {
                        status = PAMI_ERROR;
                        return;
                }
                _shared.setProducers(1, 0);
                _shared.setConsumers(_npeers - 1, 0);
                // since we hard-code topology_local, we know _peer==0 exists...
                _shared.barrier_reset(_npeers, (_peer == 0));
		_queue.__init(_gd->clientId(), _gd->contextId(), NULL, _gd->getContext(), _gd->getMM(), _gd->getAllDevs());
        }

        inline void reset_impl() {
                if (_peer == 0) {
                        _shared.reset();
                }
        }

        inline pami_result_t postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                               pami_multicast_t *mcast);
private:
	PAMI::Device::Generic::Device *_gd;
        PAMI::Device::WorkQueue::SharedWorkQueue _shared;
        unsigned _peer;
        unsigned _npeers;
	LocalBcastWQPendQ _queue;
}; // class LocalBcastWQModel

inline pami_result_t LocalBcastWQModel::postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                                  pami_multicast_t *mcast) {
        // assert((src_topo .U. dst_topo).size() == _npeers);
        // use roles to determine root status
        PAMI::Topology *src_topo = (PAMI::Topology *)mcast->src_participants;
        unsigned rootpeer = __global.topology_local.rank2Index(src_topo->index2Rank(0));
        bool isrootrole = (_peer == rootpeer);
        unsigned consumer = (_peer - (_peer > rootpeer));
        if (isrootrole) consumer = 0; // hack!
        _shared.setConsumers(_npeers - 1, consumer);
        LocalBcastWQMessage *msg =
                new (&state) LocalBcastWQMessage(_queue.getQS(),
                        mcast, _shared, isrootrole);
        _queue.__post<LocalBcastWQMessage>(msg);
        return PAMI_SUCCESS;
}

}; // namespace Device
}; // namespace PAMI

#endif // __pami_components_devices_workqueue_localbcastwqmsg_h__
