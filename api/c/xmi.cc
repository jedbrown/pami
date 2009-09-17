/**
 * \file api/c/xmi.cc
 * \brief XMI C interface common implementation
 */

#include "sys/xmi.h"

#include "config.h"
////////////////////////////////////////////////////////////////////////////////
// Functions from xmi_misc.h                                                  //
////////////////////////////////////////////////////////////////////////////////

XMI_TIME_CLASS XMI_Time;


///
/// \copydoc XMI_Error_text
///
extern "C" size_t XMI_Error_text (char * string, size_t length)
{
  return 0; // unimplemented
}

///
/// \copydoc XMI_Configuration_query
///
extern "C" xmi_result_t XMI_Configuration_query (xmi_context_t         context,
                                                 xmi_configuration_t * configuration)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;

  return ctx->queryConfiguration (configuration);
}

///
/// \copydoc XMI_Configuration_update
///
extern "C" xmi_result_t XMI_Configuration_update (xmi_context_t         context,
                                                  xmi_configuration_t * configuration)
{
  return XMI_UNIMPL;
}


///
/// \copydoc XMI_Wtime
///
double XMI_Wtime ()
{
  return XMI_Time.time();
}

///
/// \copydoc XMI_Wtick
///
double XMI_Wtick()
{
  return XMI_Time.tick();
}

///
/// \copydoc XMI_Wtimebase
///
unsigned long long XMI_Wtimebase()
{
  return XMI_Time.timebase();
}


///
/// \copydoc XMI_Client_initialize
///
extern "C" xmi_result_t XMI_Client_initialize (char         * name,
                                               xmi_client_t * client)
{
  return XMI_CLIENT_CLASS::generate (name, client);
}

///
/// \copydoc XMI_Client_finalize
///
extern "C" xmi_result_t XMI_Client_finalize (xmi_client_t client)
{
  XMI_CLIENT_CLASS::destroy ((XMI_CLIENT_CLASS *) client);
  return XMI_SUCCESS;
}

///
/// \copydoc XMI_Context_create
///
extern "C" xmi_result_t XMI_Context_create (xmi_client_t           client,
                                            xmi_configuration_t    configuration[],
                                            size_t                 count,
                                            xmi_context_t        * context)
{
  xmi_result_t result;
  XMI_CLIENT_CLASS * xmi = (XMI_CLIENT_CLASS *) client;

  *context = xmi->createContext (configuration, count, result);

  return result;
}

///
/// \copydoc XMI_Context_destroy
///
extern "C" xmi_result_t XMI_Context_destroy (xmi_context_t context)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  XMI_CLIENT_CLASS  * client = (XMI_CLIENT_CLASS *) ctx->getClientId ();

  return client->destroyContext (ctx);
}

///
/// \copydoc XMI_Context_post
///
extern "C" xmi_result_t XMI_Context_post (xmi_context_t        context,
                                          xmi_event_function   work_fn,
                                          void               * cookie)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;

  return ctx->post (work_fn, cookie);
}

///
/// \copydoc XMI_Context_advance
///
extern "C" xmi_result_t XMI_Context_advance (xmi_context_t context, size_t maximum)
{
  xmi_result_t result;
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  ctx->advance (maximum, result);

  return result;
}

///
/// \copydoc XMI_Context_multiadvance
///
extern "C" xmi_result_t XMI_Context_multiadvance (xmi_context_t context[],
                                                  size_t        count,
                                                  size_t        maximum)
{
  unsigned m, c;
  XMI_CONTEXT_CLASS * ctx;

  xmi_result_t result = XMI_EAGAIN;
  size_t events = 0;

  for (m=0; m<maximum && events==0 && result==XMI_SUCCESS; m++)
  {
    for (c=0; c<count && result==XMI_SUCCESS; c++)
    {
      ctx = (XMI_CONTEXT_CLASS *) context[c];
      events += ctx->advance (1, result);
    }
  }

  return result;
}

///
/// \copydoc XMI_Context_lock
///
extern "C" xmi_result_t XMI_Context_lock (xmi_context_t context)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->lock ();
}

///
/// \copydoc XMI_Context_trylock
///
extern "C" xmi_result_t XMI_Context_trylock (xmi_context_t context)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->trylock ();
}

///
/// \copydoc XMI_Context_unlock
///
extern "C" xmi_result_t XMI_Context_unlock (xmi_context_t context)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->unlock ();
}


