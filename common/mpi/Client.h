///
/// \file common/mpi/Client.h
/// \brief PAMI client interface specific for the MPI platform.
///
#ifndef __common_mpi_Client_h__
#define __common_mpi_Client_h__

#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "common/ClientInterface.h"
#include "Context.h"

namespace PAMI
{
  class Client : public Interface::Client<PAMI::Client>
  {
  public:

    static void shutdownfunc()
      {
        MPI_Finalize();
      }
    inline Client (const char           *name,
                   pami_result_t        &result,
                   pami_configuration_t  configuration[],
                   size_t                num_configs) :
      Interface::Client<PAMI::Client>(name, result),
      _client ((pami_client_t) this),
      _references (1),
      _ncontexts (0),
      _mm ()
      {
        static size_t next_client_id = 0;
        // Set the client name string.
        memset ((void *)_name, 0x00, sizeof(_name));
        strncpy (_name, name, sizeof(_name) - 1);

        _clientid = next_client_id++;
        // PAMI_assert(_clientid < PAMI_MAX_NUM_CLIENTS);

        // Get some shared memory for this client
        initializeMemoryManager ();


        // Initialize the world geometry
        MPI_Comm_rank(MPI_COMM_WORLD,&_myrank);
        MPI_Comm_size(MPI_COMM_WORLD,&_mysize);
        _world_geometry=(MPIGeometry*) malloc(sizeof(*_world_geometry));
        PAMI_assert(_world_geometry);
        _world_range.lo=0;
        _world_range.hi=_mysize-1;
        new(_world_geometry) MPIGeometry(NULL, &__global.mapping,0, 1,&_world_range);
        result = PAMI_SUCCESS;
      }

    inline ~Client ()
      {
      }

    static pami_result_t generate_impl (const char           *name,
                                        pami_client_t        *client,
                                        pami_configuration_t  configuration[],
                                        size_t                num_configs)
      {
        int rc = 0;
        PAMI::Client * clientp;
        clientp = (PAMI::Client *)malloc(sizeof (PAMI::Client));
        PAMI_assert(clientp != NULL);
        memset ((void *)clientp, 0x00, sizeof(PAMI::Client));
        pami_result_t res;
        new (clientp) PAMI::Client (name, res, configuration, num_configs);
        *client = (pami_client_t) clientp;
        return PAMI_SUCCESS;
      }

    static void destroy_impl (pami_client_t client)
      {
        free (client);
      }

    inline char * getName_impl ()
      {
        return _name;
      }

    inline pami_result_t createContext_impl (pami_configuration_t configuration[],
                                             size_t              count,
                                             pami_context_t     * context,
                                             size_t              ncontexts)
      {
        //_context_list->lock ();
        int n = ncontexts;
        if (_ncontexts != 0) {
          return PAMI_ERROR;
        }
        if (_ncontexts + n > 4) {
          n = 4 - _ncontexts;
        }
        if (n <= 0) { // impossible?
          return PAMI_ERROR;
        }

#ifdef USE_MEMALIGN
        int rc = posix_memalign((void **)&_contexts, 16, sizeof(PAMI::Context) * n);
        PAMI_assertf(rc==0, "posix_memalign failed for _contexts[%d], errno=%d\n", n, errno);
#else
        _contexts = (PAMI::Context*)malloc(sizeof(PAMI::Context)*n);
        PAMI_assertf(_contexts!=NULL, "malloc failed for _contexts[%d], errno=%d\n", n, errno);
#endif
        _platdevs.generate(_clientid, n, _mm);

        // This memset has been removed due to the amount of cycles it takes
        // on simulators.  Lower level initializers should be setting the
        // relevant fields of the context, so this memset should not be
        // needed anyway.
        //memset((void *)_contexts, 0, sizeof(PAMI::Context) * n);
        size_t bytes = _mm.available() / n - 16;
        int x;
        for (x = 0; x < n; ++x) {
          context[x] = (pami_context_t)&_contexts[x];
          void *base = NULL;
          _mm.enable();
          _mm.memalign((void **)&base, 16, bytes);
          _mm.disable();
          PAMI_assertf(base != NULL, "out of sharedmemory in context create\n");
          new (&_contexts[x]) PAMI::Context(this, _clientid, x, n,
                                            &_platdevs, base, bytes, _world_geometry, &_mm);
          //_context_list->pushHead((QueueElem *)&context[x]);
          //_context_list->unlock();
          _ncontexts = n;
        }
        return PAMI_SUCCESS;
      }

