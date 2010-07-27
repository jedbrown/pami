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
        TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
        static size_t next_client_id = 0;
        // Set the client name string.
        memset ((void *)_name, 0x00, sizeof(_name));
        strncpy (_name, name, sizeof(_name) - 1);

        _clientid = next_client_id++;
        // assert(_clientid < PAMI_MAX_NUM_CLIENTS);

        // Get some shared memory for this client
        initializeMemoryManager ();

        _world_range.lo = 0;
        _world_range.hi = __global.mapping.size() - 1;
        new(_world_geometry_storage) BGQGeometry(NULL, &__global.mapping, 0, 1, &_world_range);
#ifdef ENABLE_MU_CLASSROUTES
        // This must return immediately (must not enqueue non-blocking ops).
        // Passing a NULL context should ensure that.
        __MUGlobal.getMuRM().geomOptimize(_world_geometry, _clientid, 0, NULL, NULL, NULL);
        // Now, subsequent 'analyze' done on this geom will know that MU Coll is avail.
#endif

        result = PAMI_SUCCESS;
      }

      inline ~Client ()
      {
      }

      static pami_result_t generate_impl (const char * name, pami_client_t * client,
					  pami_configuration_t   configuration[],
					  size_t                 num_configs)
      {
        TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
        pami_result_t result;
        int rc = 0;

        //__client_list->lock();

        // If a client with this name is not already initialized...
        PAMI::Client * clientp = NULL;
        //if ((client = __client_list->contains (name)) == NULL)
        //{
        rc = posix_memalign((void **) & clientp, 16, sizeof (PAMI::Client));

        if (rc != 0) assert(0);

        memset ((void *)clientp, 0x00, sizeof(PAMI::Client));
        new (clientp) PAMI::Client (name, result);
        *client = clientp;
        //__client_list->pushHead ((QueueElem *) client);
        //}
        //else
        //{
        //client->incReferenceCount ();
        //}

        //__client_list->unlock();

        return result;
      }

      static void destroy_impl (pami_client_t client)
      {
        //__client_list->lock ();
        //client->decReferenceCount ();
        //if (client->getReferenceCount () == 0)
        //{
        //__client_list->remove (client);
        free ((void *) client);
        //}
        //__client_list->unlock ();
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
        TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
        //_context_list->lock ();
        int n = ncontexts;

        if (_ncontexts != 0)
          {
            return PAMI_ERROR;
          }

        if (_ncontexts + n > 4)
          {
            n = 4 - _ncontexts;
          }

        if (n <= 0)   // impossible?
          {
            return PAMI_ERROR;
          }

        int rc = posix_memalign((void **) & _contexts, 16, sizeof(*_contexts) * n);
        PAMI_assertf(rc == 0, "posix_memalign failed for _contexts[%d], errno=%d\n", n, errno);
#ifdef USE_COMMTHREADS
        // Create one comm thread semi-opaque pointer. Internally, this may be
        // one-per-context (optimal advance scenario) or some other arrangement.
        /// \todo #warning need to fix node-scoped l2atomic mm so that it allows private alloc
        _commThreads = PAMI::Device::CommThread::BgqCommThread::generate(_clientid, n, &_mm, &__global.l2atomicFactory.__nodescoped_mm);
        PAMI_assertf(_commThreads, "BgqCommThread::generate failed for _commThreads[%d]\n", n);
#endif // USE_COMMTHREADS
        int x;
        TRACE_ERR((stderr, "BGQ::Client::createContext mm available %zu\n", _mm.available()));
        _platdevs.generate(_clientid, n, _mm);
        // _platdevs.generate(_clientid, n, _mm, __global._wuRegion[_clientid]->_wu_mm);

        // This memset has been removed due to the amount of cycles it takes
        // on simulators.  Lower level initializers should be setting the
        // relevant fields of the context, so this memset should not be
        // needed anyway.
        //memset((void *)_contexts, 0, sizeof(PAMI::Context) * n);
        size_t bytes = _mm.available() / n - 16;
        TRACE_ERR((stderr, "BGQ::Client::createContext mm available %zu, bytes %zu\n", _mm.available(), bytes));

        for (x = 0; x < n; ++x)
          {
            context[x] = (pami_context_t) & _contexts[x];
            void *base = NULL;
            _mm.enable();
            TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
            _mm.memalign((void **)&base, 16, bytes);
            _mm.disable();
            PAMI_assertf(base != NULL, "out of sharedmemory in context create x=%d,n=%d,bytes=%zu,mm.size=%zu,mm.available=%zu\n", x, n, bytes, _mm.size(), _mm.available());
#ifdef USE_COMMTHREADS
            // Note, this is not inializing each comm thread but rather
            // initializing comm threads for each context. context[x] is not usable yet,
            // but it won't matter since this object can't do anything with it anyway.
            // This must initialize before the context, so that MemoryManagers are
            // setup.
            PAMI::Device::CommThread::BgqCommThread::initContext(_commThreads, _clientid, x, context[x]);
#endif // USE_COMMTHREADS
            new (&_contexts[x]) PAMI::Context(this->getClient(), _clientid, x, n,
                                              &_platdevs, base, bytes, _world_geometry);
            //_context_list->pushHead((QueueElem *)&context[x]);
            //_context_list->unlock();
          }
        _ncontexts = (size_t)n;
        TRACE_ERR((stderr,  "%s ncontexts %zu exit\n", __PRETTY_FUNCTION__,_ncontexts));

        return PAMI_SUCCESS;
      }

      inline pami_result_t destroyContext_impl (pami_context_t context)
      {
#ifdef USE_COMMTHREADS
        // This removes all contexts... only needs to be called once.
        PAMI::Device::CommThread::BgqCommThread::shutdown(_commThreads, _clientid);
#endif // USE_COMMTHREADS
        //_context_list->lock ();
        //_context_list->remove (context);
        return ((PAMI::Context *)context)->destroy ();
        //_context_list->unlock ();
      }

