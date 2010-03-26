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
class LocalBcastWQDevice : public PAMI::Device::Generic::MultiSendQSubDevice<LocalBcastWQThread,1,true> {
public:
        class Factory : public Interface::FactoryInterface<Factory,LocalBcastWQDevice,Generic::Device> {
        public:
                static inline LocalBcastWQDevice *generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager & mm);
                static inline pami_result_t init_impl(LocalBcastWQDevice *devs, size_t client, size_t contextId, pami_client_t clt, pami_context_t ctx, PAMI::Memory::MemoryManager *mm, PAMI::Device::Generic::Device *devices);
                static inline size_t advance_impl(LocalBcastWQDevice *devs, size_t client, size_t context);
                static inline LocalBcastWQDevice & getDevice_impl(LocalBcastWQDevice *devs, size_t client, size_t context);
        }; // class Factory
        inline PAMI::Memory::MemoryManager *getSysdep() { return _mm; }
protected:
        PAMI::Memory::MemoryManager *_mm;
}; // class LocalBcastWQDevice

}; // namespace Device
}; // namespace PAMI

extern PAMI::Device::LocalBcastWQDevice _g_l_bcastwq_dev;

namespace PAMI {
namespace Device {

inline LocalBcastWQDevice *LocalBcastWQDevice::Factory::generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager &mm) {
        return &_g_l_bcastwq_dev;
}

inline pami_result_t LocalBcastWQDevice::Factory::init_impl(LocalBcastWQDevice *devs, size_t client, size_t contextId, pami_client_t clt, pami_context_t ctx, PAMI::Memory::MemoryManager *mm, PAMI::Device::Generic::Device *devices) {
        if (client == 0 && contextId == 0) {
                _g_l_bcastwq_dev._mm = mm;
        }
        return _g_l_bcastwq_dev.__init(client, contextId, clt, ctx, mm, devices);
}

inline size_t LocalBcastWQDevice::Factory::advance_impl(LocalBcastWQDevice *devs, size_t client, size_t contextid) {
        return 0;
}

inline LocalBcastWQDevice & LocalBcastWQDevice::Factory::getDevice_impl(LocalBcastWQDevice *devs, size_t client, size_t contextid) {
        return _g_l_bcastwq_dev;
}

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
                return _g_l_bcastwq_dev.__postNext<LocalBcastWQMessage>(this, devQueued);
        }

        inline int setThreads(LocalBcastWQThread **th) {
                LocalBcastWQThread *t;
                int n;
                _g_l_bcastwq_dev.__getThreads(&t, &n);
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
        _shared(_g_l_bcastwq_dev.getSysdep()),
        _peer(__global.topology_local.rank2Index(__global.mapping.task())),
        _npeers(__global.topology_local.size())
        {
            TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
                // assert(device == _g_l_bcastwq_dev);
                if (!_shared.available()) {
                        status = PAMI_ERROR;
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

        inline pami_result_t postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                               pami_multicast_t *mcast);
private:
        PAMI::Device::WorkQueue::SharedWorkQueue _shared;
        unsigned _peer;
        unsigned _npeers;
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
                new (&state) LocalBcastWQMessage(_g_l_bcastwq_dev.getQS(),
                        mcast, _shared, isrootrole);
        _g_l_bcastwq_dev.__post<LocalBcastWQMessage>(msg);
        return PAMI_SUCCESS;
}

}; // namespace Device
}; // namespace PAMI

#endif // __pami_components_devices_workqueue_localbcastwqmsg_h__
