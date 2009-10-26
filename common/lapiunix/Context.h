///
/// \file common/lapiunix/Context.h
/// \brief XMI LAPI specific context implementation.
///
#ifndef __common_lapiunix_Context_h__
#define __common_lapiunix_Context_h__

#define XMI_CONTEXT_CLASS XMI::Context

#include <stdlib.h>
#include <string.h>
#include "util/lapi/lapi_util.h"
#include "common/ContextInterface.h"
#include "Geometry.h"
#include "components/devices/lapiunix/lapiunixdevice.h"
#include "components/devices/lapiunix/lapiunixmodel.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/eager/EagerSimple.h"
#include "p2p/protocols/send/eager/EagerImmediate.h"
#include "SysDep.h"
#include "components/geometry/lapiunix/lapiunixcollfactory.h"
#include "components/geometry/lapiunix/lapiunixcollregistration.h"
#include "Mapping.h"
#include <new>
#include <map>

namespace XMI
{
    typedef Device::LAPIMessage LAPIMessage;
    typedef Device::LAPIDevice<SysDep> LAPIDevice;
    typedef Device::LAPIModel<LAPIDevice,LAPIMessage> LAPIModel;
    typedef Geometry::Common<XMI_MAPPING_CLASS> LAPIGeometry;
    typedef CollFactory::LAPI<LAPIDevice, SysDep> LAPICollfactory;
    typedef CollRegistration::LAPI<LAPIGeometry, LAPICollfactory, LAPIDevice, SysDep> LAPICollreg;
    typedef XMI::Protocol::Send::Eager <LAPIModel,LAPIDevice> EagerLAPI;

    class Context : public Interface::Context<XMI::Context>
    {
    public:
      inline Context (xmi_client_t client, size_t id, void * addr, size_t bytes) :
        Interface::Context<XMI::Context> (client, id),
        _client (client),
        _id (id),
        _sysdep(_mm)
        {
          lapi_info_t   * lapi_info;     /* used as argument to LAPI_Init */
          lapi_extend_t * extend_info;   /* holds IP addresses and ports */
          lapi_udp_t    * udp_info;      /* List of ip, port info to pass to LAPI */
          int             num_tasks;     /* number of tasks (from LAPI_LIST_NAME) */
          char          * list_name;     /* name of UDP host/port list file */
          FILE          * fp;
          int             i;

          /* ------------------------------------------------------------ */
          /*        initialize request allocation pool                    */
          /* ------------------------------------------------------------ */
//	    __pgasrt_lapi_pool_init();

          /* ------------------------------------------------------------ */
          /*       allocate and initialize lapi_info                      */
          /* ------------------------------------------------------------ */

          CHECK_NULL(lapi_info,(lapi_info_t *)malloc(sizeof(lapi_info_t)));
          memset(lapi_info, 0, sizeof(lapi_info_t));

          /* ------------------------------------------------------------ */
          /* collect UDP hostnames and ports into udp_info data structure */
          /* ------------------------------------------------------------ */

          udp_info = NULL;
          list_name=getenv("LAPI_LIST_NAME");
          if (list_name)
              {
                if ((fp = fopen (list_name, "r")) == NULL) {
                  printf ("Cannot find LAPI_LIST_NAME\n");
                  abort();
                }
                fscanf(fp, "%u", &num_tasks);
                CHECK_NULL(udp_info,(lapi_udp_t *) malloc(num_tasks*sizeof(lapi_udp_t)));
                for (i = 0; i < num_tasks; i++)
                    {
                      char ip[256];
                      unsigned port;
                      fscanf(fp, "%s %u", ip, &port);
                      udp_info[i].ip_addr = inet_addr(ip);
                      udp_info[i].port_no = port;
                    }
                /* ------------------------------------------------------------ */
                /*        link up udp_info, extend_info and lapi_info           */
                /* ------------------------------------------------------------ */
                CHECK_NULL(extend_info,(lapi_extend_t *)malloc(sizeof(lapi_extend_t)));
                memset(extend_info, 0, sizeof(lapi_extend_t));
                extend_info->add_udp_addrs = udp_info;
                extend_info->num_udp_addr  = num_tasks;
                extend_info->udp_hndlr     = 0;
                lapi_info->add_info        = extend_info;
              }
          else
              {
                lapi_info->add_info        = NULL;
              }
          /* ------------------------------------------------------------ */
          /*                call LAPI_Init                                */
          /* ------------------------------------------------------------ */
          int intval = 0;
          CALL_AND_CHECK_RC((LAPI_Init(&_lapi_handle, lapi_info)));
          CALL_AND_CHECK_RC((LAPI_Senv(_lapi_handle,INTERRUPT_SET, intval)));
          CALL_AND_CHECK_RC((LAPI_Qenv(_lapi_handle,TASK_ID,
                                       (int *)&_myrank)));
          CALL_AND_CHECK_RC((LAPI_Qenv(_lapi_handle,NUM_TASKS,
                                       (int *)&_mysize)));
          free(lapi_info);

          _lapi_device.init(&_sysdep);
          _lapi_device.setLapiHandle(_lapi_handle);
          __global.mapping.init(_myrank, _mysize);

          _world_geometry=(LAPIGeometry*) malloc(sizeof(*_world_geometry));
          _world_range.lo=0;
          _world_range.hi=_mysize-1;
          new(_world_geometry) LAPIGeometry(&__global.mapping,0, 1,&_world_range);

          _collreg=(LAPICollreg*) malloc(sizeof(*_collreg));
          new(_collreg) LAPICollreg(&_lapi_device, &_sysdep);

          _world_collfactory=_collreg->analyze(_world_geometry);
          _world_geometry->setKey(XMI::Geometry::XMI_GKEY_COLLFACTORY, _world_collfactory);

          CALL_AND_CHECK_RC((LAPI_Gfence (_lapi_handle)));

        }

