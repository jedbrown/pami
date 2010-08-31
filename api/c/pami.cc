/**
 * \file api/c/pami.cc
 * \brief PAMI C interface common implementation
 */
#ifndef PAMI_LAPI_IMPL
#include "config.h"
#endif //PAMI_LAPI_IMPL

#include "Global.h"
#include "Client.h"
#include "Context.h"
#include "PipeWorkQueue.h"
#include "Topology.h"
#include <pami.h>
#include "util/common.h"

extern "C"
{
  pami_type_t PAMI_BYTE = 0;
  pami_geometry_t PAMI_NULL_GEOMETRY = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_misc.h                                                  //
////////////////////////////////////////////////////////////////////////////////

extern "C" size_t PAMI_Error_text (char * string, size_t length)
{
  return 0; // unimplemented
}

extern "C" double PAMI_Wtime ()
{
  return __global.time.time();
}

extern "C" unsigned long long PAMI_Wtimebase()
{
  return __global.time.timebase();
}

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

extern "C" pami_result_t PAMI_Fence_begin (pami_context_t      context)

{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->fence_begin ();
}

extern "C" pami_result_t PAMI_Fence_end (pami_context_t      context)

{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->fence_end ();
}

extern "C" pami_result_t PAMI_Fence_all (pami_context_t        context,
                                       pami_event_function   done_fn,
                                       void               * cookie)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->fence_all (done_fn, cookie);
}

extern "C" pami_result_t PAMI_Fence_endpoint (pami_context_t        context,
                                              pami_event_function   done_fn,
                                              void                * cookie,
                                              pami_endpoint_t       endpoint)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->fence_endpoint (done_fn, cookie, endpoint);
}

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_collectives.h                                           //
////////////////////////////////////////////////////////////////////////////////


extern "C" pami_result_t PAMI_Geometry_create_taskrange (pami_client_t           client,
                                                         pami_configuration_t    configuration[],
                                                         size_t                  num_configs,
                                                         pami_geometry_t       * geometry,
                                                         pami_geometry_t         parent,
                                                         unsigned                id,
                                                         pami_geometry_range_t * task_slices,
                                                         size_t                  slice_count,
                                                         pami_context_t          context,
                                                         pami_event_function     fn,
                                                         void                  * cookie)
{
  PAMI::Client * _client = (PAMI::Client *) client;
  return _client->geometry_create_taskrange (geometry,
                                             configuration,
                                             num_configs,
                                             parent,
                                             id,
                                             task_slices,
                                             slice_count,
                                             context,
                                             fn,
                                             cookie);
}

extern "C" pami_result_t PAMI_Geometry_create_tasklist (pami_client_t               client,
                                                        pami_configuration_t        configuration[],
                                                        size_t                      num_configs,
                                                        pami_geometry_t           * geometry,
                                                        pami_geometry_t             parent,
                                                        unsigned                    id,
                                                        pami_task_t               * tasks,
                                                        size_t                      task_count,
                                                        pami_context_t              context,
                                                        pami_event_function         fn,
                                                        void                      * cookie)
{
  PAMI::Client * _client = (PAMI::Client *) client;
  return _client->geometry_create_tasklist (geometry,
                                            configuration,
                                            num_configs,
                                            parent,
                                            id,
                                            tasks,
                                            task_count,
                                            context,
                                            fn,
                                            cookie);
}

extern "C" pami_result_t PAMI_Geometry_create_topology(pami_client_t           client,
                                                       pami_configuration_t    configuration[],
                                                       size_t                  num_configs,
                                                       pami_geometry_t       * geometry,
                                                       pami_geometry_t         parent,
                                                       unsigned                id,
                                                       pami_topology_t       * topology,
                                                       pami_context_t          context,
                                                       pami_event_function     fn,
                                                       void                  * cookie)
{
  PAMI::Client * _client = (PAMI::Client *) client;
  return _client->geometry_create_topology(geometry,
                                           configuration,
                                           num_configs,
                                           parent,
                                           id,
                                           topology,
                                           context,
                                           fn,
                                           cookie);
}


