/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/ShmemCollDevice.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_ShmemCollDevice_h__
#define __components_devices_shmemcoll_ShmemCollDevice_h__

#include <sys/uio.h>

#include "sys/pami.h"

#include "Arch.h"
#include "Memregion.h"

//#include "components/atomic/Counter.h"
//#include "components/atomic/CounterInterface.h"
//#include "components/atomic/indirect/IndirectCounter.h"

#include "components/devices/BaseDevice.h"
#include "components/devices/FactoryInterface.h"
#include "components/devices/PacketInterface.h"
#include "components/devices/generic/Device.h"
#include "components/devices/shmem/ShmemMessage.h"
#include "components/memory/MemoryAllocator.h"

#include "components/devices/shmemcoll/ShmemCollDesc.h"
#include "components/devices/ShmemCollInterface.h"
#include "common/default/PipeWorkQueue.h"

#include "components/devices/MulticastModel.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MulticombineModel.h"

//#include "components/devices/shmemcoll/msgs/ShortMcombMessage.h"
//#include "components/devices/shmemcoll/msgs/ShortMcstMessage.h"
#include "components/devices/shmemcoll/msgs/BaseMessage.h"


#undef TRACE_ERR

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

#define MATCH_DISPATCH_SIZE	256
namespace PAMI
{
  namespace Device
  {

    typedef struct match_dispatch_t
    {
      Interface::MatchFunction_t   function;
      void                      * clientdata;
    } match_dispatch_t;



    template < class T_Atomic >
      class ShmemCollDevice : public Interface::BaseDevice< ShmemCollDevice<T_Atomic> >
    {

      typedef Shmem::BaseMessage<ShmemCollDevice<T_Atomic>, Shmem::ShmemCollDesc<T_Atomic> > BaseMessage;

      protected:

      public:

        // Inner factory class
        class Factory : public Interface::FactoryInterface<Factory, ShmemCollDevice, PAMI::Device::Generic::Device>
      {
        public:
          static inline ShmemCollDevice * generate_impl (size_t clientid, size_t n, Memory::MemoryManager & mm, Generic::Device *gds)
          {
            ShmemCollDevice * devices = NULL;
            pami_result_t rc = PAMI_ERROR;
	    rc = __global.heap_mm->memalign((void **) & devices, 16, sizeof(*devices) * n);
            PAMI_assertf(rc == PAMI_SUCCESS, "alloc failed for ShmemCollDevice[%zu], errno=%d\n", n, errno);

            for (unsigned i = 0; i < n; ++i)
            {
              new (&devices[i]) ShmemCollDevice (clientid, n, i, mm, gds);
            }

            TRACE_ERR((stderr, "<< ShmemCollDevice::Factory::generate_impl()\n"));
            return devices;
          };

          static inline pami_result_t init_impl (ShmemCollDevice    * devices,
              size_t           clientid,
              size_t           contextid,
              pami_client_t     client,
              pami_context_t    context,
              Memory::MemoryManager *mm,
              Generic::Device * progress)
          {
            return getDevice_impl(devices, clientid, contextid).init (clientid, contextid, client, context, mm, progress);
          };

          static inline size_t advance_impl (ShmemCollDevice * devices,
              size_t        clientid,
              size_t        contextid)
          {
            return getDevice_impl(devices, clientid, contextid).advance ();
          };

          static inline ShmemCollDevice & getDevice_impl (ShmemCollDevice * devices,
              size_t        clientid,
              size_t        contextid)
          {
            return devices[contextid];
          };
          /// 
          /// \brief Initialize the shmem peer fifos
          ///
          /// \see PAMI::Memory::MM_INIT_FN
          ///
          static void mm_initialize(void       * memory,
              size_t       bytes,
              const char * key,
              unsigned     attributes,
              void       * cookie)
          {
            size_t  *ncontexts  = (size_t  *)memory;
            size_t   npeers     = *(size_t  *)cookie;
            TRACE_ERR((stderr, "mm_initialize() %s, npeers %zu, memory %p, cookie %p\n", key, npeers, memory, cookie));
            ncontexts[npeers] = 0; // Unnecessary? But this is all we require from init.
            // More initialization/synchronization is done back in generate_impl.
          };
      };

       inline ShmemCollDevice (size_t clientid, size_t ncontexts, size_t contextid,
                            Memory::MemoryManager & mm, PAMI::Device::Generic::Device * progress) :
        Interface::BaseDevice< ShmemCollDevice<T_Atomic> > (),
          _contextid (contextid),
          _clientid (clientid),
          _ncontexts (ncontexts),
          _progress (progress),
          _local_progress_device (&(Generic::Device::Factory::getDevice (progress, 0, contextid))),
          _desc_fifo(mm, clientid, contextid)//, _adv_obj(NULL)
      {
        TRACE_ERR((stderr, "ShmemCollDevice() constructor\n"));

        // Get the peer id for this task
        _me = 0;
        PAMI::Interface::Mapping::nodeaddr_t address;
        __global.mapping.nodeAddr (address);
        __global.mapping.node2peer (address, _me);

      };

        //inline ~ShmemCollDevice () {};
        inline ~ShmemCollDevice ();

        inline size_t getLocalRank ();

        inline PAMI::Device::Generic::Device&  getProgressDevice();
        inline PAMI::Device::Generic::Device*  getProgressDeviceNew();

        // ------------------------------------------

        /// \see PAMI::Device::Interface::BaseDevice::isInit()
        bool isInit_impl ();

        /// \see PAMI::Device::Interface::BaseDevice::peers()
        inline size_t peers_impl ();

