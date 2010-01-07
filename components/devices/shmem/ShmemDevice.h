/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
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
#include "components/devices/PacketInterface.h"
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
    class ShmemDevice : public Interface::BaseDevice<ShmemDevice<T_Fifo>, XMI::SysDep>,
        public Interface::PacketDevice<ShmemDevice<T_Fifo> >
    {
      protected:
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
        inline ShmemDevice () :
            Interface::BaseDevice<ShmemDevice<T_Fifo>, XMI::SysDep> (),
            Interface::PacketDevice<ShmemDevice<T_Fifo> > (),
            _fifo (NULL),
#ifdef EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
            __ndQ (),
            __ndpkt (),
#endif
            __sendQ (),
            __sendQMask (0)
        {
          TRACE_ERR((stderr, "ShmemDevice() constructor\n"));
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

        inline xmi_result_t writeSinglePacket (size_t        ififo,
                                               ShmemMessage * msg,
                                               size_t       & sequence);

        xmi_result_t post (size_t ififo, ShmemMessage * msg);

        ///
        /// \brief Check if the send queue to a local rank is empty
        ///
        ///
        inline bool isSendQueueEmpty (size_t peer);

        inline int init_impl (XMI::SysDep * sysdep);

        inline int advance_impl ();

        inline void pushSendQueueTail (size_t peer, QueueElem * element);

        inline QueueElem * popSendQueueHead (size_t peer);

        ///
        /// \brief Advance the send queues and process any pending messages.
        ///
        /// \note This method is intentionally \b not inlined as pending
        ///       send messages is assumed to be the uncommon case. By
        ///       moving this method to a function call the performance of
        ///       the "normal" device advance is improved.
        ///
        /// \see advance_impl
        ///
        int advance_sendQ ();

        int advance_sendQ (size_t peer);

        ///
        /// \see XMI::Device::Interface::RecvFunction_t
        ///
        static int noop (void   * metadata,
                         void   * payload,
                         size_t   bytes,
                         void   * recv_func_parm,
                         void   * cookie);


        T_Fifo * _fifo;  ///< Array of injection fifos
        T_Fifo * _rfifo; ///< Pointer to fifo to use as a reception fifo

        XMI::SysDep      * _sysdep;

        dispatch_t  _dispatch[DISPATCH_SET_COUNT*DISPATCH_SET_SIZE];

#ifdef EMULATE_NONDETERMINISTIC_SHMEM_DEVICE
        Queue                                            __ndQ;
        MemoryAllocator < sizeof(UnexpectedPacket), 16 > __ndpkt;
#endif

        Queue * __sendQ;
        unsigned          __sendQMask;

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

    ///
    /// \brief Check if the send queue to a local rank is empty
    ///
    /// \param[in] peer  \b Local rank
    ///
    template <class T_Fifo>
    inline bool ShmemDevice<T_Fifo>::isSendQueueEmpty (size_t peer)
    {
      TRACE_ERR((stderr, "   ShmemDevice<>::isSendQueueEmpty () .. ((__sendQMask >> peer) & 0x01) = %d, __sendQMask = 0x%08x, peer = %zu\n", ((__sendQMask >> peer) & 0x01), __sendQMask, peer));
      return ((__sendQMask >> peer) & 0x01) == 0;
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
      TRACE_ERR((stderr, "(%zd) 2.ShmemDevice::writeSinglePacket (%zd, %zd, %p, %p, %zd) >>\n", __global.mapping.task(), fnum, dispatch_id, metadata, iov, niov));

#ifdef EMULATE_UNRELIABLE_SHMEM_DEVICE
      unsigned long long t = __global.time.timebase ();

      if (t % EMULATE_UNRELIABLE_SHMEM_DEVICE_FREQUENCY == 0) return XMI_SUCCESS;

#endif

      size_t pktid;
      PacketImpl * pkt = (PacketImpl *) _fifo[fnum].nextInjPacket (pktid);

      if (pkt != NULL)
        {
          //if (likely(metadata!=NULL))
            pkt->writeMetadata (metadata, metasize);
          pkt->writeDispatch (dispatch_id);
          pkt->writePayload (iov);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket (pktid);

          TRACE_ERR((stderr, "(%zd) 2.ShmemDevice::writeSinglePacket (%zd, %zd, %p, %p, %zd) << CM_SUCCESS\n", __global.mapping.task(), fnum, dispatch_id, metadata, iov, niov));
          return XMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zd) 2.ShmemDevice::writeSinglePacket (%zd, %zd, %p, %p, %zd) << CM_EAGAIN\n", __global.mapping.task(), fnum, dispatch_id, metadata, iov, niov));
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
      TRACE_ERR((stderr, "(%zd) 3.ShmemDevice::writeSinglePacket (%zd, %zd, %p, %p, %zd) >>\n", __global.mapping.task(), fnum, dispatch_id, metadata, payload, length));

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

          TRACE_ERR((stderr, "(%zd) 3.ShmemDevice::writeSinglePacket (%zd, %zd, %p, %p, %zd) << CM_SUCCESS\n", __global.mapping.task(), fnum, dispatch_id, metadata, payload, length));
          return XMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zd) 3.ShmemDevice::writeSinglePacket (%zd, %zd, %p, %p, %zd) << CM_EAGAIN\n", __global.mapping.task(), fnum, dispatch_id, metadata, payload, length));
      return XMI_EAGAIN;
    };

    template <class T_Fifo>
    xmi_result_t ShmemDevice<T_Fifo>::writeSinglePacket (
      size_t                   fnum,
      ShmemMessage * msg,
      size_t                 & sequence)
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

    template <class T_Fifo>
    int ShmemDevice<T_Fifo>::advance_impl ()
    {
#ifdef TRAP_ADVANCE_DEADLOCK
      static size_t iteration = 0;
      XMI_assert (iteration++ < ADVANCE_DEADLOCK_MAX_LOOP);
#endif

      int events = 0;
      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl() >> ... __sendQMask = 0x%0x\n", __global.mapping.task(), __sendQMask));

      // Advance any pending send messages.
      if (__sendQMask != 0) events += advance_sendQ ();

      // Advance any pending receive messages.
      PacketImpl * pkt = NULL;
      uint16_t id;

      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl()    ... before _rfifo->nextRecPacket(), _rfifo = %p\n", __global.mapping.task(), _rfifo));

      while ((pkt = (PacketImpl *)_rfifo->nextRecPacket()) != NULL)
        {
          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl()    ... before pkt->getHeader()\n", __global.mapping.task()));
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
          _dispatch[id].function (meta, data, T_Fifo::packet_payload_size, _dispatch[id].clientdata, data);

          // Complete this message/packet and increment the fifo head.
          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl()    ... before _rfifo->consumePacket()\n", __global.mapping.task()));
          _rfifo->consumePacket ();
          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl()    ...  after _rfifo->consumePacket()\n", __global.mapping.task()));
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
              TRACE_ERR)(stderr, "(%zd) ShmemDevice::advance_impl()    ...         dequeue nd packet, __ndQ.size() = %3d -> %3d, uepkt->sequence = %zd\n", __global.mapping.task(), __ndQ.size() + 1, __ndQ.size(), uepkt->sequence));
              _dispatch[uepkt->id].function (uepkt->meta,
                                             uepkt->data,
                                             T_Fifo::packet_payload_size,
                                             _dispatch[uepkt->id].clientdata,
                                             uepkt->data);
              __ndpkt.returnObject ((void *) uepkt);
            }
        }

#endif

      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl()    ...  after _rfifo->nextRecPacket()\n", __global.mapping.task()));
      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl() << ... events = %d\n", __global.mapping.task(), events));

#ifdef TRAP_ADVANCE_DEADLOCK

      if (events) iteration = 0;

#endif
      return events;
    }

    ///
    /// \brief Check if the send queue to a local rank is empty
    ///
    /// \param[in] peer  \b Local rank
    ///
    template <class T_Fifo>
    inline void ShmemDevice<T_Fifo>::pushSendQueueTail (size_t peer, QueueElem * element)
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
    inline QueueElem * ShmemDevice<T_Fifo>::popSendQueueHead (size_t peer)
    {
      TRACE_ERR((stderr, "popping out from the sendQ\n"));
      QueueElem * tmp = __sendQ[peer].popHead();
      __sendQMask = __sendQMask & ~(__sendQ[peer].isEmpty() << peer);
      return tmp;
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
