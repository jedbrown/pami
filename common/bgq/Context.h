///
/// \file common/bgq/Context.h
/// \brief XMI Blue Gene\Q specific context implementation.
///
#ifndef __common_bgq_Context_h__
#define __common_bgq_Context_h__

#include <stdlib.h>
#include <string.h>

#include "sys/xmi.h"
#include "common/ContextInterface.h"

#include "components/devices/generic/GenericDevice.h"

#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemModel.h"
#include "util/fifo/FifoPacket.h"
#include "util/fifo/LinearFifo.h"

#include "components/devices/bgq/mu/MUDevice.h"
#include "components/devices/bgq/mu/MUPacketModel.h"
#include "components/devices/bgq/mu/MUInjFifoMessage.h"

#include "components/atomic/gcc/GccBuiltin.h"
#include "components/atomic/l2/L2Counter.h"
//#include "components/atomic/pthread/Pthread.h"
//#include "components/atomic/bgq/BgqAtomic.h"

#include "components/memory/MemoryAllocator.h"

#include "SysDep.h"
#include "Memregion.h"

#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/eager/EagerImmediate.h"
#include "p2p/protocols/send/eager/EagerSimple.h"

#include "p2p/protocols/get/Get.h"
#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif
#define MU_DEVICE
namespace XMI
{
  typedef XMI::Mutex::CounterMutex<XMI::Counter::GccProcCounter>  ContextLock;

  //typedef Fifo::FifoPacket <32, 992> ShmemPacket;
  typedef Fifo::FifoPacket <32, 512> ShmemPacket;
  typedef Fifo::LinearFifo<Atomic::L2Counter, ShmemPacket, 16> ShmemFifo;
  //typedef Fifo::LinearFifo<Atomic::GccBuiltin, ShmemPacket, 16> ShmemFifo;
  //typedef Device::Fifo::LinearFifo<Atomic::Pthread,ShmemPacket,16> ShmemFifo;
  //typedef Fifo::LinearFifo<Atomic::BgqAtomic,ShmemPacket,16> ShmemFifo;

  typedef Device::ShmemDevice<ShmemFifo> ShmemDevice;
  typedef Device::ShmemModel<ShmemDevice> ShmemModel;

  //
  // >> Point-to-point protocol typedefs and dispatch registration.
  typedef XMI::Protocol::Send::Eager <ShmemModel, ShmemDevice> EagerShmem;
  typedef XMI::Protocol::Send::Eager < XMI::Device::MU::MUPacketModel,
  XMI::Device::MU::MUDevice > EagerMu;
  // << Point-to-point protocol typedefs and dispatch registration.
  //

  typedef XMI::Protocol::Get::Get <ShmemModel, ShmemDevice> GetShmem;

  typedef MemoryAllocator<1152, 16> ProtocolAllocator;

  class Context : public Interface::Context<XMI::Context>
  {
	static void __work_done(xmi_context_t ctx, void *cookie, xmi_result_t result) {
		XMI::Context *context = (XMI::Context *)ctx;
		context->_workAllocator.returnObject(cookie);
	}
    public:
      inline Context (xmi_client_t client, size_t clientid, size_t id, size_t num,
				XMI::Device::Generic::Device *generics,
				void * addr, size_t bytes) :
          Interface::Context<XMI::Context> (client, id),
          _client (client),
          _context ((xmi_context_t)this),
          _clientid (clientid),
          _contextid (id),
          _mm (addr, bytes),
          _sysdep (_mm),
	  _generic(generics[id]),
#ifdef MU_DEVICE
          _mu (),
#endif
          _shmem (),
	  _workAllocator()
      {
        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        // Make sure the memory allocator is large enough for all
        // protocol classes.
        COMPILE_TIME_ASSERT(sizeof(EagerShmem) <= ProtocolAllocator::objsize);
        COMPILE_TIME_ASSERT(sizeof(EagerMu) <= ProtocolAllocator::objsize);
        COMPILE_TIME_ASSERT(sizeof(GetShmem) <= ProtocolAllocator::objsize);

        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

#ifdef MU_DEVICE
        _mu.init (&_sysdep, (xmi_context_t)this, id);
#endif
	_generic.init(_sysdep, (xmi_context_t)this, clientid, id, num, generics);
        _shmem.init (&_sysdep, (xmi_context_t)this, id);

        _get = (void *) _request.allocateObject ();
        xmi_result_t result ;

        new ((void *)_get) GetShmem(_shmem, result);

        // dispatch_impl relies on the table being initialized to NULL's.
        memset(_dispatch, 0x00, sizeof(_dispatch));

      }

