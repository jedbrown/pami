/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUMulticombineModel.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu_MUMulticombineModel_h__
#define __components_devices_bgq_mu_MUMulticombineModel_h__

#include <hwi/include/bqc/MU_PacketHeader.h>

#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/CollectiveMemoryFIFODescriptorXX.h>
#include <spi/include/kernel/memory.h>

#include "components/devices/MulticombineModel.h"

#include "components/devices/bgq/mu/MUCollDevice.h"
#include "components/devices/bgq/mu/msg/ShortInjFifoMessage.h"
#include "components/devices/bgq/mu/Dispatch.h"
#include "components/devices/bgq/mu/MUDescriptorWrapper.h"

#include <pami.h>
#include "Global.h"
#include "PipeWorkQueue.h"
#include "Topology.h"

#undef TRACE
#define TRACE(x) //fprintf x


//#define OPTIMIZE_AGGREGATE_LATENCY

namespace PAMI
{

  namespace Device
  {

    namespace MU
    {
      /// \brief Map PAMI dt/op to MU op
      const uint8_t mu_op(pami_dt dt, pami_op op);

      /// \brief Map PAMI dt to a MU datatype size
      const size_t  mu_size(pami_dt dt);

      ///////////////////////////////////////////////////////////////////////////////
      // Some structures that needed to be defined outside the class
      ///////////////////////////////////////////////////////////////////////////////
      // Receive state
      //typedef struct mcombine_recv_state
      class mcombine_recv_state_t : public Queue::Element
      {
      public:
        inline mcombine_recv_state_t() :
        Queue::Element ()
        {
        };

        size_t                               received_length;
        size_t                               expected_length;
        PAMI::PipeWorkQueue                 * rcvpwq;
        uint8_t                            * buffer;
        pami_callback_t                       cb_done;
        unsigned                             connection_id;
        pami_op                               optor;
        pami_dt                               dtype;
        bool                                 mu_reset;
        uint8_t                              mu_dtype_optor;
        uint16_t                             mu_word_length;
      } ;

      // State (request) implementation.  Caller should use uint8_t[MUMulticombine::sizeof_msg]
      typedef struct __mu_multicombine_statedata
      {
        ShortInjFifoMessage  message;
        mcombine_recv_state_t      receive_state;
      } mu_multicombine_statedata_t;

      ///////////////////////////////////////////////////////////////////////////////
      // \class MUMulticombineModel
      // \brief MU collective device Multicombine interface
      // \details
      //   - all sided
      //   - uses MU memfifo
      //   - global (uses global class route)
      //   - one destination task per node
      //   - does not fully support PipeWorkQueue (multicast_model_available_buffers_only)
      //   - all data must be received (\todo ease this limitation - see expected_length processing)
      ///////////////////////////////////////////////////////////////////////////////
      class MUMulticombineModel : public Interface::MulticombineModel < MUMulticombineModel,MUCollDevice, sizeof(mu_multicombine_statedata_t) >
      {

      protected:

      public:

        /// \see PAMI::Device::Interface::MulticombineModel::MulticombineModel
        MUMulticombineModel (MUCollDevice & device, pami_result_t &status);

        /// \see PAMI::Device::Interface::MulticombineModel::~MulticombineModel
        ~MUMulticombineModel ();

        static const bool   multicombine_model_allreduce_only          = true;
//      static const bool   multicombine_model_reduce_only             = false;
        static const size_t sizeof_msg                                 = sizeof(mu_multicombine_statedata_t);
        static const size_t multicombine_model_bytes_max               = (uint32_t) - 1; // metadata_t::sndlen
        static const size_t multicombine_model_connection_id_max       = (uint32_t) - 1; // metadata_t::connection_id \todo 64 bit?
        static const bool   multicombine_model_available_buffers_only  = true;
        static const bool   multicombine_model_op_support(pami_dt, pami_op); /// \todo add to MulticombineModel and _impl it?
        /// \see PAMI::Device::Interface::MulticombineModel::postMulticombine
        pami_result_t postMulticombine_impl(uint8_t (&state)[sizeof_msg],
                                           pami_multicombine_t *multicombine);