extern "C" pami_result_t PAMI_Geometry_query (pami_geometry_t       geometry,
                                              pami_configuration_t  configuration[],
                                              size_t                num_configs)
{
  PAMI_GEOMETRY_CLASS* _geometry = (PAMI_GEOMETRY_CLASS *) geometry;
  PAMI::Client * _client = (PAMI::Client *)_geometry->getClient();
  return _client->geometry_query(geometry, configuration, num_configs);
}

extern "C"  pami_result_t PAMI_Geometry_update (pami_geometry_t       geometry,
                                                pami_configuration_t  configuration[],
                                                size_t                num_configs,
                                                pami_context_t        context,
                                                pami_event_function   fn,
                                                void                 *cookie)
{
  PAMI_GEOMETRY_CLASS* _geometry = (PAMI_GEOMETRY_CLASS *) geometry;
  PAMI::Client * _client = (PAMI::Client *)_geometry->getClient();
  return _client->geometry_update(geometry, configuration, num_configs, context, fn, cookie);
}

extern "C" pami_result_t PAMI_Geometry_world (pami_client_t                client,
                                 pami_geometry_t            * world_geometry)
{
  PAMI::Client * _client = (PAMI::Client *) client;
  return _client->geometry_world (world_geometry);
}

extern "C" pami_result_t PAMI_Geometry_destroy(pami_client_t    client,
                                               pami_geometry_t *geometry)
{
  PAMI::Client * _client = (PAMI::Client *) client;
  pami_result_t rc = _client->geometry_destroy (*geometry);
  *geometry = NULL;
  return rc;
}

extern "C" pami_result_t PAMI_Collective (pami_context_t   context,
                                        pami_xfer_t     *parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->collective (parameters);
}

extern "C" pami_result_t PAMI_Geometry_algorithms_num (pami_context_t context,
                                                     pami_geometry_t geometry,
                                                     pami_xfer_type_t coll_type,
                                                     size_t              lists_lengths[2])
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->geometry_algorithms_num (geometry,
                                      coll_type,
                                      lists_lengths);
}

extern "C"  pami_result_t PAMI_Geometry_algorithms_query (pami_context_t context,
                                                       pami_geometry_t geometry,
                                                       pami_xfer_type_t   colltype,
                                                       pami_algorithm_t  *algs0,
                                                       pami_metadata_t   *mdata0,
                                                       size_t             num0,
                                                       pami_algorithm_t  *algs1,
                                                       pami_metadata_t   *mdata1,
                                                       size_t               num1)
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
// Functions from pami_dispatch.h                                              //
////////////////////////////////////////////////////////////////////////////////


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
// Type function implementations                                              //
////////////////////////////////////////////////////////////////////////////////

extern "C" pami_result_t PAMI_Type_create (pami_type_t * type)
{
  return PAMI_UNIMPL;
}

extern "C" pami_result_t PAMI_Type_add_simple (pami_type_t type,
                                               size_t      bytes,
                                               size_t      offset,
                                               size_t      count,
                                               size_t      stride)
{
  return PAMI_UNIMPL;
}

extern "C" pami_result_t PAMI_Type_add_typed (pami_type_t type,
                                              pami_type_t subtype,
                                              size_t      offset,
                                              size_t      count,
                                              size_t      stride)
{
  return PAMI_UNIMPL;
}

extern "C" pami_result_t PAMI_Type_complete (pami_type_t type,
                                             size_t      atom_size)
{
  return PAMI_UNIMPL;
}

extern "C" pami_result_t PAMI_Type_destroy (pami_type_t *type)
{
  *type = NULL;
  return PAMI_UNIMPL;
}

extern "C" pami_result_t PAMI_Type_serialize (pami_type_t   type,
                                              void       ** address,
                                              size_t      * size)
{
  return PAMI_UNIMPL;
}

extern "C" pami_result_t PAMI_Type_deserialize (pami_type_t * type,
                                                void        * address,
                                                size_t        size)
{
  return PAMI_UNIMPL;
}

extern "C" pami_result_t PAMI_Type_query (pami_type_t           type,
                                          pami_configuration_t  configuration[],
                                          size_t                num_configs)
{
  return PAMI_UNIMPL;
}

////////////////////////////////////////////////////////////////////////////////
// Functions from pami_pipeworkqueue.h                                         //
////////////////////////////////////////////////////////////////////////////////




