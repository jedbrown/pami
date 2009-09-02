///
/// \file components/context/bgp/BgpContext.h
/// \brief XMI BGP specific context implementation.
///
#ifndef   __components_context_bgp_bgpcontext_h__
#define   __components_context_bgp_bgpcontext_h__

#include <stdlib.h>
#include <string.h>

#include "sys/xmi.h"
#include "../Context.h"

#include "components/devices/shmem/ShmemPacketDevice.h"
#include "components/devices/shmem/ShmemPacketModel.h"
#include "components/devices/shmem/ShmemBaseMessage.h"
#include "components/devices/shmem/fifo/FifoPacket.h"
#include "components/devices/shmem/fifo/LinearFifo.h"

#include "components/atomic/gcc/GccBuiltin.h"

#include "components/memory/MemoryAllocator.h"

#include "components/sysdep/bgp/BgpSysDep.h"

#include "p2p/protocols/send/eager/EagerSimple.h"


#define XMI_CONTEXT_CLASS XMI::Context::BgpContext

namespace XMI
{
  namespace Context
  {
    typedef Device::Fifo::FifoPacket <16,240> ShmemPacket;
    typedef Device::Fifo::LinearFifo<Atomic::GccBuiltin,ShmemPacket,16> ShmemFifo;

    typedef Device::ShmemBaseMessage<ShmemPacket> ShmemMessage;
    typedef Device::ShmemPacketDevice<SysDep::BgpSysDep,ShmemFifo,ShmemPacket> ShmemDevice;
    typedef Device::ShmemPacketModel<ShmemDevice,ShmemMessage> ShmemModel;

    class BgpContext : public Context<XMI::Context::BgpContext>
    {
      public:
        inline BgpContext (xmi_client_t client) :
          Context<XMI::Context::BgpContext> (client),
          _client (client),
          _sysdep (),
          _shmem ()
        {
          _shmem.init (&_sysdep);
        }

        inline xmi_client_t getClientId_impl ()
        {
          return _client;
        }

        inline xmi_result_t destroy_impl ()
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t post_impl (xmi_event_function work_fn, void * cookie)
        {
          return XMI_UNIMPL;
        }

        inline size_t advance_impl (size_t maximum, xmi_result_t & result)
        {
          result = XMI_ERROR;

          size_t events = 0;
          unsigned i;
          for (i=0; i<maximum && events==0; i++)
          {
            events += _shmem.advance();
          }

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
#warning implement this!
#if 0
          assert (_dispatch[parameters->send.dispatch] != NULL);

          XMI::Protocol::Send::Simple * send =
            (XMI::Protocol::Send::Simple *) _dispatch[parameters->send.dispatch];
          send->start (parameters->simple.local_fn,
                       parameters->simple.remote_fn,
                       parameters->send.cookie,
                       parameters->send.task,
                       parameters->simple.addr,
                       parameters->simple.bytes,
                       parameters->send.header.addr,
                       parameters->send.header.bytes);
#endif
          return XMI_SUCCESS;
        }

        inline xmi_result_t send_impl (xmi_send_immediate_t * parameters)
        {
          return XMI_UNIMPL;
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

        inline xmi_result_t geometry_algorithm (xmi_geometry_t   geometry,
                                                xmi_algorithm_t *algorithm,
                                                int             *num)
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



        inline xmi_result_t dispatch_impl (xmi_dispatch_t             id,
                                           xmi_dispatch_callback_fn   fn,
                                           void                     * cookie,
                                           xmi_send_hint_t            options)
        {
#warning implement this!
#if 0
          if (_dispatch[id] != NULL) return XMI_ERROR;

          // Allocate memory for the protocol object.
          _dispatch[id] = (void *) _request.allocateObject ();

          // For now, only enable shmem short/eager sends.
          typedef XMI::Protocol::Send::EagerSimple <ShmemModel, ShmemDevice, ShmemMessage> EagerSimpleShmem;
          
          xmi_result_t result;
          new (_dispatch[id]) EagerSimpleShmem (id, fn, cookie, _shmem, _sysdep.mapping.task(), result);

          return result;
#endif
          return XMI_UNIMPL;
        }

        inline xmi_result_t multisend_getroles(xmi_dispatch_t  dispatch,
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

        xmi_client_t _client;

        SysDep::BgpSysDep _sysdep;

        // devices...
        ShmemDevice _shmem;

        void * _dispatch[1024];
        MemoryAllocator<1024,16> _request;

    }; // end XMI::Context::BgpContext
  }; // end namespace Context
}; // end namespace XMI

#endif // __components_context_bgp_bgpcontext_h__
