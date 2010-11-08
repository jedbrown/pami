/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bsr/bsrmultisyncmodel.h
 * \brief ???
 */

#ifndef __components_devices_bsr_bsrmultisyncmodel_h__
#define __components_devices_bsr_bsrmultisyncmodel_h__

#include <pami.h>
#include "components/devices/MultisyncModel.h"
#include "components/devices/bsr/bsrmessage.h"
#include "components/devices/bsr/SaOnNodeSyncGroup.h"
#include <list>

#ifdef TRACE
#undef TRACE
#define TRACE(x)// fprintf x
#else
#define TRACE(x)// fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class BSRMultisyncModel :
      public Interface::MultisyncModel<BSRMultisyncModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
    {
      public:
      static const size_t msync_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg              = sizeof(T_Message);
      static const bool   isLocalOnly             = true;
      BSRMultisyncModel (T_Device &device, pami_result_t &status) :
        Interface::MultisyncModel<BSRMultisyncModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>(device,status),
        _device(device)
          {
            status = PAMI_SUCCESS;
          }

      static pami_result_t checkBSRIfDone(pami_context_t context, void* cookie)
      {
        BSRMsyncMessage *m = (BSRMsyncMessage *) cookie;
        PAMI::Device::BSRMultisyncModel<T_Device, T_Message>* model =
          (PAMI::Device::BSRMultisyncModel<T_Device, T_Message>*)(m->_multisyncmodel);

        if(! model->_sync_group->IsNbBarrierDone())
          return PAMI_EAGAIN;

        /* if the barrier has done */

        /* check waiter queue and issue new barrier if there is one */
        if(!model->waiters_q.empty())
          {
            BSRMsyncMessage *waiter = model->waiters_q.front();
            m->waiters_q.pop();
            model->_sync_group.NbBarrier();
            _device->postWork(waiter);
          }
        else
          model->_in_barrier = false;

        /* call user's callback */
        m->cb_done.function(context, m->cb_done.clientdata, PAMI_SUCCESS);

        return PAMI_SUCCESS;
      }

      pami_result_t postMultisync_impl (uint8_t (&state)[msync_model_state_bytes],
          pami_multisync_t *msync,
          void             *devinfo)
      {
        PAMI::Topology *topology = (PAMI::Topology*) msync->participants;
        if(topology->size() == 1)
        {
          msync->cb_done.function(_device.getContext(), msync->cb_done.clientdata, PAMI_SUCCESS);
          return PAMI_SUCCESS;
        }

        if(!_sync_group.IsInitialized())
        {
          SaOnNodeSyncGroup::Param_t param = { false, false };
          size_t unique_id = (size_t)0x99;/* TODO:an unique id that is used to create shared memory segment */
          size_t member_id = topology->rank2Index(_device.taskid());
          if (SyncGroup::SUCCESS != _sync_group.Init(topology->size(), unique_id, member_id, &param))
            return PAMI_ERROR;
        }

        /* Check if we need to queue up */
        if (!_in_barrier)
          {
            /* entering barrier */
            _in_barrier = true;
            _sync_group.NbBarrier();
            if (_sync_group.IsNbBarrierDone())
            {
              msync->cb_done.function(_device.getContext(), msync->cb_done.clientdata, PAMI_SUCCESS);
              _in_barrier = false;
              return PAMI_SUCCESS;
            }

            BSRMsyncMessage *m = new (state) BSRMsyncMessage
              (&checkBSRIfDone, (void*)state, msync->cb_done, (void*)this);

            _device->postWork(m);
          }
        else
          {
            BSRMsyncMessage *m = new (state) BSRMsyncMessage
              (&checkBSRIfDone, (void*)state, msync->cb_done, (void*)this);
            _waiters_q.push(m);
          }

        return PAMI_SUCCESS;
      }

      SaOnNodeSyncGroup             _sync_group;
      T_Device                     &_device;
      bool                          _in_barrier;
      queue<BSRMsyncMessage*>       _waiters_q;
    };
  };
};
#endif // __components_devices_mpi_mpimultisyncmodel_h__