void PAMI_PipeWorkQueue_config_circ(pami_pipeworkqueue_t *wq,
                                   size_t bufsize)
{
    PAMI_abort();
}

void PAMI_PipeWorkQueue_config_circ_usr(pami_pipeworkqueue_t *wq,
                                       char                *buffer,
                                       size_t               bufsize)
{
    PAMI_abort();
}

void PAMI_PipeWorkQueue_config_flat(pami_pipeworkqueue_t *wq,
                                   char                *buffer,
                                   size_t               bufsize,
                                   size_t               bufinit)
{
    PAMI_abort();
}

void PAMI_PipeWorkQueue_config_noncontig(pami_pipeworkqueue_t *wq,
                                        char                *buffer,
                                        pami_type_t          *type,
                                        size_t               typecount,
                                        size_t               typeinit)
{
    PAMI_abort();
}

pami_result_t PAMI_PipeWorkQueue_export(pami_pipeworkqueue_t     *wq,
                                      pami_pipeworkqueue_ext_t *exp)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->exp (exp);
}

pami_result_t PAMI_PipeWorkQueue_import(pami_pipeworkqueue_ext_t *import,
                                      pami_pipeworkqueue_t     *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->import (import);
}

void PAMI_PipeWorkQueue_clone(pami_pipeworkqueue_t *wq,
                             pami_pipeworkqueue_t *obj)
{
}

void PAMI_PipeWorkQueue_destroy(pami_pipeworkqueue_t *wq)
{
}

void PAMI_PipeWorkQueue_reset(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->reset();
}


void PAMI_PipeWorkQueue_dump(pami_pipeworkqueue_t *wq,
                            const char          *prefix)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->dump (prefix);
}

void PAMI_PipeWorkQueue_setConsumerWakeup(pami_pipeworkqueue_t *wq,
                                         void                *vec)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->setConsumerWakeup (vec);
}

void PAMI_PipeWorkQueue_setProducerWakeup(pami_pipeworkqueue_t *wq,
                                         void                *vec)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->setProducerWakeup (vec);
}

void PAMI_PipeWorkQueue_setProducerUserInfo(pami_pipeworkqueue_t *wq,
                                         void *word1, void *word2)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->setProducerUserInfo (word1, word2);
}

void PAMI_PipeWorkQueue_setConsumerUserInfo(pami_pipeworkqueue_t *wq,
                                         void *word1, void *word2)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->setConsumerUserInfo (word1, word2);
}

void PAMI_PipeWorkQueue_getProducerUserInfo(pami_pipeworkqueue_t *wq,
                                         void **word1, void **word2)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->getProducerUserInfo (word1, word2);
}

void PAMI_PipeWorkQueue_getConsumerUserInfo(pami_pipeworkqueue_t *wq,
                                         void **word1, void **word2)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    pwq->getConsumerUserInfo (word1, word2);
}

size_t PAMI_PipeWorkQueue_bytesAvailableToProduce(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->bytesAvailableToProduce ();
}

size_t PAMI_PipeWorkQueue_bytesAvailableToConsume(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->bytesAvailableToConsume ();
}

size_t PAMI_PipeWorkQueue_getBytesProduced(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->getBytesProduced ();
}

size_t PAMI_PipeWorkQueue_getBytesConsumed(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->getBytesConsumed ();
}

char *PAMI_PipeWorkQueue_bufferToProduce(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->bufferToProduce ();
}

void PAMI_PipeWorkQueue_produceBytes(pami_pipeworkqueue_t *wq,
                                    size_t               bytes)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->produceBytes (bytes);
}

char *PAMI_PipeWorkQueue_bufferToConsume(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->bufferToConsume ();
}

void PAMI_PipeWorkQueue_consumeBytes(pami_pipeworkqueue_t *wq,
                                    size_t               bytes)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->consumeBytes (bytes);
}

int PAMI_PipeWorkQueue_available(pami_pipeworkqueue_t *wq)
{
    PAMI::PipeWorkQueue * pwq = (PAMI::PipeWorkQueue *) wq;
    return pwq->available ();
}