    // DEPRECATED!
    inline pami_result_t destroyContext_impl (pami_context_t context)
      {
        PAMI_abortf("destroyContext for single context is not supported");
        //free(context); // this can't work?!? it was allocated as one big array!
        return PAMI_SUCCESS;
      }
    inline pami_result_t destroyContext_impl (pami_context_t *context, size_t ncontexts)
      {
        PAMI_assertf(ncontexts == _ncontexts, "destroyContext called without all contexts");
        pami_result_t res = PAMI_SUCCESS;
        size_t i;
        for (i = 0; i < _ncontexts; ++i)
          {
            context[i] = NULL;
            PAMI::Context * ctx = &_contexts[i];
            pami_result_t rc = ctx->destroy ();
            if (rc != PAMI_SUCCESS) res = rc;
          }
        free(_contexts);
        _contexts = NULL;
        _ncontexts = 0;
        return res;
      }

    inline pami_result_t query_impl (pami_configuration_t configuration[],
                                     size_t               num_configs)
      {
        size_t i;
        pami_result_t result = PAMI_SUCCESS;
        for(i=0; i<num_configs; i++)
          {
            switch (configuration[i].name)
              {
                case PAMI_CLIENT_NUM_CONTEXTS:
                  configuration[i].value.intval = 1; // real value TBD
                  break;
                case PAMI_CLIENT_CONST_CONTEXTS:
                  configuration[i].value.intval = 1; // real value TBD
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
                  configuration[i].value.doubleval =__global.time.tick();
                  break;
                case PAMI_CLIENT_MEM_SIZE:
                case PAMI_CLIENT_PROCESSOR_NAME:
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

    static void cr_func(pami_context_t  context,
                        void           *cookie,
                        uint64_t       *reduce_result,
                        MPIGeometry    *g,
                        pami_result_t   result )
      {
        Client *c = (Client*)cookie;
        for(size_t n=0; n<c->_ncontexts; n++)
          {
            c->_contexts[n]._pgas_collreg->analyze_global(n,g,reduce_result[0]);
            c->_contexts[n]._p2p_ccmi_collreg->analyze_global(n,g,reduce_result[1]);
            c->_contexts[n]._oldccmi_collreg->analyze_global(n,g,reduce_result[2]);
            c->_contexts[n]._ccmi_collreg->analyze_global(n,g,reduce_result[3]);
          }
      }


    inline pami_result_t geometry_create_taskrange_impl(pami_geometry_t       *geometry,
                                                        pami_configuration_t   configuration[],
                                                        size_t                 num_configs,
                                                        pami_geometry_t        parent,
                                                        unsigned               id,
                                                        pami_geometry_range_t *rank_slices,
                                                        size_t                 slice_count,
                                                        pami_context_t         context,
                                                        pami_event_function    fn,
                                                        void                  *cookie)
      {
        MPIGeometry              *new_geometry = NULL;
        uint64_t                  to_reduce[4];
        // If our new geometry is NOT NULL, we will create a new geometry
        // for this client.  This new geometry will be populated with a
        // set of algorithms.
        if(geometry != NULL)
          {
            new_geometry=(MPIGeometry*) malloc(sizeof(*new_geometry));
            new(new_geometry)MPIGeometry((MPIGeometry*)parent,
                                         &__global.mapping,
                                         id,
                                         slice_count,
                                         rank_slices);
            for(size_t n=0; n<_ncontexts; n++)
              {
                _contexts[n]._pgas_collreg->analyze_local(n,new_geometry,&to_reduce[0]);
                _contexts[n]._p2p_ccmi_collreg->analyze_local(n,new_geometry,&to_reduce[1]);
                _contexts[n]._oldccmi_collreg->analyze_local(n,new_geometry,&to_reduce[2]);
                _contexts[n]._ccmi_collreg->analyze_local(n,new_geometry,&to_reduce[3]);
              }
	    new_geometry->processUnexpBarrier();

            *geometry=(MPIGeometry*) new_geometry;
          }

        // Now we must take care of the synchronization of this new geometry
        // First, if a new geometry is created, we will perform an allreduce
        // on the new geometry.
        // If we have a parent geometry, we perform synchronization on that
        // geometry.  If we don't have a parent geometry, we perform an
        // "unexpected" barrier on only the new geometry.  In either case,
        // any node creating a new geometry will allocate a class route.
        // When the synchronization is completed (barrier or uebarrier), it
        // will chain into an allreduce operation, which performs the allocation
        // of the classroute.  When the classroute has been allocated,
        // The "done" event is delivered to the user.
        MPIGeometry      *bargeom = (MPIGeometry*)parent;
        PAMI::Context    *ctxt    = (PAMI::Context *)context;
        if(new_geometry)
          {
            pami_algorithm_t  alg;
            new_geometry->algorithms_info(PAMI_XFER_ALLREDUCE,
                                          &alg,
                                          NULL,
                                          1,
                                          NULL,
                                          NULL,
                                          0,
                                          ctxt->getId());
            Geometry::Algorithm<MPIGeometry> *ar_algo = (Geometry::Algorithm<MPIGeometry> *)alg;
            MPIClassRouteId *cr = (MPIClassRouteId *)malloc(sizeof(MPIClassRouteId));
            new(cr)MPIClassRouteId(ar_algo,
                                   new_geometry,
                                   to_reduce,
                                   4,
                                   cr_func,
                                   (void*)this,
                                   fn,
                                   cookie);
            if(bargeom)
              bargeom->default_barrier(MPIClassRouteId::get_classroute, cr, ctxt->getId(), context);
            else
              new_geometry->ue_barrier(MPIClassRouteId::get_classroute, cr, ctxt->getId(), context);
          }
        else
          {
            if(bargeom)
              bargeom->default_barrier(fn, cookie, ctxt->getId(), context);
            else
              {
                // Null parent and null new geometry?  Why are you here?
                return PAMI_INVAL;
              }
          }
        return PAMI_SUCCESS;
      }


    inline pami_result_t geometry_create_tasklist_impl(pami_geometry_t       *geometry,
                                                       pami_configuration_t   configuration[],
                                                       size_t                 num_configs,
                                                       pami_geometry_t        parent,
                                                       unsigned               id,
                                                       pami_task_t           *tasks,
                                                       size_t                 task_count,
                                                       pami_context_t         context,
                                                       pami_event_function    fn,
                                                       void                  *cookie)
      {
        // todo:  implement this routine
        PAMI_abort();

        return PAMI_SUCCESS;
      }


    inline pami_result_t geometry_destroy_impl (pami_geometry_t geometry)
      {
        PAMI_abort();
        return PAMI_UNIMPL;
      }

  protected:

    inline pami_client_t getClient () const
      {
        return _client;
      }

  private:
    pami_client_t              _client;
    size_t                     _clientid;
    size_t                     _references;
    size_t                     _ncontexts;
    PAMI::Context             *_contexts;
    PAMI::PlatformDeviceList   _platdevs;
    char                       _name[256];
    int                        _myrank;
    int                        _mysize;
    MPIGeometry               *_world_geometry;
    pami_geometry_range_t      _world_range;
    Memory::MemoryManager      _mm;

    inline void initializeMemoryManager ()
      {
        char   shmemfile[1024];
        size_t bytes     = 1024*1024;
        size_t pagesize  = 4096;

        snprintf (shmemfile, 1023, "/pami-client-%s", _name);
        // Round up to the page size
        size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);
        int fd, rc;
        size_t n = bytes;

        // CAUTION! The following sequence MUST ensure that "rc" is "-1" iff failure.
        rc = shm_open (shmemfile, O_CREAT|O_RDWR,0600);
        if ( rc != -1 )
          {
            fd = rc;
            rc = ftruncate( fd, n );
            void * ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
            if ( ptr != MAP_FAILED )
              {
                _mm.init (ptr, n);
                return;
              }
          }
        // Abort if we *should* have created shmem, but allow single process nodes to continue
        if(__global.topology_local.size() > 1) PAMI_abort();
        // Failed to create shared memory .. fake it using the heap ??
        _mm.init (malloc (n), n);
        return;
      }

  }; // end class PAMI::Client
}; // end namespace PAMI


#endif
// __pami_mpi_mpiclient_h__
