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
#include "components/devices/bgq/mu/MUInjFifoMessage.h"
#include "components/devices/bgq/mu/Dispatch.h"
#include "components/devices/bgq/mu/MUDescriptorWrapper.h"

#include "sys/xmi.h"
#include "Global.h"
#include "PipeWorkQueue.h"
#include "Topology.h"

#ifdef TRACE
  #undef TRACE
#endif
#define TRACE(x) //fprintf x
#ifdef DUMP_DESCRIPTOR
  #undef DUMP_DESCRIPTOR
#endif
#define DUMP_DESCRIPTOR(x,d) //dumpDescriptor(x,d)


//#define OPTIMIZE_AGGREGATE_LATENCY

namespace XMI
{
  namespace Device
  {
    namespace MU
    {
      typedef struct __mu_multicombine_statedata
      {
        MUInjFifoMessage  message;
      } mu_multicombine_statedata_t;
      class MUMulticombineModel : public Interface::MulticombineModel < MUMulticombineModel, sizeof(mu_multicombine_statedata_t) >
      {
      protected:

      public:

        /// \see XMI::Device::Interface::MulticombineModel::MulticombineModel
        MUMulticombineModel (xmi_result_t &status, MUCollDevice & device, xmi_client_t client, size_t context);

        /// \see XMI::Device::Interface::MulticombineModel::~MulticombineModel
        ~MUMulticombineModel ();

        static const bool   multicombine_model_allreduce_only          = true;
//        static const bool   multicombine_model_reduce_only             = false;
        static const size_t sizeof_msg                              = sizeof(mu_multicombine_statedata_t);
        static const size_t multicombine_model_bytes_max               = (uint32_t) -1; // metadata_t::sndlen
        static const size_t multicombine_model_connection_id_max       = (uint32_t) -1; // metadata_t::connection_id \todo 64 bit?
        static const bool   multicombine_model_available_buffers_only  = true; 
        static const bool   multicombine_model_op_support(xmi_dt, xmi_op); /// \todo add to MulticombineModel and _impl it?
        /// \see XMI::Device::Interface::MulticombineModel::postMulticombine
        xmi_result_t postMulticombine_impl(uint8_t (&state)[sizeof_msg],
                                           xmi_multicombine_t *multicombine);

      protected:

        typedef struct __attribute__((__packed__)) _metadata
        {
          uint32_t              connection_id;  ///< Collective connection id \todo expand to 64 bit?
          uint32_t              sndlen;         ///< Number of bytes of application data 
        } metadata_t;

        typedef struct recv_state
        {
          bool                                 active;
          size_t                               received_length;
          size_t                               expected_length;
          XMI::PipeWorkQueue                 * rcvpwq;          
          uint8_t                            * buffer;
          xmi_callback_t                       cb_done;
          unsigned                             connection_id;
          xmi_op                               optor;
          xmi_dt                               dtype;
          bool                                 mu_reset;
          uint8_t                              mu_dtype_optor; /**< Collective OP code.  One of:
                                                                    - MUHWI_COLLECTIVE_OP_CODE_AND
                                                                    - MUHWI_COLLECTIVE_OP_CODE_OR
                                                                    - MUHWI_COLLECTIVE_OP_CODE_XOR
                                                                    - MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_ADD
                                                                    - MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_MIN
                                                                    - MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_MAX
                                                                    - MUHWI_COLLECTIVE_OP_CODE_SIGNED_ADD
                                                                    - MUHWI_COLLECTIVE_OP_CODE_SIGNED_MIN
                                                                    - MUHWI_COLLECTIVE_OP_CODE_SIGNED_MAX
                                                                    - MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_ADD
                                                                    - MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_MIN
                                                                    - MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_MAX.
                                                                   */

          uint16_t                            mu_word_length; /**< Number of bytes in a "collective word".
                                                                   The Op_Code will be performed on elements of
                                                                   this length.  Message_Length must be a
                                                                   multiple of Word_Length.  Word_Length must
                                                                   be one of:  4, 8, 16, 32, 64, 128, 256, 512.
                                                                  */
        } recv_state_t;

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

        inline bool postPayload(MUInjFifoMessage& message,
                                unsigned connection_id,
                                void* payload,
                                size_t payload_length);
        inline bool postShortPayload(MUInjFifoMessage& message,
                                unsigned connection_id,
                                void* payload,
                                size_t payload_length);
      private:
        MUCollDevice                        & _device;
        MUDescriptorWrapper                   _wrapper_model;
        MUSPI_CollectiveMemoryFIFODescriptor  _desc_model;
        xmi_client_t                          _client;
        size_t                                _context;
        recv_state_t                          _receive_state;

      }; // XMI::Device::MU::MUMulticombineModel class

