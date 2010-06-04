/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/PacketModelBase.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_PacketModelBase_h__
#define __components_devices_bgq_mu2_PacketModelBase_h__

#include <hwi/include/bqc/MU_PacketHeader.h>

#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/Pt2PtMemoryFIFODescriptorXX.h>
#include <spi/include/kernel/memory.h>

#include "components/devices/PacketInterface.h"
#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/msg/InjectDescriptorMessage.h"


#include "components/devices/bgq/mu2/trace.h"
#define DO_TRACE_ENTEREXIT 1
#define DO_TRACE_DEBUG 1


namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      template <class T_Model>
      class PacketModelBase : public Interface::PacketModel < MU::PacketModelBase<T_Model>, MU::Context, 256 >
      {
        protected :

          /// \see PAMI::Device::Interface::PacketModel::PacketModel
          PacketModelBase (MU::Context & context);

          /// \see PAMI::Device::Interface::PacketModel::~PacketModel
          virtual ~PacketModelBase () { /*PAMI_abort();*/ }

          inline void processCompletion (void                 * state,
                                         size_t                 fnum,
                                         MUSPI_InjFifo_t      * ififo,
                                         size_t                 ndesc,
                                         MUSPI_DescriptorBase * desc,
                                         pami_event_function    fn,
                                         void                 * cookie);

          inline void processCompletion_impl (void                 * state,
                                              size_t                 fnum,
                                              MUSPI_InjFifo_t      * ififo,
                                              size_t                 ndesc,
                                              MUSPI_DescriptorBase * desc,
                                              pami_event_function    fn,
                                              void                 * cookie);

          inline MU::MessageQueue::Element * createMessage (void                 * state,
                                                            MUSPI_DescriptorBase & desc,
                                                            MUSPI_InjFifo_t      * ififo,
                                                            pami_event_function    fn,
                                                            void                 * cookie);

          inline MU::MessageQueue::Element * createMessage_impl (void                 * state,
                                                                 MUSPI_DescriptorBase & desc,
                                                                 MUSPI_InjFifo_t      * ififo,
                                                                 pami_event_function    fn,
                                                                 void                 * cookie);

        public:

          /// \see PAMI::Device::Interface::PacketModel::isPacketDeterministic
          static const bool   deterministic_packet_model         = true;

          /// \see PAMI::Device::Interface::PacketModel::isPacketReliable
          static const bool   reliable_packet_model              = true;

          /// \see PAMI::Device::Interface::PacketModel::getPacketMetadataBytes
          static const size_t packet_model_metadata_bytes        =
            MemoryFifoPacketHeader::packet_singlepacket_metadata_size;

          /// \see PAMI::Device::Interface::PacketModel::getPacketMultiMetadataBytes
          static const size_t packet_model_multi_metadata_bytes  =
            MemoryFifoPacketHeader::packet_multipacket_metadata_size;

          /// \see PAMI::Device::Interface::PacketModel::getPacketPayloadBytes
          static const size_t packet_model_payload_bytes         = MU::Context::payload_size;

          /// \see PAMI::Device::Interface::PacketModel::getPacketTransferStateBytes
          static const size_t packet_model_state_bytes           = 256;

          /// \see PAMI::Device::Interface::PacketModel::init
          pami_result_t init_impl (size_t                      dispatch,
                                   Interface::RecvFunction_t   direct_recv_func,
                                   void                      * direct_recv_func_parm,
                                   Interface::RecvFunction_t   read_recv_func,
                                   void                      * read_recv_func_parm);

          /// \see PAMI::Device::Interface::PacketModel::postPacket
          template <unsigned T_Niov>
          inline bool postPacket_impl (pami_task_t    target_task,
                                       size_t         target_offset,
                                       void         * metadata,
                                       size_t         metasize,
                                       struct iovec   (&iov)[T_Niov]);

          /// \see PAMI::Device::Interface::PacketModel::postPacket
          template <unsigned T_Niov>
          inline bool postPacket_impl (uint8_t               (&state)[packet_model_state_bytes],
                                       pami_event_function   fn,
                                       void                * cookie,
                                       pami_task_t           target_task,
                                       size_t                target_offset,
                                       void                * metadata,
                                       size_t                metasize,
                                       struct iovec          (&iov)[T_Niov]);

          /// \see PAMI::Device::Interface::PacketModel::postPacket
          inline bool postPacket_impl (uint8_t              (&state)[packet_model_state_bytes],
                                       pami_event_function   fn,
                                       void                * cookie,
                                       pami_task_t           target_task,
                                       size_t                target_offset,
                                       void                * metadata,
                                       size_t                metasize,
                                       struct iovec        * iov,
                                       size_t                niov);

          /// \see PAMI::Device::Interface::PacketModel::postPacket
          inline bool postPacket_impl (uint8_t              (&state)[packet_model_state_bytes],
                                       pami_event_function   fn,
                                       void                * cookie,
                                       pami_task_t           target_task,
                                       size_t                target_offset,
                                       void                * metadata,
                                       size_t                metasize,
                                       void                * payload,
                                       size_t                length);

          /// \see PAMI::Device::Interface::PacketModel::postMultiPacket
          inline bool postMultiPacket_impl (uint8_t               (&state)[packet_model_state_bytes],
                                            pami_event_function   fn,
                                            void                * cookie,
                                            pami_task_t           target_task,
                                            size_t                target_offset,
                                            void                * metadata,
                                            size_t                metasize,
                                            void                * payload,
                                            size_t                length);


        protected:
          MUSPI_DescriptorBase            _singlepkt;
	  MUSPI_DescriptorBase            _multipkt;
	  MU::Context                   & _context;
      };

      template <class T_Model>
      PacketModelBase<T_Model>::PacketModelBase (MU::Context & context) :
          Interface::PacketModel < MU::PacketModelBase<T_Model>, MU::Context, 256 > (context),
          _context (context)
      {
        TRACE_FN_ENTER();
        COMPILE_TIME_ASSERT(sizeof(InjectDescriptorMessage<1>) <= packet_model_state_bytes);

        // Zero-out the descriptor models before initialization
        memset((void *)&_singlepkt, 0, sizeof(_singlepkt));
        memset((void *)&_multipkt, 0, sizeof(_multipkt));

        // --------------------------------------------------------------------
        // Set the common base descriptor fields
        // --------------------------------------------------------------------
        MUSPI_BaseDescriptorInfoFields_t base;
        memset((void *)&base, 0, sizeof(base));

        base.Pre_Fetch_Only  = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;
        base.Payload_Address = 0;
        base.Message_Length  = 0;
        base.Torus_FIFO_Map  = 0;
        base.Dest.Destination.Destination = 0;

        _singlepkt.setBaseFields (&base);
        _multipkt.setBaseFields (&base);


        // --------------------------------------------------------------------
        // Set the common point-to-point descriptor fields
        // --------------------------------------------------------------------
        MUSPI_Pt2PtDescriptorInfoFields_t pt2pt;
        memset((void *)&pt2pt, 0, sizeof(pt2pt));

        pt2pt.Hints_ABCD = 0;
        pt2pt.Skip       = 0;
        pt2pt.Misc1 =
          MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
          MUHWI_PACKET_DO_NOT_DEPOSIT |
          MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE;
        pt2pt.Misc2 =
          MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC;

        _singlepkt.setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
        _singlepkt.PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;
        _singlepkt.setPt2PtFields (&pt2pt);
        _multipkt.setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
        _multipkt.PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;
        _multipkt.setPt2PtFields (&pt2pt);


        // --------------------------------------------------------------------
        // Set the common memory fifo descriptor fields
        // --------------------------------------------------------------------
        MUSPI_MemoryFIFODescriptorInfoFields_t memfifo;
        memset ((void *)&memfifo, 0, sizeof(memfifo));

        memfifo.Rec_FIFO_Id    = 0;
        memfifo.Rec_Put_Offset = 0;
        memfifo.Interrupt      = MUHWI_DESCRIPTOR_DO_NOT_INTERRUPT_ON_PACKET_ARRIVAL;
        memfifo.SoftwareBit    = 0;

        _singlepkt.setMemoryFIFOFields (&memfifo);
        _singlepkt.setMessageUnitPacketType (MUHWI_PACKET_TYPE_FIFO);
        _multipkt.setMemoryFIFOFields (&memfifo);
        _multipkt.setMessageUnitPacketType (MUHWI_PACKET_TYPE_FIFO);


        // --------------------------------------------------------------------
        // Set the network header information in the descriptor models to
        // differentiate between a single-packet transfer and a multi-packet
        // transfer
        // --------------------------------------------------------------------
        MemoryFifoPacketHeader * hdr = NULL;

        hdr = (MemoryFifoPacketHeader *) & _singlepkt.PacketHeader;
        hdr->setSinglePacket (true);

        hdr = (MemoryFifoPacketHeader *) & _multipkt.PacketHeader;
        hdr->setSinglePacket (false);

        TRACE_FN_EXIT();
      };

      template <class T_Model>
      void PacketModelBase<T_Model>::processCompletion (void                 * state,
                                                        size_t                 fnum,
                                                        MUSPI_InjFifo_t      * ififo,
                                                        size_t                 ndesc,
                                                        MUSPI_DescriptorBase * desc,
                                                        pami_event_function    fn,
                                                        void                 * cookie)
      {
        static_cast<T_Model*>(this)->processCompletion_impl (state,
                                                             fnum,
                                                             ififo,
                                                             ndesc,
                                                             desc,
                                                             fn,
                                                             cookie);
      };

      template <class T_Model>
      void PacketModelBase<T_Model>::processCompletion_impl (void                 * state,
                                                             size_t                 fnum,
                                                             MUSPI_InjFifo_t      * ififo,
                                                             size_t                 ndesc,
                                                             MUSPI_DescriptorBase * desc,
                                                             pami_event_function    fn,
                                                             void                 * cookie)
      {
        // Advance the injection fifo tail pointer. This action
        // completes the injection operation.
        uint64_t sequenceNum = 0; // suppress warning
        sequenceNum = MUSPI_InjFifoAdvanceDesc (ififo);
// !!!!
// add counter completion notification stuff
// !!!!
      };


      template <class T_Model>
      MU::MessageQueue::Element * PacketModelBase<T_Model>::createMessage (void                 * state,
                                                                           MUSPI_DescriptorBase & desc,
                                                                           MUSPI_InjFifo_t      * ififo,
                                                                           pami_event_function    fn,
                                                                           void                 * cookie)
      {
        return static_cast<T_Model*>(this)->createMessage_impl (state, desc, ififo, fn, cookie);
      };

      template <class T_Model>
      MU::MessageQueue::Element * PacketModelBase<T_Model>::createMessage_impl (void                 * state,
                                                                                MUSPI_DescriptorBase & desc,
                                                                                MUSPI_InjFifo_t      * ififo,
                                                                                pami_event_function    fn,
                                                                                void                 * cookie)
      {
        InjectDescriptorMessage<1> * msg = (InjectDescriptorMessage<1> *) state;
        new (msg) InjectDescriptorMessage<1> (ififo);

        // Copy the "data mover" descriptor into the message.
        desc.clone (msg->desc[0]);

        return (MU::MessageQueue::Element *) msg;
      };




      template <class T_Model>
      pami_result_t PacketModelBase<T_Model>::init_impl (size_t                      dispatch,
                                                         Interface::RecvFunction_t   direct_recv_func,
                                                         void                      * direct_recv_func_parm,
                                                         Interface::RecvFunction_t   read_recv_func,
                                                         void                      * read_recv_func_parm)
      {
        TRACE_FN_ENTER();

        // Register the direct dispatch function. The MU context will have access
        // to the packet payload in the memory fifo at the time the dispatch
        // function is invoked and can provide a direct pointer to the packet
        // payload.
        uint16_t id = 0;

        if (_context.registerPacketHandler (dispatch,
                                            direct_recv_func,
                                            direct_recv_func_parm,
                                            id))
          {
            MemoryFifoPacketHeader * hdr = NULL;

            hdr = (MemoryFifoPacketHeader *) & _singlepkt.PacketHeader;
            hdr->setDispatchId (id);

            hdr = (MemoryFifoPacketHeader *) & _multipkt.PacketHeader;
            hdr->setDispatchId (id);

            TRACE_FORMAT("register packet handler success. dispatch id = %d", id);
            TRACE_FN_EXIT();

            return PAMI_SUCCESS;
          }

        TRACE_STRING("register packet handler FAILED. return PAMI_ERROR");
        TRACE_FN_EXIT();

        return PAMI_ERROR;
      };

      template <class T_Model>
      template <unsigned T_Niov>
      bool PacketModelBase<T_Model>::postPacket_impl (pami_task_t    target_task,
                                                      size_t         target_offset,
                                                      void         * metadata,
                                                      size_t         metasize,
                                                      struct iovec   (&iov)[T_Niov])
      {
        MUHWI_Destination_t   dest;
        MUSPI_InjFifo_t     * ififo;
        uint16_t              rfifo;
        uint64_t              map;
        uint8_t               hintsABCD;
        uint8_t               hintsE;

        size_t fnum = _context.pinFifo ((size_t) target_task,
                                        target_offset,
                                        dest,
                                        &ififo,
                                        rfifo,
                                        map,
                                        hintsABCD,
                                        hintsE);

        MUHWI_Descriptor_t * desc;
        void               * vaddr;
        uint64_t             paddr;

        size_t ndesc =
          _context.nextInjectionDescriptor (fnum, &desc, &vaddr, &paddr);

        if (likely(ndesc > 0))
          {
            // There is at least one descriptor slot available in the injection
            // fifo before a fifo-wrap event.

            // Clone the single-packet model descriptor into the injection fifo
            //MUSPI_DescriptorBase * memfifo = &_singlepkt; 

	    MUSPI_DescriptorBase * memfifo = (MUSPI_DescriptorBase *)desc; 
	    _singlepkt.clone (*(MUSPI_DescriptorBase *)desc);

            // Initialize the injection fifo descriptor in-place.
            memfifo->setDestination (dest);
            memfifo->setTorusInjectionFIFOMap (map);
            memfifo->setHints (hintsABCD, hintsE);
            memfifo->setRecFIFOId (rfifo);

	    MemoryFifoPacketHeader *hdr = (MemoryFifoPacketHeader*) 
	      &memfifo->PacketHeader;
	    //Eliminated memcpy and an if branch
	    hdr->setMetaData(metadata, metasize);

            // Copy the payload into the immediate payload buffer.
            size_t i, tbytes = 0;
            uint8_t * dst = (uint8_t *)vaddr;

            for (i = 0; i < T_Niov; i++)
            {
	      memcpy ((dst + tbytes), iov[i].iov_base, iov[i].iov_len);
	      tbytes += iov[i].iov_len;
	    }

            // Set the payload information.
            memfifo->setPayload (paddr, tbytes);

	    //Copy the model to the injection FIFO
	    //_singlepkt.clone (*(MUSPI_DescriptorBase *)desc);

            //fprintf(stderr, "Advance tail pointer, tbytes %d\n", tbytes);

            // Finally, advance the injection fifo tail pointer. This action
            // completes the injection operation.
            MUSPI_InjFifoAdvanceDesc (ififo);

            return true;
          }

        return false;
      };

      template <class T_Model>
      template <unsigned T_Niov>
      bool PacketModelBase<T_Model>::postPacket_impl (uint8_t               (&state)[packet_model_state_bytes],
                                                      pami_event_function   fn,
                                                      void                * cookie,
                                                      pami_task_t           target_task,
                                                      size_t                target_offset,
                                                      void                * metadata,
                                                      size_t                metasize,
                                                      struct iovec          (&iov)[T_Niov])
      {
        MUHWI_Destination_t   dest;
        MUSPI_InjFifo_t     * ififo;
        uint16_t              rfifo;
        uint64_t              map;
        uint8_t               hintsABCD;
        uint8_t               hintsE;

        size_t fnum = _context.pinFifo ((size_t) target_task, target_offset, dest,
                                        &ififo, rfifo, map, hintsABCD, hintsE);

        MUHWI_Descriptor_t * desc;
        void               * vaddr;
        uint64_t             paddr;

        size_t ndesc =
          _context.nextInjectionDescriptor (fnum, &desc, &vaddr, &paddr);

        if (likely(ndesc > 0))
          {
            // There is at least one descriptor slot available in the injection
            // fifo before a fifo-wrap event.

            // Clone the single-packet model descriptor into the injection fifo
            MUSPI_DescriptorBase * memfifo = (MUSPI_DescriptorBase *) desc;
            _singlepkt.clone (*memfifo);

            // Initialize the injection fifo descriptor in-place.
            memfifo->setDestination (dest);
            memfifo->setTorusInjectionFIFOMap (map);
            memfifo->setHints (hintsABCD, hintsE);
            memfifo->setRecFIFOId (rfifo);

	    MemoryFifoPacketHeader *hdr = (MemoryFifoPacketHeader*) 
	      &memfifo->PacketHeader;
	    //Eliminated memcpy and an if branch
	    hdr->setMetaData(metadata, metasize);

            // Copy the payload into the immediate payload buffer.
            size_t i, tbytes = 0;
            uint8_t * dst = (uint8_t *) vaddr;

            for (i = 0; i < T_Niov; i++)
              {
                memcpy ((dst + tbytes), iov[i].iov_base, iov[i].iov_len);
                tbytes += iov[i].iov_len;
              }

            // Set the payload information.
            memfifo->setPayload (paddr, tbytes);

            // Finally, advance the injection fifo tail pointer. This action
            // completes the injection operation.
            uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (ififo);

            // Invoke the completion callback function
            if (fn != NULL)
              {
                //fn (_context, cookie, PAMI_SUCCESS); // Descriptor is done...notify.
                sequenceNum = 0; // suppress warning for now
              }

          }
        else
          {
            // Create a message and post it to the context.
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
          }

        return true;
      };

      template <class T_Model>
      bool PacketModelBase<T_Model>::postPacket_impl (uint8_t               (&state)[packet_model_state_bytes],
                                                      pami_event_function   fn,
                                                      void                * cookie,
                                                      pami_task_t           target_task,
                                                      size_t                target_offset,
                                                      void                * metadata,
                                                      size_t                metasize,
                                                      struct iovec        * iov,
                                                      size_t                niov)
      {
        MUHWI_Destination_t   dest;
        MUSPI_InjFifo_t     * ififo;
        uint16_t              rfifo;
        uint64_t              map;
        uint8_t               hintsABCD;
        uint8_t               hintsE;

        size_t fnum = _context.pinFifo ((size_t) target_task, target_offset, dest,
                                        &ififo, rfifo, map, hintsABCD, hintsE);

        MUHWI_Descriptor_t * desc;
        void               * vaddr;
        uint64_t             paddr;

        size_t ndesc =
          _context.nextInjectionDescriptor (fnum, &desc, &vaddr, &paddr);

        if (likely(ndesc > 0))
          {
            // There is at least one descriptor slot available in the injection
            // fifo before a fifo-wrap event.

            // Clone the single-packet model descriptor into the injection fifo
            MUSPI_DescriptorBase * memfifo = (MUSPI_DescriptorBase *) desc;
            _singlepkt.clone (*memfifo);

            // Initialize the injection fifo descriptor in-place.
            memfifo->setDestination (dest);
            memfifo->setTorusInjectionFIFOMap (map);
            memfifo->setHints (hintsABCD, hintsE);
            memfifo->setRecFIFOId (rfifo);

	    MemoryFifoPacketHeader *hdr = (MemoryFifoPacketHeader*) 
	      &memfifo->PacketHeader;
	    //Eliminated memcpy and an if branch
	    hdr->setMetaData(metadata, metasize);

            // Copy the payload into the immediate payload buffer.
            size_t i, tbytes = 0;
            uint8_t * dst = (uint8_t *) vaddr;

            for (i = 0; i < niov; i++)
              {
                memcpy ((dst + tbytes), iov[i].iov_base, iov[i].iov_len);
                tbytes += iov[i].iov_len;
              }

            // Set the payload information.
            memfifo->setPayload (paddr, tbytes);

            // Finally, advance the injection fifo tail pointer. This action
            // completes the injection operation.
            uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (ififo);

            // Invoke the completion callback function
            if (fn != NULL)
              {
                //fn (_context, cookie, PAMI_SUCCESS); // Descriptor is done...notify.
                sequenceNum = 0; // suppress warning for now
              }

          }
        else
          {
            // Create a message and post it to the context.
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
          }

        return true;
      };

      template <class T_Model>
      bool PacketModelBase<T_Model>::postPacket_impl (uint8_t               (&state)[packet_model_state_bytes],
                                                      pami_event_function   fn,
                                                      void                * cookie,
                                                      pami_task_t           target_task,
                                                      size_t                target_offset,
                                                      void                * metadata,
                                                      size_t                metasize,
                                                      void                * payload,
                                                      size_t                length)
      {
        MUHWI_Destination_t   dest;
        MUSPI_InjFifo_t     * ififo;
        uint16_t              rfifo;
        uint64_t              map;
        uint8_t               hintsABCD;
        uint8_t               hintsE;

        size_t fnum = _context.pinFifo ((size_t) target_task, target_offset, dest,
                                        &ififo, rfifo, map, hintsABCD, hintsE);


        MUHWI_Descriptor_t * desc;
        void               * vaddr;
        uint64_t             paddr;

        size_t ndesc =
          _context.nextInjectionDescriptor (fnum, &desc, &vaddr, &paddr);

        if (likely(ndesc > 0))
          {
            // There is at least one descriptor slot available in the injection
            // fifo before a fifo-wrap event.

            // Clone the single-packet model descriptor into the injection fifo
            MUSPI_DescriptorBase * memfifo = (MUSPI_DescriptorBase *) desc;
            _singlepkt.clone (*memfifo);

            // Initialize the injection fifo descriptor in-place.
            memfifo->setDestination (dest);
            memfifo->setTorusInjectionFIFOMap (map);
            memfifo->setHints (hintsABCD, hintsE);
            memfifo->setRecFIFOId (rfifo);
            memfifo->setPayload (paddr, length);

	    MemoryFifoPacketHeader *hdr = (MemoryFifoPacketHeader*) 
	      &memfifo->PacketHeader;
	    //Eliminated memcpy and an if branch
	    hdr->setMetaData(metadata, metasize);

            // Copy the payload into the immediate payload buffer.
            memcpy (vaddr, payload, length);

            // Finally, advance the injection fifo tail pointer. This action
            // completes the injection operation.
            uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (ififo);

            // Invoke the completion callback function
            if (fn != NULL)
              {
                //fn (_context, cookie, PAMI_SUCCESS); // Descriptor is done...notify.
                sequenceNum = 0; // suppress warning for now
              }

          }
        else
          {
            // Create a message and post it to the context.
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
          }

        return true;
      };

      template <class T_Model>
      bool PacketModelBase<T_Model>::postMultiPacket_impl (uint8_t               (&state)[packet_model_state_bytes],
                                                           pami_event_function   fn,
                                                           void                * cookie,
                                                           pami_task_t           target_task,
                                                           size_t                target_offset,
                                                           void                * metadata,
                                                           size_t                metasize,
                                                           void                * payload,
                                                           size_t                length)
      {
        MUHWI_Destination_t   dest;
        MUSPI_InjFifo_t     * ififo;
        uint16_t              rfifo;
        uint64_t              map;
        uint8_t               hintsABCD;
        uint8_t               hintsE;

        size_t fnum = _context.pinFifo ((size_t) target_task, target_offset, dest,
                                        &ififo, rfifo, map, hintsABCD, hintsE);


        MUHWI_Descriptor_t * desc;
        void               * vaddr;
        uint64_t             paddr;

        size_t ndesc =
          _context.nextInjectionDescriptor (fnum, &desc, &vaddr, &paddr);

        if (likely(ndesc > 0))
          {
            // There is at least one descriptor slot available in the injection
            // fifo before a fifo-wrap event.

            // Clone the multi-packet model descriptor into the injection fifo
            MUSPI_DescriptorBase * memfifo = (MUSPI_DescriptorBase *) desc;
            _multipkt.clone (*memfifo);

            // Initialize the injection fifo descriptor in-place.
            memfifo->setDestination (dest);
            memfifo->setTorusInjectionFIFOMap (map);
            memfifo->setHints (hintsABCD, hintsE);
            memfifo->setRecFIFOId (rfifo);

            // Determine the physical address of the source data.
            Kernel_MemoryRegion_t memregion;
            Kernel_CreateMemoryRegion (&memregion, payload, length);
            paddr = (uint64_t) memregion.BasePa +
                    ((uint64_t) payload - (uint64_t) memregion.BaseVa);

            // Set the payload information.
            memfifo->setPayload (paddr, length);

	    MemoryFifoPacketHeader *hdr = (MemoryFifoPacketHeader*) 
	      &memfifo->PacketHeader;
	    //Eliminated memcpy and an if branch
	    hdr->setMetaData(metadata, metasize);

            // Finish the completion processing and inject the descriptor(s)
            processCompletion ((void *) state, fnum, ififo, ndesc, (MUSPI_DescriptorBase *)desc, fn, cookie);
          }
        else
          {
            //MUHWI_Descriptor_t d;
            MUSPI_DescriptorBase memfifo;// = (MUSPI_DescriptorBase *) & d;
            _multipkt.clone (memfifo);

            // Initialize the injection fifo descriptor in-place.
            memfifo.setDestination (dest);
            memfifo.setTorusInjectionFIFOMap (map);
            memfifo.setHints (hintsABCD, hintsE);
            memfifo.setRecFIFOId (rfifo);
            memfifo.setPayload (paddr, length);

	    MemoryFifoPacketHeader *hdr = (MemoryFifoPacketHeader*) 
	      &memfifo.PacketHeader;
	    //Eliminated memcpy and an if branch
	    hdr->setMetaData(metadata, metasize);

            MU::MessageQueue::Element * msg =
              createMessage (state, memfifo, ififo, fn, cookie);

            _context.post (fnum, msg);

            // Create a message and post it to the context.
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
          }

        return true;
      };

    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_PacketModelBase_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//


