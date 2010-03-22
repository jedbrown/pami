///
/// \file common/bgq/Client.h
/// \brief XMI client interface specific for the Blue Gene\Q platform.
///
#ifndef __common_bgq_Client_h__
#define __common_bgq_Client_h__

#include <stdlib.h>

#include "common/ClientInterface.h"

#include "Context.h"
#include <errno.h>

#undef USE_COMMTHREADS // define/undef
#ifdef USE_COMMTHREADS
#include "components/devices/bgq/commthread/CommThreadWakeup.h"
#endif // USE_COMMTHREADS

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
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
        static size_t next_client_id = 0;
        // Set the client name string.
        memset ((void *)_name, 0x00, sizeof(_name));
        strncpy (_name, name, sizeof(_name) - 1);

        _clientid = next_client_id++;
        // assert(_clientid < XMI_MAX_NUM_CLIENTS);

        // Get some shared memory for this client
        initializeMemoryManager ();

        result = XMI_SUCCESS;
      }

      inline ~Client ()
      {
      }

      static xmi_result_t generate_impl (const char * name, xmi_client_t * client)
      {
        xmi_result_t result;
        int rc = 0;

        //__client_list->lock();

        // If a client with this name is not already initialized...
        XMI::Client * clientp = NULL;
        //if ((client = __client_list->contains (name)) == NULL)
        //{
        rc = posix_memalign((void **) & clientp, 16, sizeof (XMI::Client));

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

        return result;
      }

      static void destroy_impl (xmi_client_t client)
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

      inline xmi_result_t createContext_impl (xmi_configuration_t   configuration[],
                                              size_t                count,
                                              xmi_context_t       * context,
                                              size_t                ncontexts)
      {
        TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
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
#ifdef USE_COMMTHREADS
        // Create one comm thread semi-opaque pointer. Internally, this may be
        // one-per-context (optimal advance scenario) or some other arrangement.
        _commThreads = XMI::Device::CommThread::BgqCommThread::generate(_clientid, n, &_mm);
        XMI_assertf(_commThreads, "BgqCommThread::generate failed for _commThreads[%d]\n", n);
#endif // USE_COMMTHREADS
        int x;
        TRACE_ERR((stderr, "%d:mm available %zd\n", __LINE__, _mm.available()));
        _platdevs.generate(_clientid, n, _mm);

        // This memset has been removed due to the amount of cycles it takes
        // on simulators.  Lower level initializers should be setting the
        // relevant fields of the context, so this memset should not be
        // needed anyway.
        //memset((void *)_contexts, 0, sizeof(XMI::Context) * n);
        TRACE_ERR((stderr, "%d:mm available %zd\n", __LINE__, _mm.available()));
        size_t bytes = _mm.available() / n - 16;

        for (x = 0; x < n; ++x)
          {
            context[x] = (xmi_context_t) & _contexts[x];
            void *base = NULL;
            _mm.enable();
            _mm.memalign((void **)&base, 16, bytes);
            _mm.disable();
            XMI_assertf(base != NULL, "out of sharedmemory in context create x=%d,n=%d,bytes=%zd,mm.size=%zd,mm.available=%zd\n", x, n, bytes, _mm.size(), _mm.available());
            new (&_contexts[x]) XMI::Context(this->getClient(), _clientid, x, n,
                                             &_platdevs, base, bytes);
#ifdef USE_COMMTHREADS
            // Note, this is not inializing each comm thread but rather
            // initializing comm threads for each context.
            XMI::Device::CommThread::BgqCommThread::initContext(_commThreads, _clientid, x, context[x]);
#endif // USE_COMMTHREADS
            //_context_list->pushHead((QueueElem *)&context[x]);
            //_context_list->unlock();
          }

        TRACE_ERR((stderr,  "%s exit\n", __PRETTY_FUNCTION__));

        return XMI_SUCCESS;
      }

      inline xmi_result_t destroyContext_impl (xmi_context_t context)
      {
        //_context_list->lock ();
        //_context_list->remove (context);
        return ((XMI::Context *)context)->destroy ();
        //_context_list->unlock ();
      }

