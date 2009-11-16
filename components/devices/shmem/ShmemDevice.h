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
      protected:
        class PacketImpl : public T_Packet
        {
          public:
            inline  PacketImpl () {};
            inline ~PacketImpl () {};

            ///
            /// \brief Write metadata and iovec data to the packet
            ///
            /// This method allows for template specialization via the T_Niov
            /// template parameter.
            ///
            /// \param[in] dispatch  Packet dispatch identifier
            /// \param[in] metadata  Pointer to packet metadata source
            /// \param[in] iov       Array of T_Niov source data iovec elements
            ///
            template <unsigned T_Niov>
            inline void write (uint16_t       dispatch,
                               void         * metadata,
                               struct iovec   (&iov)[T_Niov])
            {
              // First, bulk copy the metadata into the packet header
              if(likely(metadata!=NULL))
                this->writeHeader (metadata);

              // Next, copy the packet dispatch id into the header
              uint16_t * hdr = (uint16_t *) this->getHeader ();
              hdr[(T_Packet::headerSize_impl>>1)-1] = dispatch;

              // Finally, copy the packet payload data from the iovec
              unsigned i, j, n;
              if (T_Niov == 1)
              {
                // Constant-expression template specialization.
                n = (iov[0].iov_len >> 2) + (iov[0].iov_len & 0x03 != 0);
                uint32_t * dst = (uint32_t *) this->getPayload ();
                uint32_t * src = (uint32_t *) iov[0].iov_base;
                for (i=0; i<n; i++) dst[i] = src[i];

                return;
              }

              if (T_Niov == 2)
              {
                // Constant-expression template specialization.
                uint32_t * dst = (uint32_t *) this->getPayload ();
                uint32_t * src = (uint32_t *) iov[0].iov_base;
                n = (iov[0].iov_len >> 2) + (iov[0].iov_len & 0x03 != 0);
                for (i=0; i<n; i++) dst[i] = src[i];

                dst = (uint32_t *)((uint8_t *) dst + iov[0].iov_len);
                src = (uint32_t *) iov[1].iov_base;
                n = (iov[1].iov_len >> 2) + (iov[1].iov_len & 0x03 != 0);
                for (i=0; i<n; i++) dst[i] = src[i];

                return;
              }

              uint8_t  * payload = (uint8_t *) this->getPayload ();
              uint32_t * dst;
              uint32_t * src;
              for (i=0; i<T_Niov; i++)
              {
                dst = (uint32_t *) payload;
                src = (uint32_t *) iov[i].iov_base;
                n = (iov[i].iov_len >> 2) + (iov[i].iov_len & 0x03 != 0);
                for (j=0; j<n; j++) dst[j] = src[j];
                payload += iov[i].iov_len;
              }
            };

            ///
            /// \brief Write metadata and iovec data to the packet
            ///
            /// \param[in] dispatch  Packet dispatch identifier
            /// \param[in] metadata  Pointer to packet metadata source
            /// \param[in] iov       Array of source data iovec elements
            /// \param[in] niov      Number of iovec array elements
            ///
            inline void write (uint16_t       dispatch,
                               void         * metadata,
                               struct iovec * iov,
                               size_t         niov)
            {
              // First, bulk copy the metadata into the packet header
              if(likely(metadata!=NULL))
                this->writeHeader (metadata);

              // Next, copy the packet dispatch id into the header
              uint16_t * hdr = (uint16_t *) this->getHeader ();
              hdr[(T_Packet::headerSize_impl>>1)-1] = dispatch;

              // Finally, copy the packet payload data from the iovec
              unsigned i, j, n;
              uint8_t  * payload = (uint8_t *) this->getPayload ();
              uint32_t * dst;
              uint32_t * src;
              for (i=0; i<niov; i++)
              {
                dst = (uint32_t *) payload;
                src = (uint32_t *) iov[i].iov_base;
                n = (iov[i].iov_len >> 2) + (iov[i].iov_len & 0x03 != 0);
                for (j=0; j<n; j++) dst[j] = src[j];
                payload += iov[i].iov_len;
              }
            }

            ///
            /// \brief Retrieve the packet dispatch identifier and metadata pointer
            /// \param[out] dispatch  Packet dispatch identifier
            /// \return Pointer to packet metadata
            ///
            inline void * metadata (uint16_t & dispatch)
            {
              uint16_t * hdr = (uint16_t *) this->getHeader ();
              dispatch = hdr[(T_Packet::headerSize_impl>>1)-1];
              //fprintf(stderr, "PacketImpl::metadata(), dispatch = %d, (%d>>1)-1 = %d\n", dispatch, T_Packet::headerSize_impl, (T_Packet::headerSize_impl>>1)-1);

              return hdr;
            };
        };

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
      PacketImpl * pkt = (PacketImpl *) _fifo[fnum].nextInjPacket ();

      if (pkt != NULL)
        {
          pkt->write (dispatch_id, metadata, iov);

          sequence = _fifo[fnum].getPacketSequenceId ((T_Packet *)pkt);

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

      PacketImpl * pkt = (PacketImpl *) _fifo[fnum].nextInjPacket ();

      if (pkt != NULL)
        {
          pkt->write (dispatch_id, metadata, iov, niov);

          sequence = _fifo[fnum].getPacketSequenceId (pkt);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket ((T_Packet *)pkt);

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

      PacketImpl * pkt = (PacketImpl *) _fifo[fnum].nextInjPacket ();

      if (pkt != NULL)
        {
          struct iovec iov[1];
          iov[0].iov_base = msg->next (iov[0].iov_len, T_Packet::payloadSize_impl);
          pkt->write (msg->getDispatchId (), msg->getMetadata(), iov);

          sequence = _fifo[fnum].getPacketSequenceId (pkt);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket ((T_Packet *)pkt);

          TRACE_ERR((stderr, "(%zd) ShmemDevice::writeSinglePacket (%zd, %p) << XMI_SUCCESS\n", __global.mapping.task(), fnum, msg));
          return XMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zd) ShmemDevice::writeSinglePacket (%zd, %p) << XMI_EAGAIN\n", __global.mapping.task(), fnum, msg));
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
      PacketImpl * pkt = NULL;
      uint16_t id;

      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl()    ... before _rfifo->nextRecPacket()\n", __global.mapping.task()));

      while ((pkt = (PacketImpl *)_rfifo->nextRecPacket()) != NULL)
        {
          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_impl()    ... before pkt->getHeader()\n", __global.mapping.task()));
          void * meta = (void *) pkt->metadata (id);
          void * data = pkt->getPayload ();
          _dispatch[id].function (meta, data, T_Packet::payloadSize_impl, _dispatch[id].clientdata, data);

mem_sync (); // TODO -- is this needed?

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