////////////////////////////////////////////////////////////////////////////////
// Functions from pami_topology.h                                              //
////////////////////////////////////////////////////////////////////////////////


void PAMI_Topology_create(pami_topology_t *topo)
{
    PAMI_abort();
}

void PAMI_Topology_create_rect(pami_topology_t *topo,
                              pami_coord_t *ll, pami_coord_t *ur, unsigned char *tl)
{
    PAMI_abort();
}

void PAMI_Topology_create_axial(pami_topology_t *topo,
                               pami_coord_t *ll,
                               pami_coord_t *ur,
                               pami_coord_t *ref,
                               unsigned char *dir,
                               unsigned char *tl)
{
  PAMI_abort();
}

void PAMI_Topology_create_task(pami_topology_t *topo, pami_task_t rank)
{
    PAMI_abort();
}

void PAMI_Topology_create_range(pami_topology_t *topo, pami_task_t rank0, pami_task_t rankn)
{
  new(topo)PAMI::Topology(rank0,rankn);
}

void PAMI_Topology_create_list(pami_topology_t *topo, pami_task_t *ranks, size_t nranks)
{
  new(topo)PAMI::Topology(ranks,nranks);
}

void PAMI_Topology_destroy(pami_topology_t *topo)
{
    PAMI_abort();
}

unsigned PAMI_Topology_size_of(pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->size_of();
}

size_t PAMI_Topology_size(pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->size();
}

pami_topology_type_t pami_topology_type(pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->type();
}

pami_task_t PAMI_Topology_index2TaskID(pami_topology_t *topo, size_t ix)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->index2Rank(ix);
}

size_t PAMI_Topology_taskID2Index(pami_topology_t *topo, pami_task_t rank)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->rank2Index(rank);
}

pami_result_t PAMI_Topology_taskRange(pami_topology_t *topo, pami_task_t *first, pami_task_t *last)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->rankRange(first,last);
}

pami_result_t PAMI_Topology_taskList(pami_topology_t *topo, pami_task_t **list)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->rankList(list);
}

pami_result_t PAMI_Topology_rectSeg(pami_topology_t *topo,
                                  pami_coord_t *ll, pami_coord_t *ur,
                                  unsigned char *tl)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->rectSeg(ll,ur,tl);
}

int PAMI_Topology_isLocalToMe(pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->isLocalToMe();
}

int PAMI_Topology_isRectSeg(pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->isRectSeg();
}

void PAMI_Topology_getNthDims(pami_topology_t *topo, unsigned n,
                             unsigned *c0, unsigned *cn, unsigned char *tl)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->getNthDims(n,c0,cn,tl);
}

int PAMI_Topology_isTaskMember(pami_topology_t *topo, pami_task_t rank)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->isRankMember(rank);
}

int PAMI_Topology_isCoordMember(pami_topology_t *topo, pami_coord_t *c0)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->isCoordMember(c0);
}

void PAMI_Topology_sub_LocalToMe(pami_topology_t *_new, pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    t->subTopologyLocalToMe((PAMI::Topology *)_new);
}

void PAMI_Topology_sub_NthGlobal(pami_topology_t *_new, pami_topology_t *topo, int n)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    t->subTopologyNthGlobal((PAMI::Topology *)_new, n);
}

void PAMI_Topology_sub_ReduceDims(pami_topology_t *_new, pami_topology_t *topo, pami_coord_t *fmt)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    t->subTopologyReduceDims((PAMI::Topology *)_new, fmt);
}

void PAMI_Topology_getTaskList(pami_topology_t *topo, size_t max, pami_task_t *ranks, size_t *nranks)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    t->getRankList(max,ranks,nranks);
}

int  PAMI_Topology_analyze(pami_topology_t *topo)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->analyzeTopology();
}

int  PAMI_Topology_convert(pami_topology_t *topo, pami_topology_type_t new_type)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->convertTopology(new_type);
}

void PAMI_Topology_intersect(pami_topology_t *_new, pami_topology_t *topo, pami_topology_t *other)
{
    PAMI::Topology * t = (PAMI::Topology *) topo;
    return t->intersectTopology((PAMI::Topology *)_new, (PAMI::Topology *)other);
}

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


