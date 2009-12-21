///
/// \file common/socklinux/Context.h
/// \brief XMI Sockets on Linux specific context implementation.
///
#ifndef __common_socklinux_Context_h__
#define __common_socklinux_Context_h__

#include <stdlib.h>
#include <string.h>

#include "sys/xmi.h"
#include "common/ContextInterface.h"

//#include "components/devices/generic/GenericDevice.h"

#include "components/devices/udp/UdpDevice.h"
#include "components/devices/udp/UdpModel.h"
#include "components/devices/udp/UdpMessage.h"

#include "components/atomic/gcc/GccBuiltin.h"
//#include "components/atomic/pthread/Pthread.h"

#include "components/memory/MemoryAllocator.h"

#include "SysDep.h"
//#include "Memregion.h"

#include "p2p/protocols/send/datagram/Datagram.h"
#include "p2p/protocols/send/datagram/DatagramImmediate.h"
#include "p2p/protocols/send/datagram/DatagramSimple.h"

#include "p2p/protocols/get/Get.h"
#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
  //typedef XMI::Mutex::CounterMutex<XMI::Counter::GccProcCounter>  ContextLock;

  typedef Device::UDP::UdpDevice<SysDep> UdpDevice;
  typedef Device::UDP::UdpModel<UdpDevice,Device::UDP::UdpSendMessage> UdpModel;

  //
  // >> Point-to-point protocol typedefs and dispatch registration.
  typedef XMI::Protocol::Send::Datagram < UdpModel, UdpDevice > DatagramUdp;

  typedef MemoryAllocator<1152, 16> ProtocolAllocator;

  class Context : public Interface::Context<XMI::Context>
  {
	//static void __work_done(xmi_context_t ctx, void *cookie, xmi_result_t result) {
	//	XMI::Context *context = (XMI::Context *)ctx;
	//	context->_workAllocator.returnObject(cookie);
	//}
    public:
      inline Context (xmi_client_t client, size_t contextid) :
          Interface::Context<XMI::Context> (client, contextid),
          _client (client),
          _context ((xmi_context_t)this),
          _contextid (contextid),
          _mm (),
          _sysdep (_mm),
          _udp ()   //,
  	  //_workAllocator()
      {
        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        // Make sure the memory allocator is large enough for all
        // protocol classes.
        COMPILE_TIME_ASSERT(sizeof(DatagramUdp) <= ProtocolAllocator::objsize);

        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------
        _udp.init (&_sysdep);
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
	return XMI_UNIMPL;
      }

      inline size_t advance_impl (size_t maximum, xmi_result_t & result)
      {
//          result = XMI_EAGAIN;
        result = XMI_SUCCESS;
        size_t events = 0;
        unsigned i;

        for (i = 0; i < maximum && events == 0; i++)
          {
            //events += _shmem.advance_impl();
            events += _udp.advance();
	    //events += _generic.advance();
          }

        //if (events > 0) result = XMI_SUCCESS;

        return events;
      }

      inline xmi_result_t lock_impl ()
      {
        //_lock.acquire ();
	return XMI_SUCCESS;
      }

      inline xmi_result_t trylock_impl ()
      {
        //if (_lock.tryAcquire ()) {
		return XMI_SUCCESS;
	//}
	//return XMI_EAGAIN;
      }

      inline xmi_result_t unlock_impl ()
      {
        //_lock.release ();
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

            new ((void *)_dispatch[id]) DatagramUdp (id, fn, cookie, _udp, __global.mapping.task(), _context, _contextid, result);
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
      size_t        _contextid;

      XMI::Memory::MemoryManager _mm;  // TODO why do I have to do this for sys dep?
      SysDep _sysdep;

      // devices...
      UdpDevice _udp;

      void * _dispatch[1024];
      //void* _get; //use for now..remove later
      MemoryAllocator<1024, 16> _request;
      //ContextLock _lock;
      //MemoryAllocator<XMI::Device::ProgressFunctionMdl::sizeof_msg, 16> _workAllocator;
      ProtocolAllocator _protocolAllocator;

  }; // end XMI::Context
}; // end namespace XMI

#endif // __common_socklinux_Context_h__
