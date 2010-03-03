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

#include "sys/xmi.h"

#include "SysDep.h"
#include "Arch.h"
#include "Memregion.h"

#include "components/atomic/Counter.h"
#include "components/devices/BaseDevice.h"
#include "components/devices/FactoryInterface.h"
#include "components/devices/PacketInterface.h"
#include "components/devices/generic/Device.h"
#include "components/devices/shmem/ShmemMessage.h"
#include "components/memory/MemoryAllocator.h"
#include "util/fifo/LinearFifo.h"
#include "util/fifo/FifoPacket.h"
#include "util/queue/Queue.h"

//#define TRAP_ADVANCE_DEADLOCK
#define ADVANCE_DEADLOCK_MAX_LOOP 10000

//#define EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
#define EMULATE_NONDETERMINISTIC_SHMEM_DEVICE_FREQUENCY 4

//#define EMULATE_UNRELIABLE_SHMEM_DEVICE
#define EMULATE_UNRELIABLE_SHMEM_DEVICE_FREQUENCY 10


#define DISPATCH_SET_COUNT 256
#define DISPATCH_SET_SIZE   16


#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace XMI
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

    template < class T_Fifo >
    class ShmemDevice : public Interface::BaseDevice< ShmemDevice<T_Fifo> >,
        public Interface::PacketDevice<ShmemDevice<T_Fifo> >
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
              for (i=0; i<bytes; i++) hdr[i] = metadata[i];
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
//fprintf (stderr, "PacketImpl::writePayload (iov[%zd])\n", T_Niov);
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

//fprintf (stderr, "PacketImpl::writePayload (iov[%zd]), 1st, n = %zd, dst = %p, src = %p\n", T_Niov, n, dst, src);
                  for (i = 0; i < n; i++) dst[i] = src[i];

                  dst = (uint32_t *)((uint8_t *) dst + iov[0].iov_len);
                  src = (uint32_t *) iov[1].iov_base;
                  n = (iov[1].iov_len >> 2) + ((iov[1].iov_len & 0x03) != 0);
		  //printf("iov[1] size:%d\n", iov[1].iov_len);

//fprintf (stderr, "PacketImpl::writePayload (iov[%zd]), 2nd, n = %zd, dst = %p, src = %p\n", T_Niov, n, dst, src);
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
            /// This method allows for template specialization via the T_Niov
            /// template parameter.
            ///
            /// \param[in] payload       Address of the buffer to write to the packet payload
            /// \param[in] length        Number of bytes to write
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

#ifdef EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
        class UnexpectedPacket : public QueueElem
        {
          public:
            inline UnexpectedPacket (PacketImpl * packet, size_t s) :
                QueueElem (),
                sequence (s)
            {
              memcpy ((void *) meta, packet->metadata (id), T_Fifo::packet_header_size);
              memcpy ((void *) data, packet->getPayload (), T_Fifo::packet_payload_size);
            };

            uint16_t id;
            size_t   sequence;
            uint8_t  meta[T_Fifo::packet_header_size];
            uint8_t  data[T_Fifo::packet_payload_size];
        };