extern "C" pami_result_t PAMI_Client_create (const char           *name,
                                             pami_client_t        *client,
                                             pami_configuration_t  configuration[],
                                             size_t                num_configs)
{
  return PAMI::Client::generate (name, client, configuration, num_configs);
}

extern "C" pami_result_t PAMI_Client_destroy (pami_client_t *client)
{
  PAMI::Client::destroy ((PAMI::Client *) *client);
  *client = NULL;
  return PAMI_SUCCESS;
}



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

extern "C" pami_result_t PAMI_Context_destroyv (pami_context_t* contexts,
                                                size_t          ncontexts)
{
  PAMI_assert(contexts != NULL);
  pami_result_t result = PAMI_SUCCESS;

  // have to fudge a client object out of this, since the API is not symmetrical
  if (ncontexts >= 1)
  {
    PAMI::Context * ctx = (PAMI::Context *) contexts[0];
    PAMI::Client  * client = (PAMI::Client *) ctx->getClient ();
    result = client->destroyContext (contexts, ncontexts);
  }
  return result;
}

extern "C" pami_result_t PAMI_Client_query (pami_client_t         client,
                                            pami_configuration_t  configuration[],
                                            size_t                num_configs)
{
  PAMI::Client * cln = (PAMI::Client *) client;

  return cln->query(configuration,num_configs);
}

extern "C" pami_result_t PAMI_Client_update (pami_client_t         client,
                                             pami_configuration_t  configuration[],
                                             size_t                num_configs)
{
  PAMI::Client * cln = (PAMI::Client *) client;
  return cln->update(configuration,num_configs);
}

extern "C" pami_result_t PAMI_Dispatch_query (pami_context_t        context,
                                              size_t                dispatch,
                                              pami_configuration_t  configuration[],
                                              size_t                num_configs)
{
    PAMI::Context * ctx = (PAMI::Context *) context;
    return ctx->dispatch_query (dispatch, configuration, num_configs);
}

extern "C" pami_result_t PAMI_Dispatch_update (pami_context_t        context,
                                               size_t                dispatch,
                                               pami_configuration_t  configuration[],
                                               size_t                num_configs)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->dispatch_update(dispatch, configuration, num_configs);
}



extern "C" pami_result_t PAMI_Context_query (pami_context_t        context,
                                             pami_configuration_t  configuration[],
                                             size_t                num_configs)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->query(configuration, num_configs);
}

extern "C" pami_result_t PAMI_Context_update (pami_context_t        context,
                                              pami_configuration_t  configuration[],
                                              size_t                num_configs)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->update(configuration, num_configs);
}



extern "C" pami_result_t PAMI_Dispatch_set (pami_context_t              context,
                                          size_t                     dispatch,
                                          pami_dispatch_callback_fn   fn,
                                          void                     * cookie,
                                          pami_send_hint_t            options)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->dispatch (dispatch, fn, cookie, options);
}


extern "C" pami_result_t PAMI_Context_lock (pami_context_t context)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->lock ();
}

extern "C" pami_result_t PAMI_Context_trylock (pami_context_t context)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->trylock ();
}

extern "C" pami_result_t PAMI_Context_unlock (pami_context_t context)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->unlock ();
}


extern "C" pami_result_t PAMI_Context_advance (pami_context_t context, size_t maximum)
{
  pami_result_t result;
  PAMI::Context * ctx = (PAMI::Context *) context;
  ctx->advance (maximum, result);

  return result;
}

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

extern "C" pami_result_t PAMI_Send (pami_context_t   context,
                                  pami_send_t    * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->send (parameters);
}

extern "C" pami_result_t PAMI_Send_immediate (pami_context_t          context,
                                            pami_send_immediate_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->send (parameters);
}

extern "C" pami_result_t PAMI_Send_typed (pami_context_t      context,
                                        pami_send_typed_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->send (parameters);
}


extern "C" pami_result_t PAMI_Put (pami_context_t      context,
                                 pami_put_simple_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->put (parameters);
}


extern "C" pami_result_t PAMI_Put_typed (pami_context_t      context,
                                       pami_put_typed_t  * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->put_typed (parameters);
}

extern "C" pami_result_t PAMI_Get (pami_context_t      context,
                                 pami_get_simple_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->get (parameters);
}


