/**
 * \file xmi.cc
 * \brief XMI C interface common implementation
 */

#include "sys/xmi.h"

#include "platform.h"
////////////////////////////////////////////////////////////////////////////////
// Functions from xmi_misc.h                                                  //
////////////////////////////////////////////////////////////////////////////////

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
extern "C" xmi_result_t XMI_Configuration_query (xmi_context_t     context,
                                                 xmi_attribute_t   attribute,
                                                 void            * value)
{
  return XMI_UNIMPL;
}

///
/// \copydoc XMI_Configuration_update
///
extern "C" xmi_result_t XMI_Configuration_update (xmi_context_t     context,
                                                  xmi_attribute_t   attribute,
                                                  void            * value)
{
  return XMI_UNIMPL;
}




///
/// \copydoc XMI_Client_initialize
///
extern "C" xmi_result_t XMI_Client_initialize (char         * name,
                                               xmi_client_t * client)
{
  *client = (xmi_client_t) XMI_CLIENT_CLASS::generate (name);
  return XMI_SUCCESS;
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
  XMI_CLIENT_CLASS * xmi = (XMI_CLIENT_CLASS *) client;
  *context = (xmi_context_t) xmi->createContext (configuration, count);
  return XMI_UNIMPL;
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
xmi_result_t XMI_Geometry_initialize (xmi_context_t               context,
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
xmi_result_t XMI_Geometry_world (xmi_context_t               context,
                                 xmi_geometry_t            * world_geometry)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->geometry_world (world_geometry);
}

///
/// \copydoc XMI_Geometry_algorithm
///
xmi_result_t XMI_Geometry_algorithm (xmi_context_t              context,
                                     xmi_geometry_t             geometry,
                                     xmi_algorithm_t           *algorithm,
                                     int                       *num)
{
  XMI_CONTEXT_CLASS * ctx = (XMI_CONTEXT_CLASS *) context;
  return ctx->geometry_algorithm (geometry, algorithm, num);
}

///
/// \copydoc XMI_Geometry_finalize
///
xmi_result_t XMI_Geometry_finalize(xmi_context_t   context,
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
// Functions from xmi_dispatch.h                                              //
////////////////////////////////////////////////////////////////////////////////

///
/// \copydoc XMI_Dispatch_set
///
extern "C" xmi_result_t XMI_Dispatch_set (xmi_context_t            * context,
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
