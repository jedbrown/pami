///
/// \file common/bgq/Client.h
/// \brief PAMI client interface specific for the Blue Gene\Q platform.
///
#ifndef __common_bgq_Client_h__
#define __common_bgq_Client_h__

#include <stdlib.h>

#include "common/ClientInterface.h"

#include "Context.h"
#include <errno.h>

#ifdef USE_COMMTHREADS
#include "components/devices/bgq/commthread/CommThreadWakeup.h"
#endif // USE_COMMTHREADS

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

#include "algorithms/geometry/GeometryOptimizer.h"

namespace PAMI
{
  class Client : public Interface::Client<PAMI::Client>
  {
    public:
      inline Client (const char * name, pami_result_t &result) :
          Interface::Client<PAMI::Client>(name, result),
          _client ((pami_client_t) this),
          _references (1),
          _ncontexts (0),
          _world_geometry((BGQGeometry*)_world_geometry_storage),
          _mm ()
      {
        static size_t next_client_id = 0;
        // Set the client name string.
        memset ((void *)_name, 0x00, sizeof(_name));
        strncpy (_name, name, sizeof(_name) - 1);

        _clientid = next_client_id++;
        TRACE_ERR((stderr, "<%p:%zu>BGQ::Client() %s\n", this, _clientid, _name));
        // PAMI_assert(_clientid < PAMI_MAX_NUM_CLIENTS);

        // Get some shared memory for this client
        initializeMemoryManager ();

        _world_range.lo = 0;
        _world_range.hi = __global.mapping.size() - 1;
        /// \todo This should be using the global topology and NOT de-optimize to a range!
        /// new(_world_geometry_storage) BGQGeometry(_client, NULL, &__global.mapping, 0, &__global.topology_global);
        new(_world_geometry_storage) BGQGeometry(_client, NULL, &__global.mapping, 0, 1, &_world_range, &_geometry_map);
        // This must return immediately (must not enqueue non-blocking ops).
        // Passing a NULL context should ensure that.
        __MUGlobal.getMuRM().geomOptimize(_world_geometry, _clientid, 0, NULL, NULL, NULL);
        // Now, subsequent 'analyze' done on this geom will know that MU Coll is avail.

	_geomopt = NULL;

        result = PAMI_SUCCESS;
      }

      inline ~Client ()
      {
        if (_contexts) (void)destroyContext_impl(NULL, _ncontexts);
	if (_geomopt) __global.heap_mm->free(_geomopt);
      }

      static pami_result_t generate_impl (const char * name, pami_client_t * client,
                                          pami_configuration_t   configuration[],
                                          size_t                 num_configs)
      {
        TRACE_ERR((stderr, "<%p>BGQ::Client::generate_impl\n", client));
        pami_result_t result;

        // If a client with this name is not already initialized...
        PAMI::Client * clientp = NULL;
        result = __global.heap_mm->memalign((void **)&clientp, 16, sizeof(*clientp));
        PAMI_assertf(result == PAMI_SUCCESS, "Failed to alloc PAMI::Client"); // just return?

        memset ((void *)clientp, 0x00, sizeof(PAMI::Client));
        new (clientp) PAMI::Client (name, result);
        *client = clientp;
        return result;
      }

      static void destroy_impl (pami_client_t client)
      {
        TRACE_ERR((stderr, "<%p>BGQ::Client::destroy_impl\n", client));
        PAMI::Client *clt = (PAMI::Client *)client;
        // ensure contexts are destroyed first???
        clt->~Client();
        __global.heap_mm->free((void *)client);
      }

      inline size_t maxContexts_impl ()
      {
        /// \todo #99 Remove this hack and replace with a device interface
        size_t result;
        unsigned tSize = __global.mapping.tSize();
        switch (tSize)
          {
            // The following values are based on tests just running MPI_Init():
            //    Date:      24 Mar 2011
            //    Num nodes: 32
            //    BG_SHAREDMEMSIZE=32
            case 1:
              result = 16;
              break;
            case 2:
              result = 16;
              break;
            case 4:
              result = 8;
              break;
            case 8:
            case 16:
            case 32:
            case 64:
              result = 64/tSize;
              break;
            default:
              PAMI_abortf("Error: Do not know how to handle tSize=%u\n", tSize);
          }
        return result;
      }

      inline char * getName_impl ()
      {
        return _name;
      }

