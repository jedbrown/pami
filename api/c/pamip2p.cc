#include "config.h"
#include "Global.h"
#include "SysDep.h"
#include "Client.h"
#include "Context.h"
#include "PipeWorkQueue.h"
#include "Topology.h"
#include "sys/pami.h"
#include "util/common.h"

///
/// \copydoc PAMI_Client_initialize
///
extern "C" pami_result_t PAMI_Client_initialize (const char * name,
                                               pami_client_t * client)
{
  return PAMI::Client::generate (name, client);
}

///
/// \copydoc PAMI_Client_finalize
///
extern "C" pami_result_t PAMI_Client_finalize (pami_client_t client)
{
  PAMI::Client::destroy ((PAMI::Client *) client);
  return PAMI_SUCCESS;
}



///
/// \copydoc PAMI_Context_createv
///
extern "C" pami_result_t PAMI_Context_createv (pami_client_t          client,
                                            pami_configuration_t    configuration[],
                                            size_t                 count,
                                            pami_context_t        * context,
                                            size_t                 ncontexts)
{
  pami_result_t result;
  PAMI::Client * pami = (PAMI::Client *) client;

  result = pami->createContext (configuration, count, context, ncontexts);

  return result;
}

///
/// \copydoc PAMI_Context_destroy
///
extern "C" pami_result_t PAMI_Context_destroy (pami_context_t context)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  PAMI::Client  * client = (PAMI::Client *) ctx->getClient ();

  return client->destroyContext (ctx);
}

///
/// \copydoc PAMI_Configuration_query
///
extern "C" pami_result_t PAMI_Configuration_query (pami_client_t         client,
                                                 pami_configuration_t * configuration)
{
  PAMI::Client * cln = (PAMI::Client *) client;

  return cln->queryConfiguration (configuration);
}

///
/// \copydoc PAMI_Configuration_update
///
extern "C" pami_result_t PAMI_Configuration_update (pami_client_t         client,
                                                  pami_configuration_t * configuration)
{
  return PAMI_UNIMPL;
}

///
/// \copydoc PAMI_Dispatch_set
///
extern "C" pami_result_t PAMI_Dispatch_set (pami_context_t              context,
                                          size_t                     dispatch,
                                          pami_dispatch_callback_fn   fn,
                                          void                     * cookie,
                                          pami_send_hint_t            options)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->dispatch (dispatch, fn, cookie, options);
}


///
/// \copydoc PAMI_Context_lock
///
extern "C" pami_result_t PAMI_Context_lock (pami_context_t context)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->lock ();
}

///
/// \copydoc PAMI_Context_trylock
///
extern "C" pami_result_t PAMI_Context_trylock (pami_context_t context)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->trylock ();
}

///
/// \copydoc PAMI_Context_unlock
///
extern "C" pami_result_t PAMI_Context_unlock (pami_context_t context)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->unlock ();
}


///
/// \copydoc PAMI_Context_advance
///
extern "C" pami_result_t PAMI_Context_advance (pami_context_t context, size_t maximum)
{
  pami_result_t result;
  PAMI::Context * ctx = (PAMI::Context *) context;
  ctx->advance (maximum, result);

  return result;
}

///
/// \copydoc PAMI_Context_multiadvance
///
extern "C" pami_result_t PAMI_Context_multiadvance (pami_context_t context[],
                                                  size_t        count,
                                                  size_t        maximum)
{
  unsigned m, c;
  PAMI::Context * ctx;

  pami_result_t result = PAMI_SUCCESS;
  size_t events = 0;

  for (m=0; m<maximum && events==0 && result==PAMI_SUCCESS; m++)
  {
    for (c=0; c<count && result==PAMI_SUCCESS; c++)
    {
      ctx = (PAMI::Context *) context[c];
      events += ctx->advance (1, result);
    }
  }

  return result;
}




////////////////////////////////////////////////////////////////////////////////
// Functions from pami_p2p.h                                                   //
////////////////////////////////////////////////////////////////////////////////

///
/// \copydoc PAMI_Send
///
extern "C" pami_result_t PAMI_Send (pami_context_t   context,
                                  pami_send_t    * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->send (parameters);
}

///
/// \copydoc PAMI_Send_immediate
///
extern "C" pami_result_t PAMI_Send_immediate (pami_context_t          context,
                                            pami_send_immediate_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->send (parameters);
}

///
/// \copydoc PAMI_Send_typed
///
extern "C" pami_result_t PAMI_Send_typed (pami_context_t      context,
                                        pami_send_typed_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->send (parameters);
}


///
/// \copydoc PAMI_Put
///
extern "C" pami_result_t PAMI_Put (pami_context_t      context,
                                 pami_put_simple_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->put (parameters);
}


///
/// \copydoc PAMI_Put_typed
///
extern "C" pami_result_t PAMI_Put_typed (pami_context_t      context,
                                       pami_put_typed_t  * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->put_typed (parameters);
}

///
/// \copydoc PAMI_Get
///
extern "C" pami_result_t PAMI_Get (pami_context_t      context,
                                 pami_get_simple_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->get (parameters);
}


///
/// \copydoc PAMI_Get_typed
///
extern "C" pami_result_t PAMI_Get_typed (pami_context_t      context,
                                       pami_get_typed_t  * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->get_typed (parameters);
}


///
/// \copydoc PAMI_Rmw
///
extern "C" pami_result_t PAMI_Rmw (pami_context_t      context,
                                 pami_rmw_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->rmw (parameters);
}

///
/// \copydoc PAMI_Memregion_register
///
extern "C" pami_result_t PAMI_Memregion_register (pami_context_t     context,
                                                void            * address,
                                                size_t            bytes,
                                                pami_memregion_t * memregion)
{
    PAMI::Context   * ctx = (PAMI::Context *) context;
    return ctx->memregion_register(address, bytes, memregion);
}


///
/// \copydoc PAMI_Rput
///
extern "C" pami_result_t PAMI_Rput (pami_context_t      context,
                                 pami_rput_simple_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->rput (parameters);
}


///
/// \copydoc PAMI_Rput_typed
///
extern "C" pami_result_t PAMI_Rput_typed (pami_context_t      context,
                                       pami_rput_typed_t  * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->rput_typed (parameters);
}

///
/// \copydoc PAMI_Rget
///
extern "C" pami_result_t PAMI_Rget (pami_context_t      context,
                                 pami_rget_simple_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->rget (parameters);
}


///
/// \copydoc PAMI_Rget_typed
///
extern "C" pami_result_t PAMI_Rget_typed (pami_context_t      context,
                                        pami_rget_typed_t  * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->rget_typed (parameters);
}


///
/// \copydoc PAMI_Purge_totask
///
extern "C" pami_result_t PAMI_Purge_totask (pami_context_t   context,
                                          size_t        * dest,
                                          size_t          count)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->purge_totask (dest, count);
}


///
/// \copydoc PAMI_Resume_totask
///
extern "C" pami_result_t PAMI_Resume_totask (pami_context_t   context,
                                           size_t        * dest,
                                           size_t          count)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->resume_totask (dest, count);
}
