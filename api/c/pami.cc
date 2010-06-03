/**
 * \file api/c/pami.cc
 * \brief PAMI C interface common implementation
 */
#ifndef PAMI_LAPI_IMPL
#include "config.h"
#endif //PAMI_LAPI_IMPL

#include "Global.h"
#include "SysDep.h"
#include "Client.h"
#include "Context.h"
#include "PipeWorkQueue.h"
#include "Topology.h"
#include <pami.h>
#include "util/common.h"



////////////////////////////////////////////////////////////////////////////////
// Functions from pami_misc.h                                                  //
////////////////////////////////////////////////////////////////////////////////

///
/// \copydoc PAMI_Error_text
///
extern "C" size_t PAMI_Error_text (char * string, size_t length)
{
  return 0; // unimplemented
}

///
/// \copydoc PAMI_Wtime
///
extern "C" double PAMI_Wtime ()
{
  return __global.time.time();
}

///
/// \copydoc PAMI_Wtimebase
///
extern "C" unsigned long long PAMI_Wtimebase()
{
  return __global.time.timebase();
}

#if 0
extern "C" pami_context_t PAMI_Client_getcontext(pami_client_t client, size_t context) {
        PAMI::Client *clnt = (PAMI::Client *)client;
        return (pami_context_t)clnt->getContext(context);
}
#endif

extern "C" pami_result_t PAMI_Endpoint_create (pami_client_t     client,
                                               pami_task_t       task,
                                               size_t            offset,
                                               pami_endpoint_t * endpoint)
{
  *endpoint = PAMI_ENDPOINT_INIT(client,task,offset);
  return PAMI_SUCCESS;
}

extern "C" pami_result_t PAMI_Endpoint_createv (pami_client_t     client,
                                                pami_task_t       task,
                                                pami_endpoint_t * endpoints,
                                                size_t          * count)
{
  return PAMI_UNIMPL;
}

extern "C" pami_result_t PAMI_Endpoint_query (pami_endpoint_t   endpoint,
                                              pami_task_t     * task,
                                              size_t          * offset)
{
  PAMI_ENDPOINT_INFO(endpoint,*task,*offset);
  return PAMI_SUCCESS;
}


///
/// \copydoc PAMI_Context_post
///
extern "C" pami_result_t PAMI_Context_post (pami_context_t        context,
                                          pami_work_t         * work,
                                          pami_work_function    fn,
                                          void               * cookie)
{
  PAMI::Context * ctx = (PAMI::Context *) context;

  return ctx->post (work, fn, cookie);
}




////////////////////////////////////////////////////////////////////////////////
// Functions from pami_fence.h                                                 //
////////////////////////////////////////////////////////////////////////////////

///
/// \copydoc PAMI_Fence_begin
///
extern "C" pami_result_t PAMI_Fence_begin (pami_context_t      context)

{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->fence_begin ();
}

///
/// \copydoc PAMI_Fence_end
///
extern "C" pami_result_t PAMI_Fence_end (pami_context_t      context)

{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->fence_end ();
}

///
/// \copydoc PAMI_Fence_all
///
extern "C" pami_result_t PAMI_Fence_all (pami_context_t        context,
                                       pami_event_function   done_fn,
                                       void               * cookie)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->fence_all (done_fn, cookie);
}

///
/// \copydoc PAMI_Fence_task
///
extern "C" pami_result_t PAMI_Fence_task (pami_context_t        context,
                                        pami_event_function   done_fn,
                                        void               * cookie,
                                        size_t               task)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->fence_task (done_fn, cookie, task);
}

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_collectives.h                                           //
////////////////////////////////////////////////////////////////////////////////