#endif

      public:

	typedef XMI::Device::Generic::MultiSendQSubDevice<ShmemThread,1,true> MessageQueue;

        // Inner factory class
        class Factory : public Interface::FactoryInterface<Factory,ShmemDevice,XMI::Device::Generic::Device>
        {
          public:
            static inline ShmemDevice * generate_impl (size_t clientid, size_t n, Memory::MemoryManager & mm)
            {
              size_t i;
              TRACE_ERR((stderr, ">> ShmemDevice::Factory::generate_impl() n = %zu\n", n));

              // Determine the number of contexts on the entire node, then
              // allocate shared memory for all contexts.
              
              // Get the number of peer tasks on the node
              size_t npeers = 0;
              __global.mapping.nodePeers (npeers);
              TRACE_ERR((stderr, "   ShmemDevice::Factory::generate_impl() npeers = %zu\n", npeers));              

              // Allocate a "context count" array and a peer fifo pointer
              // array from shared memory
              size_t * ncontexts = NULL;
              size_t size = sizeof(size_t) * 2 * npeers;
              TRACE_ERR((stderr, "   ShmemDevice::Factory::generate_impl() size = %zu\n", size));
              mm.memalign ((void **)&ncontexts, 16, size);
              TRACE_ERR((stderr, "   ShmemDevice::Factory::generate_impl() ncontexts = %p\n", ncontexts));

              size_t * peer_fnum = ncontexts + npeers;
              
              // Get the peer id for this task
              size_t me = 0;
              XMI::Interface::Mapping::nodeaddr_t address;

              __global.mapping.nodeAddr (address);
              TRACE_ERR((stderr, "   ShmemDevice::Factory::generate_impl() after nodeAddr()\n"));
              __global.mapping.node2peer (address, me);
              TRACE_ERR((stderr, "   ShmemDevice::Factory::generate_impl() me = %zu\n", me));
          
              // Set the number of contexts in this peer
              ncontexts[me] = n;
              TRACE_ERR((stderr, "   ShmemDevice::Factory::generate_impl() ncontexts = %p, ncontexts[%zu] = %zu\n", ncontexts, me, ncontexts[me]));
              
              // Determine the total number of contexts on the node and the
              // number of contexts in each peer
              size_t done = 0;
              size_t total_fifos_on_node = 0;
              while (done != npeers)
              {
                // check to see if all peers have written a non-zero value
                // in the "ncontexts" field
                total_fifos_on_node = 0;
                done = 0;
                for (i=0; i<npeers; i++)
                {
                  total_fifos_on_node += ncontexts[i];
                  if (ncontexts[i] > 0) done++;
                }
              }

              // Allocate a shared memory segment for _all_ of the fifos for
              // _all_ of the contexts
              T_Fifo * all_fifos = NULL;
              size = ((sizeof(T_Fifo) + 15) & 0xfff0) * total_fifos_on_node;
              mm.memalign ((void **)&all_fifos, 16, size);

              // Assign fifo indexes to the peer fnum cache
              peer_fnum[0] = 0;
              for (i=1; i<npeers; i++)
              {
                peer_fnum[i] = peer_fnum[i-1] + ncontexts[i-1];
              }

              // Allocate an array of shared memory devices, one for each
              // context in this _task_ (from heap, not from shared memory)
              ShmemDevice * devices;
              int rc = posix_memalign((void **)&devices, 16, sizeof(*devices) * n);
              XMI_assertf(rc == 0, "posix_memalign failed for ShmemDevice[%zu], errno=%d\n", n, errno);

              // Instantiate the shared memory devices
              for (i = 0; i < n; ++i)
              {
                new (&devices[i]) ShmemDevice (i, total_fifos_on_node, all_fifos, peer_fnum);
              }

              TRACE_ERR((stderr, "<< ShmemDevice::Factory::generate_impl()\n"));
              return devices;
            };

            static inline xmi_result_t init_impl (ShmemDevice    * devices,
                                                  size_t           clientid,
                                                  size_t           contextid,
                                                  xmi_client_t     client,
                                                  xmi_context_t    context,
                                                  SysDep         * sysdep,
                                                  XMI::Device::Generic::Device * progress)
            {
              return getDevice_impl(devices, clientid, contextid).init (clientid, contextid, client, context, sysdep, progress);
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

        inline ShmemDevice (size_t contextid, size_t nfifos, T_Fifo * fifo, size_t * fnum_hash) :
            Interface::BaseDevice< ShmemDevice<T_Fifo> > (),
            Interface::PacketDevice< ShmemDevice<T_Fifo> > (),
            _fifo (fifo),
            _total_fifos (nfifos),
            _fnum_hash (fnum_hash),
            _contextid (contextid),
#ifdef EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
            __ndQ (),
            __ndpkt (),
#endif
            __sendQ (NULL),
            _progress (NULL)
//            __sendQMask (0)
        {
          TRACE_ERR((stderr, "ShmemDevice() constructor\n"));

          // Get the peer id for this task
          size_t me = 0;
          XMI::Interface::Mapping::nodeaddr_t address;
          __global.mapping.nodeAddr (address);
          __global.mapping.node2peer (address, me);

          // Cache fifo pointer the the reception fifo for this {peer,contextid}
          size_t f = fnum (me, contextid);
          _rfifo = & _fifo[f];
        };

        inline ~ShmemDevice () {};

        inline size_t getLocalRank ();

        // ------------------------------------------

        /// \see XMI::Device::Interface::BaseDevice::isInit()
        bool isInit_impl ();

        /// \see XMI::Device::Interface::BaseDevice::peers()
        inline size_t peers_impl ();

        /// \see XMI::Device::Interface::BaseDevice::task2peer()
        inline size_t task2peer_impl (size_t task);

        inline xmi_context_t getContext_impl ();

        inline size_t getContextId_impl ();
        inline size_t getContextOffset_impl ();

        // ------------------------------------------

        /// \see XMI::Device::Interface::PacketDevice::read()
        inline int read_impl (void * buf, size_t length, void * cookie);

        static const size_t metadata_size = T_Fifo::packet_header_size - sizeof(uint16_t);
        static const size_t payload_size  = T_Fifo::packet_payload_size;

        // ------------------------------------------

        ///
        /// \brief Regieter the receive function to dispatch when a packet arrives.
        ///
        /// \param[in] dispatch        Dispatch set identifier
        /// \param[in] recv_func       Receive function to dispatch
        /// \param[in] recv_func_parm  Receive function client data
        ///
        /// \return Dispatch id for this registration
        ///
        xmi_result_t registerRecvFunction (size_t                      set,
                                           Interface::RecvFunction_t   recv_func,
                                           void                      * recv_func_parm,
                                           uint16_t                  & id);

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
        inline xmi_result_t writeSinglePacket (size_t         fnum,
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
        inline xmi_result_t writeSinglePacket (size_t           fnum,
                                               uint16_t         dispatch_id,
                                               void           * metadata,
                                               size_t           metasize,
                                               struct iovec   * iov,
                                               size_t           niov,
                                               size_t         & sequence);

        inline xmi_result_t writeSinglePacket (size_t           fnum,
                                               uint16_t         dispatch_id,
                                               void           * metadata,
                                               size_t           metasize,
                                               void           * payload,
                                               size_t           length,
                                               size_t         & sequence);
#if 0
        inline xmi_result_t writeSinglePacket (size_t        ififo,
                                               ShmemMessage * msg,
                                               size_t       & sequence);
#endif
	template <class T_Message>
        xmi_result_t post (size_t ififo, T_Message * msg);

        ///
        /// \brief Check if the send queue to an injection fifo is empty
        ///
        ///
        inline bool isSendQueueEmpty (size_t fnum);
        inline XMI::Device::Generic::GenericSubDevice *getQS (size_t fnum);

        inline xmi_result_t init (size_t clientid, size_t contextid, xmi_client_t client, xmi_context_t context, SysDep * sysdep, XMI::Device::Generic::Device * progress);

        inline size_t advance ();

//        inline void pushSendQueueTail (size_t peer, XMI::Queue::Element * element);

//        inline XMI::Queue::Element * popSendQueueHead (size_t peer);

        inline size_t fnum (size_t peer, size_t offset);

        ///
        /// \see XMI::Device::Interface::RecvFunction_t
        ///
        static int noop (void   * metadata,
                         void   * payload,
                         size_t   bytes,
                         void   * recv_func_parm,
                         void   * cookie);


        T_Fifo * _fifo;         //< Injection fifo array for all node contexts
        size_t _total_fifos;    //< Injection fifo array size
        size_t * _fnum_hash;    //< Fifo number lookup table
        T_Fifo  * _rfifo;       //< Pointer to fifo to use as the reception fifo

        XMI::SysDep      * _sysdep;
        xmi_client_t       _client;
        xmi_context_t      _context;
        size_t             _contextid;

        dispatch_t  _dispatch[DISPATCH_SET_COUNT*DISPATCH_SET_SIZE];

#ifdef EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
        Queue                                            __ndQ;
        MemoryAllocator < sizeof(UnexpectedPacket), 16 > __ndpkt;
#endif

        MessageQueue   * __sendQ;
        XMI::Device::Generic::Device * _progress;
//        unsigned          __sendQMask;

        size_t            _num_procs;
        size_t            _global_task;
        size_t            _local_task;

        // Read state variables. See readData_impl()
        char     * _current_pkt_data_ptr;
        unsigned   _current_pkt_iov_bytes_read;
        unsigned   _current_pkt_iov;
    };

    template <class T_Fifo>
    inline size_t ShmemDevice<T_Fifo>::getLocalRank()
    {
      return _local_task;
    }

    template <class T_Fifo>
    inline xmi_context_t ShmemDevice<T_Fifo>::getContext_impl()
    {
      return _context;
    }

    template <class T_Fifo>
    inline size_t ShmemDevice<T_Fifo>::getContextId_impl()
    {
      return _contextid;
    }
    template <class T_Fifo>
    inline size_t ShmemDevice<T_Fifo>::getContextOffset_impl()
    {
	return getContextId_impl();
    }

    ///
    /// \brief Check if the send queue to a local rank is empty
    ///
    /// \see fnum
    /// \param[in] fnum  Local injection fifo number
    ///
    template <class T_Fifo>
    inline bool ShmemDevice<T_Fifo>::isSendQueueEmpty (size_t fnum)
    {
      return (__sendQ[fnum].size() == 0);
    }
    template <class T_Fifo>
    inline XMI::Device::Generic::GenericSubDevice * ShmemDevice<T_Fifo>::getQS (size_t fnum)
    {
      return __sendQ[fnum].getQS();
    }

    /// \see XMI::Device::Interface::PacketDevice::read()
    template <class T_Fifo>
    int ShmemDevice<T_Fifo>::read_impl (void * dst, size_t length, void * cookie)
    {
      memcpy (dst, cookie, length);
      return 0;
    }

    template <class T_Fifo>
    template <unsigned T_Niov>
    xmi_result_t ShmemDevice<T_Fifo>::writeSinglePacket (
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

      if (t % EMULATE_UNRELIABLE_SHMEM_DEVICE_FREQUENCY == 0) return XMI_SUCCESS;
#endif

      size_t pktid;
      TRACE_ERR((stderr, "   ShmemDevice<>::writeSinglePacket () .. before nextInjPacket(), fnum = %zu\n", fnum));
      PacketImpl * pkt = (PacketImpl *) _fifo[fnum].nextInjPacket (pktid);
      TRACE_ERR((stderr, "   ShmemDevice<>::writeSinglePacket () .. pkt = %p, pktid = %zd\n", pkt, pktid));

      if (pkt != NULL)
        {
          TRACE_ERR((stderr, "   ShmemDevice<>::writeSinglePacket () .. before write(), metadata = %p, metasize = %zd\n", metadata, metasize));
          //if (likely(metadata!=NULL))
		//printf("metasize:%d T_Niov:%d\n", metasize, T_Niov);
            pkt->writeMetadata ((uint8_t *) metadata, metasize);
          pkt->writeDispatch (dispatch_id);
          pkt->writePayload (iov);

          // "produce" the packet into the fifo.
          TRACE_ERR((stderr, "   ShmemDevice<>::writeSinglePacket () .. before producePacket()\n"));
          _fifo[fnum].producePacket (pktid);

          TRACE_ERR((stderr, "<< ShmemDevice<>::writeSinglePacket () .. XMI_SUCCESS\n"));
          return XMI_SUCCESS;
        }

      TRACE_ERR((stderr, "<< ShmemDevice<>::writeSinglePacket () .. XMI_EAGAIN\n"));
      return XMI_EAGAIN;
    };

    template <class T_Fifo>
    xmi_result_t ShmemDevice<T_Fifo>::writeSinglePacket (
      size_t         fnum,
      uint16_t       dispatch_id,
      void         * metadata,
      size_t         metasize,
      struct iovec * iov,
      size_t         niov,
      size_t       & sequence)
    {
      TRACE_ERR((stderr, "(%zd) 2.ShmemDevice::writeSinglePacket (%zd, %d, %p, %p, %zd) >>\n", __global.mapping.task(), fnum, dispatch_id, metadata, iov, niov));

#ifdef EMULATE_UNRELIABLE_SHMEM_DEVICE
      unsigned long long t = __global.time.timebase ();

      if (t % EMULATE_UNRELIABLE_SHMEM_DEVICE_FREQUENCY == 0) return XMI_SUCCESS;

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

          TRACE_ERR((stderr, "(%zd) 2.ShmemDevice::writeSinglePacket (%zd, %d, %p, %p, %zd) << CM_SUCCESS\n", __global.mapping.task(), fnum, dispatch_id, metadata, iov, niov));
          return XMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zd) 2.ShmemDevice::writeSinglePacket (%zd, %d, %p, %p, %zd) << CM_EAGAIN\n", __global.mapping.task(), fnum, dispatch_id, metadata, iov, niov));
      return XMI_EAGAIN;
    };

    template <class T_Fifo>
    xmi_result_t ShmemDevice<T_Fifo>::writeSinglePacket (
      size_t         fnum,
      uint16_t       dispatch_id,
      void         * metadata,
      size_t         metasize,
      void         * payload,
      size_t         length,
      size_t       & sequence)
    {
      TRACE_ERR((stderr, "(%zd) 3.ShmemDevice::writeSinglePacket (%zd, %d, %p, %p, %zd) >>\n", __global.mapping.task(), fnum, dispatch_id, metadata, payload, length));

#ifdef EMULATE_UNRELIABLE_SHMEM_DEVICE
      unsigned long long t = __global.time.timebase ();

      if (t % EMULATE_UNRELIABLE_SHMEM_DEVICE_FREQUENCY == 0) return XMI_SUCCESS;

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

          TRACE_ERR((stderr, "(%zd) 3.ShmemDevice::writeSinglePacket (%zd, %d, %p, %p, %zd) << CM_SUCCESS\n", __global.mapping.task(), fnum, dispatch_id, metadata, payload, length));
          return XMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zd) 3.ShmemDevice::writeSinglePacket (%zd, %d, %p, %p, %zd) << CM_EAGAIN\n", __global.mapping.task(), fnum, dispatch_id, metadata, payload, length));
      return XMI_EAGAIN;
    };