////////////////////////////////////////////////////////////////////////////////
// Functions from xmi_p2p.h                                                   //
////////////////////////////////////////////////////////////////////////////////

///
/// \copydoc XMI_Send
///
extern "C" xmi_result_t XMI_Send (xmi_context_t       context,
                                  xmi_send_simple_t * parameters)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->send (parameters);
}

///
/// \copydoc XMI_Send_immediate
///
extern "C" xmi_result_t XMI_Send_immediate (xmi_context_t          context,
                                            xmi_send_immediate_t * parameters)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->send (parameters);
}

///
/// \copydoc XMI_Send_typed
///
extern "C" xmi_result_t XMI_Send_typed (xmi_context_t      context,
                                        xmi_send_typed_t * parameters)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->send (parameters);
}


///
/// \copydoc XMI_Put
///
extern "C" xmi_result_t XMI_Put (xmi_context_t      context,
                                 xmi_put_simple_t * parameters)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->put (parameters);
}


///
/// \copydoc XMI_Put_typed
///
extern "C" xmi_result_t XMI_Put_typed (xmi_context_t      context,
                                       xmi_put_typed_t  * parameters)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->put_typed (parameters);
}

///
/// \copydoc XMI_Get
///
extern "C" xmi_result_t XMI_Get (xmi_context_t      context,
                                 xmi_get_simple_t * parameters)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->get (parameters);
}


///
/// \copydoc XMI_Get_typed
///
extern "C" xmi_result_t XMI_Get_typed (xmi_context_t      context,
                                       xmi_get_typed_t  * parameters)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->get_typed (parameters);
}


///
/// \copydoc XMI_Rmw
///
extern "C" xmi_result_t XMI_Rmw (xmi_context_t      context,
                                 xmi_rmw_t * parameters)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->rmw (parameters);
}

///
/// \copydoc XMI_Memregion_register
///
extern "C" xmi_result_t XMI_Memregion_register (xmi_context_t     context,
                                                void            * address,
                                                size_t            bytes,
                                                xmi_memregion_t * memregion)
{
    XMI_CONTEXT_CLASS   * ctx = (XMI_CONTEXT_CLASS *) context;
    return ctx->memregion_register(address, bytes, memregion);
}


///
/// \copydoc XMI_Rput
///
extern "C" xmi_result_t XMI_Rput (xmi_context_t      context,
                                 xmi_rput_simple_t * parameters)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->rput (parameters);
}


///
/// \copydoc XMI_Rput_typed
///
extern "C" xmi_result_t XMI_Rput_typed (xmi_context_t      context,
                                       xmi_rput_typed_t  * parameters)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->rput_typed (parameters);
}

///
/// \copydoc XMI_Rget
///
extern "C" xmi_result_t XMI_Rget (xmi_context_t      context,
                                 xmi_rget_simple_t * parameters)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->rget (parameters);
}


///
/// \copydoc XMI_Rget_typed
///
extern "C" xmi_result_t XMI_Rget_typed (xmi_context_t      context,
                                        xmi_rget_typed_t  * parameters)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->rget_typed (parameters);
}


///
/// \copydoc XMI_Purge_totask
///
extern "C" xmi_result_t XMI_Purge_totask (xmi_context_t   context,
                                          size_t        * dest,
                                          size_t          count)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->purge_totask (dest, count);
}


///
/// \copydoc XMI_Resume_totask
///
extern "C" xmi_result_t XMI_Resume_totask (xmi_context_t   context,
                                           size_t        * dest,
                                           size_t          count)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->resume_totask (dest, count);
}


////////////////////////////////////////////////////////////////////////////////
// Functions from xmi_fence.h                                                 //
////////////////////////////////////////////////////////////////////////////////

///
/// \copydoc XMI_Fence_begin
///
extern "C" xmi_result_t XMI_Fence_begin (xmi_context_t      context)

{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->fence_begin ();
}

///
/// \copydoc XMI_Fence_end
///
extern "C" xmi_result_t XMI_Fence_end (xmi_context_t      context)

{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->fence_end ();
}

///
/// \copydoc XMI_Fence_all
///
extern "C" xmi_result_t XMI_Fence_all (xmi_context_t        context,
                                       xmi_event_function   done_fn,
                                       void               * cookie)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->fence_all (done_fn, cookie);
}