      inline pami_result_t createContext_impl (pami_configuration_t   configuration[],
                                               size_t                count,
                                               pami_context_t       * context,
                                               size_t                ncontexts)
      {
        TRACE_ERR((stderr, "<%p:%zu>BGQ::Client::createContext_impl\n", this, _clientid));
        //_context_list->lock ();
        size_t n = ncontexts;

        if (_ncontexts != 0)
          {
            return PAMI_ERROR;
          }

        if (ncontexts > maxContexts_impl())
          {
            return PAMI_INVAL;
          }

        pami_result_t rc;
        rc = __global.heap_mm->memalign((void **)&_contexts, 16, sizeof(*_contexts) * n);
        PAMI_assertf(rc == PAMI_SUCCESS, "alloc failed for _contexts[%zu], errno=%d\n", n, errno);
        TRACE_ERR((stderr, "BGQ::Client::createContext mm available %zu\n", _mm.available()));
        _platdevs.generate(_clientid, n, _mm); // _mm is the client-scoped shared memory manager
        // _platdevs.generate(_clientid, n, _mm, __global._wuRegion[_clientid]->_wu_mm);

        // This memset has been removed due to the amount of cycles it takes
        // on simulators.  Lower level initializers should be setting the
        // relevant fields of the context, so this memset should not be
        // needed anyway.
        //memset((void *)_contexts, 0, sizeof(PAMI::Context) * n);
        size_t bytes = 135 * 1024;

        /// \page env_vars Environment Variables
        ///
        /// PAMI_CONTEXT_SHMEMSIZE - Size, bytes, of each Context's shmem pool.
        /// May use 'K' or 'M' suffix as multiplier. default: 135K
        ///
        char *env = getenv("PAMI_CONTEXT_SHMEMSIZE");
        if (env) {
          char *s = NULL;
          bytes = strtoull(env, &s, 0);
          if (*s == 'm' || *s == 'M') bytes *= 1024 * 1024;
          else if (*s == 'k' || *s == 'K') bytes *= 1024;
          }
        TRACE_ERR((stderr, "BGQ::Client::createContext mm bytes %zu\n", bytes));
        char key[PAMI::Memory::MMKEYSIZE];
        sprintf(key, "/pami-clt%zd-ctx-mm", _clientid);
        rc = _xmm.init(__global.shared_mm, bytes * n, 0, 0, 0, key);
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to create \"%s\" mm for %zd bytes",
                     key, bytes * n);

        for (size_t x = 0; x < n; ++x)
          {
            context[x] = (pami_context_t) & _contexts[x];
#ifdef USE_COMMTHREADS
            // Note, this is not inializing each comm thread but rather
            // initializing comm threads for each context. context[x] is not usable yet,
            // but it won't matter since this object can't do anything with it anyway.
            // This must initialize before the context, so that MemoryManagers are
            // setup.
            PAMI::Device::CommThread::BgqCommThread::initContext(_clientid, x, context[x]);
#endif // USE_COMMTHREADS
            new (&_contexts[x]) PAMI::Context(this->getClient(), _clientid, x, n,
                                   &_platdevs, &_xmm, bytes, _world_geometry, &_geometry_map);
            //_context_list->pushHead((QueueElem *)&context[x]);
            //_context_list->unlock();
          }

        _ncontexts = (size_t)n;
        TRACE_ERR((stderr,  "%s ncontexts %zu exit\n", __PRETTY_FUNCTION__, _ncontexts));

        return PAMI_SUCCESS;
      }

      // DEPRECATED!
      inline pami_result_t destroyContext_impl (pami_context_t context)
      {
        PAMI_abortf("single context destroy not supported");
        return PAMI_ERROR;
      }
      inline pami_result_t destroyContext_impl (pami_context_t *context, size_t ncontexts)
      {
        PAMI_assertf(ncontexts == _ncontexts, "destroyContext(%p,%zu) called without all contexts (expected %zu contexts)", context, ncontexts, _ncontexts);
        // for (i = 0.._ncontexts) PAMI_assertf(context[i] == &_contexts[i], "...");
#ifdef USE_COMMTHREADS
        // This removes all contexts... only needs to be called once.
        PAMI::Device::CommThread::BgqCommThread::rmContexts(_clientid, _contexts, _ncontexts);
#endif // USE_COMMTHREADS
        pami_result_t res = PAMI_SUCCESS;
        size_t i;

        for (i = 0; i < _ncontexts; ++i)
          {
            if (context != NULL)
              context[i] = NULL;

            PAMI::Context * ctx = &_contexts[i];
            ctx->~Context();
            pami_result_t rc = ctx->destroy ();

            //_context_list->unlock ();
            if (rc != PAMI_SUCCESS) res = rc;
          }

        __global.heap_mm->free(_contexts);
        _contexts = NULL;
        _ncontexts = 0;
        return res;
      }

#ifdef USE_COMMTHREADS
      // This is not standard interface... yet?
      inline pami_result_t addContextToCommThreadPool(pami_context_t ctx)
      {
          return PAMI::Device::CommThread::BgqCommThread::addContext(_clientid, ctx);
      }
#endif // USE_COMMTHREADS