      inline xmi_client_t getClient_impl ()
        {
          return _client;
        }

      inline size_t getId_impl ()
        {
          return _id;
        }


      inline xmi_result_t destroy_impl ()
        {
          LAPI_Gfence (_lapi_handle);
          CALL_AND_CHECK_RC((LAPI_Term(_lapi_handle)));
          return XMI_SUCCESS;
        }

      inline xmi_result_t queryConfiguration_impl (xmi_configuration_t * configuration)
        {
          xmi_result_t result = XMI_ERROR;

          switch (configuration->name)
              {
                  case XMI_TASK_ID:
                    configuration->value.intval = __global.mapping.task();
                    result = XMI_SUCCESS;
                    break;
                  case XMI_NUM_TASKS:
                    configuration->value.intval = __global.mapping.size();
                    result = XMI_SUCCESS;
                    break;
                  default:
                    break;
              };

          return result;
        }

      inline xmi_result_t post_impl (xmi_event_function work_fn, void * cookie)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline size_t advance_impl (size_t maximum, xmi_result_t & result)
        {
          result = XMI_SUCCESS;
          size_t events = 0;
          unsigned i;
          for (i=0; i<maximum && events==0; i++)
              {
                events += _lapi_device.advance_impl();
              }
          return events;
        }

      inline xmi_result_t lock_impl ()
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t trylock_impl ()
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t unlock_impl ()
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t send_impl (xmi_send_simple_t * parameters)
        {
          size_t id = (size_t)(parameters->send.dispatch);
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
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t put_impl (xmi_put_simple_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t put_typed_impl (xmi_put_typed_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t get_impl (xmi_get_simple_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t get_typed_impl (xmi_get_typed_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t rmw_impl (xmi_rmw_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t memregion_register_impl (void            * address,
                                                   size_t            bytes,
                                                   xmi_memregion_t * memregion)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t memregion_deregister_impl (xmi_memregion_t * memregion)
        {
          assert(0);
          return XMI_UNIMPL;
        }


      inline xmi_result_t memregion_query_impl (xmi_memregion_t    memregion,
                                                void            ** address,
                                                size_t           * bytes,
                                                size_t           * task)
        {
          assert(0);
          return XMI_UNIMPL;
        }


      inline xmi_result_t memregion_register_impl (xmi_rmw_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t rput_impl (xmi_rput_simple_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t rput_typed_impl (xmi_rput_typed_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t rget_impl (xmi_rget_simple_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t rget_typed_impl (xmi_rget_typed_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t purge_totask_impl (size_t * dest, size_t count)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t resume_totask_impl (size_t * dest, size_t count)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t fence_begin_impl ()
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t fence_end_impl ()
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t fence_all_impl (xmi_event_function   done_fn,
                                          void               * cookie)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline  xmi_result_t fence_task_impl (xmi_event_function   done_fn,
                                            void               * cookie,
                                            size_t               task)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t geometry_initialize_impl (xmi_geometry_t       * geometry,
                                                    unsigned               id,
                                                    xmi_geometry_range_t * rank_slices,
                                                    size_t                 slice_count)
        {
	  LAPIGeometry              *new_geometry;
	  LAPICollfactory           *new_collfactory;
          new_geometry=(LAPIGeometry*) malloc(sizeof(*new_geometry));
          new(new_geometry) LAPIGeometry(&__global.mapping,id, slice_count,rank_slices);
          new_collfactory=_collreg->analyze(new_geometry);
	  new_geometry->setKey(XMI::Geometry::XMI_GKEY_COLLFACTORY, new_collfactory);
	  *geometry=(LAPIGeometry*) new_geometry;
          return XMI_SUCCESS;
        }


      inline xmi_result_t geometry_world_impl (xmi_geometry_t * world_geometry)
        {
	  *world_geometry = _world_geometry;
          return XMI_SUCCESS;
        }

      inline xmi_result_t geometry_algorithm_impl (xmi_xfer_type_t  colltype,
						   xmi_geometry_t   geometry,
                                                   xmi_algorithm_t *algorithm,
                                                   int             *num)
        {
	  LAPICollfactory           *collfactory;
	  LAPIGeometry              *new_geometry = (LAPIGeometry*) geometry;
	  collfactory =(LAPICollfactory*) new_geometry->getKey(XMI::Geometry::XMI_GKEY_COLLFACTORY);
	  return collfactory->algorithm(colltype,algorithm,num);
        }

      inline xmi_result_t geometry_finalize_impl (xmi_geometry_t geometry)
        {
          assert(0);
          return XMI_UNIMPL;
        }


      inline xmi_result_t collective_impl (xmi_xfer_t * parameters)
        {
	  LAPICollfactory           *collfactory;
	  // This is ok...we can avoid a switch because all the xmi structs
	  // have the same layout.let's just use barrier for now
	  LAPIGeometry              *new_geometry = (LAPIGeometry*)parameters->xfer_barrier.geometry;
	  collfactory =(LAPICollfactory*) new_geometry->getKey(XMI::Geometry::XMI_GKEY_COLLFACTORY);
          return collfactory->collective(parameters);
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

      inline xmi_result_t multisend_getroles_impl(size_t          dispatch,
                                                  int            *numRoles,
                                                  int            *replRole)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t multicast_impl(xmi_multicast_t *mcastinfo)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t manytomany_impl(xmi_manytomany_t *m2minfo)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t multisync_impl(xmi_multisync_t *msyncinfo)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t multicombine_impl(xmi_multicombine_t *mcombineinfo)
        {
          assert(0);
          return XMI_UNIMPL;
        }
      inline xmi_result_t dispatch_impl (size_t                     id,
                                         xmi_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         xmi_send_hint_t            options)
        {
          if (_dispatch[(size_t)id] != NULL) return XMI_ERROR;
          _dispatch[(size_t)id]      = (void *) _request.allocateObject ();
          xmi_result_t result        = XMI_ERROR;
          new (_dispatch[(size_t)id]) EagerLAPI (id, fn, cookie, _lapi_device,
                                                 __global.mapping.task(), _context,
                                                 _id, result);
          return result;
        }

    private:
      std::map <unsigned, xmi_geometry_t>   _geometry_id;
      Memory::MemoryManager     _mm;
      xmi_client_t              _client;
      xmi_context_t             _context;
      size_t                    _id;
      SysDep                    _sysdep;
      lapi_handle_t             _lapi_handle;
      void                     *_dispatch[1024];
      MemoryAllocator<1024,16>  _request;
      LAPIDevice                _lapi_device;
      LAPICollreg              *_collreg;
      LAPIGeometry             *_world_geometry;
      LAPICollfactory          *_world_collfactory;
      xmi_geometry_range_t      _world_range;
      int                       _myrank;
      int                       _mysize;
      unsigned                 *_ranklist;

    }; // end XMI::Context
}; // end namespace XMI

#endif // __xmi_lapi_lapicontext_h__
