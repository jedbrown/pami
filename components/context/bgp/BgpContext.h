///
/// \file components/context/bgp/BgpContext.h
/// \brief XMI BGP specific context implementation.
///
#ifndef   __components_context_bgp_bgpcontext_h__
#define   __components_context_bgp_bgpcontext_h__

#define XMI_CONTEXT_CLASS XMI::Context::BgpContext

#include <stdlib.h>
#include <string.h>

#include "sys/xmi.h"
#include "components/context/Context.h"

//#define ENABLE_GENERIC_DEVICE
#ifdef ENABLE_GENERIC_DEVICE
#include "components/devices/generic/GenericDevice.h"
#endif

#warning shmem device must become sub-device of generic device
#include "components/devices/shmem/ShmemPacketDevice.h"
#include "components/devices/shmem/ShmemPacketModel.h"
#include "components/devices/shmem/ShmemBaseMessage.h"
#include "util/fifo/FifoPacket.h"
#include "util/fifo/LinearFifo.h"

#include "components/atomic/bgp/BgpAtomic.h"
#include "components/atomic/bgp/LockBoxCounter.h"

#include "components/memory/MemoryAllocator.h"

#include "components/sysdep/bgp/BgpSysDep.h"

#include "p2p/protocols/Send.h"
#include "p2p/protocols/send/eager/Eager.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
  namespace Context
  {
    typedef Fifo::FifoPacket <16,240> ShmemPacket;
#ifdef NOT_YET
    typedef Fifo::LinearFifo<Counter::LockBoxProcCounter<XMI::SysDep::BgpSysDep>,ShmemPacket,128> ShmemFifo;
#else
    typedef Fifo::LinearFifo<Atomic::BgpAtomic<XMI::SysDep::BgpSysDep>,ShmemPacket,128> ShmemFifo;
#endif

    typedef Device::ShmemBaseMessage<ShmemPacket> ShmemMessage;
    typedef Device::ShmemPacketDevice<SysDep::BgpSysDep,ShmemFifo,ShmemPacket> ShmemDevice;
    typedef Device::ShmemPacketModel<ShmemDevice,ShmemMessage> ShmemModel;



    //
    // >> Point-to-point protocol typedefs and dispatch registration.
    typedef XMI::Protocol::Send::Eager <ShmemModel, ShmemDevice, ShmemMessage> EagerShmem;
    // << Point-to-point protocol typedefs and dispatch registration.
    //

    typedef MemoryAllocator<1024,16> ProtocolAllocator;

    class BgpContext : public Context<XMI::Context::BgpContext>
    {
      public:
        inline BgpContext (xmi_client_t client, size_t id) :
          Context<XMI::Context::BgpContext> (client, id),
          _client (client),
          _context ((xmi_context_t)this),
          _contextid (id),
          _sysdep (),
#ifdef ENABLE_GENERIC_DEVICE
          _generic(_sysdep),
#endif
          _shmem ()
        {
#ifdef ENABLE_GENERIC_DEVICE
          _generic.init (_sysdep);
#endif
          _shmem.init (&_sysdep);
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

        inline xmi_result_t queryConfiguration_impl (xmi_configuration_t * configuration)
        {
          xmi_result_t result = XMI_ERROR;

          switch (configuration->name)
          {
            case XMI_TASK_ID:
              configuration->value.intval = _sysdep.mapping.task();
              result = XMI_SUCCESS;
              break;
            case XMI_NUM_TASKS:
              configuration->value.intval = _sysdep.mapping.size();
              result = XMI_SUCCESS;
              break;
            default:
              break;
          };

          return result;
        };

        inline xmi_result_t post_impl (xmi_event_function work_fn, void * cookie)
        {
          return XMI_UNIMPL;
        }

        inline size_t advance_impl (size_t maximum, xmi_result_t & result)
        {
//          result = XMI_EAGAIN;
          result = XMI_SUCCESS;
          size_t events = 0;
          unsigned i;
          for (i=0; i<maximum && events==0; i++)
          {
            events += _shmem.advance_impl();
#ifdef ENABLE_GENERIC_DEVICE
            events += _generic.advance();
#endif
          }
          //if (events > 0) result = XMI_SUCCESS;

          return events;
        }

        inline xmi_result_t lock_impl ()
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t trylock_impl ()
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t unlock_impl ()
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t send_impl (xmi_send_simple_t * parameters)
        {
          size_t id = (size_t)(parameters->send.dispatch);
          TRACE_ERR((stderr, ">> send_impl('simple'), _dispatch[%zd] = %p\n", id, _dispatch[id]));
          XMI_assert_debug (_dispatch[id] != NULL);

          XMI::Protocol::Send::Send * send =
            (XMI::Protocol::Send::Send *) _dispatch[id];
          send->simple (parameters->simple.local_fn,
                        parameters->simple.remote_fn,
                        parameters->send.cookie,
                        parameters->send.task,
                        parameters->simple.addr,
                        parameters->simple.bytes,
                        parameters->send.header.addr,
                        parameters->send.header.bytes);

          TRACE_ERR((stderr, "<< send_impl('simple')\n"));
          return XMI_SUCCESS;
        }

        inline xmi_result_t send_impl (xmi_send_immediate_t * parameters)
        {
          size_t id = (size_t)(parameters->send.dispatch);
          TRACE_ERR((stderr, ">> send_impl('immediate'), _dispatch[%zd] = %p\n", id, _dispatch[id]));
          XMI_assert_debug (_dispatch[id] != NULL);

          XMI::Protocol::Send::Send * send =
            (XMI::Protocol::Send::Send *) _dispatch[id];
          send->immediate (parameters->send.task,
                           parameters->immediate.addr,
                           parameters->immediate.bytes,
                           parameters->send.header.addr,
                           parameters->send.header.bytes);

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
                                                 unsigned               slice_count)
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t geometry_world (xmi_geometry_t * world_geometry)
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t geometry_algorithm (xmi_xfer_type_t  colltype,
                                                xmi_geometry_t   geometry,
                                                xmi_algorithm_t *algorithm,
                                                int             *num)
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t geometry_finalize (xmi_geometry_t geometry)
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t geometry_algorithms_num_impl (xmi_context_t context,
                                                          xmi_geometry_t geometry,
                                                          xmi_xfer_type_t ctype,
                                                          int *lists_lengths)
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t geometry_algorithm_info_impl (xmi_context_t context,
                                                          xmi_geometry_t geometry,
                                                          xmi_xfer_type_t type,
                                                          xmi_algorithm_t algorithm,
                                                          int algorithm_type,
                                                          xmi_metadata_t *mdata)
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t collective (xmi_xfer_t * parameters)
        {
          return XMI_UNIMPL;
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
            _dispatch[id] = _protocol.allocateObject ();
            TRACE_ERR((stderr, "   dispatch_impl(), before protocol init\n"));
            new (_dispatch[id]) EagerShmem (id, fn, cookie, _shmem,
                                            _sysdep.mapping.task(),
                                            _context, _contextid, result);
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
        size_t        _contextid;

        SysDep::BgpSysDep _sysdep;

        // devices...
#ifdef ENABLE_GENERIC_DEVICE
        XMI::Device::Generic::Device _generic;
#endif
        ShmemDevice _shmem;

        void * _dispatch[1024];
        ProtocolAllocator _protocol;

        static inline void compile_time_assert ()
        {
          // Make sure the memory allocator is large enough for all
          // protocol classes.
          COMPILE_TIME_ASSERT(sizeof(EagerShmem) <= ProtocolAllocator::objsize);
        };

    }; // end XMI::Context::BgpContext
  }; // end namespace Context
}; // end namespace XMI

#undef TRACE_ERR
#endif // __components_context_bgp_bgpcontext_h__