///
/// \copydoc PAMI_Geometry_create_taskrange
///
extern "C" pami_result_t PAMI_Geometry_create_taskrange (pami_client_t                client,
                                      pami_geometry_t            * geometry,
                                                       pami_geometry_t              parent,
                                      unsigned                    id,
                                                       pami_geometry_range_t      * task_slices,
                                                       size_t                      slice_count,
                                                       pami_context_t               context,
                                                       pami_event_function          fn,
                                                       void                      * cookie)
{
  PAMI::Client * _client = (PAMI::Client *) client;
  return _client->geometry_create_taskrange (geometry,
                                             parent,
                                             id,
                                             task_slices,
                                             slice_count,
                                             context,
                                             fn,
                                             cookie);
}

///
/// \copydoc PAMI_Geometry_create_taskrange
///
extern "C" pami_result_t PAMI_Geometry_create_tasklist (pami_client_t                client,
                                                      pami_geometry_t            * geometry,
                                                      pami_geometry_t              parent,
                                                      unsigned                    id,
                                                      pami_task_t                * tasks,
                                                      size_t                      task_count,
                                                      pami_context_t               context,
                                                      pami_event_function          fn,
                                                      void                      * cookie)
{
  PAMI::Client * _client = (PAMI::Client *) client;
  return _client->geometry_create_tasklist (geometry,
                                            parent,
                                            id,
                                            tasks,
                                            task_count,
                                            context,
                                            fn,
                                            cookie);
}


///
/// \copydoc PAMI_Geometry_world
///
extern "C" pami_result_t PAMI_Geometry_world (pami_client_t                client,
                                 pami_geometry_t            * world_geometry)
{
  PAMI::Client * _client = (PAMI::Client *) client;
  return _client->geometry_world (world_geometry);
}

///
/// \copydoc PAMI_Geometry_destroy
///
extern "C" pami_result_t PAMI_Geometry_destroy(pami_client_t    client,
                                               pami_geometry_t *geometry)
{
  PAMI::Client * _client = (PAMI::Client *) client;
  pami_result_t rc = _client->geometry_destroy (*geometry);
  *geometry = NULL;
  return rc;
}

///
/// \copydoc PAMI_Collective
///
extern "C" pami_result_t PAMI_Collective (pami_context_t   context,
                                        pami_xfer_t     *parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->collective (parameters);
}

/// \copydoc PAMI_Geometry_algorithms_num
///
extern "C" pami_result_t PAMI_Geometry_algorithms_num (pami_context_t context,
                                                     pami_geometry_t geometry,
                                                     pami_xfer_type_t coll_type,
                                                     int              lists_lengths[2])
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->geometry_algorithms_num (geometry,
                                      coll_type,
                                      lists_lengths);
}

/// \copydoc PAMI_Geometry_algorithms_query
///
extern "C"  pami_result_t PAMI_Geometry_algorithms_query (pami_context_t context,
                                                       pami_geometry_t geometry,
                                                       pami_xfer_type_t   colltype,
                                                       pami_algorithm_t  *algs0,
                                                       pami_metadata_t   *mdata0,
                                                       int               num0,
                                                       pami_algorithm_t  *algs1,
                                                       pami_metadata_t   *mdata1,
                                                       int               num1)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->geometry_algorithms_info (geometry,
                                        colltype,
                                        algs0,
                                        mdata0,
                                        num0,
                                        algs1,
                                        mdata1,
                                        num1);
}
///

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_multisend.h                                             //
////////////////////////////////////////////////////////////////////////////////


///
/// \copydoc PAMI_Multisend_getroles
///
extern "C" pami_result_t PAMI_Multisend_getroles(pami_context_t   context,
                                               size_t          dispatch,
                                               int            *numRoles,
                                               int            *replRole)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->multisend_getroles (dispatch,numRoles,replRole);
}

///
/// \copydoc PAMI_Multicast
///
extern "C" pami_result_t PAMI_Multicast(pami_multicast_t *mcastinfo)
{
  PAMI::Client *clnt = (PAMI::Client *)mcastinfo->client;
  PAMI::Context *ctx = clnt->getContext(mcastinfo->context);
  return ctx->multicast (mcastinfo);
}