extern "C" pami_result_t PAMI_Get_typed (pami_context_t      context,
                                       pami_get_typed_t  * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->get_typed (parameters);
}


extern "C" pami_result_t PAMI_Rmw (pami_context_t      context,
                                 pami_rmw_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->rmw (parameters);
}

extern "C" pami_result_t PAMI_Memregion_create (pami_context_t     context,
                                                void             * address,
                                                size_t             bytes_in,
                                                size_t           * bytes_out,
                                                pami_memregion_t * memregion)
{
    PAMI::Context   * ctx = (PAMI::Context *) context;
    return ctx->memregion_create (address, bytes_in, bytes_out, memregion);
}

extern "C" pami_result_t PAMI_Memregion_destroy (pami_context_t     context,
                                                 pami_memregion_t * memregion)
{
    PAMI::Context   * ctx = (PAMI::Context *) context;
    return ctx->memregion_destroy (memregion);
}

extern "C" pami_result_t PAMI_Rput (pami_context_t      context,
                                 pami_rput_simple_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->rput (parameters);
}


extern "C" pami_result_t PAMI_Rput_typed (pami_context_t      context,
                                       pami_rput_typed_t  * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->rput_typed (parameters);
}

extern "C" pami_result_t PAMI_Rget (pami_context_t      context,
                                 pami_rget_simple_t * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->rget (parameters);
}


extern "C" pami_result_t PAMI_Rget_typed (pami_context_t      context,
                                        pami_rget_typed_t  * parameters)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->rget_typed (parameters);
}


extern "C" pami_result_t PAMI_Purge_totask (pami_context_t   context,
                                          size_t        * dest,
                                          size_t          count)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->purge_totask (dest, count);
}


extern "C" pami_result_t PAMI_Resume_totask (pami_context_t   context,
                                           size_t        * dest,
                                           size_t          count)
{
  PAMI::Context * ctx = (PAMI::Context *) context;
  return ctx->resume_totask (dest, count);
}


extern "C" pami_result_t PAMI_Task2Network(pami_task_t task,
                                           pami_coord_t *ntw)
{
#if defined(__pami_target_bgq__) || defined(__pami_target_bgp__)
  return __global.mapping.task2network(task, ntw, PAMI_N_TORUS_NETWORK);
#else
  PAMI_abort();
  return PAMI_ERROR;
#endif
}


extern "C" pami_result_t PAMI_Network2Task(pami_coord_t ntw,
                                           pami_task_t *task)
{
  pami_network type;
  return __global.mapping.network2task(&ntw, task, &type);
}

namespace CCMI { namespace Adaptor { namespace Allreduce {
      extern void getReduceFunction(pami_dt, pami_op, unsigned,
                                    unsigned&, coremath&);
    }}};
extern "C" pami_result_t PAMI_Dt_query (pami_dt dt, size_t *size)
{
  coremath cb_allreduce  = NULL;
  pami_op  op            = PAMI_NOOP;
  unsigned nelems        = 0;
  unsigned sz            = 0;
  CCMI::Adaptor::Allreduce::getReduceFunction(dt,op,nelems,sz,cb_allreduce);
  *size = sz;
  return PAMI_SUCCESS;
}

#include "api/extension/Extension.h"
#include "api/extension/registry.h"

extern "C" pami_result_t PAMI_Extension_open (pami_client_t      client,
                                              const char       * name,
                                              pami_extension_t * extension)
{
  #define PAMI_EXTENSION_DEFINE(a,n,x,y)                    \
    if (strcmp(#a, name) == 0)                              \
      return PAMI::Extension::open<n>(client, #a, *extension);

  #include "api/extension/registry.def"

  #undef PAMI_EXTENSION_DEFINE

  return PAMI::Extension::open<0>(client, name, *extension);
}

extern "C" pami_result_t PAMI_Extension_close (pami_extension_t extension)
{
  PAMI::Extension * x = (PAMI::Extension *) extension;
  return x->close();
}

extern "C" void * PAMI_Extension_function (pami_extension_t   extension,
                                           const char       * fn)
{
  PAMI::Extension * x = (PAMI::Extension *) extension;
  return x->function (fn);
}