      inline xmi_client_t getClient_impl ()
      {
        return _client;
      }

      inline size_t getId_impl ()
      {
        return _contextid;
      }

      inline xmi_result_t destroy_impl ()
      {
        //return XMI_UNIMPL;
        return XMI_SUCCESS;
      }

      inline xmi_result_t post_impl (xmi_work_function work_fn, void * cookie)
      {
        XMI::Device::ProgressFunctionMsg *work =
		(XMI::Device::ProgressFunctionMsg *)_workAllocator.allocateObject();
	work->setFunc(work_fn);
	work->setCookie(cookie);
	work->setDone((xmi_callback_t){__work_done, (void *)work});
	work->setContext(_contextid);
	work->setClient(_clientid);
	work->postWorkDirect();
	return XMI_SUCCESS;
      }

      inline size_t advance_impl (size_t maximum, xmi_result_t & result)
      {
//          result = XMI_EAGAIN;
        result = XMI_SUCCESS;
        size_t events = 0;
        unsigned i;

        for (i = 0; i < maximum && events == 0; i++)
          {
            events += _shmem.advance_impl();
#ifdef MU_DEVICE
            events += _mu.advance();
#endif
	    events += _generic.advance();
          }

        //if (events > 0) result = XMI_SUCCESS;

        return events;
      }

      inline xmi_result_t lock_impl ()
      {
        _lock.acquire ();
	return XMI_SUCCESS;
      }

      inline xmi_result_t trylock_impl ()
      {
        if (_lock.tryAcquire ()) {
		return XMI_SUCCESS;
	}
	return XMI_EAGAIN;
      }

      inline xmi_result_t unlock_impl ()
      {
        _lock.release ();
	return XMI_SUCCESS;
      }

      inline xmi_result_t send_impl (xmi_send_t * parameters)
      {
        size_t id = (size_t)(parameters->send.dispatch);
        TRACE_ERR((stderr, ">> send_impl('simple'), _dispatch[%zd] = %p\n", id, _dispatch[id]));
        XMI_assert_debug (_dispatch[id] != NULL);

        XMI::Protocol::Send::Send * send =
          (XMI::Protocol::Send::Send *) _dispatch[id];
        send->simple (parameters);

        TRACE_ERR((stderr, "<< send_impl('simple')\n"));
        return XMI_SUCCESS;
      }

      inline xmi_result_t send_impl (xmi_send_immediate_t * parameters)
      {
        size_t id = (size_t)(parameters->dispatch);
        TRACE_ERR((stderr, ">> send_impl('immediate'), _dispatch[%zd] = %p\n", id, _dispatch[id]));
        XMI_assert_debug (_dispatch[id] != NULL);

        XMI::Protocol::Send::Send * send =
          (XMI::Protocol::Send::Send *) _dispatch[id];
        send->immediate (parameters);

        TRACE_ERR((stderr, "<< send_impl('immediate')\n"));
        return XMI_SUCCESS;
      }