///
/// \copydoc XMI_Fence_task
///
extern "C" xmi_result_t XMI_Fence_task (xmi_context_t        context,
                                        xmi_event_function   done_fn,
                                        void               * cookie,
                                        size_t               task)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->fence_task (done_fn, cookie, task);
}

////////////////////////////////////////////////////////////////////////////////
// Functions from xmi_collectives.h                                           //
////////////////////////////////////////////////////////////////////////////////


///
/// \copydoc XMI_Geometry_initialize
///
extern "C" xmi_result_t XMI_Geometry_initialize (xmi_context_t               context,
                                      xmi_geometry_t            * geometry,
                                      unsigned                    id,
                                      xmi_geometry_range_t      * rank_slices,
                                      unsigned                    slice_count)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->geometry_initialize (geometry,id,rank_slices,slice_count);
}

///
/// \copydoc XMI_Geometry_initialize
///
extern "C" xmi_result_t XMI_Geometry_world (xmi_context_t               context,
                                 xmi_geometry_t            * world_geometry)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->geometry_world (world_geometry);
}

///
/// \copydoc XMI_Geometry_algorithm
///
extern "C" xmi_result_t XMI_Geometry_algorithm (xmi_context_t              context,
						xmi_xfer_type_t            colltype,
						xmi_geometry_t             geometry,
						xmi_algorithm_t           *algorithm,
						int                       *num)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->geometry_algorithm (colltype,geometry, algorithm, num);
}

///
/// \copydoc XMI_Geometry_finalize
///
extern "C" xmi_result_t XMI_Geometry_finalize(xmi_context_t   context,
                                   xmi_geometry_t  geometry)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->geometry_finalize (geometry);
}

///
/// \copydoc XMI_Collective
///
extern "C" xmi_result_t XMI_Collective (xmi_context_t   context,
                                        xmi_xfer_t     *parameters)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->collective (parameters);
}

////////////////////////////////////////////////////////////////////////////////
// Functions from xmi_multisend.h                                             //
////////////////////////////////////////////////////////////////////////////////


///
/// \copydoc XMI_Multisend_getroles
///
extern "C" xmi_result_t XMI_Multisend_getroles(xmi_context_t   context,
                                               xmi_dispatch_t  dispatch,
                                               int            *numRoles,
                                               int            *replRole)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->multisend_getroles (dispatch,numRoles,replRole);
}

///
/// \copydoc XMI_Multicast
///
extern "C" xmi_result_t XMI_Multicast(xmi_context_t    context,
                                      xmi_multicast_t *mcastinfo)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->multicast (mcastinfo);
}

///
/// \copydoc XMI_Manytomany
///
extern "C" xmi_result_t XMI_Manytomany(xmi_context_t     context,
                                       xmi_manytomany_t *m2minfo)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->manytomany (m2minfo);
}

///
/// \copydoc XMI_Multisync
///
extern "C" xmi_result_t XMI_Multisync(xmi_context_t    context,
                                      xmi_multisync_t *msyncinfo)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->multisync (msyncinfo);
}

///
/// \copydoc XMI_Multicombine
///
extern "C" xmi_result_t XMI_Multicombine(xmi_context_t       context,
                                         xmi_multicombine_t *mcombineinfo)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->multicombine (mcombineinfo);
}


////////////////////////////////////////////////////////////////////////////////
// Functions from xmi_dispatch.h                                              //
////////////////////////////////////////////////////////////////////////////////

///
/// \copydoc XMI_Dispatch_set
///
extern "C" xmi_result_t XMI_Dispatch_set (xmi_context_t              context,
                                          xmi_dispatch_t             dispatch,
                                          xmi_dispatch_callback_fn   fn,
                                          void                     * cookie,
                                          xmi_send_hint_t            options)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->dispatch (dispatch, fn, cookie, options);
}


////////////////////////////////////////////////////////////////////////////////
// Functions from xmi_datatypes.h                                             //
////////////////////////////////////////////////////////////////////////////////

///
/// \copydoc XMI_Type_create
///
extern "C" xmi_result_t XMI_Type_create (xmi_type_t * type)
{
  return XMI_UNIMPL;
}

///
/// \copydoc XMI_Type_add_simple
///
extern "C" xmi_result_t XMI_Type_add_simple (xmi_type_t type,
                                             size_t     bytes,
                                             size_t     offset,
                                             size_t     count,
                                             size_t     stride)
{
  return XMI_UNIMPL;
}

