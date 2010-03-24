/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/LocalReduceWQMessage.h
 * \brief ???
 */

#ifndef __components_devices_workqueue_LocalReduceWQMessage_h__
#define __components_devices_workqueue_LocalReduceWQMessage_h__

#include "components/devices/workqueue/SharedWorkQueue.h"
#include "components/devices/workqueue/MemoryWorkQueue.h"
#include "math/math_coremath.h"
#include "Global.h"
#include "components/devices/util/SubDeviceSuppt.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/MulticombineModel.h"
#include "components/devices/FactoryInterface.h"

namespace PAMI {
namespace Device {

class LocalReduceWQModel;
class LocalReduceWQMessage;
typedef PAMI::Device::Generic::GenericAdvanceThread LocalReduceWQThread;
class LocalReduceWQDevice : public PAMI::Device::Generic::MultiSendQSubDevice<LocalReduceWQThread,1,true> {
public:
        class Factory : public Interface::FactoryInterface<Factory,LocalReduceWQDevice,Generic::Device> {
        public:
                static inline LocalReduceWQDevice *generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager & mm);
                static inline pami_result_t init_impl(LocalReduceWQDevice *devs, size_t client, size_t contextId, pami_client_t clt, pami_context_t ctx, PAMI::Memory::MemoryManager *mm, PAMI::Device::Generic::Device *devices);
                static inline size_t advance_impl(LocalReduceWQDevice *devs, size_t client, size_t context);
                static inline LocalReduceWQDevice & getDevice_impl(LocalReduceWQDevice *devs, size_t client, size_t context);
        }; // class Factory
        inline PAMI::Memory::MemoryManager *getSysdep() { return _mm; }
protected:
        PAMI::Memory::MemoryManager *_mm;
}; // class LocalReduceWQDevice

}; // namespace Device
}; // namespace PAMI

extern PAMI::Device::LocalReduceWQDevice _g_l_reducewq_dev;

namespace PAMI {
namespace Device {

inline LocalReduceWQDevice *LocalReduceWQDevice::Factory::generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager &mm) {
        return &_g_l_reducewq_dev;
}

inline pami_result_t LocalReduceWQDevice::Factory::init_impl(LocalReduceWQDevice *devs, size_t client, size_t contextId, pami_client_t clt, pami_context_t ctx, PAMI::Memory::MemoryManager *mm, PAMI::Device::Generic::Device *devices) {
        if (client == 0 && contextId == 0) {
                _g_l_reducewq_dev._mm = mm;
        }
        return _g_l_reducewq_dev.__init(client,  contextId,  clt,  ctx,  mm,  devices);
}

inline size_t LocalReduceWQDevice::Factory::advance_impl(LocalReduceWQDevice *devs, size_t client, size_t contextId) {
        return 0;
}

inline LocalReduceWQDevice & LocalReduceWQDevice::Factory::getDevice_impl(LocalReduceWQDevice *devs, size_t client, size_t contextId) {
        return _g_l_reducewq_dev;
}

class LocalReduceWQMessage : public PAMI::Device::Generic::GenericMessage {
public:

          ///
          /// \brief Local (intranode) reduce collective message.
          ///
          /// The rank designated as peer \c 0 is \b always the reduction root. Care must be
          /// taken when assigning peer identifications.
          ///
          /// \param[in] device       Shared Memory device
          /// \param[in] cb           Callback to invoke when the broadcast is complete
          /// \param[in] sharedmemory Location of the shared memory
          /// \param[in] peer         This core's peer id
          /// \param[in] peers        Number of ranks active on this node
          /// \param[in] sbuffer      Location of the source reduce buffer
          /// \param[in] rbuffer      Location of the result reduce buffer
          /// \param[in] count        Number of elements to reduce
          /// \param[in] func         Math function to invoke to perform the reduction
          /// \param[in] dtshift      Shift in byts of the elements for the reduction
          ///
          inline LocalReduceWQMessage (GenericDeviceMessageQueue *device,
                                       pami_multicombine_t *mcomb,
                                       PAMI::Device::WorkQueue::SharedWorkQueue &workqueue,
                                       unsigned          peer,
                                       unsigned          peers,
                                       unsigned          rootpeer,
                                       coremath          func,
                                       int               dtshift) :
            PAMI::Device::Generic::GenericMessage (device, mcomb->cb_done,
                                mcomb->client, mcomb->context),
            _isrootpeer (peer == rootpeer),
            //_iscopypeer (peer == ((rootpeer+1)%peers)),
            _iscopypeer (peer == ((rootpeer+1) >= peers ? (rootpeer+1) - peers : (rootpeer+1))),
            _func (func),
            _dtshift (dtshift),
            _source (*(PAMI::PipeWorkQueue *)mcomb->data),
            _result (*(PAMI::PipeWorkQueue *)mcomb->results),
            _shared (workqueue)
          {
            // Producer 0 will always be the "copy peer"
            //unsigned copypeer = (rootpeer+1)%peers;
            //unsigned producer = (peer + peers - rootpeer - 1) % peers;
            unsigned producer = (peer + peers - rootpeer - 1); if (producer >= peers) producer -= peers;
            if (_isrootpeer) producer = 0; // hack!


            _shared.setProducers (peers-1, producer);
            _shared.setConsumers (1, 0);
          }

protected:
        DECL_ADVANCE_ROUTINE(advanceThread,LocalReduceWQMessage,LocalReduceWQThread);
        inline pami_result_t __advanceThread(LocalReduceWQThread *thr) {
                // workaround for GNU compiler -fPIC -O3 bug
                volatile coremath1 shmcpy = (coremath1) PAMI::Device::WorkQueue::SharedWorkQueue::shmemcpy;
                // these should each be separate advanceThread routines...
                if (_iscopypeer) {
                        _shared.Q2Q (_source, shmcpy, 0);

                        // If all bytes have been copied from the local source buffer into
                        // the shared queue then this peer is done.
                        if (_source.bytesAvailableToConsume () == 0) {
                                thr->setStatus(PAMI::Device::Complete);
                                setStatus(PAMI::Device::Done);
                        }
                } else if (_isrootpeer) {
                        _shared.reduce2Q (_source, _result, _func, _dtshift);

                        // If all bytes have been copied from the shared queue into the
                        // local result buffer then the root is done.
                        if (_result.bytesAvailableToProduce () == 0) {
                                thr->setStatus(PAMI::Device::Complete);
                                setStatus(PAMI::Device::Done);
                        }
                } else {
                        _shared.reduceInPlace (_source, _func, _dtshift);

                        // If all bytes have been copied from the local source buffer into
                        // the shared queue then this peer is done.
                        if (_source.bytesAvailableToConsume () == 0) {
                                thr->setStatus(PAMI::Device::Complete);
                                setStatus(PAMI::Device::Done);
                        }
                }
                return getStatus() == PAMI::Device::Done ? PAMI_SUCCESS : PAMI_EAGAIN;
        }

public:
        // virtual function
        pami_context_t postNext(bool devQueued) {
                return _g_l_reducewq_dev.__postNext<LocalReduceWQMessage>(this, devQueued);
        }

