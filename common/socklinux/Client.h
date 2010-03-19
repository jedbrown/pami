///
/// \file common/socklinux/Client.h
/// \brief XMI client interface specific for the Blue Gene\Q platform.
///
#ifndef __common_socklinux_Client_h__
#define __common_socklinux_Client_h__

#include <stdlib.h>

#include "common/ClientInterface.h"

#include "Context.h"
#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace XMI
{
    class Client : public Interface::Client<XMI::Client>
    {
      public:
        inline Client (const char * name, xmi_result_t &result) :
          Interface::Client<XMI::Client>(name, result),
          _client ((xmi_client_t) this),
          _references (1),
          _ncontexts (0),
          _mm ()
        {
          TRACE_ERR((stderr, ">> Client::Client()\n"));
	  static size_t next_client_id = 0;
          // Set the client name string.
          memset ((void *)_name, 0x00, sizeof(_name));
          strncpy (_name, name, sizeof(_name) - 1);

	  _clientid = next_client_id++;
          // Get some shared memory for this client
          initializeMemoryManager ();

          result = XMI_SUCCESS;
          TRACE_ERR((stderr, "<< Client::Client()\n"));
        }

        inline ~Client ()
        {
          TRACE_ERR((stderr, ">> Client::~Client()\n"));
          TRACE_ERR((stderr, "<< Client::~Client()\n"));
        }

        static xmi_result_t generate_impl (const char * name, xmi_client_t * client)
        {
          TRACE_ERR((stderr, ">> Client::generate_impl(\"%s\", %p)\n", name, client));

          xmi_result_t result;
          int rc = 0;

          //__client_list->lock();

          // If a client with this name is not already initialized...
          XMI::Client * clientp = NULL;
          //if ((client = __client_list->contains (name)) == NULL)
          //{
            rc = posix_memalign((void **)&clientp, 16, sizeof (XMI::Client));
            if (rc != 0) assert(0);
            memset ((void *)clientp, 0x00, sizeof(XMI::Client));
            new (clientp) XMI::Client (name, result);
            *client = clientp;
            //__client_list->pushHead ((QueueElem *) client);
          //}
          //else
          //{
            //client->incReferenceCount ();
          //}

          //__client_list->unlock();

          TRACE_ERR((stderr, "<< Client::generate_impl(\"%s\", %p), result = %d\n", name, client, result));

          return result;
        }

        static void destroy_impl (xmi_client_t client)
        {
          //__client_list->lock ();
          //client->decReferenceCount ();
          //if (client->getReferenceCount () == 0)
          //{
            //__client_list->remove (client);
            Client * c = (Client *) client;
            shm_unlink (c->_shmemfile);

            free ((void *) client);
          //}
          //__client_list->unlock ();
        }

        inline char * getName_impl ()
        {
          return _name;
        }

        inline xmi_result_t createContext_impl (xmi_configuration_t   configuration[],
                                                size_t                count,
                                                xmi_context_t       * context,
                                                size_t                ncontexts)
        {
          TRACE_ERR((stderr, ">> Client::createContext_impl()\n"));
        //_context_list->lock ();
        int n = ncontexts;

        if (_ncontexts != 0)
          {
            return XMI_ERROR;
          }

        if (_ncontexts + n > 4)
          {
            n = 4 - _ncontexts;
          }

        if (n <= 0)   // impossible?
          {
            return XMI_ERROR;
          }

        int rc = posix_memalign((void **) & _contexts, 16, sizeof(*_contexts) * n);
        XMI_assertf(rc == 0, "posix_memalign failed for _contexts[%d], errno=%d\n", n, errno);
        int x;

	_platdevs.generate(_clientid, n, _mm);

        // This memset has been removed due to the amount of cycles it takes
        // on simulators.  Lower level initializers should be setting the
        // relevant fields of the context, so this memset should not be
        // needed anyway.
        //memset((void *)_contexts, 0, sizeof(XMI::Context) * n);
        size_t bytes = _mm.available() / n - 16;

        for (x = 0; x < n; ++x)
          {
            context[x] = (xmi_context_t) & _contexts[x];
            void *base = NULL;
            _mm.memalign((void **)&base, 16, bytes);
            XMI_assertf(base != NULL, "out of sharedmemory in context create\n");
            new (&_contexts[x]) XMI::Context(this->getClient(), _clientid, x, n,
                                             &_platdevs, base, bytes);
            //_context_list->pushHead((QueueElem *)&context[x]);
            //_context_list->unlock();
          }

          TRACE_ERR((stderr, "<< Client::createContext_impl()\n"));
        return XMI_SUCCESS;
        }

        inline xmi_result_t destroyContext_impl (xmi_context_t context)
        {
          //_context_list->lock ();
          //_context_list->remove (context);
          return ((XMI::Context *)context)->destroy ();
          //_context_list->unlock ();
        }

	inline xmi_result_t queryConfiguration_impl (xmi_configuration_t * configuration)
	{
		xmi_result_t result = XMI_ERROR;

		switch (configuration->name)
		{
		case XMI_NUM_CONTEXTS:
			configuration->value.intval = 64; // modified by runmode?
			result = XMI_SUCCESS;
			break;
		case XMI_CONST_CONTEXTS:
			configuration->value.intval = 1; // .TRUE.
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

	// the friend clause is actually global, but this helps us remember why...
	//friend class XMI::Device::Generic::Device;
	//friend class xmi.cc

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
        XMI_abort();
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
        XMI_abort();
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
        size_t       _clientid;
        size_t       _references;
        size_t       _ncontexts;
	XMI::Context *_contexts;
	XMI::PlatformDeviceList _platdevs;
        char         _name[256];

        Memory::MemoryManager _mm;
        char   _shmemfile[1024];

        inline void initializeMemoryManager ()
        {
          size_t bytes     = 1024*1024;
          size_t pagesize  = 4096;

          char * jobstr = getenv ("XMI_JOB_ID");
          if (jobstr)
            snprintf (_shmemfile, 1023, "/xmi-client-%s-%s", _name, jobstr);
          else
            snprintf (_shmemfile, 1023, "/xmi-client-%s", _name);

          // Round up to the page size
          size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

          int fd, rc;
          size_t n = bytes;

          // CAUTION! The following sequence MUST ensure that "rc" is "-1" iff failure.
          rc = shm_open (_shmemfile, O_CREAT | O_RDWR, 0600);
//fprintf (stderr, "initializeMemoryManager() .. shmemfile = \"%s\", rc = %d\n", shmemfile, rc);
          if ( rc != -1 )
          {
            fd = rc;
            rc = ftruncate( fd, n );
//fprintf (stderr, "initializeMemoryManager() .. rc = %d\n", rc);
            if ( rc != -1 )
            {
              void * ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//fprintf (stderr, "initializeMemoryManager() .. ptr = %p, MAP_FAILED = %p\n", ptr, MAP_FAILED);
              if ( ptr != MAP_FAILED )
              {
                _mm.init (ptr, n);
                return;
              }
            }
          }
//perror (NULL);
//fprintf (stderr, "initializeMemoryManager() .. allocate memory from heap\n");

          // Failed to create shared memory .. fake it using the heap ??
          _mm.init (malloc (n), n);

          return;
        }
    }; // end class XMI::Client
}; // end namespace XMI

#undef TRACE_ERR
#endif // __common_socklinux_Client_h__
