/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemBaseDevice.h
 * \brief ???
 */

#ifndef __components_devices_shmem_shmembasedevice_h__
#define __components_devices_shmem_shmembasedevice_h__

#include <sys/uio.h>

#include "sys/xmi.h"

#include "components/sysdep/SysDep.h"
#include "util/queue/Queue.h"

#include "components/devices/PacketDevice.h"
#include "ShmemSysDep.h"
#include "ShmemBaseMessage.h"
#include "util/fifo/LinearFifo.h"
#include "util/fifo/FifoPacket.h"

#include "components/atomic/Counter.h"

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

    template <class T_SysDep, class T_Fifo, class T_Packet>
    class ShmemBaseDevice
    {
      public:
        inline ShmemBaseDevice () :
            _fifo (NULL),
            //_dispatch_count (0),
            __sendQ (),
            __sendQMask (0),
            __doneQ (),
            __doneQMask (0)
        {
          TRACE_ERR((stderr, "ShmemBaseDevice() constructor\n"));
        };

        inline ~ShmemBaseDevice () {};

        inline size_t getLocalRank ();

        // ------------------------------------------

        /// \see XMI::Device::Interface::BaseDevice::isInit()
        bool isInit_impl ();

        /// \see XMI::Device::Interface::BaseDevice::isReliableNetwork()
        static const size_t reliable_network = true;

        /// \see XMI::Device::Interface::BaseDevice::peers()
        inline size_t peers_impl ();

        /// \see XMI::Device::Interface::BaseDevice::task2peer()
        inline size_t task2peer_impl (size_t task);

        // ------------------------------------------

        /// \see XMI::Device::Interface::BaseDevice::readData()
        inline int readData_impl (int channel, void * buf, size_t length);

        /// \see XMI::Device::Interface::PacketDevice::getPacketMetadataSize()
        static const size_t packet_metadata_size = T_Packet::headerSize_impl - sizeof(uint16_t);

        /// \see XMI::Device::Interface::PacketDevice::getPacketPayloadSize()
        static const size_t packet_payload_size = T_Packet::payloadSize_impl;


        // ------------------------------------------

        /// \see XMI::Device::Interface::MessageDevice::getMessageMetadataSize()
        static const size_t message_metadata_size = T_Packet::headerSize_impl - sizeof(uint16_t);

        /// \see XMI::Device::Interface::MessageDevice::setConnection()
        inline void setConnection_impl (size_t   fromRank,
                                        void   * arg);

        /// \see XMI::Device::Interface::MessageDevice::getConnection()
        inline void * getConnection_impl (size_t fromRank);

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
        int registerRecvFunction (size_t                      dispatch,
                                  Interface::RecvFunction_t   recv_func,
                                  void                      * recv_func_parm);

        inline xmi_result_t writeSinglePacket (size_t     fnum,
                                               uint16_t   dispatch_id,
                                               void     * metadata,
                                               size_t     metasize,
                                               void     * payload,
                                               size_t     bytes,
                                               size_t   & sequence);

        inline xmi_result_t writeSinglePacket (size_t     fnum,
                                               uint16_t   dispatch_id,
                                               void     * metadata,
                                               size_t     metasize,
                                               void     * payload0,
                                               size_t     bytes0,
                                               void     * payload1,
                                               size_t     bytes1,
                                               size_t   & sequence);

        inline xmi_result_t writeSinglePacket (size_t           fnum,
                                               uint16_t         dispatch_id,
                                               void           * metadata,
                                               size_t           metasize,
                                               struct iovec   * iov,
                                               size_t           niov,
                                               size_t & sequence);

        inline xmi_result_t writeSinglePacket (size_t                       ififo,
                                               ShmemBaseMessage<T_Packet> * msg,
                                               size_t                     & sequence);

        xmi_result_t post (size_t ififo, ShmemBaseMessage<T_Packet> * msg);

        ///
        /// \brief Check if the send queue to a local rank is empty
        ///
        /// \param[in] peer  \b Local rank
        ///
        inline bool isSendQueueEmpty (size_t peer);

      //protected:

        int init_internal (T_SysDep * sysdep);

        inline int advance_internal ();

        inline void pushSendQueueTail (size_t peer, QueueElem * element);

        inline QueueElem * popSendQueueHead (size_t peer);

        inline void pushDoneQueueTail (size_t peer, QueueElem * element);

        inline QueueElem * popDoneQueueHead (size_t peer);

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
        void advance_sendQ ();

        void advance_sendQ (size_t peer);

        void advance_doneQ ();

        void advance_doneQ (size_t peer);

        ///
        /// \see XMI::Device::Interface::RecvFunction_t
        ///
        static int noop (void   * metadata,
                         void   * payload,
                         size_t   bytes,
                         void   * recv_func_parm);


        T_Fifo * _fifo;  ///< Array of injection fifos
        T_Fifo * _rfifo; ///< Pointer to fifo to use as a reception fifo

        T_SysDep      * _sysdep;

        dispatch_t  _dispatch[256][256];
        //size_t      _dispatch_count[256];

        Queue * __sendQ;
        unsigned          __sendQMask;

        Queue * __doneQ;
        unsigned          __doneQMask;

        size_t            _num_procs;
        size_t            _global_task;
        size_t            _local_task;

        // Read state variables. See readData_impl()
        char     * _current_pkt_data_ptr;
        unsigned   _current_pkt_iov_bytes_read;
        unsigned   _current_pkt_iov;
    };

    template <class T_SysDep, class T_Fifo, class T_Packet>
    inline size_t ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::getLocalRank()
    {
      return _local_task;
    }

    ///
    /// \brief Check if the send queue to a local rank is empty
    ///
    /// \param[in] peer  \b Local rank
    ///
    template <class T_SysDep, class T_Fifo, class T_Packet>
    inline bool ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::isSendQueueEmpty (size_t peer)
    {
      return ((__sendQMask >> peer) & 0x01) == 0;
    }

    /// \see XMI::Device::Interface::BaseDevice::readData()
    template <class T_SysDep, class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::readData_impl (int channel, void * buf, size_t length)
    {
      abort();
      return 0;
    }

    template <class T_SysDep, class T_Fifo, class T_Packet>
    xmi_result_t ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::writeSinglePacket (size_t   fnum,
                                                                      uint16_t   dispatch_id,
                                                                      void   * metadata,
                                                                      size_t   metasize,
                                                                      void   * payload,
                                                                      size_t   bytes,
                                                                      size_t & sequence)
    {
      T_Packet * pkt = _fifo[fnum].nextInjPacket ();

      if (pkt != NULL)
        {
          uint16_t * hdr = (uint16_t *) pkt->getHeader ();

          // First 2 bytes is the dispatch id.
          hdr[0] = dispatch_id;

          // Remaining header bytes are metadata.
          TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::writeSinglePacket () .. metadata = %p\n", _sysdep->mapping.task(), metadata));
          memcpy ((void *) &hdr[1], metadata, metasize);

          // Write the packet payload data
          void * data = (void *) pkt->getPayload ();
          memcpy (data, payload, bytes);

          sequence = _fifo[fnum].getPacketSequenceId (pkt);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket (pkt);

          return XMI_SUCCESS;
        }

      return XMI_EAGAIN;
    };

    template <class T_SysDep, class T_Fifo, class T_Packet>
    xmi_result_t ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::writeSinglePacket (size_t   fnum,
                                                                      uint16_t   dispatch_id,
                                                                      void   * metadata,
                                                                      size_t   metasize,
                                                                      void   * payload0,
                                                                      size_t   bytes0,
                                                                      void   * payload1,
                                                                      size_t   bytes1,
                                                                      size_t & sequence)
    {
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::writeSinglePacket (%zd, %zd, ...) >>\n", _sysdep->mapping.task(), fnum, dispatch_id));

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
          memcpy ((void *)data, payload0, bytes0);
          memcpy ((void *)(data + bytes0), payload1, bytes1);

          sequence = _fifo[fnum].getPacketSequenceId (pkt);

          // "produce" the packet into the fifo.
          _fifo[fnum].producePacket (pkt);

          TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::writeSinglePacket (%zd, %zd, ...) << CM_SUCCESS\n", _sysdep->mapping.task(), fnum, dispatch_id));
          return XMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::writeSinglePacket (%zd, %zd, ...) << CM_EAGAIN\n", _sysdep->mapping.task(), fnum, dispatch_id));
      return XMI_EAGAIN;
    };

    template <class T_SysDep, class T_Fifo, class T_Packet>
    xmi_result_t ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::writeSinglePacket (size_t         fnum,
                                                                      uint16_t         dispatch_id,
                                                                      void         * metadata,
                                                                      size_t         metasize,
                                                                      struct iovec * iov,
                                                                      size_t         niov,
                                                                      size_t       & sequence)
    {
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::writeSinglePacket (%zd, %zd, %p, %p, %zd) >>\n", _sysdep->mapping.task(), fnum, dispatch_id, metadata, iov, niov));

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

          TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::writeSinglePacket (%zd, %zd, %p, %p, %zd) << CM_SUCCESS\n", _sysdep->mapping.task(), fnum, dispatch_id, metadata, iov, niov));
          return XMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::writeSinglePacket (%zd, %zd, %p, %p, %zd) << CM_EAGAIN\n", _sysdep->mapping.task(), fnum, dispatch_id, metadata, iov, niov));
      return XMI_EAGAIN;
    };

    template <class T_SysDep, class T_Fifo, class T_Packet>
    xmi_result_t ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::writeSinglePacket (size_t                       fnum,
                                                                      ShmemBaseMessage<T_Packet> * msg,
                                                                      size_t                     & sequence)
    {
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::writeSinglePacket (%zd, %p) >>\n", _sysdep->mapping.task(), fnum, msg));

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

          TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::writeSinglePacket (%zd, %p) << CM_SUCCESS\n", _sysdep->mapping.task(), fnum, msg));
          return XMI_SUCCESS;
        }

      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::writeSinglePacket (%zd, %p) << CM_EAGAIN\n", _sysdep->mapping.task(), fnum, msg));
      return XMI_EAGAIN;
    };

    template <class T_SysDep, class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::advance_internal ()
    {
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_internal() >> ... __sendQMask = 0x%0x\n", _sysdep->mapping.task(), __sendQMask));

      // Advance any pending send messages.
      if (__sendQMask != 0) advance_sendQ ();

      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_internal()    ... __doneQMask = 0x%0x\n", _sysdep->mapping.task(), __doneQMask));

      // Advance any pending done messages.
      if (__doneQMask != 0) advance_doneQ ();

      // Advance any pending receive messages.
      T_Packet * pkt = NULL;

      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_internal()    ... before _rfifo->nextRecPacket()\n", _sysdep->mapping.task()));
      while ((pkt = _rfifo->nextRecPacket()) != NULL)
        {
          TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_internal()    ... before pkt->getHeader()\n", _sysdep->mapping.task()));
          uint8_t * hdr = (uint8_t *) pkt->getHeader ();

          uint16_t id = *((uint16_t *) & hdr[0]);
          void * meta = (void *) & hdr[2];
          void * data = pkt->getPayload ();
          TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_internal()    ... hdr = %p, hdr[0] = 0x%0x, hdr[1] = 0x%0x, id = %d, meta = %p, data = %p\n", _sysdep->mapping.task(), hdr, hdr[0], hdr[1], id, meta, data));

          mem_sync (); // TODO -- is this needed?

          _dispatch[hdr[0]][hdr[1]].function (meta, data, pkt->payloadSize(), _dispatch[hdr[0]][hdr[1]].clientdata);

          // Complete this message/packet and increment the fifo head.
          TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_internal()    ... before _rfifo->consumePacket()\n", _sysdep->mapping.task()));
          _rfifo->consumePacket (pkt);
          TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_internal()    ...  after _rfifo->consumePacket()\n", _sysdep->mapping.task()));
        }
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_internal()    ...  after _rfifo->nextRecPacket()\n", _sysdep->mapping.task()));

      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_internal() <<\n", _sysdep->mapping.task()));
      return 0;
    }

    ///
    /// \brief Check if the send queue to a local rank is empty
    ///
    /// \param[in] peer  \b Local rank
    ///
    template <class T_SysDep, class T_Fifo, class T_Packet>
    inline void ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::pushSendQueueTail (size_t peer, QueueElem * element)
    {
      TRACE_ERR ((stderr, "(%zd) pushSendQueueTail(%zd, %p), __sendQMask = %d -> %d\n", _sysdep->mapping.task(), peer, element, __sendQMask, __sendQMask | (1 << peer)));
      __sendQ[peer].pushTail (element);
      __sendQMask |= (1 << peer);
    }

    ///
    /// \brief Check if the send queue to a local rank is empty
    ///
    /// \param[in] peer  \b Local rank
    ///
    template <class T_SysDep, class T_Fifo, class T_Packet>
    inline QueueElem * ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::popSendQueueHead (size_t peer)
    {
      TRACE_ERR((stderr, "popping out from the sendQ\n"));
      QueueElem * tmp = __sendQ[peer].popHead();
      __sendQMask = __sendQMask & ~(__sendQ[peer].isEmpty() << peer);
      return tmp;
    }

    ///
    /// \brief Check if the send queue to a local rank is empty
    ///
    /// \param[in] peer  \b Local rank
    ///
    template <class T_SysDep, class T_Fifo, class T_Packet>
    inline void ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::pushDoneQueueTail (size_t peer, QueueElem * element)
    {
      TRACE_ERR ((stderr, "(%zd) pushDoneQueueTail(%zd, %p), __doneQMask = %d -> %d\n", _sysdep->mapping.task(), peer, element, __doneQMask, __doneQMask | (1 << peer)));
      __doneQ[peer].pushTail (element);
      __doneQMask |= (1 << peer);
    }

    ///
    /// \brief Check if the send queue to a local rank is empty
    ///
    /// \param[in] peer  \b Local rank
    ///
    template <class T_SysDep, class T_Fifo, class T_Packet>
    inline QueueElem * ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::popDoneQueueHead (size_t peer)
    {
      TRACE_ERR((stderr, "popping out from the doneQ\n"));
      QueueElem * tmp = __doneQ[peer].popHead();
      __doneQMask = __doneQMask & ~(__doneQ[peer].isEmpty() << peer);
      return tmp;
    }
  };
};
#undef TRACE_ERR

// Include the non-inline method definitions
#include "ShmemBaseDevice_impl.h"

#endif // __components_devices_shmem_shmembasedevice_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
