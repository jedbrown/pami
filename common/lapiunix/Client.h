///
/// \file common/lapiunix/Client.h
/// \brief XMI client interface specific for the LAPI platform.
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

namespace XMI
{
    class Client : public Interface::Client<XMI::Client>
    {
    public:

      static void shutdownfunc()
        {
      }



    inline xmi_result_t initializeLapiHandle(lapi_handle_t *out_lapi,
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
        //lapi_info->protocol_name = "xmi";
        CheckLapiRC(lapi_init(out_lapi, lapi_info));
        CheckLapiRC(lapi_senv(*out_lapi,INTERRUPT_SET, intval));
        CheckLapiRC(lapi_qenv(*out_lapi,TASK_ID,
                              out_myrank));
        CheckLapiRC(lapi_qenv(*out_lapi,
                              NUM_TASKS,
                              out_mysize));
        free(lapi_info);
        return XMI_SUCCESS;
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

                      context[x] = (xmi_context_t)&_contexts[x];
                      void *base = NULL;
                      _mm.memalign((void **)&base, 16, bytes);
                      XMI_assertf(base != NULL, "out of sharedmemory in context create\n");
                      new (&_contexts[x]) XMI::Context(this->getClient(),
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
        LAPIGeometry              *new_geometry;

        if(geometry != NULL)
            {
              new_geometry=(LAPIGeometry*) malloc(sizeof(*new_geometry));
              new(new_geometry) LAPIGeometry((XMI::Geometry::Common*)parent,
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
    LAPIGeometry                 *_world_geometry;
    xmi_geometry_range_t          _world_range;
        Memory::MemoryManager _mm;
    lapi_handle_t                 _main_lapi_handle;

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
// __xmi_lapi_lapiclient_h__