#ifdef USE_COMMTHREADS
      // This is not standard interface... yet?
      inline pami_result_t addContextToCommThreadPool(pami_context_t ctx)
      {
#if 0

        // hook for testing...
        if (ctx == NULL)
          {
            return PAMI::Device::CommThread::BgqCommThread::wakeUp(_commThreads, _clientid);
          }
        else
#endif // !HAVE_WU_ARMWITHADDRESS
          return PAMI::Device::CommThread::BgqCommThread::addContext(_commThreads, _clientid, ctx);
      }
#endif // USE_COMMTHREADS

    inline pami_result_t query_impl (pami_configuration_t configuration[],
                                     size_t               num_configs)
      {
        pami_result_t result = PAMI_SUCCESS;
        size_t i;
        for(i=0; i<num_configs; i++)
          {
            switch (configuration[i].name)
              {
                case PAMI_CLIENT_NUM_CONTEXTS:

                  /// \todo #80 #99 Remove this when the DMA supports >1 context.
                  if (__global.useMU())
                    configuration[i].value.intval = 1;
                  else
                    configuration[i].value.intval = 64;
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
                  /// the BGP DCMF version:
                  /// "Rank 0 of 128 (0,0,0,0)  R00-M0-N10-J01"
                  rc = snprintf(pn, 128, "Task %zu of %zu", __global.mapping.task(), __global.mapping.size());
                  pn[128-1] = 0;
                  configuration[i].value.chararray = pn;
                  if (rc <= 0)
                    result = PAMI_INVAL;
                }
                break;
                case PAMI_CLIENT_MEM_SIZE:
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
        TRACE_ERR((stderr,  "%s enter geometry %p/%p\n", __PRETTY_FUNCTION__,geometry,*geometry));
#ifdef ENABLE_MU_CLASSROUTES

	// simple for now: only PAMI_GEOMETRY_OPTIMIZE
        if (num_config != 0 && (num_config > 1 || configuration[0].name != PAMI_GEOMETRY_OPTIMIZE))
        {
          return PAMI_INVAL;
        }
        BGQGeometry *new_geometry;
        BGQGeometry *bargeom = (BGQGeometry *)parent;
        PAMI::Context *ctxt = (PAMI::Context *)context;

        if (geometry != NULL)
        {
            new_geometry = (BGQGeometry *)malloc(sizeof(*new_geometry)); /// \todo use allocator
            new (new_geometry) BGQGeometry((PAMI::Geometry::Common *)parent,
                                      &__global.mapping,
                                      id,
                                      slice_count,
                                      rank_slices);

            TRACE_ERR((stderr,  "%s analyze %zu geometry %p\n", __PRETTY_FUNCTION__,_ncontexts,new_geometry));
            for (size_t n = 0; n < _ncontexts; n++)
            {
              TRACE_ERR((stderr,  "%s analyze %p geometry %p\n", __PRETTY_FUNCTION__,&_contexts[n],new_geometry));
                _contexts[n].analyze(n, new_geometry, 0);
            }
            *geometry = (pami_geometry_t) new_geometry;

            /// \todo  deliver completion to the appropriate context
            new_geometry->setCompletion(fn, cookie);
            new_geometry->addCompletion(); // ensure completion doesn't happen until
            				   // all have been analyzed (_geom_opt_finish).
	    if (num_config) // must be PAMI_GEOMETRY_OPTIMIZE...
	    {
              bargeom->default_barrier(_geom_opt_start, (void *)new_geometry,
							ctxt->getId(), context);
            }
	    else
	    {
              bargeom->default_barrier(_geom_finish, (void *)new_geometry,
							ctxt->getId(), context);
	    }
        }
        else
        {
	  // non-participant members of parent won't know if new geom exists...
          bargeom->default_barrier(fn, cookie, ctxt->getId(), context);
	}

#else
        BGQGeometry              *new_geometry;

        if (geometry != NULL)
          {
            new_geometry=(BGQGeometry*) malloc(sizeof(*new_geometry)); /// \todo use allocator
            new(new_geometry) BGQGeometry((PAMI::Geometry::Common*)parent,
                                      &__global.mapping,
                                      id,
                                      slice_count,
                                      rank_slices);

            TRACE_ERR((stderr,  "%s analyze %zu geometry %p\n", __PRETTY_FUNCTION__,_ncontexts,new_geometry));
            for (size_t n = 0; n < _ncontexts; n++)
              {
              TRACE_ERR((stderr,  "%s analyze %p geometry %p\n", __PRETTY_FUNCTION__,&_contexts[n],new_geometry));
                _contexts[n].analyze(n, (BGQGeometry*)new_geometry, 0);
              }
            *geometry = (pami_geometry_t) new_geometry;

            /// \todo  deliver completion to the appropriate context
          }

        BGQGeometry *bargeom = (BGQGeometry*)parent;
        PAMI::Context *ctxt = (PAMI::Context *)context;
        bargeom->default_barrier(fn, cookie, ctxt->getId(), context);

#endif
        TRACE_ERR((stderr,  "%s exit geometry %p/%p\n", __PRETTY_FUNCTION__,geometry,*geometry));
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
#ifdef ENABLE_MU_CLASSROUTES

	// simple for now: only PAMI_GEOMETRY_OPTIMIZE, and not allowed here.
        if (num_config)
          {
            return PAMI_INVAL;
          }

#endif
        // todo:  implement this routine
        PAMI_abortf("geometry_create_tasklist_impl");
        return PAMI_SUCCESS;
      }

#ifdef ENABLE_MU_CLASSROUTES
      static void _geom_newopt_start(pami_context_t context, void *cookie, pami_result_t err)
      {
	BGQGeometry *gp = (BGQGeometry *)cookie;
	if (err != PAMI_SUCCESS)
	{
	  _geom_newopt_finish(context, cookie, err);
	  return;
	}
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
        BGQGeometry *gp = (BGQGeometry *)cookie;
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

	// non-fatal errors - do not destroy geometry (caller might have different plans)
        gp->rmCompletion(context, err);
      }

      static void _geom_newopt_finish(pami_context_t context, void *cookie, pami_result_t err)
      {
        BGQGeometry *gp = (BGQGeometry *)cookie;
        PAMI::Context *ctxt = (PAMI::Context *)context;

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

      inline pami_result_t geometry_query_impl(pami_geometry_t geometry,
                                          pami_configuration_t configuration[],
					  size_t num_configs)
      {
	// for now, this must be very simple...
	if (num_configs != 1 || configuration[0].name != PAMI_GEOMETRY_OPTIMIZE) {
		return PAMI_INVAL;
	}

        // is it stored in geometry? or just implied by key/vals?
        // configuration[0].value.intval = gp->???;
        void *v1 = geom->getKey(PAMI::Geometry::PAMI_GKEY_BGQCOLL_CLASSROUTE);
        void *v2 = geom->getKey(PAMI::Geometry::PAMI_GKEY_BGQGI_CLASSROUTE);
        configuration[0].value.intval = (v1 != NULL && v1 != PAMI_CR_GKEY_FAIL) ||
                                      (v2 != NULL && v2 != PAMI_CR_GKEY_FAIL);
	return PAMI_SUCCESS;
      }

      inline pami_result_t geometry_update_impl(pami_geometry_t geometry,
                                           pami_configuration_t configuration[],
					   size_t num_configs,
                                           pami_context_t context,
                                           pami_event_function fn,
                                           void *cookie)
      {
	// for now, this must be very simple...
	if (num_configs != 1 || configuration[0].name != PAMI_GEOMETRY_OPTIMIZE) {
		return PAMI_INVAL;
	}

	// If this ever involves more than one "facet" of optimization then
	// it becomes much more complicated - must detect errors and "roll back".
        BGQGeometry *gp = (BGQGeometry *)geometry;
        PAMI::Context *ctxt = (PAMI::Context *)context;
        if (configuration[0].value.intval != 0)
          {
            gp->setCompletion(fn, cookie);
            gp->addCompletion();        // ensure completion doesn't happen until
            				// all have been analyzed (_geom_opt_finish).
            pami_result_t rc = __MUGlobal.getMuRM().geomOptimize(gp, _clientid,
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
#endif

      inline pami_result_t geometry_destroy_impl (pami_geometry_t geometry)
      {
	/// \todo #warning must free up geometry resources, etc.
#ifdef ENABLE_MU_CLASSROUTES
        BGQGeometry *gp = (BGQGeometry *)geometry;
        return __MUGlobal.getMuRM().geomDeoptimize(gp);
#else
        PAMI_abortf("geometry_destroy_impl");
        return PAMI_UNIMPL;
#endif
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

      Memory::MemoryManager _mm;
#ifdef USE_COMMTHREADS
      PAMI::Device::CommThread::BgqCommThread *_commThreads;
#endif // USE_COMMTHREADS

      inline void initializeMemoryManager ()
      {
        TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
        char   shmemfile[1024];
        //size_t bytes     = 1024*1024;
        size_t bytes     = 2048 * 1024;
        //size_t pagesize  = 4096;

        char *env = getenv("PAMI_CLIENT_SHMEMSIZE");

        if (env)
          {
            bytes = strtoull(env, NULL, 0) * 1024 * 1024;
          }

        snprintf (shmemfile, 1023, "/pami-client-%s", _name);

        // Round up to the page size
        //size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

        int fd, rc;
        size_t n = bytes;

#if 1
        // CAUTION! The following sequence MUST ensure that "rc" is "-1" iff failure.

        rc = shm_open (shmemfile, O_CREAT | O_RDWR, 0600);

        if (rc == -1)
          fprintf(stderr, "BGQ::Client shm_open(<%s>,O_RDWR) rc = %d, errno = %d, %s\n", shmemfile,  rc,  errno,  strerror(errno));

        TRACE_ERR((stderr,  "BGQ::Client() shm_open %d\n", rc));

        void * ptr = NULL;

        if ( rc != -1 )
          {
            fd = rc;
            rc = ftruncate( fd, n );

            if ( rc != -1 )
              {
                ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                TRACE_ERR((stderr,  "BGQ::Client() mmap(NULL, %zu, PROT_READ | PROT_WRITE, MAP_SHARED, %d, 0); rc = %d \n", n, fd, rc));

                if ( ptr != MAP_FAILED )
                  {
                    TRACE_ERR((stderr, "BGQ::Client:shmem file <%s> %zu bytes mapped at %p\n", shmemfile, n, ptr));
                    _mm.init (ptr, n);
                    return;
                  }
              }
          }

        fprintf(stderr, "%s:%d BGQ::Client Failed to create shared memory <%s> (rc=%d, ptr=%p, n=%zu) errno %d %s\n", __FILE__, __LINE__, shmemfile, rc, ptr, n, errno, strerror(errno));
        //PAMI_abortf(stderr,"Failed to create shared memory (rc=%d, ptr=%p, n=%zu)\n", rc, ptr, n);

        // Failed to create shared memory .. fake it using the heap ??
        _mm.init (malloc (n), n);
#endif
        return;
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