      protected:

        // Metadata passed in the packet header
        typedef struct __attribute__((__packed__)) _metadata
        {
          uint32_t              connection_id;  ///< Collective connection id \todo expand to 64 bit?
          uint32_t              sndlen;         ///< Number of bytes of application data
        } metadata_t;

        inline static int dispatch (void   * metadata,
                                    void   * payload,
                                    size_t   bytes,
                                    void   * arg,
                                    void   * cookie);
        inline void processData   (metadata_t    *  metadata,
                                   uint8_t * payload,
                                   size_t    bytes);
        inline void initializeDescriptor (MUSPI_DescriptorBase * desc,
                                          uint64_t               payloadPa,
                                          size_t                 bytes);

        inline bool postPayload(InjFifoMessage& message,
                                unsigned connection_id,
                                void* payload,
                                size_t payload_length);

        inline bool postShortPayload(ShortInjFifoMessage& message,
                                     unsigned connection_id,
                                     void* payload,
                                     size_t payload_length);

      private:
        MUCollDevice                        & _device;
        MUDescriptorWrapper                   _wrapper_model;
        MUSPI_CollectiveMemoryFIFODescriptor  _desc_model;
        Queue                                 _recvQ;
        //std::map<unsigned,mcombine_recv_state_t*, std::less<unsigned>, __gnu_cxx::malloc_allocator<std::pair<const unsigned,mcombine_recv_state_t*> > >      _recvQ;            // _recvQ[connection_id]
        mcombine_recv_state_t               * _receive_state;    // current state, only valid during postMulticombine call.

      }; // PAMI::Device::MU::MUMulticombineModel class

      inline void MUMulticombineModel::initializeDescriptor (MUSPI_DescriptorBase* desc,
                                                             uint64_t payloadPa,
                                                             size_t bytes)
      {
        TRACE((stderr, "<%p>:MUMulticombineModel::initializeDescriptor(%p, %p, %zu)\n", this, desc, (void *)payloadPa, bytes));

        // opcode or datatype changed so we need to reset our model.
        if (_receive_state->mu_reset)
        {
          _desc_model.setOpCode(_receive_state->mu_dtype_optor);
          _desc_model.setWordLength(_receive_state->mu_word_length);
        }
#ifdef ENABLE_MAMBO_WORKAROUNDS
        // Mambo can not handle zero-byte payloads.
        if (bytes == 0)
        {
          payloadPa = (uint64_t) &desc->PacketHeader;
          bytes  = _receive_state->mu_word_length;
        }
#endif
        // Clone the model descriptor.
        _desc_model.clone (*desc);
        desc->setPayload (payloadPa, bytes);

        DUMP_DESCRIPTOR("initializeDescriptor() ..done", desc);

      }; // PAMI::Device::MU::MUMulticombineModel::initializeDescriptor (MUSPI_DescriptorBase * desc,

