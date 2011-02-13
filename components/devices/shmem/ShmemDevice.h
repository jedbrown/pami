/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemDevice.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemDevice_h__
#define __components_devices_shmem_ShmemDevice_h__

#include <sys/uio.h>

#include <pami.h>

#include "components/memory/MemoryManager.h"
#include "Arch.h"
#include "Memregion.h"

#include "components/atomic/CounterInterface.h"
#include "components/devices/BaseDevice.h"
#include "components/devices/FactoryInterface.h"
#include "components/devices/PacketInterface.h"
#include "components/devices/generic/Device.h"
#include "components/devices/shmem/ShmemDispatch.h"
#include "components/devices/shmem/ShmemMessage.h"
#include "components/devices/shmem/ShmemPacket.h"
#include "components/devices/shmem/ShmemWork.h"
#include "components/devices/shmem/shaddr/NoShaddr.h"
#include "components/devices/shmem/wakeup/WakeupNoop.h"
#include "components/memory/MemoryAllocator.h"
#include "util/queue/Queue.h"
#include "util/queue/CircularQueue.h"

#include "components/devices/shmem/ShmemCollDesc.h"
#include "components/devices/shmem/msgs/BaseMessage.h"

#include "common/default/PipeWorkQueue.h"

#include "components/devices/ShmemCollInterface.h"
#include "components/devices/MulticastModel.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MulticombineModel.h"

#include "components/atomic/indirect/IndirectCounter.h"

//#define TRAP_ADVANCE_DEADLOCK
#define ADVANCE_DEADLOCK_MAX_LOOP 10000

//#define EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
#define EMULATE_NONDETERMINISTIC_SHMEM_DEVICE_FREQUENCY 4

