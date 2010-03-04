///
/// \file common/bgp/Context.h
/// \brief XMI BGP specific context implementation.
///
#ifndef __common_bgp_Context_h__
#define __common_bgp_Context_h__

#include <stdlib.h>
#include <string.h>

#include "sys/xmi.h"
#include "common/ContextInterface.h"

#include "components/devices/generic/GenericDevice.h"

#warning shmem device must become sub-device of generic device
#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemModel.h"
#include "components/devices/shmem/ShmemMessage.h"
#include "util/fifo/FifoPacket.h"
#include "util/fifo/LinearFifo.h"

#include "components/atomic/bgp/BgpAtomic.h"
#include "components/atomic/bgp/LockBoxCounter.h"
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"

#include "components/memory/MemoryAllocator.h"
#include "components/memory/MemoryManager.h"

#include "SysDep.h"
#include "Memregion.h"

#include "p2p/protocols/Send.h"
#include "p2p/protocols/send/eager/Eager.h"
//#include "p2p/protocols/send/adaptive/Adaptive.h"
//#include "p2p/protocols/send/datagram/Datagram.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
  typedef XMI::Mutex::CounterMutex<XMI::Counter::GccProcCounter>  ContextLock;

  typedef Fifo::FifoPacket <16, 256> ShmemPacket;
  typedef Fifo::LinearFifo<Counter::LockBoxProcCounter, ShmemPacket, 128> ShmemFifo;
  //typedef Fifo::LinearFifo<Atomic::GccBuiltin, ShmemPacket, 128> ShmemFifo;
  typedef Device::ShmemDevice<ShmemFifo> ShmemDevice;
  typedef Device::ShmemModel<ShmemDevice> ShmemModel;

  //
  // >> Point-to-point protocol typedefs and dispatch registration.
  //typedef XMI::Protocol::Send::Eager <ShmemModel, ShmemDevice> EagerShmem;
  // << Point-to-point protocol typedefs and dispatch registration.
  //

  typedef MemoryAllocator<1024, 16> ProtocolAllocator;


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
          _shmem (),
          _lock (),
	  _workAllocator ()
      {
        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        // Make sure the memory allocator is large enough for all
        // protocol classes.
        //COMPILE_TIME_ASSERT(sizeof(EagerShmem) <= ProtocolAllocator::objsize);

        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        _lock.init(&_sysdep);

        _generic.init (_sysdep, (xmi_context_t)this, clientid, id, num, generics);
        _shmem.init (&_sysdep, _context, _contextid);

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
	work->setClient(_clientid);	// need client ID here, too
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
            events += _shmem.advance();
            events += _generic.advance();
          }

        //if (events > 0) result = XMI_SUCCESS;

        return events;
      }

      inline xmi_result_t lock_impl ()
      {
        TRACE_ERR((stderr, ">> lock_impl()\n"));
        _lock.acquire ();
        TRACE_ERR((stderr, "<< lock_impl()\n"));
        return XMI_SUCCESS;
      }

      inline xmi_result_t trylock_impl ()
      {
        TRACE_ERR((stderr, ">> trylock_impl()\n"));

        if (_lock.tryAcquire ())
          {
            TRACE_ERR((stderr, "<< trylock_impl(), XMI_SUCCESS\n"));
            return XMI_SUCCESS;
          }

        TRACE_ERR((stderr, "<< trylock_impl(), XMI_EAGAIN\n"));
        return XMI_EAGAIN;
      }

      inline xmi_result_t unlock_impl ()
      {
        TRACE_ERR((stderr, ">> release_impl()\n"));
        _lock.release ();
        TRACE_ERR((stderr, "<< release_impl()\n"));
        return XMI_SUCCESS;
      }

      inline xmi_result_t send_impl (xmi_send_t * parameters)
      {
        size_t id = (size_t)(parameters->send.dispatch);
        TRACE_ERR((stderr, ">> send_impl('simple'), _dispatch[%zd] = %p\n", id, _dispatch[id]));
        XMI_assert_debug (_dispatch[id] != NULL);

        XMI::Protocol::Send::Send * send =
          (XMI::Protocol::Send::Send *) _dispatch[id];
        xmi_result_t result = send->simple (parameters);

        TRACE_ERR((stderr, "<< send_impl('simple')\n"));
        return result;
      }

      inline xmi_result_t send_impl (xmi_send_immediate_t * parameters)
      {
        size_t id = (size_t)(parameters->dispatch);
        TRACE_ERR((stderr, ">> send_impl('immediate'), _dispatch[%zd] = %p\n", id, _dispatch[id]));
        XMI_assert_debug (_dispatch[id] != NULL);

        XMI::Protocol::Send::Send * send =
          (XMI::Protocol::Send::Send *) _dispatch[id];
        xmi_result_t result = send->immediate (parameters);

        TRACE_ERR((stderr, "<< send_impl('immediate')\n"));
        return result;
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
        return XMI_UNIMPL;
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

      inline xmi_result_t geometry_algorithms_num_impl (xmi_geometry_t geometry,
                                                        xmi_xfer_type_t ctype,
                                                        int *lists_lengths)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t geometry_algorithms_info_impl (xmi_geometry_t geometry,
                                                           xmi_xfer_type_t colltype,
                                                       xmi_algorithm_t  *algs0,
                                                       xmi_metadata_t   *mdata0,
                                                       int               num0,
                                                       xmi_algorithm_t  *algs1,
                                                       xmi_metadata_t   *mdata1,
                                                       int               num1)
      {
	XMI_abort();
	return XMI_SUCCESS;
      }

      inline xmi_result_t collective (xmi_xfer_t * parameters)
      {
        return XMI_UNIMPL;
      }

    inline xmi_result_t amcollective_dispatch_impl (xmi_algorithm_t            algorithm,
                                                    size_t                     dispatch,
                                                    xmi_dispatch_callback_fn   fn,
                                                    void                     * cookie,
                                                    xmi_collective_hint_t      options)
      {
	XMI_abort();
	return XMI_SUCCESS;
      }


      inline xmi_result_t dispatch_impl (size_t                     id,
                                         xmi_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         xmi_send_hint_t            options)
      {
        xmi_result_t result = XMI_ERROR;
        size_t index = (size_t) id;
        TRACE_ERR((stderr, ">> dispatch_impl(), _dispatch[%zd] = %p\n", index, _dispatch[index]));

        if (_dispatch[index] == NULL)
          {
            TRACE_ERR((stderr, "   dispatch_impl(), before protocol init\n"));

            if (options.no_long_header == 1)
              {
                _dispatch[id] = _protocol.allocateObject ();
                new (_dispatch[id])
//                Protocol::Send::Datagram <ShmemModel, ShmemDevice, false>
//                Protocol::Send::Adaptive <ShmemModel, ShmemDevice, false>
                Protocol::Send::Eager <ShmemModel, ShmemDevice, false>
                (id, fn, cookie, _shmem, result);
              }
            else
              {
                _dispatch[id] = _protocol.allocateObject ();
                new (_dispatch[id])
                Protocol::Send::Eager <ShmemModel, ShmemDevice, true>
//                Protocol::Send::Adaptive <ShmemModel, ShmemDevice, true>
//                Protocol::Send::Datagram <ShmemModel, ShmemDevice, true>
                (id, fn, cookie, _shmem, result);
              }

            TRACE_ERR((stderr, "   dispatch_impl(),  after protocol init, result = %zd\n", result));

            if (result != XMI_SUCCESS)
              {
                _protocol.returnObject (_dispatch[id]);
                _dispatch[id] = NULL;
              }
          }

        TRACE_ERR((stderr, "<< dispatch_impl(), result = %zd, _dispatch[%zd] = %p\n", result, index, _dispatch[index]));
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
          TRACE_ERR((stderr, "<< dispatch_new_impl(), result = %zd, _dispatch[%zd] = %p\n", result, index, _dispatch[index]));
          return result;
      }

      inline xmi_result_t multisend_getroles_impl(size_t          dispatch,
                                                  int            *numRoles,
                                                  int            *replRole)
      {
        return XMI_UNIMPL;
      };

      inline xmi_result_t multicast_impl(xmi_multicast_t *mcastinfo)
      {
        return XMI_UNIMPL;
      };


      inline xmi_result_t manytomany_impl(xmi_manytomany_t *m2minfo)
      {
        return XMI_UNIMPL;
      };


      inline xmi_result_t multisync_impl(xmi_multisync_t *msyncinfo)
      {
        return XMI_UNIMPL;
      };


      inline xmi_result_t multicombine_impl(xmi_multicombine_t *mcombineinfo)
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
      ShmemDevice _shmem;
      ContextLock _lock;

      void * _dispatch[1024];
      ProtocolAllocator _protocol;
      MemoryAllocator<XMI::Device::ProgressFunctionMdl::sizeof_msg, 16> _workAllocator;

  }; // end XMI::Context
}; // end namespace XMI

#undef TRACE_ERR

#endif // __components_context_bgp_bgpcontext_h__