      inline pami_result_t query_impl (pami_configuration_t configuration[],
                                       size_t               num_configs)
      {
        pami_result_t result = PAMI_SUCCESS;
        size_t i;

        for (i = 0; i < num_configs; i++)
          {
            switch (configuration[i].name)
              {
                case PAMI_CLIENT_NUM_CONTEXTS:
                  configuration[i].value.intval = maxContexts_impl();
                  break;
                case PAMI_CLIENT_CONST_CONTEXTS:
                  configuration[i].value.intval = true;
                  break;
                case PAMI_CLIENT_TASK_ID:
                  configuration[i].value.intval = __global.mapping.task();
                  break;
                case PAMI_CLIENT_NUM_TASKS:
                  configuration[i].value.intval = __global.mapping.size();
                  break;
                case PAMI_CLIENT_NUM_LOCAL_TASKS:
                  __global.mapping.nodePeers (configuration[i].value.intval);
                  break;
                case PAMI_CLIENT_LOCAL_TASKS:
                  configuration[i].value.intarray = __global.mapping.getPeer2TaskArray();
                  break;
                case PAMI_CLIENT_CLOCK_MHZ:
                case PAMI_CLIENT_WTIMEBASE_MHZ:
                  configuration[i].value.intval = __global.time.clockMHz();
                  break;
                case PAMI_CLIENT_WTICK:
                  configuration[i].value.doubleval = __global.time.tick();
                  break;
                case PAMI_CLIENT_PROCESSOR_NAME:
                {
                  int rc;
                  char* pn = __global.processor_name;
                  /// \todo This should be more descriptive and the
                  /// snprintf() should be run at init only.  This is
                  /// the BGP version:
                  /// "Rank 0 of 128 (0,0,0,0)  R00-M0-N10-J01"
                  rc = snprintf(pn, 128, "Task %zu of %zu", __global.mapping.task(), __global.mapping.size());
                  pn[128-1] = 0;
                  configuration[i].value.chararray = pn;

                  if (rc <= 0)
                    result = PAMI_INVAL;
                }
                break;
                case PAMI_CLIENT_HWTHREADS_AVAILABLE:
                  configuration[i].value.intval = 64 / __global.mapping.tSize();
                  break;
                case PAMI_CLIENT_MEM_SIZE:
                  configuration[i].value.intval = __global.personality.memSize();
                  break;
                default:
                  result = PAMI_INVAL;
              }
          }

        return result;
      }

      inline pami_result_t update_impl (pami_configuration_t configuration[],
                                        size_t               num_configs)
      {
        return PAMI_INVAL;
      }





      // the friend clause is actually global, but this helps us remember why...
      //friend class PAMI::Device::Generic::Device;
      //friend class pami.cc

      inline size_t getNumContexts()
      {
        return _ncontexts;
      }

      inline PAMI::Context *getContext(size_t ctx)
      {
        return _contexts + ctx;
      }

      inline PAMI::Context *getContexts()
      {
        return _contexts;
      }

      inline size_t getClientId()
      {
        return _clientid;
      }
      inline pami_result_t geometry_world_impl (pami_geometry_t * world_geometry)
      {
        *world_geometry = _world_geometry;
        return PAMI_SUCCESS;
      }

      static void _geom_newopt_start(pami_context_t context, void *cookie, pami_result_t err)
      {
        TRACE_ERR((stderr, "BGQ::Client::_geom_newopt_start cookie %p, context %p, error %u\n", cookie, context, err));
        PAMI_assertf(context, "Geometry create barrier callback with NULL context");

        if (err != PAMI_SUCCESS)
          {
            _geom_newopt_finish(context, cookie, err);
            return;
          }

        BGQGeometry *gp = (BGQGeometry *)cookie;
        PAMI::Context *ctxt = (PAMI::Context *)context;
        Client *thus = (Client *)ctxt->getClient();
        pami_result_t rc = __MUGlobal.getMuRM().geomOptimize(gp, thus->_clientid,
                                                             ctxt->getId(), context, _geom_newopt_finish, cookie);

        if (rc != PAMI_SUCCESS)
          {
            _geom_newopt_finish(context, cookie, rc);
            return;
          }
      }