#if 0
    template <class T_Fifo, class T_Progress>
    xmi_result_t ShmemDevice<T_Fifo,T_Progress>::writeSinglePacket (
      size_t         fnum,
      ShmemMessage * msg,
      size_t       & sequence)
    {
      TRACE_ERR((stderr, "(%zd) 4.ShmemDevice::writeSinglePacket (%zd, %p) >>\n", __global.mapping.task(), fnum, msg));

      size_t pktid;
      PacketImpl * pkt = (PacketImpl *) _fifo[fnum].nextInjPacket (pktid);

      if (pkt != NULL)
        {
          struct iovec iov[1];
          iov[0].iov_base = msg->next (iov[0].iov_len, T_Fifo::packet_payload_size);
          //if (likely(metadata!=NULL))
            pkt->writeMetadata ((uint8_t *) msg->getMetadata(), metadata_size);
          pkt->writeDispatch (msg->getDispatchId());
          pkt->writePayload (iov);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket (pktid);

          TRACE_ERR((stderr, "(%zd) 4.ShmemDevice::writeSinglePacket (%zd, %p) << XMI_SUCCESS\n", __global.mapping.task(), fnum, msg));
          return XMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zd) 4.ShmemDevice::writeSinglePacket (%zd, %p) << XMI_EAGAIN\n", __global.mapping.task(), fnum, msg));
      return XMI_EAGAIN;
    };