///
/// \copydoc XMI_Type_add_typed
///
extern "C" xmi_result_t XMI_Type_add_typed (xmi_type_t type,
                                            xmi_type_t subtype,
                                            size_t     offset,
                                            size_t     count,
                                            size_t     stride)
{
  return XMI_UNIMPL;
}

///
/// \copydoc XMI_Type_complete
///
extern "C" xmi_result_t XMI_Type_complete (xmi_type_t type)
{
  return XMI_UNIMPL;
}

///
/// \copydoc XMI_Type_sizeof
///
extern "C" xmi_result_t XMI_Type_sizeof (xmi_type_t type)
{
  return XMI_UNIMPL;
}

///
/// \copydoc XMI_Type_destroy
///
extern "C" xmi_result_t XMI_Type_destroy (xmi_type_t type)
{
  return XMI_UNIMPL;
}

///
/// \copydoc XMI_Type_pack_data
///
extern "C" xmi_result_t XMI_Type_pack_data (xmi_type_t src_type,
                                            size_t     src_offset,
                                            void     * src_addr,
                                            void     * dst_addr,
                                            size_t     dst_size)
{
  return XMI_UNIMPL;
}

///
/// \copydoc XMI_Type_unpack_data
///
extern "C" xmi_result_t XMI_Type_unpack (xmi_type_t dst_type,
                                         size_t     dst_offset,
                                         void     * dst_addr,
                                         void     * src_addr,
                                         size_t     src_size)
{
  return XMI_UNIMPL;
}


////////////////////////////////////////////////////////////////////////////////
// Functions from xmi_pipeworkqueue.h                                         //
////////////////////////////////////////////////////////////////////////////////




///
/// \copydoc XMI_PipeWorkQueue_config_circ
///
void XMI_PipeWorkQueue_config_circ(xmi_pipeworkqueue_t *wq,
                                   size_t bufsize)
{
    assert (0);
}

///
/// \copydoc XMI_PipeWorkQueue_config_circ_usr
///
void XMI_PipeWorkQueue_config_circ_usr(xmi_pipeworkqueue_t *wq,
                                       char                *buffer,
                                       size_t               bufsize)
{
    assert(0);
}

///
/// \copydoc XMI_PipeWorkQueue_config_flat
///
void XMI_PipeWorkQueue_config_flat(xmi_pipeworkqueue_t *wq,
                                   char                *buffer,
                                   size_t               bufsize,
                                   size_t               bufinit)
{
    assert(0);
}

///
/// \copydoc XMI_PipeWorkQueue_config_noncontig
///
void XMI_PipeWorkQueue_config_noncontig(xmi_pipeworkqueue_t *wq,
                                        char                *buffer,
                                        xmi_type_t          *type,
                                        size_t               typecount,
                                        size_t               typeinit)
{
    assert(0);
}

///
/// \copydoc XMI_PipeWorkQueue_export
///
xmi_result_t XMI_PipeWorkQueue_export(xmi_pipeworkqueue_t     *wq,
                                      xmi_pipeworkqueue_ext_t *exp)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    return pwq->exp (exp);
}

///
/// \copydoc XMI_PipeWorkQueue_import
///
xmi_result_t XMI_PipeWorkQueue_import(xmi_pipeworkqueue_ext_t *import,
                                      xmi_pipeworkqueue_t     *wq)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    return pwq->import (import);
}


///
/// \copydoc XMI_PipeWorkQueue_clone
///
void XMI_PipeWorkQueue_clone(xmi_pipeworkqueue_t *wq,
                             xmi_pipeworkqueue_t *obj)
{
}


///
/// \copydoc XMI_PipeWorkQueue_destroy
///
void XMI_PipeWorkQueue_destroy(xmi_pipeworkqueue_t *wq)
{
}

///
/// \copydoc XMI_PipeWorkQueue_reset
///
void XMI_PipeWorkQueue_reset(xmi_pipeworkqueue_t *wq)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    pwq->reset();
}


///
/// \copydoc XMI_PipeWorkQueue_dump
///
void XMI_PipeWorkQueue_dump(xmi_pipeworkqueue_t *wq,
                            const char          *prefix)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    pwq->dump (prefix);
}