      static void _geom_opt_finish(pami_context_t context, void *cookie, pami_result_t err)
      {
        TRACE_ERR((stderr, "BGQ::Client::_geom_opt_finish cookie %p, context %p, error %u\n", cookie, context, err));
        BGQGeometry *gp = (BGQGeometry *)cookie;

        if (context)   // HACK! until no one calls completion with NULL context!
          {
            PAMI::Context *ctxt = (PAMI::Context *)context;

            /// \todo #warning must destroy the new geometry on error
            if (err == PAMI_SUCCESS)
              {
                Client *thus = (Client *)ctxt->getClient();

                for (size_t n = 0; n < thus->_ncontexts; n++)
                  {
                    thus->_contexts[n].analyze(n, gp, 1);
                  }
              }
          }

        // non-fatal errors - do not destroy geometry (caller might have different plans)
        gp->rmCompletion(context, err);
      }

      static void _geom_newopt_finish(pami_context_t context, void *cookie, pami_result_t err)
      {
        TRACE_ERR((stderr, "BGQ::Client::_geom_newopt_finish cookie %p, context %p, error %u\n", cookie, context, err));
        BGQGeometry *gp = (BGQGeometry *)cookie;
        //PAMI::Context *ctxt = (PAMI::Context *)context;

        if (err == PAMI_SUCCESS)
          {
            gp->addCompletion();
            _geom_opt_finish(context, cookie, err);
            gp->rmCompletion(context, err); // completion happens here instead of
            // inside _geom_opt_finish().
            // trivial diff, right now.
            return;
          }

        /// \todo #warning must destroy the new geometry on error
        // but, how to do that while we're standing on it...
        gp->rmCompletion(context, err);
        // we have to assume geomOptimize() cleaned up after itself, but
        // what else do we need to cleanup? What might the analyze phases
        // have done? Right now, nothing does proper cleanup...
        // gp->~BGQGeometry(); ???
      }

      inline pami_result_t geometry_create_taskrange_impl(pami_geometry_t       * geometry,
                                                          pami_configuration_t   configuration[],
                                                          size_t                 num_configs,
                                                          pami_geometry_t         parent,
                                                          unsigned               id,
                                                          pami_geometry_range_t * rank_slices,
                                                          size_t                 slice_count,
                                                          pami_context_t          context,
                                                          pami_event_function     fn,
                                                          void                 * cookie)
      {
        TRACE_ERR((stderr, "<%p:%zu>BGQ::Client::geometry_create_taskrange_impl  geometry %p/%p\n", this, _clientid, geometry, *geometry));

        // simple for now: only PAMI_GEOMETRY_OPTIMIZE
        if (num_configs != 0 && (num_configs > 1 || configuration[0].name != PAMI_GEOMETRY_OPTIMIZE))
          {
            return PAMI_INVAL;
          }

        BGQGeometry *new_geometry;
        BGQGeometry *bargeom = (BGQGeometry *)parent;
        PAMI::Context *ctxt = (PAMI::Context *)context;

        if (geometry != NULL)
          {
          pami_result_t rc;
          rc = __global.heap_mm->memalign((void **)&new_geometry, 0,
                                          sizeof(*new_geometry)); /// \todo use allocator
          PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc new_geometry");
            new (new_geometry) BGQGeometry(_client,
                                           (PAMI::Geometry::Common *)parent,
                                           &__global.mapping,
                                           id,
                                           slice_count,
                                           rank_slices,
                                           &_geometry_map);

            TRACE_ERR((stderr,  "%s analyze %zu geometry %p\n", __PRETTY_FUNCTION__, _ncontexts, new_geometry));

            for (size_t n = 0; n < _ncontexts; n++)
              {
                TRACE_ERR((stderr,  "%s analyze %p geometry %p\n", __PRETTY_FUNCTION__, &_contexts[n], new_geometry));
                _contexts[n].analyze(n, new_geometry, 0);
              }

            *geometry = (pami_geometry_t) new_geometry;

            /// \todo  deliver completion to the appropriate context
            new_geometry->setCompletion(fn, cookie);
            new_geometry->addCompletion(); // ensure completion doesn't happen until
                                           // all have been analyzed (_geom_opt_finish).

            // Start the barrier (and then the global analyze and (maybe) the optimize ...
            start_barrier(bargeom, new_geometry,
                          ctxt->getId(), context,
                          num_configs? PAMI_GEOMETRY_OPTIMIZE: (pami_attribute_name_t)-1);

            new_geometry->processUnexpBarrier(&_ueb_queue,
                                              &_ueb_allocator);
          }
        else
          {
            // non-participant members of parent won't know if new geom exists...
            bargeom->default_barrier(fn, cookie, ctxt->getId(), context);
          }

        TRACE_ERR((stderr,  "%s exit geometry %p/%p\n", __PRETTY_FUNCTION__, geometry, *geometry));
        return PAMI_SUCCESS;
      }


