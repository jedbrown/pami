///
/// \file components/devices/bgq/mu2/model/MUMultisync.h
/// \brief ???
///

#ifndef __components_devices_bgq_mu2_model_MUMultisync_h__
#define __components_devices_bgq_mu2_model_MUMultisync_h__

#include <spi/include/kernel/gi.h>
#include <spi/include/mu/GIBarrier.h>

#include "components/devices/bgq/mu2/Context.h"
#include "sys/pami.h"
#include "components/devices/MultisyncModel.h"

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

#define MU_BLOCKING_BARRIER   0 
#define POST_TO_MUCONTEXT     0

#define MUMULTISYNC_TIMEOUT 10000

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      static const size_t  msync_bytes     = 0;
      static const size_t  NumClassRoutes  = 16;
      static const size_t  cw_classroute   = 0; 
      class MUMultisyncModel : public Interface::MultisyncModel<MUMultisyncModel, MU::Context, msync_bytes>
      {
      public:
        static const size_t sizeof_msg = msync_bytes;

        class CompletionMsg : public MessageQueue::Element
        {
        public:
          pami_context_t        context;
          pami_event_function   cb_done;
          void                * cookie;
          MUSPI_GIBarrier_t   * barrier;

          CompletionMsg () : MessageQueue::Element()
          {
          }

          virtual bool advance ();
        };

        MUMultisyncModel (pami_client_t     client,
                          pami_context_t    context,
                          MU::Context     & mucontext,
                          pami_result_t   & status):
        Interface::MultisyncModel<MUMultisyncModel, MU::Context, msync_bytes> (mucontext, status),
        _context(context),
        _mucontext (mucontext),
        _gdev(*_mucontext.getProgressDevice())
        {
          TRACE_FN_ENTER();
          //Initialize comm world
          memset (_inited, 0, sizeof(_inited));
          int rc = 0;
          rc = MUSPI_GIBarrierInit ( &_giBarrier[cw_classroute], cw_classroute );
          PAMI_assert (rc == 0);
          _inited[cw_classroute] = 1;

          new (&_work) PAMI::Device::Generic::GenericThread(advance, &_completionmsg);
          _completionmsg.context = context;
          TRACE_FN_EXIT();
        } 

        /// \see PAMI::Device::Interface::MultisyncModel::postMultisync
        pami_result_t postMultisyncImmediate_impl(size_t            client,
                                                  size_t            context, 
                                                  pami_multisync_t *msync,
                                                  void             *devinfo = NULL) 
        {
          TRACE_FN_ENTER();
          size_t classroute = 0;
          PAMI_assert(devinfo);
          classroute = (size_t)devinfo - 1;

          if (!_inited[classroute])
          {
            TRACE_FORMAT( "<%p> %p connection id %u, devinfo %p", this, msync, msync->connection_id, devinfo);
            int rc = 0;
            rc = MUSPI_GIBarrierInit ( &_giBarrier[classroute], classroute );
            PAMI_assert (rc == 0);
            _inited[classroute] = 1;
            //fprintf (stderr, "Initialize class route %ld\n", classroute);
          }

#if    MU_BLOCKING_BARRIER
          MUSPI_GIBarrierEnterAndWait (&_giBarrier[classroute]);
          if (msync->cb_done.function)
            msync->cb_done.function(_context, msync->cb_done.clientdata, PAMI_SUCCESS);
#elif  POST_TO_MUCONTEXT
          MUSPI_GIBarrierEnter (&_giBarrier[classroute]);
          int rc = MUSPI_GIBarrierPollWithTimeout  (&_giBarrier[classroute], MUMULTISYNC_TIMEOUT);
          if (rc==0 && msync->cb_done.function)
          {
            msync->cb_done.function(_context, msync->cb_done.clientdata, PAMI_SUCCESS);
            return PAMI_SUCCESS;
          }
          //Post message to MU channel 0
          _completionmsg.cb_done = msync->cb_done.function;
          _completionmsg.cookie  = msync->cb_done.clientdata;
          _completionmsg.barrier = &_giBarrier[classroute];       
          _mucontext.injectionGroup.channel[0].post(&_completionmsg);
#else	  
          MUSPI_GIBarrierEnter (&_giBarrier[classroute]);
          int rc = MUSPI_GIBarrierPollWithTimeout  (&_giBarrier[classroute], MUMULTISYNC_TIMEOUT);
          if (rc==0 && msync->cb_done.function)
          {
            msync->cb_done.function(_context, msync->cb_done.clientdata, PAMI_SUCCESS);
            return PAMI_SUCCESS;
          }
          //Create a work object and post work to generic device
          _completionmsg.cb_done = msync->cb_done.function;
          _completionmsg.cookie  = msync->cb_done.clientdata;
          _completionmsg.barrier = &_giBarrier[classroute];

          PAMI::Device::Generic::GenericThread *work = (PAMI::Device::Generic::GenericThread *)&_work;
          _gdev.postThread(work);
#endif

          TRACE_FN_EXIT();
          return PAMI_SUCCESS;
        }

        pami_result_t postMultisync_impl(uint8_t (&state)[msync_bytes],
                                         size_t            client,
                                         size_t            context, 
                                         pami_multisync_t *msync,
                                         void             *devinfo = NULL) 
        {
          TRACE_FN_ENTER();
          TRACE_FN_EXIT();
          return PAMI_ERROR;
        }

        static pami_result_t advance (pami_context_t     context,
                                      void             * cookie)
        {
          CompletionMsg *msg = (CompletionMsg *) cookie;    
          bool flag = msg->CompletionMsg::advance();    
          if (flag)
            return PAMI_SUCCESS;

          return PAMI_EAGAIN;
        }

      protected:
        pami_context_t             _context;
        MU::Context              & _mucontext;
        Generic::Device          & _gdev;
        uint8_t                    _inited [NumClassRoutes];
        MUSPI_GIBarrier_t          _giBarrier[NumClassRoutes];  
        pami_work_t                _work;
        CompletionMsg              _completionmsg;
      };

      bool  MUMultisyncModel::CompletionMsg::advance ()
      {
        int rc = MUSPI_GIBarrierPoll (barrier);

        if (rc == 0)
        {
          if (cb_done)
            cb_done(context, cookie, PAMI_SUCCESS);
          return true;
        }

        return false;
      }

    };
  };
};

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif
