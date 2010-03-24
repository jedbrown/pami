///
/// \file common/lapiunix/Client.h
/// \brief PAMI client interface specific for the LAPI platform.
///
#ifndef __common_lapiunix_Client_h__
#define __common_lapiunix_Client_h__

#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "common/ClientInterface.h"
#include "Context.h"
#include "algorithms/geometry/Geometry.h"

namespace PAMI
{
    class Client : public Interface::Client<PAMI::Client>
    {
    public:

      static void shutdownfunc()
        {
      }



    inline pami_result_t initializeLapiHandle(lapi_handle_t *out_lapi,
                                int           *out_myrank,
                                int           *out_mysize)
      {
        lapi_info_t   * lapi_info;     /* used as argument to LAPI_Init */
        lapi_extend_t * extend_info;   /* holds IP addresses and ports */
        lapi_udp_t    * udp_info;      /* List of ip, port info to pass to LAPI */
        int             num_tasks;     /* number of tasks (from LAPI_LIST_NAME) */
        char          * list_name;     /* name of UDP host/port list file */
        FILE          * fp;
        int             i;

        /* ------------------------------------------------------------ */
        /*       allocate and initialize lapi_info                      */
        /* ------------------------------------------------------------ */
        CheckNULL(lapi_info,(lapi_info_t *)malloc(sizeof(lapi_info_t)));
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
              CheckNULL(udp_info,(lapi_udp_t *) malloc(num_tasks*sizeof(lapi_udp_t)));
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
              CheckNULL(extend_info,(lapi_extend_t *)malloc(sizeof(lapi_extend_t)));
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
        //lapi_info->protocol_name = "pami";
        CheckLapiRC(lapi_init(out_lapi, lapi_info));
        CheckLapiRC(lapi_senv(*out_lapi,INTERRUPT_SET, intval));
        CheckLapiRC(lapi_qenv(*out_lapi,TASK_ID,
                              out_myrank));
        CheckLapiRC(lapi_qenv(*out_lapi,
                              NUM_TASKS,
                              out_mysize));
        free(lapi_info);
        return PAMI_SUCCESS;
      }




      inline Client (const char * name, pami_result_t & result) :
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
          initializeMemoryManager ();

        initializeLapiHandle(&_main_lapi_handle,
                             &_myrank,
                             &_mysize);


        __global.mapping.init(_myrank, _mysize);
        _world_geometry=(LAPIGeometry*) malloc(sizeof(*_world_geometry));
        _world_range.lo=0;
        _world_range.hi=_mysize-1;
        new(_world_geometry) LAPIGeometry(NULL, &__global.mapping,0, 1,&_world_range);
        CheckLapiRC(lapi_gfence (_main_lapi_handle));
          result = PAMI_SUCCESS;
        }

      inline ~Client ()
        {
        }

      static pami_result_t generate_impl (const char * name, pami_client_t * client)
        {
          int rc = 0;
          PAMI::Client * clientp;
          clientp = (PAMI::Client *)malloc(sizeof (PAMI::Client));
          assert(clientp != NULL);
          memset ((void *)clientp, 0x00, sizeof(PAMI::Client));
          pami_result_t res;
          new (clientp) PAMI::Client (name, res);
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
		size_t bytes = _mm.size() / n;
		int x;
                int first_context = 0;
                for (x = 0; x < n; ++x)
                    {
                      lapi_handle_t lhandle;
                      if(!first_context)
                          {
                            lhandle=_main_lapi_handle;
                            first_context = 1;
                          }
                      else
                          {
                            int mrank, msize;
                            initializeLapiHandle(&lhandle,
                                                 &mrank,
                                                 &msize);
                          }

                      context[x] = (pami_context_t)&_contexts[x];
                      void *base = NULL;
                      _mm.enable();
                      _mm.memalign((void **)&base, 16, bytes);
                      _mm.disable();
                      PAMI_assertf(base != NULL, "out of sharedmemory in context create\n");
                      new (&_contexts[x]) PAMI::Context(this->getClient(),
                                                       _clientid,
                                                       x,
                                                       n,
                                                       &_platdevs,
                                                       base,
                                                       bytes,
                                                       _world_geometry,
                                                       lhandle);
                      _ncontexts++;
                    }
                CheckLapiRC(lapi_gfence (_main_lapi_handle));

		return PAMI_SUCCESS;
        }

      inline pami_result_t destroyContext_impl (pami_context_t context)
        {
          free(context);
          return PAMI_SUCCESS;
        }

	inline pami_result_t queryConfiguration_impl (pami_configuration_t * configuration)
	{
		pami_result_t result = PAMI_ERROR;

		switch (configuration->name)
		{
		case PAMI_NUM_CONTEXTS:
			configuration->value.intval = 1; // real value TBD
			result = PAMI_SUCCESS;
			break;
		case PAMI_CONST_CONTEXTS:
			configuration->value.intval = 0; // real value TBD
			result = PAMI_SUCCESS;
			break;
		case PAMI_TASK_ID:
			configuration->value.intval = __global.mapping.task();
			result = PAMI_SUCCESS;
			break;
		case PAMI_NUM_TASKS:
			configuration->value.intval = __global.mapping.size();
			result = PAMI_SUCCESS;
			break;
		case PAMI_CLOCK_MHZ:
		case PAMI_WTIMEBASE_MHZ:
			configuration->value.intval = __global.time.clockMHz();
			result = PAMI_SUCCESS;
			break;
		case PAMI_WTICK:
			configuration->value.doubleval =__global.time.tick();
			result = PAMI_SUCCESS;
			break;
		case PAMI_MEM_SIZE:
		case PAMI_PROCESSOR_NAME:
		default:
			break;
		}
		return result;
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

    inline pami_result_t geometry_create_taskrange_impl(pami_geometry_t       * geometry,
                                                       pami_geometry_t         parent,
                                                       unsigned               id,
                                                       pami_geometry_range_t * rank_slices,
                                                       size_t                 slice_count,
                                                       pami_context_t          context,
                                                       pami_event_function     fn,
                                                       void                 * cookie)
      {
        LAPIGeometry              *new_geometry;

        if(geometry != NULL)
            {
              new_geometry=(LAPIGeometry*) malloc(sizeof(*new_geometry));
              new(new_geometry) LAPIGeometry((PAMI::Geometry::Common*)parent,
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
              *geometry=(LAPIGeometry*) new_geometry;
              // todo:  deliver completion to the appropriate context
            }
        LAPIGeometry *bargeom = (LAPIGeometry*)parent;
        PAMI::Context *ctxt = (PAMI::Context *)context;
        bargeom->default_barrier(fn, cookie, ctxt->getId(), context);
        return PAMI_SUCCESS;
      }

    inline pami_result_t geometry_create_tasklist_impl(pami_geometry_t       * geometry,
                                                      pami_geometry_t         parent,
                                                      unsigned               id,
                                                      pami_task_t           * tasks,
                                                      size_t                 task_count,
                                                      pami_context_t          context,
                                                      pami_event_function     fn,
                                                      void                 * cookie)
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
      pami_client_t _client;
      size_t _clientid;
      size_t       _references;
      size_t       _ncontexts;
	PAMI::Context *_contexts;
	PAMI::PlatformDeviceList _platdevs;

        char         _name[256];
    int                           _myrank;
    int                           _mysize;
    LAPIGeometry                 *_world_geometry;
    pami_geometry_range_t          _world_range;
        Memory::MemoryManager _mm;
    lapi_handle_t                 _main_lapi_handle;

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
          // Failed to create shared memory .. fake it using the heap ??
          _mm.init (malloc (n), n);

          return;
        }
    }; // end class PAMI::Client
}; // end namespace PAMI


#endif
// __pami_lapi_lapiclient_h__
