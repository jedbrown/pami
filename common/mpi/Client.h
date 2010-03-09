///
/// \file common/mpi/Client.h
/// \brief XMI client interface specific for the MPI platform.
///
#ifndef __common_mpi_Client_h__
#define __common_mpi_Client_h__

#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "common/ClientInterface.h"
#include "Context.h"

namespace XMI
{
    class Client : public Interface::Client<XMI::Client>
    {
    public:

      static void shutdownfunc()
        {
          MPI_Finalize();
        }
      inline Client (const char * name, xmi_result_t & result) :
        Interface::Client<XMI::Client>(name, result),
        _client ((xmi_client_t) this),
        _references (1),
        _ncontexts (0),
        _mm ()
        {
	  static size_t next_client_id = 0;
          // Set the client name string.
          memset ((void *)_name, 0x00, sizeof(_name));
          strncpy (_name, name, sizeof(_name) - 1);

	  _clientid = next_client_id++;
	  // assert(_clientid < XMI_MAX_NUM_CLIENTS);

          // Get some shared memory for this client
          initializeMemoryManager ();


        // Initialize the world geometry
        MPI_Comm_rank(MPI_COMM_WORLD,&_myrank);
        MPI_Comm_size(MPI_COMM_WORLD,&_mysize);
        _world_geometry=(MPIGeometry*) malloc(sizeof(*_world_geometry));
        assert(_world_geometry);
        _world_range.lo=0;
        _world_range.hi=_mysize-1;
        new(_world_geometry) MPIGeometry(NULL, &__global.mapping,0, 1,&_world_range);
          result = XMI_SUCCESS;
        }

      inline ~Client ()
        {
        }

      static xmi_result_t generate_impl (const char * name, xmi_client_t * client)
        {
          int rc = 0;
          XMI::Client * clientp;
          clientp = (XMI::Client *)malloc(sizeof (XMI::Client));
          assert(clientp != NULL);
          memset ((void *)clientp, 0x00, sizeof(XMI::Client));
          xmi_result_t res;
          new (clientp) XMI::Client (name, res);
          *client = (xmi_client_t) clientp;
          return XMI_SUCCESS;
        }

      static void destroy_impl (xmi_client_t client)
        {
          free (client);
        }

      inline char * getName_impl ()
        {
          return _name;
        }

      inline xmi_result_t createContext_impl (xmi_configuration_t configuration[],
                                              size_t              count,
                                              xmi_context_t     * context,
                                              size_t              ncontexts)
        {
		//_context_list->lock ();
		int n = ncontexts;
		if (_ncontexts != 0) {
			return XMI_ERROR;
		}
		if (_ncontexts + n > 4) {
			n = 4 - _ncontexts;
		}
		if (n <= 0) { // impossible?
			return XMI_ERROR;
		}

#ifdef USE_MEMALIGN
		int rc = posix_memalign((void **)&_contexts, 16, sizeof(XMI::Context) * n);
		XMI_assertf(rc==0, "posix_memalign failed for _contexts[%d], errno=%d\n", n, errno);
#else
                _contexts = (XMI::Context*)malloc(sizeof(XMI::Context)*n);
		XMI_assertf(_contexts!=NULL, "malloc failed for _contexts[%d], errno=%d\n", n, errno);
#endif
		_platdevs.generate(_clientid, n, _mm);

		// This memset has been removed due to the amount of cycles it takes
		// on simulators.  Lower level initializers should be setting the
		// relevant fields of the context, so this memset should not be
		// needed anyway.
		//memset((void *)_contexts, 0, sizeof(XMI::Context) * n);
		size_t bytes = _mm.available() / n - 16;
		int x;
		for (x = 0; x < n; ++x) {
			context[x] = (xmi_context_t)&_contexts[x];
			void *base = NULL;
			_mm.memalign((void **)&base, 16, bytes);
			XMI_assertf(base != NULL, "out of sharedmemory in context create\n");
			new (&_contexts[x]) XMI::Context(this->getClient(), _clientid, x, n,
							&_platdevs, base, bytes, _world_geometry);
			//_context_list->pushHead((QueueElem *)&context[x]);
			//_context_list->unlock();
          _ncontexts = n;
		}
		return XMI_SUCCESS;
        }

      inline xmi_result_t destroyContext_impl (xmi_context_t context)
        {
          free(context);
          return XMI_SUCCESS;
        }