      inline pami_result_t geometry_create_topology_impl(pami_geometry_t       *geometry,
                                                         pami_configuration_t   configuration[],
                                                         size_t                 num_configs,
                                                         pami_geometry_t        parent,
                                                         unsigned               id,
                                                         pami_topology_t       *topology,
                                                         pami_context_t         context,
                                                         pami_event_function    fn,
                                                         void                  *cookie)
      {
        TRACE_ERR((stderr, "<%p:%zu>BGQ::Client::geometry_create_topology_impl geometry %p/%p\n", this, _clientid, geometry, *geometry));

        // simple for now: only PAMI_GEOMETRY_OPTIMIZE
        if (num_configs != 0 && (num_configs > 1 || configuration[0].name != PAMI_GEOMETRY_OPTIMIZE))
          {
            return PAMI_INVAL;
          }

        BGQGeometry *new_geometry;
        BGQGeometry *bargeom = (BGQGeometry *)parent;
        PAMI::Context *ctxt = (PAMI::Context *)context;

        if (geometry != NULL)
          {
          pami_result_t rc;
          rc = __global.heap_mm->memalign((void **)&new_geometry, 0,
                                          sizeof(*new_geometry)); /// \todo use allocator
          PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc new_geometry");
            new (new_geometry) BGQGeometry(_client,
                                           (PAMI::Geometry::Common *)parent,
                                           &__global.mapping,
                                           id,
                                           (PAMI::Topology *)topology,
                                           &_geometry_map);

            TRACE_ERR((stderr,  "%s analyze %zu geometry %p\n", __PRETTY_FUNCTION__, _ncontexts, new_geometry));

            for (size_t n = 0; n < _ncontexts; n++)
              {
                TRACE_ERR((stderr,  "%s analyze %p geometry %p\n", __PRETTY_FUNCTION__, &_contexts[n], new_geometry));
                _contexts[n].analyze(n, new_geometry, 0);
              }

            *geometry = (pami_geometry_t) new_geometry;

            /// \todo  deliver completion to the appropriate context
            new_geometry->setCompletion(fn, cookie);
            new_geometry->addCompletion(); // ensure completion doesn't happen until
                                           // all have been analyzed (_geom_opt_finish).

            // Start the barrier (and then the global analyze and (maybe) the optimize ...
            start_barrier(bargeom, new_geometry,
                          ctxt->getId(), context,
                          num_configs? PAMI_GEOMETRY_OPTIMIZE: (pami_attribute_name_t)-1);

            new_geometry->processUnexpBarrier(&_ueb_queue,
                                              &_ueb_allocator);
          }
        else
          {
            // non-participant members of parent won't know if new geom exists...
            bargeom->default_barrier(fn, cookie, ctxt->getId(), context);
          }

        TRACE_ERR((stderr,  "%s exit geometry %p/%p\n", __PRETTY_FUNCTION__, geometry, *geometry));
        return PAMI_SUCCESS;
      }