      inline pami_result_t MUMulticombineModel::postMulticombine_impl(uint8_t (&state)[MUMulticombineModel::sizeof_msg],
                                                                     pami_multicombine_t *multicombine)
      {
//        PAMI_assert(!multicombine_model_all_sided || multicombine->src != ... not sure what I was trying to assert but think about it...
        mu_multicombine_statedata_t *state_data = (mu_multicombine_statedata_t*) & state;

        // multicombine_model_available_buffers_only semantics: If you're sending data, it must all be ready in the pwq. \todo assert?
        PAMI::PipeWorkQueue *pwq = (PAMI::PipeWorkQueue *)multicombine->data;
        size_t length = pwq ? pwq->bytesAvailableToConsume() : 0;

        void* payload = NULL;

        if (length)
        {
          payload = (void*)pwq->bufferToConsume();
          pwq->consumeBytes(length);
        }

        _receive_state = &state_data->receive_state;
        _recvQ.pushTail(_receive_state);

        _receive_state->cb_done = multicombine->cb_done;
        _receive_state->connection_id = multicombine->connection_id;
        _receive_state->rcvpwq = (PAMI::PipeWorkQueue*)multicombine->results;
        _receive_state->expected_length = _receive_state->rcvpwq ? _receive_state->rcvpwq->bytesAvailableToProduce() : 0; /// \todo assert something?
        _receive_state->buffer = _receive_state->rcvpwq ? (uint8_t*)_receive_state->rcvpwq->bufferToProduce() : NULL; /// \todo assert something?
        _receive_state->received_length = 0;

        if ((_receive_state->dtype != multicombine->dtype) || (_receive_state->optor != multicombine->optor))
        {
          TRACE((stderr, "<%p>:MUMulticombineModel::postMulticombine_imp dt %d, op %d, support %s\n", this,
                 multicombine->dtype, multicombine->optor, multicombine_model_op_support(multicombine->dtype, multicombine->optor) ? "true" : "false"));
          PAMI_assertf(multicombine_model_op_support(multicombine->dtype, multicombine->optor),"Unsupported dt %d, op %d\n",multicombine->dtype, multicombine->optor);

          _receive_state->dtype = multicombine->dtype;
          _receive_state->optor = multicombine->optor;
          _receive_state->mu_reset = true;

          /// Map PAMI dt/op to MU op
          _receive_state->mu_dtype_optor =  mu_op(multicombine->dtype, multicombine->optor);
          /// Map PAMI dt to a MU work length
          _receive_state->mu_word_length =  mu_size(multicombine->dtype);
        }

        TRACE((stderr, "<%p>:MUMulticombineModel::postMulticombine_impl() connection_id %#X, data length %zu/%p/%p, results length %zu/%p/%p\n",
               this, multicombine->connection_id,
               length, pwq, payload,
               _receive_state->expected_length, _receive_state->rcvpwq, _receive_state->buffer));

        // Post the multicombine to the device
        if (length < MUCollDevice::payload_size) // payload in one packet
        {
          postShortPayload(state_data->message, multicombine->connection_id,  payload, length);
        }
        else // payload in (multi-)packets
        {
          postPayload(state_data->message, multicombine->connection_id,  payload, length);
        }

        _receive_state = NULL; // ONLY VALID DURING THIS CALL so reset it.

        return PAMI_SUCCESS;

      }; // PAMI::Device::MU::MUMulticombineModel::postMulticombine_impl

