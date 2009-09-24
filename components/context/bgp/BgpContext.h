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
#include "../Context.h"

#include "components/devices/shmem/ShmemPacketDevice.h"
#include "components/devices/shmem/ShmemPacketModel.h"
#include "components/devices/shmem/ShmemBaseMessage.h"
#include "util/fifo/FifoPacket.h"
#include "util/fifo/LinearFifo.h"

//#include "components/atomic/gcc/GccBuiltin.h"
//#include "components/atomic/pthread/Pthread.h"
#include "components/atomic/bgp/BgpAtomic.h"

#include "components/memory/MemoryAllocator.h"

#include "components/sysdep/bgp/BgpSysDep.h"

#include "p2p/protocols/send/eager/EagerImmediate.h"
#include "p2p/protocols/send/eager/EagerSimple.h"



namespace XMI
{
  namespace Context
  {
    typedef Fifo::FifoPacket <16,240> ShmemPacket;
    //typedef Device::Fifo::LinearFifo<Atomic::GccBuiltin,ShmemPacket,16> ShmemFifo;
    //typedef Device::Fifo::LinearFifo<Atomic::Pthread,ShmemPacket,16> ShmemFifo;
    typedef Fifo::LinearFifo<Atomic::BgpAtomic,ShmemPacket,16> ShmemFifo;

    typedef Device::ShmemBaseMessage<ShmemPacket> ShmemMessage;
    typedef Device::ShmemPacketDevice<SysDep::BgpSysDep,ShmemFifo,ShmemPacket> ShmemDevice;
    typedef Device::ShmemPacketModel<ShmemDevice,ShmemMessage> ShmemModel;



    //
    // >> Point-to-point protocol typedefs and dispatch registration.
    //
    typedef XMI::Protocol::Send::EagerSimple <ShmemModel, ShmemDevice, ShmemMessage> EagerSimpleShmem;
    typedef XMI::Protocol::Send::EagerImmediate <ShmemModel, ShmemDevice, ShmemMessage> EagerImmediateShmem;

    typedef struct
    {
      EagerSimpleShmem    send_simple_shmem;
      EagerImmediateShmem send_immediate_shmem;
    } protocol_t;
    //
    // << Point-to-point protocol typedefs and dispatch registration.
    //

    class BgpContext : public Context<XMI::Context::BgpContext>
    {
      public:
        inline BgpContext (xmi_client_t client) :
          Context<XMI::Context::BgpContext> (client),
          _client (client),
          _context ((xmi_context_t)this),
          _sysdep (),
          _shmem ()
        {
          //fprintf(stderr, "BgpContext() >>\n");
          XMI::Mapping::Interface::nodeaddr_t addr;
          _shmem.init (&_sysdep);
          //fprintf(stderr, "BgpContext() <<\n");
        }

        inline xmi_client_t getClientId_impl ()
        {
          return _client;
        }

        inline xmi_result_t destroy_impl ()
        {
          //return XMI_UNIMPL;
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
          assert (_dispatch[id] != NULL);

          _dispatch[id]->send_simple_shmem.start (parameters->simple.local_fn,
                                                     parameters->simple.remote_fn,
                                                     parameters->send.cookie,
                                                     parameters->send.task,
                                                     parameters->simple.addr,
                                                     parameters->simple.bytes,
                                                     parameters->send.header.addr,
                                                     parameters->send.header.bytes);

          return XMI_SUCCESS;
        }

        inline xmi_result_t send_impl (xmi_send_immediate_t * parameters)
        {
          size_t id = (size_t)(parameters->send.dispatch);
          assert (_dispatch[id] != NULL);

          _dispatch[id]->send_immediate_shmem.start (parameters->send.task,
                                                        parameters->immediate.addr,
                                                        parameters->immediate.bytes,
                                                        parameters->send.header.addr,
                                                        parameters->send.header.bytes);

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
          size_t index = (size_t) id;

          if (_dispatch[index] != NULL) return XMI_ERROR;

          // Allocate memory for the protocol objects.
          protocol_t * p = (protocol_t *) _protocol.allocateObject ();

          xmi_result_t result = XMI_ERROR;

          new ((void *)&(p->send_simple_shmem))
            EagerSimpleShmem (id, fn, cookie, _shmem, _sysdep.mapping.task(), _context, result);

          new ((void *)&(p->send_immediate_shmem))
            EagerImmediateShmem (id, fn, cookie, _shmem, _sysdep.mapping.task(), _context, result);

          _dispatch[index] = p;
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

        SysDep::BgpSysDep _sysdep;

        // devices...
        ShmemDevice _shmem;

        protocol_t * _dispatch[1024];
        MemoryAllocator<sizeof(protocol_t),16> _protocol;

    }; // end XMI::Context::BgpContext
  }; // end namespace Context
}; // end namespace XMI

#endif // __components_context_bgp_bgpcontext_h__