      inline pami_result_t geometry_create_tasklist_impl(pami_geometry_t       * geometry,
                                                         pami_configuration_t   configuration[],
                                                         size_t                 num_configs,
                                                         pami_geometry_t         parent,
                                                         unsigned               id,
                                                         pami_task_t           * tasks,
                                                         size_t                 task_count,
                                                         pami_context_t          context,
                                                         pami_event_function     fn,
                                                         void                 * cookie)
      {
        TRACE_ERR((stderr, "<%p:%zu>BGQ::Client::geometry_create_tasklist_impl geometry %p/%p\n", this, _clientid, geometry, *geometry));

        // simple for now: only PAMI_GEOMETRY_OPTIMIZE
        if (num_configs != 0 && (num_configs > 1 || configuration[0].name != PAMI_GEOMETRY_OPTIMIZE))
          {
            return PAMI_INVAL;
          }

        BGQGeometry *new_geometry;
        BGQGeometry *bargeom = (BGQGeometry *)parent;
        PAMI::Context *ctxt = (PAMI::Context *)context;

        if (geometry != NULL)
          {
          pami_result_t rc;
          rc = __global.heap_mm->memalign((void **)&new_geometry, 0,
                                          sizeof(*new_geometry)); /// \todo use allocator
          PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc new_geometry");
            new (new_geometry) BGQGeometry(_client,
                                           (PAMI::Geometry::Common *)parent,
                                           &__global.mapping,
                                           id,
                                           task_count,
                                           tasks,
                                           &_geometry_map);

            TRACE_ERR((stderr,  "%s analyze %zu geometry %p\n", __PRETTY_FUNCTION__, _ncontexts, new_geometry));

            for (size_t n = 0; n < _ncontexts; n++)
              {
                TRACE_ERR((stderr,  "%s analyze %p geometry %p\n", __PRETTY_FUNCTION__, &_contexts[n], new_geometry));
                _contexts[n].analyze(n, new_geometry, 0);
              }

            *geometry = (pami_geometry_t) new_geometry;

            /// \todo  deliver completion to the appropriate context
            new_geometry->setCompletion(fn, cookie);
            new_geometry->addCompletion(); // ensure completion doesn't happen until
                                           // all have been analyzed (_geom_opt_finish).

            // Start the barrier (and then the global analyze and (maybe) the optimize ...
            start_barrier(bargeom, new_geometry,
                          ctxt->getId(), context,
                          num_configs? PAMI_GEOMETRY_OPTIMIZE: (pami_attribute_name_t)-1);

            new_geometry->processUnexpBarrier(&_ueb_queue,
                                              &_ueb_allocator);
          }
        else
          {
            // non-participant members of parent won't know if new geom exists...
            bargeom->default_barrier(fn, cookie, ctxt->getId(), context);
          }

        TRACE_ERR((stderr,  "%s exit geometry %p/%p\n", __PRETTY_FUNCTION__, geometry, *geometry));
        return PAMI_SUCCESS;
      }

      inline pami_result_t geometry_query_impl(pami_geometry_t geometry,
                                               pami_configuration_t configuration[],
                                               size_t num_configs)
      {
        TRACE_ERR((stderr, "<%p:%zu>BGQ::Client::geometry_query_impl\n", this, _clientid));
        // for now, this must be very simple...
        if (num_configs != 1 || configuration[0].name != PAMI_GEOMETRY_OPTIMIZE)
          {
            return PAMI_INVAL;
          }

        BGQGeometry *geom = (BGQGeometry *)geometry;
        // is it stored in geometry? or just implied by key/vals?
        // configuration[0].value.intval = gp->???;
        void *v1 = geom->getKey(PAMI::Geometry::GKEY_MCAST_CLASSROUTEID);
        void *v2 = geom->getKey(PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID);
        int b1 = (v1 != PAMI_CR_GKEY_FAIL ? (int)((uintptr_t)v1 & 0x0ff) : 0);
        int b2 = (v2 != PAMI_CR_GKEY_FAIL ? (int)((uintptr_t)v2 & 0x0ff) : 0);
        configuration[0].value.intval = b1 | (b2 << 8);
        return PAMI_SUCCESS;
      }

      inline pami_result_t geometry_update_impl(pami_geometry_t geometry,
                                                pami_configuration_t configuration[],
                                                size_t num_configs,
                                                pami_context_t context,
                                                pami_event_function fn,
                                                void *cookie)
      {
        TRACE_ERR((stderr, "<%p:%zu>BGQ::Client::geometry_update_impl\n", this, _clientid));
        // for now, this must be very simple...
        if (num_configs != 1 || configuration[0].name != PAMI_GEOMETRY_OPTIMIZE)
          {
            return PAMI_INVAL;
          }

        // If this ever involves more than one "facet" of optimization then
        // it becomes much more complicated - must detect errors and "roll back".
        BGQGeometry *gp = (BGQGeometry *)geometry;
        PAMI::Context *ctxt = (PAMI::Context *)context;
        pami_result_t rc = PAMI_SUCCESS;

        if (configuration[0].value.intval != 0)
          {
            gp->setCompletion(fn, cookie);
            gp->addCompletion();        // ensure completion doesn't happen until
            // all have been analyzed (_geom_opt_finish).
            /// \todo should this be _geom_newopt_finish?
            rc = __MUGlobal.getMuRM().geomOptimize(gp, _clientid,
                                                   ctxt->getId(), context, _geom_opt_finish, (void *)gp);
          }
        else
          {
            pami_result_t rc = __MUGlobal.getMuRM().geomDeoptimize(gp);

            if (rc == PAMI_SUCCESS)
              {
                for (size_t n = 0; n < _ncontexts; n++)
                  {
                    _contexts[n].analyze(n, (BGQGeometry *)geometry, -1);
                  }
              }

            if (fn)
              {
                fn(context, cookie, rc);
              }
          }

        return rc;
      }