      inline bool MUMulticombineModel::postShortPayload(ShortInjFifoMessage &message,
                                                        unsigned connection_id,
                                                        void* payload,
                                                        size_t payload_length)
      {
        TRACE((stderr, "<%p>:MUMulticombineModel::postShortPayload()\n", this));

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

        if (_device.nextInjectionDescriptor (&injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
        {
          TRACE((stderr, "<%p>:MUMulticombineModel::postShortPayload().. nextInjectionDescriptor injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", this, injfifo, hwi_desc, payloadVa, payloadPa));
          MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

          // Pack the msginfo and payload into the packet payload.
          uint8_t * data = (uint8_t *) payloadVa;

          // Now the payload
          TRACE((stderr, "<%p>:MUMulticombineModel::postShortPayload()..payload memcpy(%p,%p,%zu)\n", this, data, payload, payload_length));

          if (payload_length) memcpy (data, payload, payload_length);

          // Initialize the descriptor directly in the injection fifo.
          initializeDescriptor (desc, (uint64_t) payloadPa, payload_length);

          // Put the metadata into the network header in the descriptor.
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & desc->PacketHeader;

          metadata_t *metadata = (metadata_t*) & hdr->dev.multipkt.metadata;

          metadata->connection_id = connection_id;
          metadata->sndlen        = payload_length;

          // Advance the injection fifo descriptor tail which actually enables
          // the MU hardware to process the descriptor and send the packet
          // on the torus.
          DUMP_DESCRIPTOR("MUMulticombineModel::postShortPayload().. before MUSPI_InjFifoAdvanceDesc()", desc);

          MUSPI_InjFifoAdvanceDesc (injfifo);
        }
        else
        {
          TRACE((stderr, "<%p>:MUMulticombineModel::postShortPayload().. nextInjectionDescriptor failed\n", this));
          // Construct a message and post to the device to be processed later.
          new (&message) ShortInjFifoMessage ();

          message.copyPayload (payload, payload_length);
          Kernel_MemoryRegion_t kmr;
          Kernel_CreateMemoryRegion (&kmr, message.getPayload(), payload_length);

          // Initialize the descriptor
          MUSPI_DescriptorBase * desc = message.getDescriptor ();
          initializeDescriptor (desc, (uint64_t)kmr.BasePa, payload_length);

          // Enable the "single packet message" bit so that it uses my iov src buffer
          desc->setSoftwareBit (1);


          // Put the metadata into the network header in the descriptor.
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);

          metadata_t *metadata = (metadata_t*) & hdr->dev.singlepkt.metadata;
          metadata->connection_id = connection_id;
          metadata->sndlen        = payload_length;

          DUMP_DESCRIPTOR("MUMulticombineModel::postShortPayload().. before addToSendQ                ", desc);
          // Add this message to the send queue to be processed when there is
          // space available in the injection fifo.
          _device.addToSendQ ((PAMI::Queue::Element *) &message);
        }

        return true;

      }; // PAMI::Device::MU::MUMulticombineModel::postShortPayload

      inline bool MUMulticombineModel::postPayload(InjFifoMessage &message,
                                                   unsigned connection_id,
                                                   void* payload,
                                                   size_t payload_length)
      {
        TRACE((stderr, "<%p>:MUMulticombineModel::postPayload()\n", this));

        // Determine the physical address of the source buffer.
        //
        // TODO - need to have a different way of finding/pinning the buffer that
        //        does not involve a syscall or memory region. If we use a memory
        //        region, when is the region destroyed?  It should be *after* the
        //        descriptor is completed, right?
        uint32_t rc;
        Kernel_MemoryRegion_t memRegion; // Memory region associated with the buffer.
        rc = Kernel_CreateMemoryRegion (&memRegion, payload, payload_length);
        PAMI_assert ( rc == 0 );

        uint64_t paddr = (uint64_t)memRegion.BasePa +
                         ((uint64_t)payload - (uint64_t)memRegion.BaseVa);

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

        if (_device.nextInjectionDescriptor (&injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
        {
          TRACE((stderr, "<%p>:MUMulticombineModel::postPayload().. nextInjectionDescriptor injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", this, injfifo, hwi_desc, payloadVa, payloadPa));
          MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

          // Initialize the descriptor directly in the injection fifo.
          initializeDescriptor (desc, paddr, payload_length);

          // Put the metadata into the network header in the descriptor.
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & desc->PacketHeader;

          metadata_t *metadata = (metadata_t*) & hdr->dev.multipkt.metadata;
          metadata->connection_id = connection_id;
          metadata->sndlen        = payload_length;

          // Advance the injection fifo descriptor tail which actually enables
          // the MU hardware to process the descriptor and send the packet
          // on the torus.
          DUMP_DESCRIPTOR("MUMulticombineModel::postPayload().. before MUSPI_InjFifoAdvanceDesc()", desc);
          MUSPI_InjFifoAdvanceDesc (injfifo);

        }
        else
        {
          TRACE((stderr, "<%p>:MUMulticombineModel::postPayload().. nextInjectionDescriptor failed\n", this));
          // Construct a message and post to the device to be processed later.
          new (&message) InjFifoMessage ();
          //message.setSourceBuffer (payload, payload_length);

          // Initialize the descriptor directly in the injection fifo.
          MUSPI_DescriptorBase * desc = message.getDescriptor ();
          initializeDescriptor (desc, paddr, payload_length);

          // Put the metadata into the network header in the descriptor.
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);

          metadata_t *metadata = (metadata_t*) & hdr->dev.multipkt.metadata;
          metadata->connection_id = connection_id;
          metadata->sndlen        = payload_length;

          DUMP_DESCRIPTOR("MUMulticombineModel::postPayload().. before addToSendQ                ", desc);
          // Add this message to the send queue to be processed when there is
          // space available in the injection fifo.
          _device.addToSendQ ((PAMI::Queue::Element *) &message);
        }

        return true;

      }; // PAMI::Device::MU::MUMulticombineModel::postPayload


      ///
      /// \brief Direct multi-packet send data packet callback.
      ///
      /// Copies incoming packet data from the device to the destination buffer.
      ///
      /// The eager simple send protocol will register this dispatch function
      /// if and only if the device \b does provide direct access to
      /// data which has already been read from the network by the device.
      ///
      /// The data dispatch function is invoked by the packet device
      /// to process the subsequent packets of a multi-packet message
      /// after the first envelope packet.
      ///
      /// \see PAMI::Device::Interface::RecvFunction_t
      ///
      inline void MUMulticombineModel::processData   (metadata_t   *  metadata,
                                                      uint8_t * payload,
                                                      size_t    bytes)
      {
        TRACE((stderr, "<%p>:MUMulticombineModel::processData()\n", this));

        mcombine_recv_state_t* receive_state = (mcombine_recv_state_t*)_recvQ.peekHead();
        // probably the head, but (unlikely) search if it isn't
        while(receive_state && receive_state->connection_id != metadata->connection_id)
          receive_state = (mcombine_recv_state_t*)_recvQ.nextElem(receive_state);
        PAMI_assert(receive_state); // all-sided and sync'd by MU so this shouldn't be unexpected data

        // Number of bytes left to copy into the destination buffer
        size_t nleft = receive_state->expected_length - receive_state->received_length;

        // Number of bytes left to copy from this packet
        if (nleft > bytes) nleft = bytes;

        if (nleft)
        {
          TRACE((stderr, "<%p>:MUMulticombineModel::processData memcpy(%p,%p,%zu)\n", this, receive_state->buffer, payload, nleft));
          memcpy (receive_state->buffer, payload, nleft);
          //_device.read ((uint8_t *)(state->info.data.simple.addr) + nbyte, nleft, cookie);

          // Update the receive state
          receive_state->received_length += nleft;
          receive_state->rcvpwq->produceBytes(nleft);

          if (receive_state->received_length == receive_state->expected_length)
          {
            /// \todo handle unwanted data?  stay active and toss it as it arrives

            // Invoke the receive done callback.
            if (receive_state->cb_done.function)
              receive_state->cb_done.function (_device.getContext(),
                                               receive_state->cb_done.clientdata,
                                               PAMI_SUCCESS);
            _recvQ.deleteElem(receive_state);
          }
        }
        else PAMI_abortf("%s<%d>\n",__FILE__,__LINE__);  /// \todo toss unwanted data?

        return ;
      }; // PAMI::Device::MU::MUMulticombineModel::processData

      ///
      /// \brief Direct multi-packet send envelope packet dispatch.
      ///
      /// The eager simple send protocol will register this dispatch
      /// function if and only if the device \b does provide direct access
      /// to data which has already been read from the network by the
      /// device.
      ///
      /// The envelope dispatch function is invoked by the message device
      /// to process the first packet of a multi-packet message. The eager
      /// simple send protocol transfers protocol metadata and application
      /// metadata in a single packet. Application data will arrive in
      /// subsequent eager simple send data packets and will be processed
      /// by the data dispatch function.
      ///
      /// \see PAMI::Device::Interface::RecvFunction_t
      ///
      inline
      int MUMulticombineModel::dispatch (void   * metadata,
                                         void   * payload,
                                         size_t   bytes,
                                         void   * arg,
                                         void   * cookie)
      {
        metadata_t * m = (metadata_t*)metadata;
        TRACE ((stderr, "<%p>:MUMulticombineModel::dispatch(), bytes = %zu/%d, connection id %#X\n", arg, bytes, m->sndlen, m->connection_id));

        MUMulticombineModel * model = (MUMulticombineModel *) arg;
        model->processData(m, (uint8_t*)payload, bytes);
        return 0;
      }; // PAMI::Device::MU::MUMulticombineModel::dispatch


    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#undef TRACE

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