#endif
    template <class T_Fifo>
    size_t ShmemDevice<T_Fifo>::advance ()
    {
#ifdef TRAP_ADVANCE_DEADLOCK
      static size_t iteration = 0;
      XMI_assert (iteration++ < ADVANCE_DEADLOCK_MAX_LOOP);
#endif

      size_t events = 0;
      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance() >>\n", __global.mapping.task()));

      // Advance any pending receive messages.
      PacketImpl * pkt = NULL;
      uint16_t id;

      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance()    ... before _rfifo->nextRecPacket(), _rfifo = %p\n", __global.mapping.task(), _rfifo));

      while ((pkt = (PacketImpl *)_rfifo->nextRecPacket()) != NULL)
        {
          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance()    ... before pkt->getHeader(), pkt = %p\n", __global.mapping.task(), pkt));
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
          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance()    ... before  dispatch .. _dispatch[%d].function = %p, _dispatch[%d].clientdata = %p\n", __global.mapping.task(), id, _dispatch[id].function, id, _dispatch[id].clientdata));
          _dispatch[id].function (meta, data, T_Fifo::packet_payload_size, _dispatch[id].clientdata, data);

          // Complete this message/packet and increment the fifo head.
          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance()    ... before _rfifo->consumePacket()\n", __global.mapping.task()));
          _rfifo->consumePacket ();
          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance()    ...  after _rfifo->consumePacket()\n", __global.mapping.task()));
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
              TRACE_ERRi((stderr, "(%zd) ShmemDevice::advance()    ...         dequeue nd packet, __ndQ.size() = %3d -> %3d, uepkt->sequence = %zd\n", __global.mapping.task(), __ndQ.size() + 1, __ndQ.size(), uepkt->sequence));
              _dispatch[uepkt->id].function (uepkt->meta,
                                             uepkt->data,
                                             T_Fifo::packet_payload_size,
                                             _dispatch[uepkt->id].clientdata,
                                             uepkt->data);
              __ndpkt.returnObject ((void *) uepkt);
            }
        }