      inline void MUMulticombineModel::initializeDescriptor (MUSPI_DescriptorBase* desc,
                                                          uint64_t payloadPa,
                                                          size_t bytes)
      {
        TRACE((stderr, "<%p>:MUMulticombineModel::initializeDescriptor(%p, %p, %zd)\n", this, desc, (void *)payloadPa, bytes));
        if(_receive_state.mu_reset)
        {
          DUMP_DESCRIPTOR("initializeDescriptor() ..model before reset",&_desc_model);
          _desc_model.setOpCode(_receive_state.mu_dtype_optor);
          _desc_model.setWordLength(_receive_state.mu_word_length);
        }
        // Clone the model descriptor.
        DUMP_DESCRIPTOR("initializeDescriptor() ..model before clone",&_desc_model);
        _desc_model.clone (*desc);
        DUMP_DESCRIPTOR("initializeDescriptor() ..desc after clone",desc);

        DUMP_DESCRIPTOR("initializeDescriptor() ..before setPayload",desc);
        desc->setPayload (payloadPa, bytes);

        DUMP_DESCRIPTOR("initializeDescriptor() ..done",desc);

      }; // XMI::Device::MU::MUMulticombineModel::initializeDescriptor (MUSPI_DescriptorBase * desc,

      inline xmi_result_t MUMulticombineModel::postMulticombine_impl(uint8_t (&state)[MUMulticombineModel::sizeof_msg],
                                                               xmi_multicombine_t *multicombine)
      {
//        XMI_assert(!multicombine_model_all_sided || multicombine->src != ... not sure what I was trying to assert but think about it...
        mu_multicombine_statedata_t *state_data = (mu_multicombine_statedata_t*)&state;

        // multicombine_model_available_buffers_only semantics: If you're sending data, it must all be ready in the pwq. \todo assert?
        XMI::PipeWorkQueue *pwq = (XMI::PipeWorkQueue *)multicombine->data;
        size_t length = pwq ?pwq->bytesAvailableToConsume():0;

        void* payload = NULL;
        if(length) 
        {
            payload = (void*)pwq->bufferToConsume();
            pwq->consumeBytes(length);
        }

        _receive_state.active = true;
        _receive_state.cb_done = multicombine->cb_done;
        _receive_state.connection_id = multicombine->connection_id;
        _receive_state.rcvpwq = (XMI::PipeWorkQueue*)multicombine->results;
        _receive_state.expected_length = _receive_state.rcvpwq? _receive_state.rcvpwq->bytesAvailableToProduce():0; /// \todo assert something?
        _receive_state.buffer = _receive_state.rcvpwq? (uint8_t*)_receive_state.rcvpwq->bufferToProduce():NULL; /// \todo assert something?
        _receive_state.received_length = 0;
        if((_receive_state.dtype != multicombine->dtype) || (_receive_state.optor != multicombine->optor))
        {
          TRACE((stderr, "<%p>:MUMulticombineModel::postMulticombine_imp dt %d, op %d, support %s\n", this, 
                 multicombine->dtype,multicombine->optor,multicombine_model_op_support(multicombine->dtype,multicombine->optor)?"true":"false"));
          XMI_assert(multicombine_model_op_support(multicombine->dtype,multicombine->optor));
          _receive_state.dtype = multicombine->dtype;
          _receive_state.optor = multicombine->optor;
          /// \todo map to MU dt/op/wordlength
          _receive_state.mu_reset = true;
          _receive_state.mu_dtype_optor = MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_MIN ;
          _receive_state.mu_word_length = 4;
        }
        
        TRACE((stderr, "<%p>:MUMulticombineModel::postMulticombine_impl() connection_id %#X, data length %zd/%p/%p, results length %zd/%p/%p\n", 
               this, multicombine->connection_id, 
               length, pwq, payload, 
               _receive_state.expected_length, _receive_state.rcvpwq, _receive_state.buffer));

        // Post the multicombine to the device
        if(length < MUCollDevice::payload_size) // payload in one packet
        {
          postShortPayload(state_data->message, multicombine->connection_id,  payload, length);
        }
        else // payload in (multi-)packets
        {
          postPayload(state_data->message, multicombine->connection_id,  payload, length);
        }

        return XMI_SUCCESS;

      }; // XMI::Device::MU::MUMulticombineModel::postMulticombine_impl
      inline bool MUMulticombineModel::postShortPayload(MUInjFifoMessage &message,
                                                unsigned connection_id,
                                                void* payload,
                                                size_t payload_length)
      {
        TRACE((stderr, "<%p>:MUMulticombineModel::postShortPayload()\n", this));

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

        if(_device.nextInjectionDescriptor (&injfifo,
                                            &hwi_desc,
                                            &payloadVa,
                                            &payloadPa))
        {
          TRACE((stderr, "<%p>:MUMulticombineModel::postShortPayload().. nextInjectionDescriptor injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", this, injfifo, hwi_desc, payloadVa, payloadPa));
          MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

          // Pack the msginfo and payload into the packet payload.
          uint8_t * data = (uint8_t *) payloadVa;

          // Now the payload
          TRACE((stderr, "<%p>:MUMulticombineModel::postShortPayload()..payload memcpy(%p,%p,%zd)\n", this, data, payload, payload_length));
          if(payload_length) memcpy (data, payload, payload_length);

          // Initialize the descriptor directly in the injection fifo.
          initializeDescriptor (desc, (uint64_t) payloadPa, payload_length);

          // Put the metadata into the network header in the descriptor.
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & desc->PacketHeader;

          metadata_t *metadata = (metadata_t*) &hdr->dev.multipkt.metadata;
          DUMP_DESCRIPTOR("MUMulticombineModel::postShortPayload().. before metadata connection_id    ",desc);
          metadata->connection_id = connection_id;
          DUMP_DESCRIPTOR("MUMulticombineModel::postShortPayload().. before metadata payload_length   ",desc);
          metadata->sndlen        = payload_length;

          // Advance the injection fifo descriptor tail which actually enables
          // the MU hardware to process the descriptor and send the packet
          // on the torus.
          DUMP_DESCRIPTOR("MUMulticombineModel::postShortPayload().. before MUSPI_InjFifoAdvanceDesc()",desc);
          uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (injfifo);

          TRACE((stderr, "<%p>:MUMulticombineModel::postShortPayload().. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld\n", this, sequenceNum));

        }
        else
        {
          TRACE((stderr, "<%p>:MUMulticombineModel::postShortPayload().. nextInjectionDescriptor failed\n", this));
          // Construct a message and post to the device to be processed later.
          new (&message) MUInjFifoMessage (NULL, NULL, _client, _context);

          // Initialize the descriptor directly in the injection fifo.
          MUSPI_DescriptorBase * desc = message.getDescriptor ();
          initializeDescriptor (desc, 0, 0);

          // Enable the "single packet message" bit so that it uses my iov src buffer \todo why?
          desc->setSoftwareBit (1);

          message.setSourceBuffer (payload, payload_length);

          // Put the metadata into the network header in the descriptor. (I don't use the extra singlepkt meta bytes).
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);

          metadata_t *metadata = (metadata_t*) &hdr->dev.multipkt.metadata;
          DUMP_DESCRIPTOR("MUMulticombineModel::postShortPayload().. before metadata connection_id    ",desc);
          metadata->connection_id = connection_id;
          DUMP_DESCRIPTOR("MUMulticombineModel::postShortPayload().. before metadata payload_length   ",desc);
          metadata->sndlen        = payload_length;

          DUMP_DESCRIPTOR("MUMulticombineModel::postShortPayload().. before addToSendQ                ",desc);
          // Add this message to the send queue to be processed when there is
          // space available in the injection fifo.
          _device.addToSendQ ((QueueElem *) &message);
        }