///
/// \copydoc XMI_PipeWorkQueue_setConsumerWakeup
///
void XMI_PipeWorkQueue_setConsumerWakeup(xmi_pipeworkqueue_t *wq,
                                         void                *vec)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    pwq->setConsumerWakeup (vec);
}

///
/// \copydoc XMI_PipeWorkQueue_setProducerWakeup
///
void XMI_PipeWorkQueue_setProducerWakeup(xmi_pipeworkqueue_t *wq,
                                         void                *vec)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    pwq->setProducerWakeup (vec);
}

///
/// \copydoc XMI_PipeWorkQueue_bytesAvailableToProduce
///
size_t XMI_PipeWorkQueue_bytesAvailableToProduce(xmi_pipeworkqueue_t *wq)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    return pwq->bytesAvailableToProduce ();
}

///
/// \copydoc XMI_PipeWorkQueue_bytesAvailableToConsume
///
size_t XMI_PipeWorkQueue_bytesAvailableToConsume(xmi_pipeworkqueue_t *wq)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    return pwq->bytesAvailableToConsume ();
}

///
/// \copydoc XMI_PipeWorkQueue_getBytesProduced
///
size_t XMI_PipeWorkQueue_getBytesProduced(xmi_pipeworkqueue_t *wq)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    return pwq->getBytesProduced ();
}

///
/// \copydoc XMI_PipeWorkQueue_getBytesConsumed
///
size_t XMI_PipeWorkQueue_getBytesConsumed(xmi_pipeworkqueue_t *wq)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    return pwq->getBytesConsumed ();
}

///
/// \copydoc XMI_PipeWorkQueue_bufferToProduce
///
char *XMI_PipeWorkQueue_bufferToProduce(xmi_pipeworkqueue_t *wq)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    return pwq->bufferToProduce ();
}

///
/// \copydoc XMI_PipeWorkQueue_produceBytes
///
void XMI_PipeWorkQueue_produceBytes(xmi_pipeworkqueue_t *wq,
                                    size_t               bytes)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    return pwq->produceBytes (bytes);
}

///
/// \copydoc XMI_PipeWorkQueue_bufferToConsume
///
char *XMI_PipeWorkQueue_bufferToConsume(xmi_pipeworkqueue_t *wq)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    return pwq->bufferToConsume ();
}

///
/// \copydoc XMI_PipeWorkQueue_consumeBytes
///
void XMI_PipeWorkQueue_consumeBytes(xmi_pipeworkqueue_t *wq,
                                    size_t               bytes)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    return pwq->consumeBytes (bytes);
}

///
/// \copydoc XMI_PipeWorkQueue_available
///
int XMI_PipeWorkQueue_available(xmi_pipeworkqueue_t *wq)
{
    XMI_PIPEWORKQUEUE_CLASS * pwq = (XMI_PIPEWORKQUEUE_CLASS *) wq;
    return pwq->available ();
}



////////////////////////////////////////////////////////////////////////////////
// Functions from xmi_topology.h                                              //
////////////////////////////////////////////////////////////////////////////////


///
/// \copydoc XMI_Topology_create
///
void XMI_Topology_create(xmi_topology_t *topo)
{
    assert(0);
}

///
/// \copydoc XMI_Topology_create_rect
///
void XMI_Topology_create_rect(xmi_topology_t *topo,
                              xmi_coord_t *ll, xmi_coord_t *ur, unsigned char *tl)
{
    assert(0);
}

///
/// \copydoc XMI_Topology_create_rank
///
void XMI_Topology_create_rank(xmi_topology_t *topo, size_t rank)
{
    assert(0);
}

///
/// \copydoc XMI_Topology_create_range
///
void XMI_Topology_create_range(xmi_topology_t *topo, size_t rank0, size_t rankn)
{
    assert(0);
}

///
/// \copydoc XMI_Topology_create_list
///
void XMI_Topology_create_list(xmi_topology_t *topo, size_t *ranks, size_t nranks)
{
    assert(0);
}

///
/// \copydoc XMI_Topology_destroy
///
void XMI_Topology_destroy(xmi_topology_t *topo)
{
    assert(0);
}

///
/// \copydoc XMI_Topology_size_of
///
unsigned XMI_Topology_size_of(xmi_topology_t *topo)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->size_of();
}

///
/// \copydoc XMI_Topology_size
///
size_t XMI_Topology_size(xmi_topology_t *topo)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->size();
}

///
/// \copydoc XMI_Topology_type
///
xmi_topology_type_t xmi_topology_type(xmi_topology_t *topo)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->type();
}