	inline xmi_result_t queryConfiguration_impl (xmi_configuration_t * configuration)
	{
		xmi_result_t result = XMI_ERROR;

		switch (configuration->name)
		{
		case XMI_NUM_CONTEXTS:
			configuration->value.intval = 1; // real value TBD
			result = XMI_SUCCESS;
			break;
		case XMI_CONST_CONTEXTS:
			configuration->value.intval = 0; // real value TBD
			result = XMI_SUCCESS;
			break;
		case XMI_TASK_ID:
			configuration->value.intval = __global.mapping.task();
			result = XMI_SUCCESS;
			break;
		case XMI_NUM_TASKS:
			configuration->value.intval = __global.mapping.size();
			result = XMI_SUCCESS;
			break;
		case XMI_CLOCK_MHZ:
		case XMI_WTIMEBASE_MHZ:
			configuration->value.intval = __global.time.clockMHz();
			result = XMI_SUCCESS;
			break;
		case XMI_WTICK:
			configuration->value.doubleval =__global.time.tick();
			result = XMI_SUCCESS;
			break;
		case XMI_MEM_SIZE:
		case XMI_PROCESSOR_NAME:
		default:
			break;
		}
		return result;
	}

	inline size_t getNumContexts()
	{
		return _ncontexts;
	}

	inline XMI::Context *getContext(size_t ctx)
	{
		return _contexts + ctx;
	}
	inline XMI::Context *getContexts()
	{
		return _contexts;
	}

	inline size_t getClientId()
	{
		return _clientid;
	}

    inline xmi_result_t geometry_world_impl (xmi_geometry_t * world_geometry)
      {
        *world_geometry = _world_geometry;
        return XMI_SUCCESS;
      }

    inline xmi_result_t geometry_create_taskrange_impl(xmi_geometry_t       * geometry,
                                                       xmi_geometry_t         parent,
                                                       unsigned               id,
                                                       xmi_geometry_range_t * rank_slices,
                                                       size_t                 slice_count,
                                                       xmi_context_t          context,
                                                       xmi_event_function     fn,
                                                       void                 * cookie)
      {
        MPIGeometry              *new_geometry;

        if(geometry != NULL)
            {
              new_geometry=(MPIGeometry*) malloc(sizeof(*new_geometry));
              new(new_geometry) MPIGeometry((XMI::Geometry::Common*)parent,
                                            &__global.mapping,
                                            id,
                                            slice_count,
                                            rank_slices);
              for(size_t n=0; n<_ncontexts; n++)
                  {
                    _contexts[n]._pgas_collreg->analyze(n,new_geometry);
                    _contexts[n]._oldccmi_collreg->analyze(n,new_geometry);
                    _contexts[n]._ccmi_collreg->analyze(n,new_geometry);
                  }
              *geometry=(MPIGeometry*) new_geometry;
              // todo:  deliver completion to the appropriate context
            }
        MPIGeometry *bargeom = (MPIGeometry*)parent;
        XMI::Context *ctxt = (XMI::Context *)context;
        bargeom->default_barrier(fn, cookie, ctxt->getId(), context);
        return XMI_SUCCESS;
      }


    inline xmi_result_t geometry_create_tasklist_impl(xmi_geometry_t       * geometry,
                                                      xmi_geometry_t         parent,
                                                      unsigned               id,
                                                      xmi_task_t           * tasks,
                                                      size_t                 task_count,
                                                      xmi_context_t          context,
                                                      xmi_event_function     fn,
                                                      void                 * cookie)
      {
        // todo:  implement this routine
        XMI_abort();

        return XMI_SUCCESS;
      }


    inline xmi_result_t geometry_destroy_impl (xmi_geometry_t geometry)
      {
        XMI_abort();
        return XMI_UNIMPL;
      }

    protected:

      inline xmi_client_t getClient () const
        {
          return _client;
        }

    private:
      xmi_client_t _client;
      size_t _clientid;
      size_t       _references;
      size_t       _ncontexts;
	XMI::Context *_contexts;
	XMI::PlatformDeviceList _platdevs;

        char         _name[256];
    int                           _myrank;
    int                           _mysize;
    MPIGeometry                  *_world_geometry;
    xmi_geometry_range_t          _world_range;

        Memory::MemoryManager _mm;

        inline void initializeMemoryManager ()
        {
          char   shmemfile[1024];
          size_t bytes     = 1024*1024;
          size_t pagesize  = 4096;

          snprintf (shmemfile, 1023, "/xmi-client-%s", _name);
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
          // Failed to create shared memory .. fake it using the heap ??
          _mm.init (malloc (n), n);

          return;
        }

    }; // end class XMI::Client
}; // end namespace XMI


#endif
// __xmi_mpi_mpiclient_h__