      inline pami_result_t geometry_algorithms_num_impl (pami_geometry_t geometry,
                                                         pami_xfer_type_t colltype,
                                                         size_t *lists_lengths)
      {
/// \todo #warning "BOB FIX CONTEXTID, last parameter"
/// There's no contextid on the api, and we don't have context-specific geometries, so what's to fix now?
        BGQGeometry *_geometry = (BGQGeometry*) geometry;
        return _geometry->algorithms_num(colltype, lists_lengths, 0);
      }

      inline pami_result_t geometry_algorithms_info_impl (pami_geometry_t geometry,
                                                          pami_xfer_type_t colltype,
                                                          pami_algorithm_t  *algs0,
                                                          pami_metadata_t   *mdata0,
                                                          size_t               num0,
                                                          pami_algorithm_t  *algs1,
                                                          pami_metadata_t   *mdata1,
                                                          size_t               num1)
      {
/// \todo #warning "BOB FIX CONTEXTID, last parameter"
/// There's no contextid on the api, and we don't have context-specific geometries, so what's to fix now?
        BGQGeometry *_geometry = (BGQGeometry*) geometry;
        return _geometry->algorithms_info(colltype,
                                          algs0,
                                          mdata0,
                                          num0,
                                          algs1,
                                          mdata1,
                                          num1,
                                          0);
      }



      inline pami_result_t geometry_destroy_impl (pami_geometry_t geometry)
      {
        TRACE_ERR((stderr, "<%p:%zu>BGQ::Client::geometry_destroy_impl\n", this, _clientid));
        /// \todo #warning must free up geometry resources, etc.
        BGQGeometry *gp = (BGQGeometry *)geometry;
        return __MUGlobal.getMuRM().geomDeoptimize(gp);
      }

    inline pami_geometry_t mapidtogeometry_impl (int comm)
      {
        pami_geometry_t g = _geometry_map[comm];
        TRACE_ERR((stderr, "<%p>%s\n", g, __PRETTY_FUNCTION__));
        return g;
      }

    inline void registerUnexpBarrier_impl (unsigned     comm,
                                           pami_quad_t &info,
                                           unsigned     peer,
                                           unsigned     algorithm)
      {
        Geometry::UnexpBarrierQueueElement *ueb =
          (Geometry::UnexpBarrierQueueElement *) _ueb_allocator.allocateObject();
        new (ueb) Geometry::UnexpBarrierQueueElement (comm, info, peer, algorithm);
        _ueb_queue.pushTail(ueb);
      }



    protected:

      inline pami_client_t getClient () const
      {
        return _client;
      }

    private:

      pami_client_t _client;
      size_t _clientid;

      size_t       _references;
      size_t       _ncontexts;
      PAMI::Context *_contexts;
      PAMI::PlatformDeviceList _platdevs;
      char         _name[256];
      BGQGeometry                  *_world_geometry;
      uint8_t                       _world_geometry_storage[sizeof(BGQGeometry)];
      pami_geometry_range_t         _world_range;
      // This is a map of geometries to geometry id's
      std::map<unsigned, pami_geometry_t>          _geometry_map;


      Memory::GenMemoryManager _mm;
      Memory::GenMemoryManager _xmm; // used to fill context mm, from single OS alloc.
      //  Unexpected Barrier allocator
      MemoryAllocator <sizeof(PAMI::Geometry::UnexpBarrierQueueElement), 16> _ueb_allocator;
      //  Unexpected Barrier match queue
      MatchQueue                                                             _ueb_queue;
      Geometry::GeometryOptimizer<BGQGeometry>                             * _geomopt;