        return true;

      }; // XMI::Device::MU::MUMulticombineModel::postShortPayload
      inline bool MUMulticombineModel::postPayload(MUInjFifoMessage &message,
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
        XMI_assert ( rc == 0 );

        uint64_t paddr = (uint64_t)memRegion.BasePa +
                         ((uint64_t)payload - (uint64_t)memRegion.BaseVa);

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

        if(_device.nextInjectionDescriptor (&injfifo,
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

          metadata_t *metadata = (metadata_t*) &hdr->dev.multipkt.metadata;
          DUMP_DESCRIPTOR("MUMulticombineModel::postPayload().. before metadata connection_id    ",desc);
          metadata->connection_id = connection_id;
          DUMP_DESCRIPTOR("MUMulticombineModel::postPayload().. before metadata payload_length   ",desc);
          metadata->sndlen        = payload_length;

          // Advance the injection fifo descriptor tail which actually enables
          // the MU hardware to process the descriptor and send the packet
          // on the torus.
          DUMP_DESCRIPTOR("MUMulticombineModel::postPayload().. before MUSPI_InjFifoAdvanceDesc()",desc);
          uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (injfifo);

          TRACE((stderr, "<%p>:MUMulticombineModel::postPayload().. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld\n", this, sequenceNum));

        }
        else
        {
          TRACE((stderr, "<%p>:MUMulticombineModel::postPayload().. nextInjectionDescriptor failed\n", this));
          // Construct a message and post to the device to be processed later.
          new (&message) MUInjFifoMessage (NULL, NULL, _client, _context);
          //message.setSourceBuffer (payload, payload_length);

          // Initialize the descriptor directly in the injection fifo.
          MUSPI_DescriptorBase * desc = message.getDescriptor ();
          initializeDescriptor (desc, paddr, payload_length);

          // Put the metadata into the network header in the descriptor.
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);

          metadata_t *metadata = (metadata_t*) &hdr->dev.multipkt.metadata;
          DUMP_DESCRIPTOR("MUMulticombineModel::postPayload().. before metadata connection_id    ",desc);
          metadata->connection_id = connection_id;
          DUMP_DESCRIPTOR("MUMulticombineModel::postPayload().. before metadata payload_length   ",desc);
          metadata->sndlen        = payload_length;

          DUMP_DESCRIPTOR("MUMulticombineModel::postPayload().. before addToSendQ                ",desc);
          // Add this message to the send queue to be processed when there is
          // space available in the injection fifo.
          _device.addToSendQ ((QueueElem *) &message);
        }

        return true;

      }; // XMI::Device::MU::MUMulticombineModel::postPayload


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
      /// \see XMI::Device::Interface::RecvFunction_t
      ///
      inline void MUMulticombineModel::processData   (metadata_t   *  metadata,
                                                      uint8_t * payload,
                                                      size_t    bytes)
      {
        TRACE((stderr, "<%p>:MUMulticombineModel::processData()\n", this));

        XMI_assert(_receive_state.active);

        // Number of bytes left to copy into the destination buffer
        size_t nleft = _receive_state.expected_length - _receive_state.received_length;

        // Number of bytes left to copy from this packet
        if(nleft > bytes) nleft = bytes;

        if(nleft)
        {
          TRACE((stderr, "<%p>:MUMulticombineModel::processData memcpy(%p,%p,%zd)\n", this, _receive_state.buffer, payload, nleft));
          memcpy (_receive_state.buffer, payload, nleft);
          //_device.read ((uint8_t *)(state->info.data.simple.addr) + nbyte, nleft, cookie);

          // Update the receive state 
          _receive_state.received_length += nleft;
          _receive_state.rcvpwq->produceBytes(nleft);

          if(_receive_state.received_length == _receive_state.expected_length)
          {
            /// \todo handle unwanted data?  stay active and toss it as it arrives

            // No more data packets will be received on this connection.
            _receive_state.active = false;

            // Invoke the receive done callback.
            if(_receive_state.cb_done.function)
              _receive_state.cb_done.function (XMI_Client_getcontext(_client, _context),
                                               _receive_state.cb_done.clientdata,
                                               XMI_SUCCESS);
          }
        }
        else XMI_abort();  /// \todo toss unwanted data? 

        return ;
      }; // XMI::Device::MU::MUMulticombineModel::processData

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
      /// \see XMI::Device::Interface::RecvFunction_t
      ///
      inline
      int MUMulticombineModel::dispatch (void   * metadata,
                                      void   * payload,
                                      size_t   bytes,
                                      void   * arg,
                                      void   * cookie)
      {
        metadata_t * m = (metadata_t*)metadata;
        TRACE ((stderr, "<%p>:MUMulticombineModel::dispatch(), bytes = %zd/%d, connection id %#X\n",arg,bytes, m->sndlen, m->connection_id));

        MUMulticombineModel * model = (MUMulticombineModel *) arg;
        model->processData(m, (uint8_t*)payload, bytes);
        return 0;
      }; // XMI::Device::MU::MUMulticombineModel::dispatch


    };   // XMI::Device::MU namespace
  };     // XMI::Device namespace
};       // XMI namespace

#undef TRACE
#undef DUMP_DESCRIPTOR

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//