///
/// \copydoc XMI_Topology_index2Rank
///
size_t XMI_Topology_index2Rank(xmi_topology_t *topo, size_t ix)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->index2Rank(ix);
}

///
/// \copydoc XMI_Topology_rank2Index
///
size_t XMI_Topology_rank2Index(xmi_topology_t *topo, size_t rank)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->rank2Index(rank);
}

///
/// \copydoc XMI_Topology_rankRange
///
xmi_result_t XMI_Topology_rankRange(xmi_topology_t *topo, size_t *first, size_t *last)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->rankRange(first,last);
}

///
/// \copydoc XMI_Topology_rankList
///
xmi_result_t XMI_Topology_rankList(xmi_topology_t *topo, size_t **list)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->rankList(list);
}

///
/// \copydoc XMI_Topology_rectSeg
///
xmi_result_t XMI_Topology_rectSeg(xmi_topology_t *topo,
                                  xmi_coord_t *ll, xmi_coord_t *ur,
                                  unsigned char *tl)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->rectSeg(ll,ur,tl);
}

///
/// \copydoc XMI_Topology_isLocalToMe
///
int XMI_Topology_isLocalToMe(xmi_topology_t *topo)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->isLocalToMe();
}

///
/// \copydoc XMI_Topology_isRectSeg
///
int XMI_Topology_isRectSeg(xmi_topology_t *topo)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->isRectSeg();
}

///
/// \copydoc XMI_Topology_getNthDims
///
void XMI_Topology_getNthDims(xmi_topology_t *topo, unsigned n,
                             unsigned *c0, unsigned *cn, unsigned char *tl)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->getNthDims(n,c0,cn,tl);
}

///
/// \copydoc XMI_Topology_isRankMember
///
int XMI_Topology_isRankMember(xmi_topology_t *topo, size_t rank)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->isRankMember(rank);
}

///
/// \copydoc XMI_Topology_isCoordMember
///
int XMI_Topology_isCoordMember(xmi_topology_t *topo, xmi_coord_t *c0)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->isCoordMember(c0);
}

///
/// \copydoc XMI_Topology_sub_LocalToMe
///
void XMI_Topology_sub_LocalToMe(xmi_topology_t *_new, xmi_topology_t *topo)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    t->subTopologyLocalToMe((XMI_TOPOLOGY_CLASS *)_new);
}

///
/// \copydoc XMI_Topology_sub_NthGlobal
///
void XMI_Topology_sub_NthGlobal(xmi_topology_t *_new, xmi_topology_t *topo, int n)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    t->subTopologyNthGlobal((XMI_TOPOLOGY_CLASS *)_new, n);
}

///
/// \copydoc XMI_Topology_sub_ReduceDims
///
void XMI_Topology_sub_ReduceDims(xmi_topology_t *_new, xmi_topology_t *topo, xmi_coord_t *fmt)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    t->subTopologyReduceDims((XMI_TOPOLOGY_CLASS *)_new, fmt);
}

///
/// \copydoc XMI_Topology_getRankList
///
void XMI_Topology_getRankList(xmi_topology_t *topo, size_t max, size_t *ranks, size_t *nranks)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    t->getRankList(max,ranks,nranks);
}

///
/// \copydoc XMI_Topology_analyze
///
int  XMI_Topology_analyze(xmi_topology_t *topo)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->analyzeTopology();
}

///
/// \copydoc XMI_Topology_convert
///
int  XMI_Topology_convert(xmi_topology_t *topo, xmi_topology_type_t new_type)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->convertTopology(new_type);
}

///
/// \copydoc XMI_Topology_intersect
///
void XMI_Topology_intersect(xmi_topology_t *_new, xmi_topology_t *topo, xmi_topology_t *other)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->intersectTopology((XMI_TOPOLOGY_CLASS *)_new, (XMI_TOPOLOGY_CLASS *)other);
}

///
/// \copydoc XMI_Topology_subtract
///
void XMI_Topology_subtract(xmi_topology_t *_new, xmi_topology_t *topo, xmi_topology_t *other)
{
    XMI_TOPOLOGY_CLASS * t = (XMI_TOPOLOGY_CLASS *) topo;
    return t->subtractTopology((XMI_TOPOLOGY_CLASS *)_new, (XMI_TOPOLOGY_CLASS *)other);
}