      /// \page env_vars Environment Variables
      ///
      /// PAMI_CLIENT_SHMEMSIZE - Size, bytes, of each Client shmem pool.
      /// May use 'K' or 'M' suffix as multiplier. default: 2MB
      ///

      inline void initializeMemoryManager ()
      {
        TRACE_ERR((stderr, "<%p:%zu>BGQ::Client::initializeMemoryManager\n", this, _clientid));
        char   shmemfile[PAMI::Memory::MMKEYSIZE];
#if 0
        size_t num_ctx = __MUGlobal.getMuRM().getPerProcessMaxPamiResources();
        // may need to factor in others such as shmem?
#else
        size_t num_ctx = 64;
#endif
        // 18K * Ncontexts...
        size_t bytes = (32*1024) * num_ctx; // 32k for each context in the client

        /// \page env_vars Environment Variables
        ///
        /// PAMI_CLIENT_SHMEMSIZE - Size, bytes, per-Client shared memory.
        /// May use 'K' or 'M' suffix as multiplier. default: 8800 * maxnctx * nproc;
        ///
        char *env = getenv("PAMI_CLIENT_SHMEMSIZE");
        if (env) {
          char *s = NULL;
          bytes = strtoull(env, &s, 0);
          if (*s == 'm' || *s == 'M') bytes *= 1024 * 1024;
          else if (*s == 'k' || *s == 'K') bytes *= 1024;
        }

        snprintf (shmemfile, sizeof(shmemfile) - 1, "/pami-client-%s", _name);

        // Round up to the page size
        //size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

        pami_result_t rc;
        rc = _mm.init(__global.shared_mm, bytes, 1, 1, 0, shmemfile);
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to create \"%s\" mm for %zd bytes",
                     shmemfile, bytes);
        return;
      }

      void start_barrier(BGQGeometry          *bargeom,
                         BGQGeometry          *new_geometry,
                         size_t                context_id,
                         pami_context_t        context,
                         pami_attribute_name_t optimize)
      {
	//Assume algorithm 0 always works
	pami_algorithm_t  alg;
        new_geometry->algorithms_info(PAMI_XFER_ALLREDUCE,
                                      &alg,
                                      NULL,
                                      1,
                                      NULL,
                                      NULL,
                                      0,
                                      context_id);
	
        Geometry::Algorithm<BGQGeometry> *ar_algo = (Geometry::Algorithm<BGQGeometry> *)alg;
	

	pami_event_function done_fn = _geom_newopt_finish;
	if (optimize == PAMI_GEOMETRY_OPTIMIZE) 
	  done_fn = _geom_newopt_start;
	
	
	Geometry::GeometryOptimizer<BGQGeometry> *go = _geomopt;
	int rc  = 0;
	if (go == NULL) {
	  rc = __global.heap_mm->memalign((void **)&go, 0, sizeof(*go));
	  PAMI_assertf(rc == PAMI_SUCCESS, "alloc failed for GlobalAnalyzer<BGQGeometry> %zd", sizeof(*go));
	  _geomopt = go;	
	}

	new (go) Geometry::GeometryOptimizer<BGQGeometry>(context,
							  new_geometry,
							  ar_algo,
							  done_fn,
							  (void*)new_geometry);	
		
	for (size_t i = 0; i < _ncontexts; ++i) 
	{
	  _contexts[i].registerWithOptimizer(go);
	}
	
	TRACE_ERR((stderr, "<%p:%zu>BGQ::Client::start_barrier() context %p  %s\n", this, _clientid, context, optimize == PAMI_GEOMETRY_OPTIMIZE? "Optimized":" "));

  if (bargeom)
    if(bargeom->nranks() == 1)
      Geometry::GeometryOptimizer<BGQGeometry>::optimizer_start(context, (void *)go, PAMI_SUCCESS);
    else
      bargeom->default_barrier(Geometry::GeometryOptimizer<BGQGeometry>::optimizer_start, (void *)go, context_id, context);
  else
    if(new_geometry->nranks() == 1)
      Geometry::GeometryOptimizer<BGQGeometry>::optimizer_start(context, (void *)go, PAMI_SUCCESS);
    else
      new_geometry->ue_barrier(Geometry::GeometryOptimizer<BGQGeometry>::optimizer_start, (void *)go, context_id, context);
      }

  }; // end class PAMI::Client
}; // end namespace PAMI
#endif // __components_client_bgq_bgqclient_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