///
/// \copydoc PAMI_Manytomany
///
extern "C" pami_result_t PAMI_Manytomany(pami_manytomany_t *m2minfo)
{
  PAMI::Client *clnt = (PAMI::Client *)m2minfo->client;
  PAMI::Context *ctx = clnt->getContext(m2minfo->context);
  return ctx->manytomany (m2minfo);
}

///
/// \copydoc PAMI_Multisync
///
extern "C" pami_result_t PAMI_Multisync(pami_multisync_t *msyncinfo)
{
  PAMI::Client *clnt = (PAMI::Client *)msyncinfo->client;
  PAMI::Context *ctx = clnt->getContext(msyncinfo->context);
  return ctx->multisync (msyncinfo);
}

///
/// \copydoc PAMI_Multicombine
///
extern "C" pami_result_t PAMI_Multicombine(pami_multicombine_t *mcombineinfo)
{
  PAMI::Client *clnt = (PAMI::Client *)mcombineinfo->client;
  PAMI::Context *ctx = clnt->getContext(mcombineinfo->context);
  return ctx->multicombine (mcombineinfo);
}


////////////////////////////////////////////////////////////////////////////////
// Functions from pami_dispatch.h                                              //
////////////////////////////////////////////////////////////////////////////////


//#ifdef __pami_target_mpi__
extern "C" pami_result_t PAMI_Dispatch_set_new(pami_context_t  context,
                                 size_t                     dispatch,
                                 pami_dispatch_callback_fn   fn,
                                 void                     * cookie,
                                 pami_dispatch_hint_t        options)
  {
    PAMI::Context * ctx = (PAMI::Context *) context;
    return ctx->dispatch_new(dispatch, fn, cookie, options);
  }
//#endif

///
/// \copydoc PAMI_AMCollective_dispatch_set
///
extern "C" pami_result_t PAMI_AMCollective_dispatch_set(pami_context_t              context,
                                                      pami_algorithm_t            algorithm,
                                                      size_t                     dispatch,
                                                      pami_dispatch_callback_fn   fn,
                                                      void                     * cookie,
                                                      pami_collective_hint_t      options)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->amcollective_dispatch (algorithm, dispatch, fn, cookie, options);
}

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_datatypes.h                                             //
////////////////////////////////////////////////////////////////////////////////

///
/// \copydoc PAMI_Type_create
///
extern "C" pami_result_t PAMI_Type_create (pami_type_t * type)
{
  return PAMI_UNIMPL;
}

///
/// \copydoc PAMI_Type_add_simple
///
extern "C" pami_result_t PAMI_Type_add_simple (pami_type_t type,
                                             size_t     bytes,
                                             size_t     offset,
                                             size_t     count,
                                             size_t     stride)
{
  return PAMI_UNIMPL;
}

///
/// \copydoc PAMI_Type_add_typed
///
extern "C" pami_result_t PAMI_Type_add_typed (pami_type_t type,
                                            pami_type_t subtype,
                                            size_t     offset,
                                            size_t     count,
                                            size_t     stride)
{
  return PAMI_UNIMPL;
}

///
/// \copydoc PAMI_Type_complete
///
extern "C" pami_result_t PAMI_Type_complete (pami_type_t type)
{
  return PAMI_UNIMPL;
}

///
/// \copydoc PAMI_Type_sizeof
///
extern "C" pami_result_t PAMI_Type_sizeof (pami_type_t type)
{
  return PAMI_UNIMPL;
}

///
/// \copydoc PAMI_Type_destroy
///
extern "C" pami_result_t PAMI_Type_destroy (pami_type_t *type)
{
  *type = NULL;
  return PAMI_UNIMPL;
}

///
/// \copydoc PAMI_Type_pack_data
///
extern "C" pami_result_t PAMI_Type_pack_data (pami_type_t src_type,
                                            size_t     src_offset,
                                            void     * src_addr,
                                            void     * dst_addr,
                                            size_t     dst_size)
{
  return PAMI_UNIMPL;
}

