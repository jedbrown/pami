///
/// \file components/devices/bgq/mu2/model/CollectiveMulticastDmaModel.h
/// \brief ???
///
#ifndef __components_devices_bgq_mu2_model_CollectiveMulticastDmaModel_h__
#define __components_devices_bgq_mu2_model_CollectiveMulticastDmaModel_h__

#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/model/CollectiveDmaModelBase.h"
#include "sys/pami.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
#define ZERO_BYTES 65536

      static const size_t mcast_state_bytes = 0;
      class CollectiveMulticastDmaModel: public CollectiveDmaModelBase,
          public Interface::MulticastModel < CollectiveMulticastDmaModel, MU::Context, mcast_state_bytes >
      {
          size_t               _myrank;
          static char        * _zeroBuf;
          static uint32_t      _zeroBytes;

        public:
          static const size_t sizeof_msg = mcast_state_bytes;
          static const size_t immediate_bytes = TEMP_BUF_SIZE;

          CollectiveMulticastDmaModel (pami_client_t    client,
                                       pami_context_t   context,
                                       MU::Context                 & device,
                                       pami_result_t               & status) :
              CollectiveDmaModelBase(device, status),
              Interface::MulticastModel<CollectiveMulticastDmaModel, MU::Context, mcast_state_bytes >  (device, status), _myrank(__global.mapping.task())
          {
            _zeroBytes = ZERO_BYTES;

            if (_zeroBuf == NULL)
              {
                _zeroBuf = (char *) malloc (_zeroBytes * sizeof(char));
                memset (_zeroBuf, 0, _zeroBytes);
              }
          }

          pami_result_t postMulticastImmediate_impl(size_t                client,
                                                    size_t                context,
                                                    pami_multicast_t    * mcast,
                                                    void                * devinfo = NULL)
          {
            pami_result_t rc = PAMI_ERROR;
            pami_task_t *ranks = NULL;
            ((Topology *)mcast->src_participants)->rankList(&ranks);
            bool isroot = (ranks[0] == _myrank);

            unsigned classroute = 0;

            if (devinfo)
              classroute = ((uint32_t)(uint64_t)devinfo) - 1;

            if ( likely(mcast->bytes <= immediate_bytes) )
              {
                char *src = NULL;
                PipeWorkQueue *dst = NULL;
                unsigned sbytes = 0;

                if (isroot)
                  {
                    PipeWorkQueue *spwq = (PipeWorkQueue *) mcast->src;
                    src = spwq->bufferToConsume();
                    sbytes = spwq->bytesAvailableToConsume();

                    if (sbytes != mcast->bytes)
                      goto longmsg;
                  }
                else
                  {
                    src = _zeroBuf;
                    dst = (PipeWorkQueue*) mcast->dst;
                  }

                rc = CollectiveDmaModelBase::postShortCollective (MUHWI_COLLECTIVE_OP_CODE_OR,
                                                                  4,
                                                                  mcast->bytes,
                                                                  src,
                                                                  dst,
                                                                  mcast->cb_done.function,
                                                                  mcast->cb_done.clientdata,
                                                                  classroute);
              }

            if (rc == PAMI_SUCCESS)
              return rc;

longmsg:
            return CollectiveDmaModelBase::postBroadcast (mcast->bytes,
                                                          (PipeWorkQueue *) mcast->src,
                                                          (PipeWorkQueue *) mcast->dst,
                                                          mcast->cb_done.function,
                                                          mcast->cb_done.clientdata,
                                                          _zeroBuf,
                                                          _zeroBytes,
                                                          isroot,
                                                          classroute);
          }

          /// \see PAMI::Device::Interface::MulticastModel::postMulticast
          pami_result_t postMulticast_impl(uint8_t (&state)[mcast_state_bytes],
                                           size_t            client,
                                           size_t            context,
                                           pami_multicast_t *mcast,
                                           void             *devinfo = NULL)
          {
            //TRACE_FN_ENTER();
            return PAMI_ERROR;
            //TRACE_FN_EXIT();
          }
      };
    };
  };
};

#endif