#ifdef USE_COMMTHREADS
      // This is not standard interface... yet?
      inline xmi_result_t addContextToCommThreadPool(xmi_context_t ctx) {
	return XMI::Device::CommThread::BgqCommThread::addContext(_commThreads, _clientid, ctx);
      }
#endif // USE_COMMTHREADS

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
              configuration->value.doubleval = __global.time.tick();
              result = XMI_SUCCESS;
              break;
            case XMI_PROCESSOR_NAME:
            {
              int rc;
              char* pn = __global.processor_name;
              /// \todo This should be more descriptive and the
              /// snprintf() should be run at init only.  This is
              /// the BGP DCMF version:
              /// "Rank 0 of 128 (0,0,0,0)  R00-M0-N10-J01"
              rc = snprintf(pn, 128, "Task %zu of %zu", __global.mapping.task(), __global.mapping.size());
              pn[128-1] = 0;
              configuration->value.chararray = pn;

              if (rc > 0)
                result = XMI_SUCCESS;
            }
            break;
            case XMI_MEM_SIZE:
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
        XMI_abortf("%s<%d>\n", __FILE__, __LINE__);
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
        XMI_abortf("%s<%d>\n", __FILE__, __LINE__);
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
        XMI_abortf("%s<%d>\n", __FILE__, __LINE__);
        return XMI_SUCCESS;
      }

      inline xmi_result_t geometry_destroy_impl (xmi_geometry_t geometry)
      {
        XMI_abortf("%s<%d>\n", __FILE__, __LINE__);
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

      Memory::MemoryManager _mm;
#ifdef USE_COMMTHREADS
      XMI::Device::CommThread::BgqCommThread *_commThreads;
#endif // USE_COMMTHREADS

      inline void initializeMemoryManager ()
      {
        char   shmemfile[1024];
        //size_t bytes     = 1024*1024;
        size_t bytes     = 768 * 1024;
        //size_t pagesize  = 4096;

        snprintf (shmemfile, 1023, "/xmi-client-%s", _name);

        // Round up to the page size
        //size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

        int fd, rc;
        size_t n = bytes;

#if 1
        // CAUTION! The following sequence MUST ensure that "rc" is "-1" iff failure.

        rc = shm_open (shmemfile, O_CREAT | O_RDWR, 0600);
        if (rc == -1)
          fprintf(stderr, "shm_open(<%s>,O_RDWR) rc = %d, errno = %d, %s\n", shmemfile,  rc,  errno,  strerror(errno));

        void * ptr = NULL;

        if ( rc != -1 )
          {
            fd = rc;
            rc = ftruncate( fd, n );

            if ( rc != -1 )
              {
                ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

                while (ptr == MAP_FAILED)
                  {
                    fprintf(stderr, "%s:%d Failed to mmap (rc=%d, ptr=%p, n=%zd) errno %d %s\n", __FILE__, __LINE__, rc, ptr, n, errno, strerror(errno));
                    n /= 2;

                    if (n < (32*1024)) break;

                    ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                  }

                if ( ptr != MAP_FAILED )
                  {
#ifdef ENABLE_MAMBO_WORKAROUNDS
                    fprintf(stderr, "Client:shmem file <%s> %zd bytes mapped at %p\n", shmemfile, n, ptr);
#endif
                    TRACE_ERR((stderr, "Client:shmem file <%s> %zd bytes mapped at %p\n", shmemfile, n, ptr));
                    _mm.init (ptr, n);
                    return;
                  }
              }
          }

        fprintf(stderr, "%s:%d Failed to create shared memory <%s> (rc=%d, ptr=%p, n=%zd) errno %d %s\n", __FILE__, __LINE__, shmemfile, rc, ptr, n, errno, strerror(errno));
        //XMI_abortf(stderr,"Failed to create shared memory (rc=%d, ptr=%p, n=%zd)\n", rc, ptr, n);

        // Failed to create shared memory .. fake it using the heap ??
        _mm.init (malloc (n), n);
#endif
        return;
      }
  }; // end class XMI::Client
}; // end namespace XMI
#endif // __components_client_bgq_bgqclient_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