///
/// \copydoc PAMI_Type_unpack_data
///
extern "C" pami_result_t PAMI_Type_unpack (pami_type_t dst_type,
                                         size_t     dst_offset,
                                         void     * dst_addr,
                                         void     * src_addr,
                                         size_t     src_size)
{
  return PAMI_UNIMPL;
}


////////////////////////////////////////////////////////////////////////////////
// Functions from pami_pipeworkqueue.h                                         //
////////////////////////////////////////////////////////////////////////////////




///
/// \copydoc PAMI_PipeWorkQueue_config_circ
///
void PAMI_PipeWorkQueue_config_circ(pami_pipeworkqueue_t *wq,
                                   size_t bufsize)
{
    assert (0);
}

///
/// \copydoc PAMI_PipeWorkQueue_config_circ_usr
///
void PAMI_PipeWorkQueue_config_circ_usr(pami_pipeworkqueue_t *wq,
                                       char                *buffer,
                                       size_t               bufsize)
{
    assert(0);
}

///
/// \copydoc PAMI_PipeWorkQueue_config_flat
///
void PAMI_PipeWorkQueue_config_flat(pami_pipeworkqueue_t *wq,
                                   char                *buffer,
                                   size_t               bufsize,
                                   size_t               bufinit)
{
    assert(0);
}

///
/// \copydoc PAMI_PipeWorkQueue_config_noncontig
///
void PAMI_PipeWorkQueue_config_noncontig(pami_pipeworkqueue_t *wq,
                                        char                *buffer,
                                        pami_type_t          *type,
                                        size_t               typecount,
                                        size_t               typeinit)
{
    assert(0);
}

///
/// \copydoc PAMI_PipeWorkQueue_export
///
pami_result_t PAMI_PipeWorkQueue_export(pami_pipeworkqueue_t     *wq,
                                      pami_pipeworkqueue_ext_t *exp)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->exp (exp);
}

///
/// \copydoc PAMI_PipeWorkQueue_import
///
pami_result_t PAMI_PipeWorkQueue_import(pami_pipeworkqueue_ext_t *import,
                                      pami_pipeworkqueue_t     *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->import (import);
}


///
/// \copydoc PAMI_PipeWorkQueue_clone
///
void PAMI_PipeWorkQueue_clone(pami_pipeworkqueue_t *wq,
                             pami_pipeworkqueue_t *obj)
{
}


///
/// \copydoc PAMI_PipeWorkQueue_destroy
///
void PAMI_PipeWorkQueue_destroy(pami_pipeworkqueue_t *wq)
{
}

///
/// \copydoc PAMI_PipeWorkQueue_reset
///
void PAMI_PipeWorkQueue_reset(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->reset();
}


///
/// \copydoc PAMI_PipeWorkQueue_dump
///
void PAMI_PipeWorkQueue_dump(pami_pipeworkqueue_t *wq,
                            const char          *prefix)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->dump (prefix);
}

///
/// \copydoc PAMI_PipeWorkQueue_setConsumerWakeup
///
void PAMI_PipeWorkQueue_setConsumerWakeup(pami_pipeworkqueue_t *wq,
                                         void                *vec)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->setConsumerWakeup (vec);
}

///
/// \copydoc PAMI_PipeWorkQueue_setProducerWakeup
///
void PAMI_PipeWorkQueue_setProducerWakeup(pami_pipeworkqueue_t *wq,
                                         void                *vec)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->setProducerWakeup (vec);
}

///
/// \copydoc PAMI_PipeWorkQueue_setProducerUserInfo
///
void PAMI_PipeWorkQueue_setProducerUserInfo(pami_pipeworkqueue_t *wq,
                                         void *word1, void *word2)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->setProducerUserInfo (word1, word2);
}

///
/// \copydoc PAMI_PipeWorkQueue_setConsumerUserInfo
///
void PAMI_PipeWorkQueue_setConsumerUserInfo(pami_pipeworkqueue_t *wq,
                                         void *word1, void *word2)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->setConsumerUserInfo (word1, word2);
}