#endif

      //TRACE_ERR((stderr, "(%zd) ShmemDevice::advance()    ...  after _rfifo->nextRecPacket()\n", __global.mapping.task()));
      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance() << ... events = %zu\n", __global.mapping.task(), events));

#ifdef TRAP_ADVANCE_DEADLOCK

      if (events) iteration = 0;

#endif
      return events;
    }
#if 0
    ///
    /// \brief Check if the send queue to a local rank is empty
    ///
    /// \param[in] peer  \b Local rank
    ///
    template <class T_Fifo>
    inline void ShmemDevice<T_Fifo>::pushSendQueueTail (size_t peer, XMI::Queue::Element * element)
    {
      TRACE_ERR ((stderr, "(%zd) pushSendQueueTail(%zd, %p), __sendQMask = %d -> %d\n", __global.mapping.task(), peer, element, __sendQMask, __sendQMask | (1 << peer)));
      __sendQ[peer].pushTail (element);
      __sendQMask |= (1 << peer);
    }

    ///
    /// \brief Check if the send queue to a local rank is empty
    ///
    /// \param[in] peer  \b Local rank
    ///
    template <class T_Fifo>
    inline XMI::Queue::Element * ShmemDevice<T_Fifo>::popSendQueueHead (size_t peer)
    {
      TRACE_ERR((stderr, "popping out from the sendQ\n"));
      XMI::Queue::Element * tmp = __sendQ[peer].popHead();
      __sendQMask = __sendQMask & ~(__sendQ[peer].isEmpty() << peer);
      return tmp;
    }
#endif
    template <class T_Fifo>
    inline size_t ShmemDevice<T_Fifo>::fnum (size_t peer, size_t offset)
    {
      TRACE_ERR((stderr,">> ShmemDevice::fnum(%zu, %zu), _fnum_hash = %p\n", peer, offset, _fnum_hash));
      TRACE_ERR((stderr,"<< ShmemDevice::fnum(%zu, %zu), _fnum_hash[%zu] = %zu\n", peer, offset, peer, _fnum_hash[peer]));
      return _fnum_hash[peer] + offset;
    }
  };
};
#undef TRACE_ERR

// Include the non-inline method definitions
#include "ShmemDevice_impl.h"

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
