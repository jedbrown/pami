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

#include "SysDep.h"
#include "Arch.h"
#include "Memregion.h"

#include "components/atomic/Counter.h"
#include "components/devices/BaseDevice.h"
#include "components/devices/FactoryInterface.h"
#include "components/devices/PacketInterface.h"
#include "components/devices/generic/Device.h"
#include "components/devices/shmem/ShmemMessage.h"
#include "components/devices/shmem/shaddr/NoShaddr.h"
#include "components/memory/MemoryAllocator.h"
#include "util/fifo/LinearFifo.h"
#include "util/fifo/FifoPacket.h"
#include "util/queue/Queue.h"
#include "util/queue/CircularQueue.h"

//#define TRAP_ADVANCE_DEADLOCK
#define ADVANCE_DEADLOCK_MAX_LOOP 10000

//#define EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
#define EMULATE_NONDETERMINISTIC_SHMEM_DEVICE_FREQUENCY 4

//#define EMULATE_UNRELIABLE_SHMEM_DEVICE
#define EMULATE_UNRELIABLE_SHMEM_DEVICE_FREQUENCY 10


#define DISPATCH_SET_COUNT 256
#define DISPATCH_SET_SIZE   16

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

namespace PAMI
{
  namespace Device
  {
    ///
    /// \brief Dispatch array structure.
    ///
    typedef struct dispatch_t
    {
      Interface::RecvFunction_t   function;
      void                      * clientdata;
    } dispatch_t;

