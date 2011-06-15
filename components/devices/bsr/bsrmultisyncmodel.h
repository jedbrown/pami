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
    class BSRMulticastModel :
      public Interface::MulticastModel<BSRMulticastModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
    {
      public:
      static const size_t sizeof_msg              = sizeof(T_Message);
      static const size_t msync_model_state_bytes = sizeof(T_Message);
      BSRMulticastModel (T_Device &device, pami_result_t &status) :
        Interface::MulticastModel<BSRMulticastModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>(device,status)
          {
            status = PAMI_ERROR;
          }
      inline pami_result_t postMulticastImmediate_impl(size_t            client,
                                                       size_t            context,
                                                       pami_multicast_t *mcast,
                                                       void             *devinfo=NULL)
        {
          PAMI_assertf(0, "postMulticast is not available on BSR Device");
          return PAMI_SUCCESS;
        }
      inline pami_result_t postMulticast_impl(uint8_t (&state)[msync_model_state_bytes],
                                              size_t            client,
                                              size_t            context,
                                              pami_multicast_t *mcast,
                                              void             *devinfo=NULL)
        {
          PAMI_assertf(0, "postMulticast is not available on BSR Device");
          return PAMI_SUCCESS;
        }
      inline pami_result_t registerMcastRecvFunction (int                         dispatch_id,
                                                      pami_dispatch_multicast_function recv_func,
                                                      void                       *async_arg)
        {
          PAMI_assertf(0, "registerMcastRecvFunction is not available on BSR Device");
          return PAMI_SUCCESS;
        }
    };

    template <class T_Device, class T_Message>
    class BSRMulticombineModel :
      public Interface::MulticombineModel<BSRMulticombineModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
    {
      public:
      static const size_t sizeof_msg              = sizeof(T_Message);
      static const size_t msync_model_state_bytes = sizeof(T_Message);
      BSRMulticombineModel (T_Device &device, pami_result_t &status) :
        Interface::MulticombineModel<BSRMulticombineModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>(device,status)
        {
          status = PAMI_ERROR;
        }
      inline pami_result_t postMulticombine (uint8_t (&state)[msync_model_state_bytes],
                                             size_t               client,
                                             size_t               context,
                                             pami_multicombine_t *mcomb,
                                             void                *devinfo=NULL)
        {
          PAMI_assertf(0, "postMulticombine is not available on BSR Device");
          return PAMI_SUCCESS;
        }

      inline pami_result_t postMulticombineImmediate (size_t               client,
                                                      size_t               context,
                                                      pami_multicombine_t *mcomb,
                                                      void                *devinfo=NULL)
        {
          PAMI_assertf(0, "postMulticombine is not available on BSR Device");
          return PAMI_SUCCESS;
        }
    };

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


      // This function is executed by the generic device, and will execute
      // as long as do_msync returns PAMI_EAGAIN.
      static pami_result_t do_msync(pami_context_t context, void *cookie)
        {
          // Capture all message information needed to free
          // before the message storage may be freed as a result of
          // a message cb_done
          T_Message              *m      = (T_Message*) cookie;
          // Advance the message
          pami_result_t           result = m->advance();

          // Free the message if not PAMI_EAGAIN.
          if(result == PAMI_SUCCESS)
          {
            Generic::GenericThread *t      = m->_workfcn;
            T_Device               *device = (T_Device *)m->_device;
            device->freeWork(t);
          }
          return result;
        }
      
      pami_result_t postMultisync_impl (uint8_t (&state)[msync_model_state_bytes],
                                        size_t            client,
                                        size_t            context,
                                        pami_multisync_t *msync,
                                        void             *devinfo)
      {
        BSRGeometryInfo  *bsrinfo        = (BSRGeometryInfo *)devinfo;
        T_Message        *msg            = new(state) T_Message(&_device,
                                                                bsrinfo,
                                                                msync->cb_done.function,
                                                                msync->cb_done.clientdata);        
        pami_result_t rc = msg->advance();
        if(rc == PAMI_EAGAIN)
          msg->_workfcn = _device.postWork(do_msync, msg);

        return PAMI_SUCCESS;
      }
      T_Device                     &_device;
    };
  };
};

#endif // __components_devices_mpi_mpimultisyncmodel_h__