///
/// \copydoc PAMI_PipeWorkQueue_getProducerUserInfo
///
void PAMI_PipeWorkQueue_getProducerUserInfo(pami_pipeworkqueue_t *wq,
                                         void **word1, void **word2)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->getProducerUserInfo (word1, word2);
}

///
/// \copydoc PAMI_PipeWorkQueue_getConsumerUserInfo
///
void PAMI_PipeWorkQueue_getConsumerUserInfo(pami_pipeworkqueue_t *wq,
                                         void **word1, void **word2)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->getConsumerUserInfo (word1, word2);
}

///
/// \copydoc PAMI_PipeWorkQueue_bytesAvailableToProduce
///
size_t PAMI_PipeWorkQueue_bytesAvailableToProduce(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->bytesAvailableToProduce ();
}

///
/// \copydoc PAMI_PipeWorkQueue_bytesAvailableToConsume
///
size_t PAMI_PipeWorkQueue_bytesAvailableToConsume(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->bytesAvailableToConsume ();
}

///
/// \copydoc PAMI_PipeWorkQueue_getBytesProduced
///
size_t PAMI_PipeWorkQueue_getBytesProduced(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->getBytesProduced ();
}

///
/// \copydoc PAMI_PipeWorkQueue_getBytesConsumed
///
size_t PAMI_PipeWorkQueue_getBytesConsumed(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->getBytesConsumed ();
}

///
/// \copydoc PAMI_PipeWorkQueue_bufferToProduce
///
char *PAMI_PipeWorkQueue_bufferToProduce(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->bufferToProduce ();
}

///
/// \copydoc PAMI_PipeWorkQueue_produceBytes
///
void PAMI_PipeWorkQueue_produceBytes(pami_pipeworkqueue_t *wq,
                                    size_t               bytes)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->produceBytes (bytes);
}

///
/// \copydoc PAMI_PipeWorkQueue_bufferToConsume
///
char *PAMI_PipeWorkQueue_bufferToConsume(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->bufferToConsume ();
}

///
/// \copydoc PAMI_PipeWorkQueue_consumeBytes
///
void PAMI_PipeWorkQueue_consumeBytes(pami_pipeworkqueue_t *wq,
                                    size_t               bytes)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->consumeBytes (bytes);
}

///
/// \copydoc PAMI_PipeWorkQueue_available
///
int PAMI_PipeWorkQueue_available(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->available ();
}



////////////////////////////////////////////////////////////////////////////////
// Functions from pami_topology.h                                              //
////////////////////////////////////////////////////////////////////////////////


///
/// \copydoc PAMI_Topology_create
///
void PAMI_Topology_create(pami_topology_t *topo)
{
    assert(0);
}

///
/// \copydoc PAMI_Topology_create_rect
///
void PAMI_Topology_create_rect(pami_topology_t *topo,
                              pami_coord_t *ll, pami_coord_t *ur, unsigned char *tl)
{
    assert(0);
}

///
/// \copydoc PAMI_Topology_create_axial
///
void PAMI_Topology_create_axial(pami_topology_t *topo,
                               pami_coord_t *ll,
                               pami_coord_t *ur,
                               pami_coord_t *ref,
                               unsigned char *dir,
                               unsigned char *tl)
{
  assert(0);
}

///
/// \copydoc PAMI_Topology_create_task
///
void PAMI_Topology_create_task(pami_topology_t *topo, pami_task_t rank)
{
    assert(0);
}

///
/// \copydoc PAMI_Topology_create_range
///
void PAMI_Topology_create_range(pami_topology_t *topo, pami_task_t rank0, pami_task_t rankn)
{
  new(topo)PAMI::Topology(rank0,rankn);
}

///
/// \copydoc PAMI_Topology_create_list
///
void PAMI_Topology_create_list(pami_topology_t *topo, pami_task_t *ranks, size_t nranks)
{
  new(topo)PAMI::Topology(ranks,nranks);
}