//#define EMULATE_UNRELIABLE_SHMEM_DEVICE
#define EMULATE_UNRELIABLE_SHMEM_DEVICE_FREQUENCY 10

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

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

    template < class T_Fifo, class T_Shaddr = Shmem::NoShaddr, unsigned T_FifoCount = 64 >
    class ShmemDevice : public Interface::BaseDevice< ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount> >,
        public Interface::PacketDevice<ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount> >
    {
      public:

        typedef Shmem::ShmemCollDescFifo<Counter::Indirect<Counter::Native> >  CollectiveFifo;
        typedef Shmem::ShmemCollDesc< Counter::Indirect<Counter::Native> >     CollectiveDescriptor;
#if 0  // Todo:  find platform agnostic way to enable BGQ::IndirectL2
        typedef Shmem::ShmemCollDescFifo<PAMI::Counter::BGQ::IndirectL2>  CollectiveFifo;
        typedef Shmem::ShmemCollDesc<PAMI::Counter::BGQ::IndirectL2>     CollectiveDescriptor;
#endif      
        typedef Shmem::BaseMessage<ShmemDevice<T_Fifo,T_Shaddr,T_FifoCount> > BaseMessage;


        // Inner factory class
        class Factory : public Interface::FactoryInterface<Factory, ShmemDevice, PAMI::Device::Generic::Device>
        {
          public:
            static inline ShmemDevice * generate_impl (size_t clientid,
                                                       size_t ncontexts,
                                                       Memory::MemoryManager & mm,
                                                       PAMI::Device::Generic::Device * progress)
            {
              TRACE_ERR((stderr, ">> ShmemDevice::Factory::generate_impl() ncontexts = %zu\n", ncontexts));

              // Allocate an array of shared memory devices, one for each
              // context in this task (from heap, not from shared memory)
              ShmemDevice * devices;
              pami_result_t mmrc;
	      mmrc = __global.heap_mm->memalign((void **) & devices, 16, sizeof(*devices) * ncontexts);
              PAMI_assertf(mmrc == PAMI_SUCCESS, "memalign failed for shared memory devices, rc=%d\n", mmrc);

              // Instantiate the shared memory devices
              unsigned i;

              for (i = 0; i < ncontexts; i++)
                {
                  new (&devices[i]) ShmemDevice (clientid, ncontexts, mm, i, progress);
                }

              TRACE_ERR((stderr, "<< ShmemDevice::Factory::generate_impl()\n"));
              return devices;
            };

            static inline pami_result_t init_impl (ShmemDevice     * devices,
                                                   size_t            clientid,
                                                   size_t            contextid,
                                                   pami_client_t     client,
                                                   pami_context_t    context,
                                                   Memory::MemoryManager *mm,
                                                   PAMI::Device::Generic::Device * progress)
            {
              return getDevice_impl(devices, clientid, contextid).init (client, context);
            };

            static inline size_t advance_impl (ShmemDevice * devices,
                                               size_t        clientid,
                                               size_t        contextid)
            {
              return devices[contextid].advance ();
            };

            static inline ShmemDevice & getDevice_impl (ShmemDevice * devices,
                                                        size_t        clientid,
                                                        size_t        contextid)
            {
              return devices[contextid];
            };
        };

        // Inner system shared address information class
        class SystemShaddrInfo
        {
          public:
            inline SystemShaddrInfo (Memregion * origin_mr,
                                     Memregion * target_mr,
                                     size_t      origin_offset,
                                     size_t      target_offset,
                                     size_t      bytes) :
                _origin_mr (*origin_mr),
                _target_mr (*target_mr),
                _origin_offset (origin_offset),
                _target_offset (target_offset),
                _bytes (bytes)
            {
            }

            ///
            /// \see PAMI::Device::Interface::RecvFunction_t
            ///
            static int system_shaddr_read (void   * metadata,
                                           void   * payload,
                                           size_t   bytes,
                                           void   * recv_func_parm,
                                           void   * cookie)
            {
              TRACE_ERR((stderr, ">> SystemShaddrInfo::system_shaddr_read():%d .. recv_func_parm = %p\n", __LINE__, recv_func_parm));

              T_Shaddr * shaddr = (T_Shaddr *) recv_func_parm;

              SystemShaddrInfo * info = (SystemShaddrInfo *) payload;
              size_t total_bytes = info->_bytes;
              size_t bytes_copied = 0;
              size_t target_offset = info->_target_offset;
              size_t origin_offset = info->_origin_offset;

              while (bytes_copied < total_bytes)
              {
                bytes_copied += shaddr->read (&(info->_target_mr),
                                              target_offset + bytes_copied,
                                              &(info->_origin_mr),
                                              origin_offset + bytes_copied,
                                              total_bytes - bytes_copied);
              }

              TRACE_ERR((stderr, "<< SystemShaddrInfo::system_shaddr_read():%d\n", __LINE__));
              return 0;
            }

            ///
            /// \see PAMI::Device::Interface::RecvFunction_t
            ///
            static int system_shaddr_write (void   * metadata,
                                            void   * payload,
                                            size_t   bytes,
                                            void   * recv_func_parm,
                                            void   * cookie)
            {
              TRACE_ERR((stderr, ">> SystemShaddrInfo::system_shaddr_write():%d\n", __LINE__));

              T_Shaddr * shaddr = (T_Shaddr *) recv_func_parm;

              SystemShaddrInfo * info = (SystemShaddrInfo *) payload;
              size_t total_bytes = info->_bytes;
              size_t bytes_copied = 0;
              size_t target_offset = info->_target_offset;
              size_t origin_offset = info->_origin_offset;

              while (bytes_copied < total_bytes)
              {
                bytes_copied += shaddr->read (&(info->_origin_mr),
                                              origin_offset + bytes_copied,
                                              &(info->_target_mr),
                                              target_offset + bytes_copied,
                                              total_bytes - bytes_copied);
              }

              TRACE_ERR((stderr, "<< SystemShaddrInfo::system_shaddr_write():%d\n", __LINE__));
              return 0;
            }

            Memregion _origin_mr;
            Memregion _target_mr;
            size_t    _origin_offset;
            size_t    _target_offset;
            size_t    _bytes;
        };

        inline ShmemDevice (size_t clientid, size_t ncontexts,
                            Memory::MemoryManager & mm, size_t contextid,
                            PAMI::Device::Generic::Device * progress) :
            Interface::BaseDevice< ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount> > (),
            Interface::PacketDevice< ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount> > (),
            _clientid (clientid),
            _contextid (contextid),
            _ncontexts (ncontexts),
            _nfifos (0),
#ifdef EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
            __ndQ (),
            __ndpkt (),
#endif
            _progress (progress),
            _local_progress_device (&(Generic::Device::Factory::getDevice (progress, 0, contextid))),
            shaddr (),
            _desc_fifo ()//, _adv_obj (NULL)

        {
          TRACE_ERR((stderr, "ShmemDevice() constructor\n"));
          
          // Create a unique string, useful for memory manager alloaction, etc.
          snprintf(_unique_str, 15, "%2.2zu-%2.2zu", _clientid, _contextid);

          // Get the peer information for this task
          _peer = 0;
          PAMI::Interface::Mapping::nodeaddr_t address;
          __global.mapping.nodeAddr (address);
          __global.mapping.node2peer (address, _peer);
          __global.mapping.nodePeers (_npeers);
          _task = __global.mapping.task();

          // Initialize all fifos on the node
          size_t i, j;
          _nfifos = _npeers * _ncontexts;
          char fifokey[PAMI::Memory::MMKEYSIZE];

          for (i = 0; i < _npeers; i++)
          {
            for (j = 0; j < _ncontexts; j++)
            {
              size_t fnum = (i * _ncontexts) + j;
              snprintf (fifokey, PAMI::Memory::MMKEYSIZE - 1, "/device-shmem-client-%zu-fifo-%zu", _clientid, fnum);
              _fifo[fnum].initialize (&mm, fifokey, _npeers, i);
            }
          }

          // Initialize the reception fifo assigned to this context
          _rfifo.initialize (_fifo[fnum(_peer, _contextid)]);

          // Initialize the local send queue for each fifo on the node
          TRACE_ERR((stderr, "ShmemDevice() constructor: _nfifos = %zu, _npeers = %zu, _ncontexts = %zu\n", _nfifos, _npeers, _ncontexts));

          for (i = 0; i < _nfifos; i++)
            {
              PAMI_assert (_local_progress_device != NULL);
              _sendQ[i].init(_local_progress_device);
            }

          // Register system dispatch functions
          _dispatch.registerSystemDispatch (SystemShaddrInfo::system_shaddr_read,
                                            &shaddr, system_ro_put_dispatch);
                                            
                                            
          // Initialize the collective descriptor fifo
          _desc_fifo.init (mm, _unique_str);
        };

        inline ~ShmemDevice () {};

        inline size_t getLocalRank ();

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
        inline size_t getContextCount_impl ();

        // ------------------------------------------

        /// \see PAMI::Device::Interface::PacketDevice::read()
        inline int read_impl (void * buf, size_t length, void * cookie);

        static const bool reliable      = true;
        static const bool deterministic = true;

        static const size_t metadata_size = T_Fifo::packet_header_size - sizeof(uint16_t);
        static const size_t payload_size  = T_Fifo::packet_payload_size;

        // ------------------------------------------

        static const bool shaddr_va_supported    = T_Shaddr::shaddr_va_supported;
        static const bool shaddr_mr_supported    = T_Shaddr::shaddr_mr_supported;

        static const bool shaddr_read_supported  = T_Shaddr::shaddr_read_supported;
        static const bool shaddr_write_supported = T_Shaddr::shaddr_write_supported;

        // ------------------------------------------

        static const size_t completion_work_size = sizeof(Shmem::RecPacketWork<T_Fifo>);

        uint16_t system_ro_put_dispatch;

        ///
        /// \brief Register the receive function to dispatch when a packet arrives.
        ///
        /// \param [in]  set            Dispatch set identifier
        /// \param [in]  recv_func      Receive function to dispatch
        /// \param [in]  recv_func_parm Receive function client data
        /// \param [out] id             Dispatch id for this registration
        ///
        /// \return registration result
        ///
        pami_result_t registerRecvFunction (size_t                      set,
                                            Interface::RecvFunction_t   recv_func,
                                            void                      * recv_func_parm,
                                            uint16_t                  & id);

        ///
        /// \brief Register the system receive function to dispatch when a packet arrives.
        ///
        /// System dispatch ids are allocated by the device.
        ///
        /// \param [in]  recv_func      Receive function to dispatch
        /// \param [in]  recv_func_parm Receive function client data
        /// \param [out] id             Dispatch id for this registration
        ///
        /// \return registration result
        ///
        pami_result_t registerSystemRecvFunction (Interface::RecvFunction_t   recv_func,
                                                  void                      * recv_func_parm,
                                                  uint16_t                  & id);

        void dispatch (uint16_t id, void * metadata, void * payload, size_t bytes);

        pami_result_t post (size_t ififo, Shmem::SendQueue::Message * msg);

        pami_result_t postCompletion (uint8_t               state[completion_work_size],
                                      pami_event_function   local_fn,
                                      void                * cookie,
                                      size_t                fnum,
                                      size_t                sequence);

        ///
        /// \brief Check if the send queue to an injection fifo is empty
        ///
        ///
        inline bool isSendQueueEmpty (size_t fnum);

        inline Shmem::SendQueue *getQS (size_t fnum);

        inline pami_result_t init (pami_client_t client, pami_context_t context);

        inline size_t advance ();

        inline size_t fnum (size_t peer, size_t offset);

        inline bool activePackets (size_t fnum);
        
        inline char * getUniqueString ();
        
        inline PAMI::Device::Generic::Device * getLocalProgressDevice ();

        pami_result_t getShmemWorldDesc(CollectiveDescriptor **my_desc);
        pami_result_t registerMatchDispatch (Interface::MatchFunction_t   match_func, void * recv_func_parm, uint16_t &id);

        T_Fifo _fifo[T_FifoCount];  //< Injection fifo array for all node contexts
        T_Fifo  _rfifo;             //< Fifo to use as the reception fifo
        size_t * _last_inj_sequence_id;

        pami_client_t       _client;
        pami_context_t      _context;
        size_t             _clientid;
        size_t             _contextid;
        size_t             _ncontexts;
        size_t             _nfifos;

#ifdef EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
        Queue                                            __ndQ;
        MemoryAllocator < sizeof(UnexpectedPacket), 16 > __ndpkt;
#endif

        Shmem::SendQueue    _sendQ[T_FifoCount];
        PAMI::Device::Generic::Device * _progress;
        PAMI::Device::Generic::Device * _local_progress_device;

        T_Shaddr          shaddr;

        size_t            _npeers;
        size_t            _task;
        size_t            _peer;

        Shmem::Dispatch<Shmem::Packet<typename T_Fifo::Packet> >  _dispatch;
        
        // -------------------------------------------------------------
        // Collectives
        // -------------------------------------------------------------
        CollectiveFifo  _desc_fifo;
        //Shmem::BaseMessage<ShmemDevice<T_Fifo,T_Shaddr,T_FifoCount>, Shmem::ShmemCollDesc<T_Atomic> > *_adv_obj;
        //inline void post_obj(Shmem::BaseMessage<ShmemDevice<T_Fifo,T_Shaddr,T_FifoCount>, Shmem::ShmemCollDesc<T_Atomic> > *obj){_adv_obj = obj;};
        //Shmem::BaseMessage<ShmemDevice<T_Fifo,T_Shaddr,T_FifoCount> > *_adv_obj;
        //inline void post_obj(Shmem::BaseMessage<ShmemDevice<T_Fifo,T_Shaddr,T_FifoCount> > *obj){_adv_obj = obj;};

        PAMI::Queue     _collectiveQ;
        inline void post_obj(BaseMessage *obj)
            {_collectiveQ.enqueue(obj);}
	PAMI::Queue::Iterator _Iter;

        match_dispatch_t  _match_dispatch[MATCH_DISPATCH_SIZE];
        
        char _unique_str[16];
    };

    template <class T_Fifo, class T_Shaddr, unsigned T_FifoCount>
    inline size_t ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount>::getLocalRank()
    {
      return _peer;
    }

    template <class T_Fifo, class T_Shaddr, unsigned T_FifoCount>
    inline pami_context_t ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount>::getContext_impl()
    {
      return _context;
    }

    template <class T_Fifo, class T_Shaddr, unsigned T_FifoCount>
    inline size_t ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount>::getContextId_impl()
    {
      return _contextid;
    }
    template <class T_Fifo, class T_Shaddr, unsigned T_FifoCount>
    inline size_t ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount>::getContextOffset_impl()
    {
      return getContextId_impl();
    }

    template <class T_Fifo, class T_Shaddr, unsigned T_FifoCount>
    inline size_t ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount>::getContextCount_impl()
    {
      return _ncontexts;
    }

    ///
    /// \brief Check if the send queue to a local rank is empty
    ///
    /// \see fnum
    /// \param[in] fnum  Local injection fifo number
    ///
    template <class T_Fifo, class T_Shaddr, unsigned T_FifoCount>
    inline bool ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount>::isSendQueueEmpty (size_t fnum)
    {
      return (_sendQ[fnum].size() == 0);
    }

    template <class T_Fifo, class T_Shaddr, unsigned T_FifoCount>
    inline Shmem::SendQueue * ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount>::getQS (size_t fnum)
    {
      return &_sendQ[fnum];
    }

    /// \see PAMI::Device::Interface::PacketDevice::read()
    template <class T_Fifo, class T_Shaddr, unsigned T_FifoCount>
    int ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount>::read_impl (void * dst, size_t length, void * cookie)
    {
      memcpy (dst, cookie, length);
      return 0;
    }

    template <class T_Fifo, class T_Shaddr, unsigned T_FifoCount>
    size_t ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount>::advance ()
    {
      size_t events = 0;
      while (_rfifo.consumePacket(_dispatch))
      {
        events++;
      }

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
        events++;
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

#ifdef TRAP_ADVANCE_DEADLOCK
      static size_t iteration = 0;
      TRACE_ERR((stderr, "(%zu) ShmemDevice::advance() iteration %zu \n", __global.mapping.task(), iteration));
      PAMI_assert (iteration++ < ADVANCE_DEADLOCK_MAX_LOOP);

      if (events) iteration = 0;

#endif
      return events;
    }

    template <class T_Fifo, class T_Shaddr, unsigned T_FifoCount>
    inline size_t ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount>::fnum (size_t peer, size_t offset)
    {
      TRACE_ERR((stderr, "(%zu) ShmemDevice::fnum(%zu,%zu) -> %zu \n", __global.mapping.task(), peer, offset, _ncontexts * peer + offset));
      return _ncontexts * peer + offset;
    }

    template <class T_Fifo, class T_Shaddr, unsigned T_FifoCount>
    inline bool ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount>::activePackets (size_t fnum)
    {
      return (_fifo[fnum].lastPacketConsumed() < _fifo[fnum].lastPacketProduced());
    }

    template <class T_Fifo, class T_Shaddr, unsigned T_FifoCount>
    inline PAMI::Device::Generic::Device * ShmemDevice<T_Fifo, T_Shaddr, T_FifoCount>::getLocalProgressDevice ()
    {
      return _local_progress_device;
    }

  };
};
#undef TRACE_ERR

// Include the non-inline method definitions
#include "components/devices/shmem/ShmemDevice_impl.h"

#endif // __components_devices_shmem_ShmemDevice_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