    template < class T_Fifo, class T_Shaddr = Shmem::NoShaddr >
    class ShmemDevice : public Interface::BaseDevice< ShmemDevice<T_Fifo,T_Shaddr> >,
        public Interface::PacketDevice<ShmemDevice<T_Fifo,T_Shaddr> >
    {
      protected:

        // Inner class
        class PacketImpl : public T_Fifo::PacketObject
        {
          public:
            inline  PacketImpl () {};
            inline ~PacketImpl () {};

            inline void writeDispatch (uint16_t dispatch)
            {
              uint16_t * hdr = (uint16_t *) this->getHeader ();
              hdr[(T_Fifo::packet_header_size>>1)-1] = dispatch;
              //printf("hdr:%p\n packet_header_size:%d\n", hdr, T_Fifo::packet_header_size);
            };

            template <unsigned T_Bytes>
            inline void writeMetadata (uint8_t (&metadata)[T_Bytes])
            {
              writeMetadata (metadata, T_Bytes);
            };

            inline void writeMetadata (uint8_t * metadata,
                                       size_t    bytes)
            {
              unsigned i;
              uint8_t * hdr = (uint8_t *) this->getHeader ();

              for (i = 0; i < bytes; i++) hdr[i] = metadata[i];
            };

            ///
            /// \brief Write iovec payload data to the packet
            ///
            /// This method allows for template specialization via the T_Niov
            /// template parameter.
            ///
            /// \param[in] iov       Array of T_Niov source data iovec elements
            ///
            template <unsigned T_Niov>
            inline void writePayload (struct iovec (&iov)[T_Niov])
            {
              unsigned i, j, n;

//fprintf (stderr, "PacketImpl::writePayload (iov[%zu])\n", T_Niov);
              if (T_Niov == 1)
                {
                  // Constant-expression template specialization.
                  n = (iov[0].iov_len >> 2) + ((iov[0].iov_len & 0x03) != 0);
                  uint32_t * dst = (uint32_t *) this->getPayload ();
                  uint32_t * src = (uint32_t *) iov[0].iov_base;

                  for (i = 0; i < n; i++) dst[i] = src[i];

                  return;
                }

              if (T_Niov == 2)
                {
                  // Constant-expression template specialization.
                  uint32_t * dst = (uint32_t *) this->getPayload ();
                  uint32_t * src = (uint32_t *) iov[0].iov_base;
                  n = (iov[0].iov_len >> 2) + ((iov[0].iov_len & 0x03) != 0);
                  //printf("iov[0] size:%d\n", iov[0].iov_len);

//fprintf (stderr, "PacketImpl::writePayload (iov[%zu]), 1st, n = %zu, dst = %p, src = %p\n", T_Niov, n, dst, src);
                  for (i = 0; i < n; i++) dst[i] = src[i];

                  dst = (uint32_t *)((uint8_t *) dst + iov[0].iov_len);
                  src = (uint32_t *) iov[1].iov_base;
                  n = (iov[1].iov_len >> 2) + ((iov[1].iov_len & 0x03) != 0);
                  //printf("iov[1] size:%d\n", iov[1].iov_len);

//fprintf (stderr, "PacketImpl::writePayload (iov[%zu]), 2nd, n = %zu, dst = %p, src = %p\n", T_Niov, n, dst, src);
                  for (i = 0; i < n; i++) dst[i] = src[i];

                  return;
                }

              uint8_t  * payload = (uint8_t *) this->getPayload ();
              uint32_t * dst;
              uint32_t * src;

              for (i = 0; i < T_Niov; i++)
                {
                  dst = (uint32_t *) payload;
                  src = (uint32_t *) iov[i].iov_base;
                  n = (iov[i].iov_len >> 2) + ((iov[i].iov_len & 0x03) != 0);

                  for (j = 0; j < n; j++) dst[j] = src[j];

                  payload += iov[i].iov_len;
                }
            };

            ///
            /// \brief Write a single contiguous payload data buffer to the packet
            ///
            /// \param[in] payload       Address of the buffer to write to the packet payload
            /// \param[in] bytes         Number of bytes to write
            ///
            inline void writePayload (void * payload, size_t bytes)
            {
              unsigned i, n;

              n = (bytes >> 2) + ((bytes & 0x03) != 0);
              uint32_t * dst = (uint32_t *) this->getPayload ();
              uint32_t * src = (uint32_t *) payload;

              for (i = 0; i < n; i++) dst[i] = src[i];

              return;
            }

            ///
            /// \brief Write a "full packet" payload data buffer to the packet
            ///
            /// \param[in] payload       Address of the buffer to write to the packet payload
            ///
            inline void writePayload (void * payload)
            {
              TRACE_ERR((stderr, ">> PacketImpl::writePayload(%p)\n", payload));
              this->copyPayload (payload);
              TRACE_ERR((stderr, "<< PacketImpl::writePayload(%p)\n", payload));
            }

            ///
            /// \brief Write iovec data to the packet payload
            ///
            /// \param[in] iov       Array of source data iovec elements
            /// \param[in] niov      Number of iovec array elements
            ///
            inline void writePayload (struct iovec * iov,
                                      size_t         niov)
            {
              unsigned i, j, n;
              uint8_t  * payload = (uint8_t *) this->getPayload ();
              uint32_t * dst;
              uint32_t * src;

              for (i = 0; i < niov; i++)
                {
                  dst = (uint32_t *) payload;
                  src = (uint32_t *) iov[i].iov_base;
                  n = (iov[i].iov_len >> 2) + ((iov[i].iov_len & 0x03) != 0);

                  for (j = 0; j < n; j++) dst[j] = src[j];

                  payload += iov[i].iov_len;
                }
            }

            ///
            /// \brief Retrieve the packet dispatch identifier
            ///
            inline uint16_t getDispatch ()
            {
              uint16_t * hdr = (uint16_t *) this->getHeader ();
              return hdr[(T_Fifo::packet_header_size>>1)-1];
            };

            ///
            /// \brief Retrieve the packet metadata pointer
            /// \return Pointer to packet metadata
            ///
            inline void * getMetadata ()
            {
              return this->getHeader ();
            };
        };

        class UnexpectedPacket : public CircularQueue::Element
        {
          public:
            inline UnexpectedPacket (PacketImpl * packet, size_t s = 0) :
                CircularQueue::Element (),
                sequence (s)
            {
              id = packet->getDispatch ();
              bytes = T_Fifo::packet_payload_size;
              memcpy ((void *) meta, packet->getMetadata (), T_Fifo::packet_header_size);
              memcpy ((void *) data, packet->getPayload (), T_Fifo::packet_payload_size);
            };

            uint16_t id;
            size_t   sequence;
            uint8_t  meta[T_Fifo::packet_header_size];
            uint8_t  data[T_Fifo::packet_payload_size];
            size_t   bytes;
        };

      public:

        // Inner factory class
        class Factory : public Interface::FactoryInterface<Factory, ShmemDevice, PAMI::Device::Generic::Device>
        {
          public:
            static inline ShmemDevice * generate_impl (size_t clientid, size_t n, Memory::MemoryManager & mm, PAMI::Device::Generic::Device *gds)
            {
              size_t i;
              TRACE_ERR((stderr, ">> ShmemDevice::Factory::generate_impl() n = %zu\n", n));

              // Determine the number of contexts on the entire node, then
              // allocate shared memory for all contexts.

              // Get the number of peer tasks on the node
              size_t npeers = 0;
              __global.mapping.nodePeers (npeers);
              TRACE_ERR((stderr, "ShmemDevice::Factory::generate_impl() npeers = %zu\n", npeers));

              // Allocate a "context count" array and a peer fifo pointer
              // array from shared memory
              volatile size_t * ncontexts = NULL;
              size_t size = sizeof(size_t) * (2 * npeers + 1);
              TRACE_ERR((stderr, "ShmemDevice::Factory::generate_impl() size = %zu\n", size));
              mm.memalign ((void **)&ncontexts, 16, size);
              TRACE_ERR((stderr, "ShmemDevice::Factory::generate_impl() ncontexts = %p\n", ncontexts));

              size_t * peer_fnum = (size_t *)ncontexts + npeers + 1;

              // Get the peer id for this task
              size_t me = 0;
              PAMI::Interface::Mapping::nodeaddr_t address;

              __global.mapping.nodeAddr (address);
              TRACE_ERR((stderr, "ShmemDevice::Factory::generate_impl() after nodeAddr() global %zu, local %zu\n",address.global,address.local));
              __global.mapping.node2peer (address, me);
              TRACE_ERR((stderr, "ShmemDevice::Factory::generate_impl() me = %zu\n", me));

	      // will there always be a "0"?
	      local_barriered_shmemzero((void *)ncontexts, size, npeers, me == 0);
	      ncontexts[me] = n;
	      __sync_fetch_and_add(&ncontexts[npeers], 1);
	      mem_sync(); // paranoia?
	      while (ncontexts[npeers] < npeers);
	      mem_sync(); // paranoia?
	      size_t total_fifos_on_node = 0;
              // Assign fifo indexes to the peer fnum cache while computing total
	      for (i = 0; i < npeers; i++)
	      {
		// This should only be done by one peer, but all write the same data
		// and this way no extra synchronization is needed.
		peer_fnum[i] = total_fifos_on_node;
	      	total_fifos_on_node += ncontexts[i];
	      }

              TRACE_ERR((stderr, "ShmemDevice::Factory::generate_impl() ncontexts = %p sync'd\n", ncontexts));

              // Allocate a shared memory segment for _all_ of the fifos for
              // _all_ of the contexts
              T_Fifo * all_fifos = NULL;
              size = ((sizeof(T_Fifo) + 15) & 0xfff0) * total_fifos_on_node;
              mm.memalign ((void **)&all_fifos, 16, size);

              // Allocate an array of shared memory devices, one for each
              // context in this _task_ (from heap, not from shared memory)
              ShmemDevice * devices;
              int rc = posix_memalign((void **) & devices, 16, sizeof(*devices) * n);
              PAMI_assertf(rc == 0, "posix_memalign failed for ShmemDevice[%zu], errno=%d\n", n, errno);

              // Instantiate the shared memory devices
              for (i = 0; i < n; ++i)
                {
                  new (&devices[i]) ShmemDevice (i, total_fifos_on_node, all_fifos, peer_fnum);
                }

              TRACE_ERR((stderr, "<< ShmemDevice::Factory::generate_impl()\n"));
              return devices;
            };

            static inline pami_result_t init_impl (ShmemDevice    * devices,
                                                  size_t           clientid,
                                                  size_t           contextid,
                                                  pami_client_t     client,
                                                  pami_context_t    context,
                                                  Memory::MemoryManager *mm,
                                                  PAMI::Device::Generic::Device * progress)
            {
              return getDevice_impl(devices, clientid, contextid).init (clientid, contextid, client, context, mm, progress);
            };

            static inline size_t advance_impl (ShmemDevice * devices,
                                               size_t        clientid,
                                               size_t        contextid)
            {
              return getDevice_impl(devices, clientid, contextid).advance ();
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

            Memregion _origin_mr;
            Memregion _target_mr;
            size_t    _origin_offset;
            size_t    _target_offset;
            size_t    _bytes;
        };

        inline ShmemDevice (size_t contextid, size_t nfifos, T_Fifo * fifo, size_t * fnum_hash) :
            Interface::BaseDevice< ShmemDevice<T_Fifo,T_Shaddr> > (),
            Interface::PacketDevice< ShmemDevice<T_Fifo,T_Shaddr> > (),
            _fifo (fifo),
            _total_fifos (nfifos),
            _fnum_hash (fnum_hash),
            _contextid (contextid),
#ifdef EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
            __ndQ (),
            __ndpkt (),
#endif
            //__ueQ (),
            __sendQ (NULL),
            _progress (NULL),
            shaddr ()
        {
          TRACE_ERR((stderr, "ShmemDevice() constructor\n"));

          // Get the peer id for this task
          size_t me = 0;
          PAMI::Interface::Mapping::nodeaddr_t address;
          __global.mapping.nodeAddr (address);
          __global.mapping.node2peer (address, me);

          // Cache fifo pointer the the reception fifo for this {peer,contextid}
          size_t f = fnum (me, contextid);
          _rfifo = & _fifo[f];
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

        // ------------------------------------------

        /// \see PAMI::Device::Interface::PacketDevice::read()
        inline int read_impl (void * buf, size_t length, void * cookie);

        static const size_t metadata_size = T_Fifo::packet_header_size - sizeof(uint16_t);
        static const size_t payload_size  = T_Fifo::packet_payload_size;

        // ------------------------------------------

        static const bool shaddr_va_supported    = T_Shaddr::shaddr_va_supported;
        static const bool shaddr_mr_supported    = T_Shaddr::shaddr_mr_supported;

        static const bool shaddr_read_supported  = T_Shaddr::shaddr_read_supported;
        static const bool shaddr_write_supported = T_Shaddr::shaddr_write_supported;

        // ------------------------------------------

        static const uint16_t system_ro_put_dispatch = (DISPATCH_SET_COUNT * DISPATCH_SET_SIZE) - 1;

        ///
        /// \brief Register the receive function to dispatch when a packet arrives.
        ///
        /// \param[in]  set            Dispatch set identifier
        /// \param[in]  recv_func      Receive function to dispatch
        /// \param[in]  recv_func_parm Receive function client data
        /// \param[out] id             Dispatch id for this registration
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
        /// \param[in] recv_func      Receive function to dispatch
        /// \param[in] recv_func_parm Receive function client data
        /// \param[in] id             Dispatch id for this registration
        ///
        /// \return registration result
        ///
        pami_result_t registerSystemRecvFunction (Interface::RecvFunction_t   recv_func,
                                                  void                      * recv_func_parm,
                                                  uint16_t                    id);

        ///
        /// \brief Write a single packet into the injection fifo.
        ///
        /// \param[in]  fnum        Injection fifo identifier
        /// \param[in]  dispatch_id Packet dispatch identifier
        /// \param[in]  metadata    Packet header metadata source buffer
        /// \param[in]  metasize    Packet header metadata size in bytes
        /// \param[in]  iov         Iovec array to pack into the packet payload
        /// \param[out] sequence    Packet sequence number
        ///
        template <unsigned T_Niov>
        inline pami_result_t writeSinglePacket (size_t         fnum,
                                               uint16_t       dispatch_id,
                                               void         * metadata,
                                               size_t         metasize,
                                               struct iovec   (&iov)[T_Niov],
                                               size_t       & sequence);

        ///
        /// \brief Write a single packet into the injection fifo.
        ///
        /// \param[in]  fnum        Injection fifo identifier
        /// \param[in]  dispatch_id Packet dispatch identifier
        /// \param[in]  metadata    Packet header metadata source buffer
        /// \param[in]  metasize    Packet header metadata size in bytes
        /// \param[in]  iov         Iovec array to pack into the packet payload
        /// \param[in]  niov        Number of iovec array elements
        /// \param[out] sequence    Packet sequence number
        ///
        inline pami_result_t writeSinglePacket (size_t           fnum,
                                               uint16_t         dispatch_id,
                                               void           * metadata,
                                               size_t           metasize,
                                               struct iovec   * iov,
                                               size_t           niov,
                                               size_t         & sequence);

        inline pami_result_t writeSinglePacket (size_t           fnum,
                                               uint16_t         dispatch_id,
                                               void           * metadata,
                                               size_t           metasize,
                                               void           * payload,
                                               size_t           length,
                                               size_t         & sequence);

        inline pami_result_t writeSinglePacket (size_t           fnum,
                                               uint16_t         dispatch_id,
                                               void           * metadata,
                                               size_t           metasize,
                                               void           * payload,
                                               size_t         & sequence);

        pami_result_t post (size_t ififo, Shmem::SendQueue::Message * msg);

        pami_result_t post (PAMI::Device::Generic::GenericThread * work);

        //pami_result_t post (size_t ififo, Shmem::DoneQueue::Message * msg);

        ///
        /// \brief Check if the send queue to an injection fifo is empty
        ///
        ///
        inline bool isSendQueueEmpty (size_t fnum);

        inline Shmem::SendQueue *getQS (size_t fnum);

        inline pami_result_t init (size_t clientid, size_t contextid, pami_client_t client, pami_context_t context, Memory::MemoryManager *mm, PAMI::Device::Generic::Device * progress);

        inline size_t advance ();

        inline size_t fnum (size_t peer, size_t offset);
        
        inline size_t lastRecSequenceId (size_t fnum);

        inline size_t lastInjSequenceId (size_t fnum);

        inline bool activePackets (size_t fnum);

        ///
        /// \see PAMI::Device::Interface::RecvFunction_t
        ///
        static int unexpected (void   * metadata,
                               void   * payload,
                               size_t   bytes,
                               void   * recv_func_parm,
                               void   * cookie);

        static int system_shaddr_read (void   * metadata,
                                       void   * payload,
                                       size_t   bytes,
                                       void   * recv_func_parm,
                                       void   * cookie);

        static int system_shaddr_write (void   * metadata,
                                        void   * payload,
                                        size_t   bytes,
                                        void   * recv_func_parm,
                                        void   * cookie);


        T_Fifo * _fifo;         //< Injection fifo array for all node contexts
        size_t _total_fifos;    //< Injection fifo array size
        size_t * _fnum_hash;    //< Fifo number lookup table
        T_Fifo  * _rfifo;       //< Pointer to fifo to use as the reception fifo
        size_t * _last_inj_sequence_id;

        Memory::MemoryManager *_mm;
        pami_client_t       _client;
        pami_context_t      _context;
        size_t             _contextid;

        dispatch_t  _dispatch[DISPATCH_SET_COUNT*DISPATCH_SET_SIZE];

#ifdef EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
        Queue                                            __ndQ;
        MemoryAllocator < sizeof(UnexpectedPacket), 16 > __ndpkt;
#endif

        CircularQueue        __ueQ[DISPATCH_SET_COUNT];
        Shmem::SendQueue   * __sendQ;
        PAMI::Device::Generic::Device * _progress;
        PAMI::Device::Generic::Device * _local_progress_device;

        T_Shaddr          shaddr;

        size_t            _num_procs;
        size_t            _global_task;
        size_t            _local_task;

        // Read state variables. See readData_impl()
        char     * _current_pkt_data_ptr;
        unsigned   _current_pkt_iov_bytes_read;
        unsigned   _current_pkt_iov;
    };

    template <class T_Fifo, class T_Shaddr>
    inline size_t ShmemDevice<T_Fifo,T_Shaddr>::getLocalRank()
    {
      return _local_task;
    }

    template <class T_Fifo, class T_Shaddr>
    inline pami_context_t ShmemDevice<T_Fifo,T_Shaddr>::getContext_impl()
    {
      return _context;
    }

    template <class T_Fifo, class T_Shaddr>
    inline size_t ShmemDevice<T_Fifo,T_Shaddr>::getContextId_impl()
    {
      return _contextid;
    }
    template <class T_Fifo, class T_Shaddr>
    inline size_t ShmemDevice<T_Fifo,T_Shaddr>::getContextOffset_impl()
    {
      return getContextId_impl();
    }

    ///
    /// \brief Check if the send queue to a local rank is empty
    ///
    /// \see fnum
    /// \param[in] fnum  Local injection fifo number
    ///
    template <class T_Fifo, class T_Shaddr>
    inline bool ShmemDevice<T_Fifo,T_Shaddr>::isSendQueueEmpty (size_t fnum)
    {
      return (__sendQ[fnum].size() == 0);
    }

    template <class T_Fifo, class T_Shaddr>
    inline Shmem::SendQueue * ShmemDevice<T_Fifo,T_Shaddr>::getQS (size_t fnum)
    {
      return &__sendQ[fnum];
    }

    /// \see PAMI::Device::Interface::PacketDevice::read()
    template <class T_Fifo, class T_Shaddr>
    int ShmemDevice<T_Fifo,T_Shaddr>::read_impl (void * dst, size_t length, void * cookie)
    {
      memcpy (dst, cookie, length);
      return 0;
    }

    template <class T_Fifo, class T_Shaddr>
    template <unsigned T_Niov>
    pami_result_t ShmemDevice<T_Fifo,T_Shaddr>::writeSinglePacket (
      size_t         fnum,
      uint16_t       dispatch_id,
      void         * metadata,
      size_t         metasize,
      struct iovec   (&iov)[T_Niov],
      size_t       & sequence)
    {
      TRACE_ERR((stderr, ">> ShmemDevice<>::writeSinglePacket () .. T_Niov = %d\n", T_Niov));

#ifdef EMULATE_UNRELIABLE_SHMEM_DEVICE
      unsigned long long t = __global.time.timebase ();

      if (t % EMULATE_UNRELIABLE_SHMEM_DEVICE_FREQUENCY == 0) return PAMI_SUCCESS;

#endif

      size_t pktid;
      TRACE_ERR((stderr, "ShmemDevice<>::writeSinglePacket () .. before nextInjPacket(), fnum = %zu\n", fnum));
      PacketImpl * pkt = (PacketImpl *) _fifo[fnum].nextInjPacket (pktid);
      TRACE_ERR((stderr, "ShmemDevice<>::writeSinglePacket () .. pkt = %p, pktid = %zu\n", pkt, pktid));

      if (pkt != NULL)
        {
          TRACE_ERR((stderr, "ShmemDevice<>::writeSinglePacket () .. before write(), metadata = %p, metasize = %zu\n", metadata, metasize));
          //if (likely(metadata!=NULL))
          //printf("metasize:%d T_Niov:%d\n", metasize, T_Niov);
          pkt->writeMetadata ((uint8_t *) metadata, metasize);
          pkt->writeDispatch (dispatch_id);
          pkt->writePayload (iov);

          // "produce" the packet into the fifo.
          TRACE_ERR((stderr, "ShmemDevice<>::writeSinglePacket () .. before producePacket()\n"));
          _fifo[fnum].producePacket (pktid);
          _last_inj_sequence_id[fnum] = pktid;
          sequence = pktid;

          TRACE_ERR((stderr, "<< ShmemDevice<>::writeSinglePacket () .. PAMI_SUCCESS\n"));
          return PAMI_SUCCESS;
        }

      TRACE_ERR((stderr, "<< ShmemDevice<>::writeSinglePacket () .. PAMI_EAGAIN\n"));
      return PAMI_EAGAIN;
    };

    template <class T_Fifo, class T_Shaddr>
    pami_result_t ShmemDevice<T_Fifo,T_Shaddr>::writeSinglePacket (
      size_t         fnum,
      uint16_t       dispatch_id,
      void         * metadata,
      size_t         metasize,
      struct iovec * iov,
      size_t         niov,
      size_t       & sequence)
    {
      TRACE_ERR((stderr, "(%zu) 2.ShmemDevice::writeSinglePacket (%zu, %d, %p, %p, %zu) >>\n", __global.mapping.task(), fnum, dispatch_id, metadata, iov, niov));

#ifdef EMULATE_UNRELIABLE_SHMEM_DEVICE
      unsigned long long t = __global.time.timebase ();

      if (t % EMULATE_UNRELIABLE_SHMEM_DEVICE_FREQUENCY == 0) return PAMI_SUCCESS;

#endif

      size_t pktid;
      PacketImpl * pkt = (PacketImpl *) _fifo[fnum].nextInjPacket (pktid);

      if (pkt != NULL)
        {
          //if (likely(metadata!=NULL))
          pkt->writeMetadata ((uint8_t *)metadata, metasize);
          pkt->writeDispatch (dispatch_id);
          pkt->writePayload (iov, niov);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket (pktid);
          _last_inj_sequence_id[fnum] = pktid;
          sequence = pktid;

          TRACE_ERR((stderr, "(%zu) 2.ShmemDevice::writeSinglePacket (%zu, %d, %p, %p, %zu) << CM_SUCCESS\n", __global.mapping.task(), fnum, dispatch_id, metadata, iov, niov));
          return PAMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zu) 2.ShmemDevice::writeSinglePacket (%zu, %d, %p, %p, %zu) << CM_EAGAIN\n", __global.mapping.task(), fnum, dispatch_id, metadata, iov, niov));
      return PAMI_EAGAIN;
    };

    template <class T_Fifo, class T_Shaddr>
    pami_result_t ShmemDevice<T_Fifo,T_Shaddr>::writeSinglePacket (
      size_t         fnum,
      uint16_t       dispatch_id,
      void         * metadata,
      size_t         metasize,
      void         * payload,
      size_t         length,
      size_t       & sequence)
    {
      TRACE_ERR((stderr, "(%zu) 3.ShmemDevice::writeSinglePacket (%zu, %d, %p, %p, %zu) >>\n", __global.mapping.task(), fnum, dispatch_id, metadata, payload, length));

#ifdef EMULATE_UNRELIABLE_SHMEM_DEVICE
      unsigned long long t = __global.time.timebase ();

      if (t % EMULATE_UNRELIABLE_SHMEM_DEVICE_FREQUENCY == 0) return PAMI_SUCCESS;

#endif

      size_t pktid;
      PacketImpl * pkt = (PacketImpl *) _fifo[fnum].nextInjPacket (pktid);

      if (pkt != NULL)
        {
          //if (likely(metadata!=NULL))
          pkt->writeMetadata ((uint8_t *) metadata, metasize);
          pkt->writeDispatch (dispatch_id);
          pkt->writePayload (payload, length);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket (pktid);
          _last_inj_sequence_id[fnum] = pktid;
          sequence = pktid;

          TRACE_ERR((stderr, "(%zu) 3.ShmemDevice::writeSinglePacket (%zu, %d, %p, %p, %zu) << CM_SUCCESS\n", __global.mapping.task(), fnum, dispatch_id, metadata, payload, length));
          return PAMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zu) 3.ShmemDevice::writeSinglePacket (%zu, %d, %p, %p, %zu) << CM_EAGAIN\n", __global.mapping.task(), fnum, dispatch_id, metadata, payload, length));
      return PAMI_EAGAIN;
    };

    template <class T_Fifo, class T_Shaddr>
    pami_result_t ShmemDevice<T_Fifo,T_Shaddr>::writeSinglePacket (
      size_t         fnum,
      uint16_t       dispatch_id,
      void         * metadata,
      size_t         metasize,
      void         * payload,
      size_t       & sequence)
    {
      TRACE_ERR((stderr, "(%zu) 4.ShmemDevice::writeSinglePacket (%zu, %d, %p, %p) >>\n", __global.mapping.task(), fnum, dispatch_id, metadata, payload));

#ifdef EMULATE_UNRELIABLE_SHMEM_DEVICE
      unsigned long long t = __global.time.timebase ();

      if (t % EMULATE_UNRELIABLE_SHMEM_DEVICE_FREQUENCY == 0) return PAMI_SUCCESS;

#endif

      size_t pktid;
      PacketImpl * pkt = (PacketImpl *) _fifo[fnum].nextInjPacket (pktid);

      if (pkt != NULL)
        {
          //if (likely(metadata!=NULL))
          pkt->writeMetadata ((uint8_t *) metadata, metasize);
          pkt->writeDispatch (dispatch_id);
          pkt->writePayload (payload);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket (pktid);
          _last_inj_sequence_id[fnum] = pktid;
          sequence = pktid;

          TRACE_ERR((stderr, "(%zu) 4.ShmemDevice::writeSinglePacket (%zu, %d, %p, %p) << PAMI_SUCCESS\n", __global.mapping.task(), fnum, dispatch_id, metadata, payload));
          return PAMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zu) 4.ShmemDevice::writeSinglePacket (%zu, %d, %p, %p) << PAMI_EAGAIN\n", __global.mapping.task(), fnum, dispatch_id, metadata, payload));
      return PAMI_EAGAIN;
    };


    template <class T_Fifo, class T_Shaddr>
    size_t ShmemDevice<T_Fifo,T_Shaddr>::advance ()
    {
#ifdef TRAP_ADVANCE_DEADLOCK
      static size_t iteration = 0;
      TRACE_ERR((stderr, "(%zu) ShmemDevice::advance() iteration %zu \n", __global.mapping.task(), iteration));
      PAMI_assert (iteration++ < ADVANCE_DEADLOCK_MAX_LOOP);
#endif

      size_t events = 0;
      //TRACE_ERR((stderr, "(%zu) ShmemDevice::advance() >>\n", __global.mapping.task()));

      // Advance any pending receive messages.
      PacketImpl * pkt = NULL;
      uint16_t id;

      //TRACE_ERR((stderr, "(%zu) ShmemDevice::advance()    ... before _rfifo->nextRecPacket(), _rfifo = %p\n", __global.mapping.task(), _rfifo));

      while ((pkt = (PacketImpl *)_rfifo->nextRecPacket()) != NULL)
        {
          TRACE_ERR((stderr, "(%zu) ShmemDevice::advance()    ... before pkt->getHeader(), pkt = %p\n", __global.mapping.task(), pkt));
          //mem_sync ();
          mem_isync ();

#ifdef EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
          UnexpectedPacket * uepkt = (UnexpectedPacket *) __ndpkt.allocateObject();
          new (uepkt) UnexpectedPacket (pkt);
          unsigned long long t = __global.time.timebase ();
          size_t position = t % __ndQ.size();

          if (__ndQ.size() == 0)
            __ndQ.pushHead ((QueueElem *) uepkt);
          else
            __ndQ.insertElem ((QueueElem *) uepkt, position);

#else
          id = pkt->getDispatch ();
          void * meta = (void *) pkt->getMetadata ();
          void * data = pkt->getPayload ();
          TRACE_ERR((stderr, "(%zu) ShmemDevice::advance()    ... before  dispatch .. _dispatch[%d].function = %p, _dispatch[%d].clientdata = %p\n", __global.mapping.task(), id, _dispatch[id].function, id, _dispatch[id].clientdata));
          _dispatch[id].function (meta, data, T_Fifo::packet_payload_size, _dispatch[id].clientdata, data);

          // Complete this message/packet and increment the fifo head.
          TRACE_ERR((stderr, "(%zu) ShmemDevice::advance()    ... before _rfifo->consumePacket()\n", __global.mapping.task()));
          _rfifo->consumePacket ();
          TRACE_ERR((stderr, "(%zu) ShmemDevice::advance()    ...  after _rfifo->consumePacket()\n", __global.mapping.task()));
          events++;
#endif
        }

#ifdef EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
      // Advance any "nondeterministic" packets. This is not done on every
      // advance to allow the nondeterministic queue to grow so that the
      // packet insertion will randomize the packet order when the queue is
      // advanced.
      unsigned long long t = __global.time.timebase ();

      if (t % EMULATE_NONDETERMINISTIC_SHMEM_DEVICE_FREQUENCY == 0 && !__ndQ.isEmpty())
        //if (!__ndQ.isEmpty())
        {
          UnexpectedPacket * uepkt = NULL;

          while ((uepkt = (UnexpectedPacket *) __ndQ.popHead()) != NULL)
            {
              TRACE_ERRi((stderr, "(%zu) ShmemDevice::advance()    ...         dequeue nd packet, __ndQ.size() = %3d -> %3d, uepkt->sequence = %zu\n", __global.mapping.task(), __ndQ.size() + 1, __ndQ.size(), uepkt->sequence));
              _dispatch[uepkt->id].function (uepkt->meta,
                                             uepkt->data,
                                             T_Fifo::packet_payload_size,
                                             _dispatch[uepkt->id].clientdata,
                                             uepkt->data);
              __ndpkt.returnObject ((void *) uepkt);
            }
        }

#endif

      //TRACE_ERR((stderr, "(%zu) ShmemDevice::advance()    ...  after _rfifo->nextRecPacket()\n", __global.mapping.task()));
      //TRACE_ERR((stderr, "(%zu) ShmemDevice::advance() << ... events = %zu\n", __global.mapping.task(), events));

#ifdef TRAP_ADVANCE_DEADLOCK

      if (events) iteration = 0;

#endif
      return events;
    }

    template <class T_Fifo, class T_Shaddr>
    inline size_t ShmemDevice<T_Fifo,T_Shaddr>::fnum (size_t peer, size_t offset)
    {
      TRACE_ERR((stderr, ">> ShmemDevice::fnum(%zu, %zu), _fnum_hash = %p\n", peer, offset, _fnum_hash));
      TRACE_ERR((stderr, "<< ShmemDevice::fnum(%zu, %zu), _fnum_hash[%zu] = %zu\n", peer, offset, peer, _fnum_hash[peer]));
      return _fnum_hash[peer] + offset;
    }
    
    template <class T_Fifo, class T_Shaddr>
    inline size_t ShmemDevice<T_Fifo,T_Shaddr>::lastRecSequenceId (size_t fnum)
    {
      return _fifo[fnum].lastRecSequenceId ();
    }
    
    template <class T_Fifo, class T_Shaddr>
    inline size_t ShmemDevice<T_Fifo,T_Shaddr>::lastInjSequenceId (size_t fnum)
    {
      return _last_inj_sequence_id[fnum];
    }
    
    template <class T_Fifo, class T_Shaddr>
    inline bool ShmemDevice<T_Fifo,T_Shaddr>::activePackets (size_t fnum)
    {
      return (lastRecSequenceId (fnum) < lastInjSequenceId (fnum));
    }
  };
};
#undef TRACE_ERR

// Include the non-inline method definitions
#include "components/devices/shmem/ShmemDevice_impl.h"

#undef DISPATCH_SET_COUNT
#undef DISPATCH_SET_SIZE

#endif // __components_devices_shmem_shmembasedevice_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