      inline xmi_result_t send_impl (xmi_send_typed_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t put (xmi_put_simple_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t put_typed (xmi_put_typed_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t get (xmi_get_simple_t * parameters)
      {

        return XMI_UNIMPL;
      }

      inline xmi_result_t get_typed (xmi_get_typed_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t rmw (xmi_rmw_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t memregion_register (void            * address,
                                              size_t            bytes,
                                              xmi_memregion_t * memregion)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t memregion_deregister (xmi_memregion_t * memregion)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t memregion_query (xmi_memregion_t    memregion,
                                           void            ** address,
                                           size_t           * bytes,
                                           size_t           * task)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t rput (xmi_rput_simple_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t rput_typed (xmi_rput_typed_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t rget (xmi_rget_simple_t * parameters)
      {
        ((GetShmem*)_get)->getimpl (	parameters->rma.done_fn,
                                parameters->rma.cookie,
                                parameters->rma.dest,
                                parameters->rget.bytes,
                                (Memregion*)parameters->rget.local_mr,
                                (Memregion*)parameters->rget.remote_mr,
                                parameters->rget.local_offset,
                                parameters->rget.remote_offset);
        return XMI_SUCCESS;
      }

      inline xmi_result_t rget_typed (xmi_rget_typed_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t purge_totask (size_t *dest, size_t count)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t resume_totask (size_t *dest, size_t count)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t fence_begin ()
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t fence_end ()
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t fence_all (xmi_event_function   done_fn,
                                     void               * cookie)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t fence_task (xmi_event_function   done_fn,
                                      void               * cookie,
                                      size_t               task)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t geometry_initialize (xmi_geometry_t       * geometry,
                                               unsigned               id,
                                               xmi_geometry_range_t * rank_slices,
                                               size_t                 slice_count)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t geometry_world (xmi_geometry_t * world_geometry)
      {
        return XMI_UNIMPL;
      }


      inline xmi_result_t geometry_finalize (xmi_geometry_t geometry)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t collective (xmi_xfer_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t geometry_algorithms_num_impl (xmi_geometry_t geometry,
                                                        xmi_xfer_type_t ctype,
                                                        int *lists_lengths)
      {
        return XMI_UNIMPL;
      }

      inline
      xmi_result_t geometry_algorithms_info_impl (xmi_geometry_t geometry,
                                                  xmi_xfer_type_t colltype,
                                                  xmi_algorithm_t *algs,
                                                  xmi_metadata_t *mdata,
                                                  int algorithm_type,
                                                  int num)
        {
          return XMI_UNIMPL;

        }

      inline xmi_result_t dispatch_impl (size_t                     id,
                                         xmi_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         xmi_send_hint_t            options)
      {
        xmi_result_t result = XMI_ERROR;

        if (_dispatch[id] == NULL)
          {
            // Allocate memory for the protocol object.
            _dispatch[id] = (void *) _protocolAllocator.allocateObject ();

#ifdef MU_DEVICE
            new ((void *)_dispatch[id]) EagerMu (id, fn, cookie, _mu, result);
#else
            new ((void *)_dispatch[id]) EagerShmem (id, fn, cookie, _shmem, result);
#endif
          }

        return result;
      }

    inline xmi_result_t dispatch_new_impl (size_t                     id,
                                           xmi_dispatch_callback_fn   fn,
                                           void                     * cookie,
                                           xmi_dispatch_hint_t        options)
    {
      xmi_result_t result        = XMI_ERROR;
      if(options.type == XMI_P2P_SEND)
      {
        return dispatch_impl (id,
                              fn,
                              cookie,
                              options.hint.send);
      }
        return result;
    }

      inline xmi_result_t multisend_getroles(size_t          dispatch,
                                             int            *numRoles,
                                             int            *replRole)
      {
        return XMI_UNIMPL;
      };

      inline xmi_result_t multicast(xmi_multicast_t *mcastinfo)
      {
        return XMI_UNIMPL;
      };


      inline xmi_result_t manytomany(xmi_manytomany_t *m2minfo)
      {
        return XMI_UNIMPL;
      };


      inline xmi_result_t multisync(xmi_multisync_t *msyncinfo)
      {
        return XMI_UNIMPL;
      };


      inline xmi_result_t multicombine(xmi_multicombine_t *mcombineinfo)
      {
        return XMI_UNIMPL;
      };



    private:

      xmi_client_t  _client;
      xmi_context_t _context;
      size_t        _clientid;
      size_t        _contextid;

      XMI::Memory::MemoryManager _mm;
      SysDep _sysdep;

      // devices...
      XMI::Device::Generic::Device &_generic;
#ifdef MU_DEVICE
      Device::MU::MUDevice _mu;
#endif
      ShmemDevice          _shmem;

      void * _dispatch[1024];
      void* _get; //use for now..remove later
      MemoryAllocator<1024, 16> _request;
      ContextLock _lock;
      MemoryAllocator<XMI::Device::ProgressFunctionMdl::sizeof_msg, 16> _workAllocator;
      ProtocolAllocator _protocolAllocator;
  }; // end XMI::Context
}; // end namespace XMI

#endif // __components_context_bgq_bgqcontext_h__