///
/// \copydoc PAMI_Topology_destroy
///
void PAMI_Topology_destroy(pami_topology_t *topo)
{
    assert(0);
}

///
/// \copydoc PAMI_Topology_size_of
///
unsigned PAMI_Topology_size_of(pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->size_of();
}

///
/// \copydoc PAMI_Topology_size
///
size_t PAMI_Topology_size(pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->size();
}

///
/// \copydoc PAMI_Topology_type
///
pami_topology_type_t pami_topology_type(pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->type();
}

///
/// \copydoc PAMI_Topology_index2TaskID
///
pami_task_t PAMI_Topology_index2TaskID(pami_topology_t *topo, size_t ix)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->index2Rank(ix);
}

///
/// \copydoc PAMI_Topology_taskID2Index
///
size_t PAMI_Topology_taskID2Index(pami_topology_t *topo, pami_task_t rank)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->rank2Index(rank);
}

///
/// \copydoc PAMI_Topology_taskRange
///
pami_result_t PAMI_Topology_taskRange(pami_topology_t *topo, pami_task_t *first, pami_task_t *last)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->rankRange(first,last);
}

///
/// \copydoc PAMI_Topology_taskList
///
pami_result_t PAMI_Topology_taskList(pami_topology_t *topo, pami_task_t **list)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->rankList(list);
}

///
/// \copydoc PAMI_Topology_rectSeg
///
pami_result_t PAMI_Topology_rectSeg(pami_topology_t *topo,
                                  pami_coord_t *ll, pami_coord_t *ur,
                                  unsigned char *tl)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->rectSeg(ll,ur,tl);
}

///
/// \copydoc PAMI_Topology_isLocalToMe
///
int PAMI_Topology_isLocalToMe(pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->isLocalToMe();
}

///
/// \copydoc PAMI_Topology_isRectSeg
///
int PAMI_Topology_isRectSeg(pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->isRectSeg();
}

///
/// \copydoc PAMI_Topology_getNthDims
///
void PAMI_Topology_getNthDims(pami_topology_t *topo, unsigned n,
                             unsigned *c0, unsigned *cn, unsigned char *tl)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->getNthDims(n,c0,cn,tl);
}

///
/// \copydoc PAMI_Topology_isTaskMember
///
int PAMI_Topology_isTaskMember(pami_topology_t *topo, pami_task_t rank)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->isRankMember(rank);
}

///
/// \copydoc PAMI_Topology_isCoordMember
///
int PAMI_Topology_isCoordMember(pami_topology_t *topo, pami_coord_t *c0)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->isCoordMember(c0);
}

///
/// \copydoc PAMI_Topology_sub_LocalToMe
///
void PAMI_Topology_sub_LocalToMe(pami_topology_t *_new, pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    t->subTopologyLocalToMe((PAMI::Topology *)_new);
}

///
/// \copydoc PAMI_Topology_sub_NthGlobal
///
void PAMI_Topology_sub_NthGlobal(pami_topology_t *_new, pami_topology_t *topo, int n)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    t->subTopologyNthGlobal((PAMI::Topology *)_new, n);
}

///
/// \copydoc PAMI_Topology_sub_ReduceDims
///
void PAMI_Topology_sub_ReduceDims(pami_topology_t *_new, pami_topology_t *topo, pami_coord_t *fmt)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    t->subTopologyReduceDims((PAMI::Topology *)_new, fmt);
}

///
/// \copydoc PAMI_Topology_getTaskList
///
void PAMI_Topology_getTaskList(pami_topology_t *topo, size_t max, pami_task_t *ranks, size_t *nranks)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    t->getRankList(max,ranks,nranks);
}

///
/// \copydoc PAMI_Topology_analyze
///
int  PAMI_Topology_analyze(pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->analyzeTopology();
}

///
/// \copydoc PAMI_Topology_convert
///
int  PAMI_Topology_convert(pami_topology_t *topo, pami_topology_type_t new_type)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->convertTopology(new_type);
}

