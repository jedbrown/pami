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
#include "components/devices/PacketModel.h"
#include "components/devices/shmem/ShmemMessage.h"

#include "util/fifo/LinearFifo.h"
#include "util/fifo/FifoPacket.h"
#include "util/queue/Queue.h"

//#define TRAP_ADVANCE_DEADLOCK
#define ADVANCE_DEADLOCK_MAX_LOOP 10000

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
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

    template < class T_Fifo, class T_Packet >
    class ShmemDevice : public Interface::BaseDevice<ShmemDevice<T_Fifo, T_Packet>, XMI::SysDep>,
        public Interface::PacketDevice<ShmemDevice<T_Fifo, T_Packet> >
    {
      public:
        inline ShmemDevice () :
            Interface::BaseDevice<ShmemDevice<T_Fifo, T_Packet>, XMI::SysDep> (),
            Interface::PacketDevice<ShmemDevice<T_Fifo, T_Packet> > (),
            _fifo (NULL),
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

        static const size_t metadata_size = T_Packet::headerSize_impl - sizeof(uint16_t);
        static const size_t payload_size  = T_Packet::payloadSize_impl;

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
        /// This method is a \b template \b specialization for the case where
        /// a single iovec element describes the packet payload, a.k.a. a
        /// contiguous source buffer.
        ///
        /// \param[in]  fnum        Injection fifo identifier
        /// \param[in]  dispatch_id Packet dispatch identifier
        /// \param[in]  metadata    Packet header metadata source buffer
        /// \param[in]  metasize    Packet header metadata size in bytes
        /// \param[in]  iov         Single element iovec array to copy into
        ///                         the packet payload
        /// \param[out] sequence    Packet sequence number
        ///
        inline xmi_result_t writeSinglePacket (size_t         fnum,
                                               uint16_t       dispatch_id,
                                               void         * metadata,
                                               size_t         metasize,
                                               struct iovec   (&iov)[1],
                                               size_t       & sequence);

        ///
        /// \brief Write a single packet into the injection fifo.
        ///
        /// This method is a \b template \b specialization for the case where
        /// two iovec elements describe the packet payload.
        ///
        /// \param[in]  fnum        Injection fifo identifier
        /// \param[in]  dispatch_id Packet dispatch identifier
        /// \param[in]  metadata    Packet header metadata source buffer
        /// \param[in]  metasize    Packet header metadata size in bytes
        /// \param[in]  iov         Two element iovec array to pack into
        ///                         the packet payload
        /// \param[out] sequence    Packet sequence number
        ///
        inline xmi_result_t writeSinglePacket (size_t         fnum,
                                               uint16_t       dispatch_id,
                                               void         * metadata,
                                               size_t         metasize,
                                               struct iovec   (&iov)[2],
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

        inline xmi_result_t writeSinglePacket (size_t                   ififo,
                                               ShmemMessage<T_Packet> * msg,
                                               size_t                 & sequence);

        xmi_result_t post (size_t ififo, ShmemMessage<T_Packet> * msg);

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

        dispatch_t  _dispatch[256*256];

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

    template <class T_Fifo, class T_Packet>
    inline size_t ShmemDevice<T_Fifo, T_Packet>::getLocalRank()
    {
      return _local_task;
    }

    ///
    /// \brief Check if the send queue to a local rank is empty
    ///
    /// \param[in] peer  \b Local rank
    ///
    template <class T_Fifo, class T_Packet>
    inline bool ShmemDevice<T_Fifo, T_Packet>::isSendQueueEmpty (size_t peer)
    {
      return ((__sendQMask >> peer) & 0x01) == 0;
    }

    /// \see XMI::Device::Interface::PacketDevice::read()
    template <class T_Fifo, class T_Packet>
    int ShmemDevice<T_Fifo, T_Packet>::read_impl (void * dst, size_t length, void * cookie)
    {
      memcpy (dst, cookie, length);
      return 0;
    }

    template <class T_Fifo, class T_Packet>
    template <unsigned T_Niov>
    xmi_result_t ShmemDevice<T_Fifo, T_Packet>::writeSinglePacket (
        size_t         fnum,
        uint16_t       dispatch_id,
        void         * metadata,
        size_t         metasize,
        struct iovec   (&iov)[T_Niov],
        size_t       & sequence)
    {
      TRACE_ERR((stderr, "ShmemDevice<>::writeSinglePacket () .. T_Niov = %d\n", T_Niov));
      T_Packet * pkt = _fifo[fnum].nextInjPacket ();

      if (pkt != NULL)
        {
          uint16_t * hdr = (uint16_t *) pkt->getHeader ();

          // First 2 bytes is the dispatch id.
          hdr[0] = dispatch_id;

          // Remaining header bytes are metadata.
          TRACE_ERR((stderr, "(%zd) ShmemDevice::writeSinglePacket () .. metadata = %p\n", __global.mapping.task(), metadata));
          memcpy ((void *) &hdr[1], metadata, metasize);

          // Write the packet payload data
          uint8_t * data = (uint8_t *) pkt->getPayload ();
          unsigned i;
          for (i=0; i<T_Niov; i++)
          {
            memcpy ((void *) data, iov[i].iov_base, iov[i].iov_len);
            data += iov[i].iov_len;
          }

          sequence = _fifo[fnum].getPacketSequenceId (pkt);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket (pkt);

          return XMI_SUCCESS;
        }

      return XMI_EAGAIN;
    };

    template <class T_Fifo, class T_Packet>
    xmi_result_t ShmemDevice<T_Fifo, T_Packet>::writeSinglePacket (
        size_t         fnum,
        uint16_t       dispatch_id,
        void         * metadata,
        size_t         metasize,
        struct iovec   (&iov)[1],
        size_t       & sequence)
    {
      TRACE_ERR((stderr, "ShmemDevice<>::writeSinglePacket () .. template specialization, niov = 1\n"));
      T_Packet * pkt = _fifo[fnum].nextInjPacket ();

      if (pkt != NULL)
        {
          uint16_t * hdr = (uint16_t *) pkt->getHeader ();

          // First 2 bytes is the dispatch id.
          hdr[0] = dispatch_id;

          // Remaining header bytes are metadata.
          TRACE_ERR((stderr, "(%zd) ShmemDevice::writeSinglePacket () .. metadata = %p\n", __global.mapping.task(), metadata));
          memcpy ((void *) &hdr[1], metadata, metasize);

          // Write the packet payload data
          uint8_t * data = (uint8_t *) pkt->getPayload ();
          memcpy (data, iov[0].iov_base, iov[0].iov_len);

          sequence = _fifo[fnum].getPacketSequenceId (pkt);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket (pkt);

          return XMI_SUCCESS;
        }

      return XMI_EAGAIN;
    };

    template <class T_Fifo, class T_Packet>
    xmi_result_t ShmemDevice<T_Fifo, T_Packet>::writeSinglePacket (
        size_t         fnum,
        uint16_t       dispatch_id,
        void         * metadata,
        size_t         metasize,
        struct iovec   (&iov)[2],
        size_t       & sequence)
    {
      TRACE_ERR((stderr, "ShmemDevice<>::writeSinglePacket () .. template specialization, niov = 2\n"));
      T_Packet * pkt = _fifo[fnum].nextInjPacket ();

      if (pkt != NULL)
        {
          uint16_t * hdr = (uint16_t *) pkt->getHeader ();

          // First 2 bytes is the dispatch id.
          hdr[0] = dispatch_id;

          // Remaining header bytes are metadata.
          TRACE_ERR((stderr, "(%zd) ShmemDevice::writeSinglePacket () .. metadata = %p\n", __global.mapping.task(), metadata));
          memcpy ((void *) &hdr[1], metadata, metasize);

          // Write the packet payload data
          uint8_t * data = (uint8_t *) pkt->getPayload ();
          memcpy (data, iov[0].iov_base, iov[0].iov_len);
          memcpy (data+iov[0].iov_len, iov[1].iov_base, iov[1].iov_len);

          sequence = _fifo[fnum].getPacketSequenceId (pkt);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket (pkt);

          return XMI_SUCCESS;
        }

      return XMI_EAGAIN;
    };

    template <class T_Fifo, class T_Packet>
    xmi_result_t ShmemDevice<T_Fifo, T_Packet>::writeSinglePacket (
        size_t         fnum,
        uint16_t       dispatch_id,
        void         * metadata,
        size_t         metasize,
        struct iovec * iov,
        size_t         niov,
        size_t       & sequence)
    {
      TRACE_ERR((stderr, "(%zd) ShmemDevice::writeSinglePacket (%zd, %zd, %p, %p, %zd) >>\n", __global.mapping.task(), fnum, dispatch_id, metadata, iov, niov));

      T_Packet * pkt = _fifo[fnum].nextInjPacket ();

      if (pkt != NULL)
        {
          uint16_t * hdr = (uint16_t *) pkt->getHeader ();

          // First 2 bytes is the dispatch id.
          hdr[0] = dispatch_id;

          // Remaining header bytes are metadata.
          memcpy ((void *) &hdr[1], metadata, metasize);

          // Write the packet payload data
          uint8_t * data = (uint8_t *) pkt->getPayload ();
          unsigned i, n = 0;

          for (i = 0; i < niov; i++)
            {
              memcpy ((void *)&data[n], iov[i].iov_base, iov[i].iov_len);
              n += iov[i].iov_len;
            }

          sequence = _fifo[fnum].getPacketSequenceId (pkt);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket (pkt);

          TRACE_ERR((stderr, "(%zd) ShmemDevice::writeSinglePacket (%zd, %zd, %p, %p, %zd) << CM_SUCCESS\n", __global.mapping.task(), fnum, dispatch_id, metadata, iov, niov));
          return XMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zd) ShmemDevice::writeSinglePacket (%zd, %zd, %p, %p, %zd) << CM_EAGAIN\n", __global.mapping.task(), fnum, dispatch_id, metadata, iov, niov));
      return XMI_EAGAIN;
    };

    template <class T_Fifo, class T_Packet>
    xmi_result_t ShmemDevice<T_Fifo, T_Packet>::writeSinglePacket (
        size_t                   fnum,
        ShmemMessage<T_Packet> * msg,
        size_t                 & sequence)
    {
      TRACE_ERR((stderr, "(%zd) ShmemDevice::writeSinglePacket (%zd, %p) >>\n", __global.mapping.task(), fnum, msg));

      T_Packet * pkt = _fifo[fnum].nextInjPacket ();

      if (pkt != NULL)
        {
          uint16_t * hdr = (uint16_t *) pkt->getHeader ();

          // First 2 bytes is the dispatch id.
          hdr[0] = msg->getDispatchId ();

          // Remaining header bytes are metadata.
          memcpy (&hdr[1], msg->getMetadata(), T_Packet::headerSize_impl - sizeof(uint16_t));

          // Write the packet payload data
          uint8_t * data = (uint8_t *) pkt->getPayload ();
          size_t n = 0;
          size_t max = pkt->payloadSize();

          void * payload;
          size_t bytes;

          do
            {
              msg->next (&payload, bytes, max);
              memcpy ((void *)(data + n), payload, bytes);
              n += bytes;
            }
          while (!msg->done() && (n < max));

          sequence = _fifo[fnum].getPacketSequenceId (pkt);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket (pkt);

          TRACE_ERR((stderr, "(%zd) ShmemDevice::writeSinglePacket (%zd, %p) << CM_SUCCESS\n", __global.mapping.task(), fnum, msg));
          return XMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zd) ShmemDevice::writeSinglePacket (%zd, %p) << CM_EAGAIN\n", __global.mapping.task(), fnum, msg));
      return XMI_EAGAIN;
    };

    template <class T_Fifo, class T_Packet>
    int ShmemDevice<T_Fifo, T_Packet>::advance_impl ()
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
      T_Packet * pkt = NULL;

      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl()    ... before _rfifo->nextRecPacket()\n", __global.mapping.task()));

      while ((pkt = _rfifo->nextRecPacket()) != NULL)
        {
          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl()    ... before pkt->getHeader()\n", __global.mapping.task()));
          uint8_t * hdr = (uint8_t *) pkt->getHeader ();

          uint16_t id = *((uint16_t *) & hdr[0]);
          void * meta = (void *) & hdr[2];
          void * data = pkt->getPayload ();
          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl()    ... hdr = %p, hdr[0] = 0x%0x, hdr[1] = 0x%0x, id = %d, meta = %p, data = %p\n", __global.mapping.task(), hdr, hdr[0], hdr[1], id, meta, data));

          mem_sync (); // TODO -- is this needed?

          _dispatch[id].function (meta, data, pkt->payloadSize(), _dispatch[id].clientdata, data);

          // Complete this message/packet and increment the fifo head.
          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl()    ... before _rfifo->consumePacket()\n", __global.mapping.task()));
          _rfifo->consumePacket (pkt);
          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl()    ...  after _rfifo->consumePacket()\n", __global.mapping.task()));
          events++;
        }

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
    template <class T_Fifo, class T_Packet>
    inline void ShmemDevice<T_Fifo, T_Packet>::pushSendQueueTail (size_t peer, QueueElem * element)
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
    template <class T_Fifo, class T_Packet>
    inline QueueElem * ShmemDevice<T_Fifo, T_Packet>::popSendQueueHead (size_t peer)
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

#endif // __components_devices_shmem_shmembasedevice_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