        /// \see PAMI::Device::Interface::BaseDevice::task2peer()
        inline size_t task2peer_impl (size_t task);

        /// \see PAMI::Device::Interface::BaseDevice::isPeer()
        inline bool isPeer_impl (size_t task);

        inline pami_context_t getContext_impl ();

        inline size_t getContextId_impl ();
        inline size_t getContextOffset_impl ();

        /// \see PAMI::Device::Interface::PacketDevice::read()
        inline int read_impl (void * buf, size_t length, void * cookie);

        //static const size_t metadata_size = T_Desc::packet_header_size - sizeof(uint16_t);
        //static const size_t payload_size  = T_Desc::packet_payload_size;

        pami_result_t getShmemWorldDesc(Shmem::ShmemCollDesc<T_Atomic> **my_desc);
        ///
        /// \brief Check if the send queue to an injection fifo is empty
        ///
        ///

        inline pami_result_t init (size_t clientid, size_t contextid, pami_client_t client, pami_context_t context, Memory::MemoryManager *mm, PAMI::Device::Generic::Device * progress);

        inline size_t advance ();

        pami_result_t registerMatchDispatch (Interface::MatchFunction_t   match_func, void * recv_func_parm, uint16_t &id);

        pami_client_t       _client;
        pami_context_t      _context;
        size_t             _contextid;
        size_t             _clientid;
        size_t              _ncontexts;        
        PAMI::Device::Generic::Device * _progress;
        PAMI::Device::Generic::Device * _local_progress_device;
        Shmem::ShmemCollDescFifo<T_Atomic>  _desc_fifo;

        //Shmem::ShortMcombMessage<ShmemCollDevice<T_Atomic>, Shmem::ShmemCollDesc<T_Atomic> > *_adv_obj;
        //inline void post_obj(Shmem::ShortMcombMessage<ShmemCollDevice<T_Atomic>, Shmem::ShmemCollDesc<T_Atomic> > *obj){_adv_obj = obj;};
        /*Shmem::ShortMcstMessage<ShmemCollDevice<T_Atomic>, Shmem::ShmemCollDesc<T_Atomic> > *_adv_obj;
        inline void post_obj(Shmem::ShortMcstMessage<ShmemCollDevice<T_Atomic>, Shmem::ShmemCollDesc<T_Atomic> > *obj){_adv_obj = obj;};*/
        /*Shmem::BaseMessage<ShmemCollDevice<T_Atomic>, Shmem::ShmemCollDesc<T_Atomic> > *_adv_obj;
        inline void post_obj(Shmem::BaseMessage<ShmemCollDevice<T_Atomic>, Shmem::ShmemCollDesc<T_Atomic> > *obj){_adv_obj = obj;};*/

        PAMI::Queue     _collectiveQ;
        inline void post_obj(Shmem::BaseMessage<ShmemCollDevice<T_Atomic>, Shmem::ShmemCollDesc<T_Atomic> > *obj)
            {_collectiveQ.enqueue(obj);}
	PAMI::Queue::Iterator _Iter;

        match_dispatch_t  _dispatch[MATCH_DISPATCH_SIZE];

        size_t            _num_procs;
        size_t            _global_task;
        size_t            _local_task;
        size_t            _me;

    };

    template <class T_Atomic>
      inline ShmemCollDevice<T_Atomic>::~ShmemCollDevice()
      {
      }

    template <class T_Atomic>
      inline PAMI::Device::Generic::Device&  ShmemCollDevice<T_Atomic>::getProgressDevice()
      {
        return *_local_progress_device;
        return *_progress;
      }

    template <class T_Atomic>
      inline PAMI::Device::Generic::Device*  ShmemCollDevice<T_Atomic>::getProgressDeviceNew()
      {
        return _progress;
      }

    template <class T_Atomic>
      inline size_t ShmemCollDevice<T_Atomic>::getLocalRank()
      {
        return _local_task;
      }

    template <class T_Atomic>
      inline pami_context_t ShmemCollDevice<T_Atomic>::getContext_impl()
      {
        return _context;
      }

    template <class T_Atomic>
      inline size_t ShmemCollDevice<T_Atomic>::getContextId_impl()
      {
        return _contextid;
      }

    template <class T_Atomic>
      inline size_t ShmemCollDevice<T_Atomic>::getContextOffset_impl()
      {
        return getContextId_impl();
      }

    template <class T_Atomic>
      size_t ShmemCollDevice<T_Atomic>::advance ()
      {
        size_t events = 0;
  
        /*pami_result_t res = PAMI_EAGAIN;
        if (_adv_obj != NULL)
        {
          res = _adv_obj->__advance(_context, (void*)_adv_obj);
          if (res == PAMI_SUCCESS) _adv_obj = NULL;
        }*/

        pami_result_t res;
        BaseMessage *msg;
        _collectiveQ.iter_begin(&_Iter);
        for (; _collectiveQ.iter_check(&_Iter); _collectiveQ.iter_end(&_Iter)) {
          msg = (BaseMessage *)_collectiveQ.iter_current(&_Iter);
          res = msg->__advance(_context,(void*)msg);
          if (res != PAMI_EAGAIN){
            _collectiveQ.iter_remove(&_Iter);
            continue;
          }
        }

        /* Releasing done descriptors for comm world communicators */
        if (!_desc_fifo.is_empty())
        {
          //printf("Calling shmem colldevice advance\n");
          _desc_fifo.release_done_descriptors();
          events++;
        }

        return events;
      }

  };
};
#undef TRACE_ERR

// Include the non-inline method definitions
#include "components/devices/shmemcoll/ShmemCollDevice_impl.h"

#endif // __components_devices_shmem_shmembasedevice_h__