        inline int setThreads(LocalReduceWQThread **th) {
                LocalReduceWQThread *t;
                int n;
                _g_l_reducewq_dev.__getThreads(&t, &n);
                t[0].setMsg(this);
                t[0].setAdv(advanceThread);
                t[0].setStatus(PAMI::Device::Ready);
                __advanceThread(t);
                *th = t;
                return 1;
        }

protected:

          bool              _isrootpeer;
          bool              _iscopypeer;
          coremath          _func;
          int               _dtshift;
          PAMI::PipeWorkQueue   &_source;
          PAMI::PipeWorkQueue   &_result;
          PAMI::Device::WorkQueue::SharedWorkQueue & _shared;
}; // class LocalReduceWQMessage

class LocalReduceWQModel : public PAMI::Device::Interface::MulticombineModel<LocalReduceWQModel,LocalReduceWQDevice,sizeof(LocalReduceWQMessage)> {
public:
        static const int NUM_ROLES = 2;
        static const int REPL_ROLE = 1;
        static const size_t sizeof_msg = sizeof(LocalReduceWQMessage);

        LocalReduceWQModel(LocalReduceWQDevice &device, pami_result_t &status) :
        PAMI::Device::Interface::MulticombineModel<LocalReduceWQModel,LocalReduceWQDevice,sizeof(LocalReduceWQMessage)>(device, status),
        _shared(_g_l_reducewq_dev.getSysdep()),
        _peer(__global.topology_local.rank2Index(__global.mapping.task())),
        _npeers(__global.topology_local.size())
        {
                // assert(device == _g_l_reducewq_dev);
                if (!_shared.available()) {
                        status = PAMI_ERROR;
                        return;
                }
                _shared.setProducers(_npeers, _peer);
                _shared.setConsumers(_npeers, _peer);
                // since we hard-code topology_local, we know _peer==0 exists...
                _shared.barrier_reset(_npeers, (_peer == 0));
        }

        inline void reset_impl() {
                if (_peer == 0) {
                        _shared.reset();
                }
        }

        inline pami_result_t postMulticombine_impl(uint8_t (&state)[sizeof_msg], pami_multicombine_t *mcomb);

private:
        PAMI::Device::WorkQueue::SharedWorkQueue _shared;
        unsigned _peer;
        unsigned _npeers;
}; // class LocalReduceWQModel

inline pami_result_t LocalReduceWQModel::postMulticombine_impl(uint8_t (&state)[sizeof_msg], pami_multicombine_t *mcomb) {
        PAMI::Topology *results_topo = (PAMI::Topology *)mcomb->results_participants;
        // assert((data_topo .U. results_topo).size() == _npeers);
        // This is a LOCAL reduce, results_topo must be a valid local rank!
        // assert(_g_topology_local->rank2Index(results_topo->index2Rank(0)) != -1);
        int dtshift = pami_dt_shift[mcomb->dtype];
        coremath func = MATH_OP_FUNCS(mcomb->dtype, mcomb->optor, 2);
        unsigned rootpeer = __global.topology_local.rank2Index(results_topo->index2Rank(0));
        LocalReduceWQMessage *msg =
                new (&state) LocalReduceWQMessage(_g_l_reducewq_dev.getQS(),
                                mcomb, _shared, _peer, _npeers, rootpeer,
                                func, dtshift);
        _g_l_reducewq_dev.__post<LocalReduceWQMessage>(msg);
        return PAMI_SUCCESS;
}

}; // namespace Device
}; // namespace PAMI

#endif // __components_devices_workqueue_localreducewqmsg_h__
