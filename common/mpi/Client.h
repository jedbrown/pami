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
#include "Geometry.h"

namespace XMI
{
    class Client : public Interface::Client<XMI::Client,XMI::Context>
    {
    public:

      static void shutdownfunc()
        {
          MPI_Finalize();
        }
      inline Client (const char * name, xmi_result_t & result) :
        Interface::Client<XMI::Client,XMI::Context>(name, result),
        _client ((xmi_client_t) this),
        _references (1),
        _contexts (0),
        _mm ()
        {
          // Set the client name string.
          memset ((void *)_name, 0x00, sizeof(_name));
          strncpy (_name, name, sizeof(_name) - 1);

          // Get some shared memory for this client
          initializeMemoryManager ();

          result = XMI_SUCCESS;
        }

      inline ~Client ()
        {
        }

      static xmi_result_t generate_impl (const char * name, xmi_client_t * in_client)
        {
          int rc = 0;
          XMI::Client * client;
          client = (XMI::Client *)malloc(sizeof (XMI::Client));
          assert(client != NULL);
          memset ((void *)client, 0x00, sizeof(XMI::Client));
          xmi_result_t res;
          new (client) XMI::Client (name, res);
          *in_client = (xmi_client_t*) client;
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
                                               xmi_context_t *contexts,
						int *ncontexts)
        {
		//_context_list->lock ();
		int n = *ncontexts;
		if (_contexts != 0) {
			*ncontexts = 0;
			return XMI_ERROR;
		}
		if (_contexts + n > 4) {
			n = 4 - _contexts;
		}
		*ncontexts = n;
		if (n <= 0) { // impossible?
			return XMI_ERROR;
		}
		XMI::Context *context = NULL;

#ifdef USE_MEMALIGN                
		int rc = posix_memalign((void **)&context, 16, sizeof(XMI::Context) * n);
		XMI_assertf(rc==0, "posix_memalign failed for context[%d], errno=%d\n", n, errno);
#else
                context = (XMI::Context*)malloc(sizeof(XMI::Context)*n);
		XMI_assertf(context!=NULL, "malloc failed for context[%d], errno=%d\n", n, errno);
#endif
		memset((void *)context, 0, sizeof(XMI::Context) * n);
		size_t bytes = _mm.size() / n;
		int x;
		for (x = 0; x < n; ++x) {
			contexts[x] = &context[x];
			void *base = NULL;
			_mm.memalign((void **)&base, 16, bytes);
			XMI_assertf(base != NULL, "out of sharedmemory in context create\n");
			new (&context[x]) XMI::Context(this->getClient(), _contexts++, base, bytes);
			//_context_list->pushHead((QueueElem *)&context[x]);
			//_context_list->unlock();
		}
		return XMI_SUCCESS;
        }

      inline xmi_result_t destroyContext_impl (xmi_context_t context)
        {
          free(context);
          return XMI_SUCCESS;
        }

    protected:

      inline xmi_client_t getClient () const
        {
          return _client;
        }

    private:
      xmi_client_t _client;
      size_t       _references;
      size_t       _contexts;
        char         _name[256];

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
          rc = shm_open (shmemfile, O_CREAT | O_RDWR, 0600);
          if ( rc != -1 )
          {
            fd = rc;
            rc = ftruncate( fd, n );
            if ( rc != -1 )
            {
              void * ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
              if ( ptr != MAP_FAILED )
              {
                _mm.init (ptr, n);
                return;
              }
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