///
/// \copydoc PAMI_Topology_intersect
///
void PAMI_Topology_intersect(pami_topology_t *_new, pami_topology_t *topo, pami_topology_t *other)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->intersectTopology((PAMI::Topology *)_new, (PAMI::Topology *)other);
}

///
/// \copydoc PAMI_Topology_subtract
///
void PAMI_Topology_subtract(pami_topology_t *_new, pami_topology_t *topo, pami_topology_t *other)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->subtractTopology((PAMI::Topology *)_new, (PAMI::Topology *)other);
}

#ifdef USE_COMMTHREADS // from (bgq/) Client.h
/// \todo #warning PAMI_Client_add_commthread_context() is non-standard API
extern "C" pami_result_t PAMI_Client_add_commthread_context(pami_client_t client,
                                            pami_context_t context)
{
  pami_result_t result;
  PAMI::Client *pami = (PAMI::Client *)client;

  result = pami->addContextToCommThreadPool(context);
  return result;
}
#endif // USE_COMMTHREADS




///
/// \copydoc PAMI_Client_create
///
extern "C" pami_result_t PAMI_Client_create (const char * name,
                                               pami_client_t * client)
{
  return PAMI::Client::generate (name, client);
}

///
/// \copydoc PAMI_Client_destroy
///
extern "C" pami_result_t PAMI_Client_destroy (pami_client_t *client)
{
  PAMI::Client::destroy ((PAMI::Client *) *client);
  *client = NULL;
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
/// \copydoc PAMI_Context_destroyv
///
extern "C" pami_result_t PAMI_Context_destroyv (pami_context_t* contexts,
                                                size_t          ncontexts)
{
  PAMI_assert(contexts != NULL);
  pami_result_t result = PAMI_SUCCESS;

  for (size_t i = 0; i<ncontexts; ++i) {
    PAMI_assert(contexts[i] != NULL);
    PAMI::Context * ctx = (PAMI::Context *) contexts[i];
    PAMI::Client  * client = (PAMI::Client *) ctx->getClient ();
    pami_result_t rc = client->destroyContext (ctx);
    contexts[i] = NULL;
    if (result == PAMI_SUCCESS)
      result = rc;
  }

  return result;
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
  PAMI::Client * cln = (PAMI::Client *) client;

  return cln->queryConfiguration (configuration);
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
/// \copydoc PAMI_Context_advancev
///
extern "C" pami_result_t PAMI_Context_advancev (pami_context_t context[],
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
/// \copydoc PAMI_Memregion_create
///
extern "C" pami_result_t PAMI_Memregion_create (pami_context_t     context,
                                                void             * address,
                                                size_t             bytes_in,
                                                size_t           * bytes_out,
                                                pami_memregion_t * memregion)
{
    PAMI::Context   * ctx = (PAMI::Context *) context;
    return ctx->memregion_create (address, bytes_in, bytes_out, memregion);
}

///
/// \copydoc PAMI_Memregion_destroy
///
extern "C" pami_result_t PAMI_Memregion_destroy (pami_context_t     context,
                                                 pami_memregion_t * memregion)
{
    PAMI::Context   * ctx = (PAMI::Context *) context;
    return ctx->memregion_destroy (memregion);
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


///
/// \copydoc PAMI_Task2Network
///
extern "C" pami_result_t PAMI_Task2Network(pami_task_t task,
                                           pami_coord_t *ntw)
{
#if defined(PLATFORM_BGP) || defined(PLATFORM_BGQ)  
  return __global.mapping.task2network(task, ntw, PAMI_N_TORUS_NETWORK);
#else
  assert(0);
#endif
}


///
/// \copydoc PAMI_Network2Task
///
extern "C" pami_result_t PAMI_Network2Task(pami_coord_t ntw,
                                           pami_task_t *task)
{
  pami_network type;
  return __global.mapping.network2task(&ntw, task, &type);
}
