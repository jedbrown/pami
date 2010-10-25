/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUMulticastModel.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu_MUMulticastModel_h__
#define __components_devices_bgq_mu_MUMulticastModel_h__

#include <hwi/include/bqc/MU_PacketHeader.h>

#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/CollectiveMemoryFIFODescriptorXX.h>
#include <spi/include/kernel/memory.h>

#include "components/devices/MulticastModel.h"

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
      ///////////////////////////////////////////////////////////////////////////////
      // Some structures that needed to be defined outside the class
      ///////////////////////////////////////////////////////////////////////////////
      // Structure used to pass user's msgdata with alignment and padding.
      typedef struct __attribute__((__packed__)) __mu_multicast_msgdata
      {
        unsigned          msgcount;
        unsigned          msgpad[7]; /// \todo pad to 32 bytes to workaround unknown payload length problem
        pami_quad_t        msginfo;
      } mu_multicast_msgdata_t __attribute__ ((__aligned__ (16)));

      // State (request) implementation.  Caller should use uint8_t[MUMulticast::sizeof_msg]
      typedef struct __mu_multicast_statedata
      {
        struct            iovec iov[3];
        size_t            niov;
        uint8_t           msghead[sizeof(mu_multicast_msgdata_t().msgcount) + sizeof(mu_multicast_msgdata_t().msgpad)];
        ShortInjFifoMessage  message[2];
      } mu_multicast_statedata_t;

      ///////////////////////////////////////////////////////////////////////////////
      // \class MUMulticastModel
      // \brief MU collective device Multicast interface
      // \details
      //   - active message model
      //   - uses MU memfifo
      //   - global (uses global class route)
      //   - one destination task per node
      //   - does not fully support PipeWorkQueue (multicast_model_available_buffers_only)
      //   - all data must be received (\todo ease this limitation - see expected_length processing)
      ///////////////////////////////////////////////////////////////////////////////
      class MUMulticastModel : public Interface::AMMulticastModel < MUMulticastModel, MUCollDevice, sizeof(mu_multicast_statedata_t) >
      {

      protected:

      public:

        /// \see PAMI::Device::Interface::MulticastModel::MulticastModel
        MUMulticastModel (MUCollDevice & device, pami_result_t &status);

        /// \see PAMI::Device::Interface::MulticastModel::~MulticastModel
        ~MUMulticastModel ();

        /// \brief Multicast model constants/attributes
//      static const bool   multicast_model_all_sided               = false;
        static const bool   multicast_model_active_message          = true;
        static const bool   multicast_model_available_buffers_only  = true;

        static const size_t sizeof_msg                              = sizeof(mu_multicast_statedata_t);
        static const size_t multicast_model_msgcount_max            = (MUCollDevice::payload_size - (sizeof(mu_multicast_msgdata_t().msgcount) + sizeof(mu_multicast_msgdata_t().msgpad))) / sizeof(mu_multicast_msgdata_t().msginfo);
        static const size_t multicast_model_bytes_max               = (uint32_t) - 1; // metadata_t::sndlen
        static const size_t multicast_model_connection_id_max       = (uint32_t) - 1; // metadata_t::connection_id \todo 64 bit?

        /// \see PAMI::Device::Interface::MulticastModel::registerMcastRecvFunction
        pami_result_t registerMcastRecvFunction_impl(int                        dispatch_id,
                                                    pami_dispatch_multicast_function func,
                                                    void                      *arg);
        /// \see PAMI::Device::Interface::MulticastModel::postMulticast
        pami_result_t postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                        pami_multicast_t *mcast);

      protected:
        // Metadata passed in the packet header
        typedef struct __attribute__((__packed__)) _metadata
        {
          uint32_t              connection_id;  ///< Collective connection id \todo expand to 64 bit?
          uint32_t              root;           ///< Root of the collective
          uint32_t              sndlen;         ///< Number of bytes of application data
        } metadata_t;

        // Receive state (after receiving a header/metadata
        typedef struct mcast_recv_state
        {
          bool                                 active;
          size_t                               received_length;
          size_t                               expected_length;
          PAMI::PipeWorkQueue                 * rcvpwq;
          uint8_t                            * buffer;
          pami_callback_t                       cb_done;
          unsigned                             connection_id;
        } mcast_recv_state_t;

        /// \brief MU dispatch function
        inline static int dispatch (void   * metadata,
                                    void   * payload,
                                    size_t   bytes,
                                    void   * arg,
                                    void   * cookie);

        /// \brief Process header/metadata packets
        inline void processHeader (metadata_t * metadata,
                                   mu_multicast_msgdata_t  * msgdata,
                                   size_t       bytes);

        /// \brief Process user data packets
        inline void processData   (metadata_t    *  metadata,
                                   uint8_t * payload,
                                   size_t    bytes);

        /// \brief initialize a descriptor from the model
        inline void initializeDescriptor (MUSPI_DescriptorBase * desc,
                                          uint64_t               payloadPa,
                                          size_t                 bytes);

        /// \brief post a header (first packet) with metadata and possible msginfo and/or user data
        inline bool postHeader(mu_multicast_statedata_t* state,
                               ShortInjFifoMessage& message,
                               unsigned connection_id,
                               size_t total_length,
                               void* msginfo,
                               unsigned msgcount,
                               void* optional_payload = NULL,
                               size_t optional_payload_length = 0);
        /// \brief  post a packet with user data
        inline bool postData(InjFifoMessage& message,
                             unsigned connection_id,
                             void* payload,
                             size_t payload_length);

      private:
        MUCollDevice                        & _device;
        MUDescriptorWrapper                   _wrapper_model;
        MUSPI_CollectiveMemoryFIFODescriptor  _desc_model;
        pami_dispatch_multicast_function       _dispatch_function;
        void                                * _dispatch_arg;
        // We only need one receive state because we only support one active collective at a time
        mcast_recv_state_t                          _receive_state;

      }; // PAMI::Device::MU::MUMulticastModel class

      ///////////////////////////////////////////////////////////////////////////////
      // Inline implementations
      ///////////////////////////////////////////////////////////////////////////////

      inline pami_result_t MUMulticastModel::postMulticast_impl(uint8_t (&state)[MUMulticastModel::sizeof_msg],
                                                               pami_multicast_t *mcast)
      {
//      PAMI_assert(!multicast_model_all_sided || mcast->src != ... not sure what I was trying to assert but think about it...
        mu_multicast_statedata_t *state_data = (mu_multicast_statedata_t*) & state;

        // Get the source data buffer/length and validate (assert) inputs
        size_t length = mcast->bytes;
        PAMI::PipeWorkQueue *pwq = (PAMI::PipeWorkQueue *)mcast->src;

        // multicast_model_available_buffers_only semantics: If you're sending data, it must all be ready in the pwq.
        PAMI_assert((length == 0) || (multicast_model_available_buffers_only && pwq && pwq->bytesAvailableToConsume() == length));

        TRACE((stderr, "<%p>:MUMulticastModel::postMulticast_impl() dispatch %zu, connection_id %#X, msgcount %d/%p, bytes %zu/%p/%p\n",
               this, mcast->dispatch, mcast->connection_id,
               mcast->msgcount, mcast->msginfo,
               mcast->bytes, pwq, pwq ? pwq->bufferToConsume() : NULL));

        void* payload = NULL;

        if (length)
        {
          payload = (void*)pwq->bufferToConsume();
          pwq->consumeBytes(length);
        }

        // Get the msginfo buffer/length and validate (assert) inputs
        void* msgdata = (void*)mcast->msginfo;

        PAMI_assert(multicast_model_msgcount_max >= mcast->msgcount);

        // calc how big our msginfo needs to be
        size_t msgsize = sizeof(mu_multicast_msgdata_t().msgcount) + sizeof(mu_multicast_msgdata_t().msgpad) + (mcast->msgcount * sizeof(mu_multicast_msgdata_t().msginfo));
        size_t total = msgsize + mcast->bytes;

        // Post the multicast to the device in one or more packets
        if (total < MUCollDevice::payload_size)
        { // pack msginfo and payload into one (single) header packet
          postHeader(state_data, state_data->message[0], mcast->connection_id,
                     total, msgdata, mcast->msgcount, payload, length);
        }
        else
        { // post >= 2 packets- the msginfo header packet and the data (multi-)packet
          postHeader(state_data, state_data->message[0], mcast->connection_id,
                     msgsize, msgdata, mcast->msgcount, NULL, length);
          postData(state_data->message[1], mcast->connection_id,
                   payload, length);
        }

        TRACE((stderr, "<%p>:MUMulticastModel::postMulticast_impl() dispatch %zu, connection_id %#X exit\n",
               this, mcast->dispatch, mcast->connection_id));

        return PAMI_SUCCESS;

      }; // PAMI::Device::MU::MUMulticastModel::postMulticast_impl

      inline void MUMulticastModel::initializeDescriptor (MUSPI_DescriptorBase* desc,
                                                          uint64_t payloadPa,
                                                          size_t bytes)
      {
        TRACE((stderr, "<%p>:MUMulticastModel::initializeDescriptor(%p, %p, %zu)\n", this, desc, (void *)payloadPa, bytes));

        // Clone the model descriptor.
        _desc_model.clone (*desc);

        desc->setPayload (payloadPa, bytes);
        DUMP_DESCRIPTOR("initializeDescriptor() ..done", desc);

      }; // PAMI::Device::MU::MUMulticastModel::initializeDescriptor (MUSPI_DescriptorBase * desc,

      inline bool MUMulticastModel::postHeader(mu_multicast_statedata_t      * state,
                                               ShortInjFifoMessage  & message,
                                               unsigned            connection_id,
                                               size_t              total_length,
                                               void*               msginfo,
                                               unsigned            msgcount,
                                               void*               payload,
                                               size_t              payload_length)
      {
        TRACE((stderr, "<%p>:MUMulticastModel::postHeader() connection_id %u, total_length %zu, msgcount %u, payload %p, payload_length %zu\n", this,connection_id, total_length, msgcount, payload,payload_length));

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

        // Check if we can inject directly to to fifo
        if ((_device.emptySendQ ()) &&
            (_device.nextInjectionDescriptor (&injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa)))
        {
          TRACE((stderr, "<%p>:MUMulticastModel::postHeader().. nextInjectionDescriptor injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", this, injfifo, hwi_desc, payloadVa, payloadPa));
          MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

          // Pack the msginfo and (optional) payload into the packet payload.
          uint8_t * data = (uint8_t *) payloadVa;

          // First the msgcount
          *(unsigned*)data = msgcount;
          *(((unsigned*)data)+1) = 0xB0BCF001;//debug
          *(((unsigned*)data)+2) = 0xB0BCF002;//debug
          *(((unsigned*)data)+3) = 0xB0BCF003;//debug
          *(((unsigned*)data)+4) = 0xB0BCF004;//debug
          *(((unsigned*)data)+5) = 0xB0BCF005;//debug
          *(((unsigned*)data)+6) = 0xB0BCF006;//debug
          *(((unsigned*)data)+7) = 0xB0BCF007;//debug
          // Skip the msgcount and padding
          data += sizeof(mu_multicast_msgdata_t().msgcount) + sizeof(mu_multicast_msgdata_t().msgpad);

          // Now the msginfo
          size_t msglength = msgcount * sizeof(mu_multicast_msgdata_t().msginfo);
          TRACE((stderr, "<%p>:MUMulticastModel::postHeader()..msginfo memcpy(%p,%p,%zu)\n", this, data, msginfo, msglength));
          memcpy (data, msginfo, msglength);
          DUMP_HEXDATA("MUMulticastModel::postHeader()",(uint32_t*)payloadVa, (sizeof(mu_multicast_msgdata_t().msgcount) + sizeof(mu_multicast_msgdata_t().msgpad) + msglength)/sizeof(unsigned));
          data += msglength;

          // Now the (optional) payload
          TRACE((stderr, "<%p>:MUMulticastModel::postHeader()..payload memcpy(%p,%p,%zu)\n", this, data, payload, payload?payload_length:0));

          if (payload) memcpy (data, payload, payload_length);
          DUMP_HEXDATA("MUMulticastModel::postHeader()",(uint32_t*)data,payload?payload_length/sizeof(unsigned):0);

          // Initialize the descriptor directly in the injection fifo.
          initializeDescriptor (desc, (uint64_t) payloadPa, total_length);
          DUMP_HEXDATA("MUMulticastModel::postHeader() payload",(uint32_t*)payloadVa, total_length/sizeof(unsigned));

          // Put the metadata into the network header in the descriptor.
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & desc->PacketHeader;

          metadata_t *metadata = (metadata_t*) & hdr->dev.multipkt.metadata;

          metadata->connection_id = connection_id;
          metadata->root          = __global.mapping.task();
          metadata->sndlen        = payload_length;

          DUMP_HEXDATA("MUMulticastModel::postHeader() header",(uint32_t*)hdr, sizeof(MemoryFifoPacketHeader_t)/sizeof(unsigned));
          // Advance the injection fifo descriptor tail which actually enables
          // the MU hardware to process the descriptor and send the packet on the torus.
          DUMP_DESCRIPTOR("MUMulticastModel::postHeader().. before MUSPI_InjFifoAdvanceDesc()", desc);

          // We don't care when it completes the send, we are done when the broadcast back (receive) is done
          MUSPI_InjFifoAdvanceDesc (injfifo);

        }
        else
        {
          TRACE((stderr, "<%p>:MUMulticastModel::postHeader().. nextInjectionDescriptor failed\n", this));
          // Construct a message and post to the device to be processed later.
          new (&message) ShortInjFifoMessage ();

          // Initialize the descriptor in the message
          MUSPI_DescriptorBase * desc = message.getDescriptor ();
          Kernel_MemoryRegion_t kmr;
          Kernel_CreateMemoryRegion (&kmr, message.getPayload(), total_length);
          initializeDescriptor (desc, (uint64_t) kmr.BasePa, total_length);

          // Enable the "single packet message" bit so that it uses my iov src buffer
          desc->setSoftwareBit (1);

          // Set the payload to my 3 part iov: msghead, msginfo, (optional) payload
          *(unsigned*)state->msghead = msgcount;

          state->niov = 1;
          state->iov[0].iov_base = state->msghead;
          state->iov[0].iov_len = sizeof(state->msghead);
          DUMP_HEXDATA("MUMulticastModel::postHeader() msghead",(uint32_t*)state->iov[0].iov_base, state->iov[0].iov_len/sizeof(unsigned));

          if (msgcount) /// \todo if! then set a bit in metadata and don't send any msghead?
          {
            state->iov[1].iov_base = msginfo;
            state->iov[1].iov_len = msgcount * sizeof(mu_multicast_msgdata_t().msginfo);
            DUMP_HEXDATA("MUMulticastModel::postHeader() msginfo",(uint32_t*)state->iov[1].iov_base, state->iov[1].iov_len/sizeof(unsigned));
            state->niov++;
          }

          if (payload)
          {
            state->iov[state->niov].iov_base = payload;
            state->iov[state->niov].iov_len = payload_length;
            DUMP_HEXDATA("MUMulticastModel::postHeader() payload",(uint32_t*)state->iov[state->niov].iov_base, state->iov[state->niov].iov_len/sizeof(unsigned));
            state->niov++;
          }

          //message.setSourceBuffer (state->iov, state->niov);
          message.copyPayload (state->iov, state->niov);

          // Put the metadata into the network header in the descriptor.
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);

          metadata_t *metadata = (metadata_t*) & hdr->dev.singlepkt.metadata;
          metadata->connection_id = connection_id;
          metadata->root          = __global.mapping.task();
          metadata->sndlen        = payload_length;

          DUMP_DESCRIPTOR("MUMulticastModel::postHeader().. before addToSendQ                ", desc);
          // Add this message to the send queue to be processed when there is
          // space available in the injection fifo.
          _device.addToSendQ ((PAMI::Queue::Element *) &message);
        }

        TRACE((stderr, "<%p>:MUMulticastModel::postHeader() exit\n", this));

        return true;

      }; // PAMI::Device::MU::MUMulticastModel::postHeader

      inline bool MUMulticastModel::postData(InjFifoMessage &message,
                                             unsigned connection_id,
                                             void* payload,
                                             size_t payload_length)
      {
        TRACE((stderr, "<%p>:MUMulticastModel::postData() connection_id %u, payload %p, payload_length %zu\n", this,connection_id, payload, payload_length));

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

        if ((_device.emptySendQ ()) &&
            (_device.nextInjectionDescriptor (&injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa)))
        {
          TRACE((stderr, "<%p>:MUMulticastModel::postData().. nextInjectionDescriptor injfifo = %p, hwi_desc = %p, payload = %p, payloadVa = %p, payloadPa = %p, paddr = %p\n", this, injfifo, hwi_desc, payload, payloadVa, payloadPa, (void*)paddr));
          MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

          DUMP_HEXDATA("MUMulticastModel::postData() payload",(uint32_t*)payload, payload_length/sizeof(unsigned));

          // Initialize the descriptor directly in the injection fifo.
          initializeDescriptor (desc, paddr, payload_length);

          // Put the metadata into the network header in the descriptor.
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & desc->PacketHeader;

          metadata_t *metadata = (metadata_t*) & hdr->dev.multipkt.metadata;
          metadata->connection_id = connection_id;
          metadata->root          = __global.mapping.task();
          metadata->sndlen        = payload_length;

          // Advance the injection fifo descriptor tail which actually enables
          // the MU hardware to process the descriptor and send the packet
          // on the torus.
          DUMP_DESCRIPTOR("MUMulticastModel::postData().. before MUSPI_InjFifoAdvanceDesc()", desc);

          // We don't care when it completes the send, we are done when the broadcast back (receive) is done
          MUSPI_InjFifoAdvanceDesc (injfifo);

        }
        else
        {
          TRACE((stderr, "<%p>:MUMulticastModel::postData().. nextInjectionDescriptor failed\n", this));
          DUMP_HEXDATA("MUMulticastModel::postData() payload",(uint32_t*)paddr, payload_length/sizeof(unsigned));

          // Construct a message and post to the device to be processed later.
          new (&message) InjFifoMessage ();

          // Initialize the descriptor in the message
          MUSPI_DescriptorBase * desc = message.getDescriptor ();
          initializeDescriptor (desc, paddr, payload_length);

          // Put the metadata into the network header in the descriptor.
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);

          metadata_t *metadata = (metadata_t*) & hdr->dev.multipkt.metadata;
          metadata->connection_id = connection_id;
          metadata->root          = __global.mapping.task();
          metadata->sndlen        = payload_length;

          DUMP_DESCRIPTOR("MUMulticastModel::postData().. before addToSendQ                ", desc);
          // Add this message to the send queue to be processed when there is
          // space available in the injection fifo.
          _device.addToSendQ ((PAMI::Queue::Element *) &message);
        }

        return true;



      }; // PAMI::Device::MU::MUMulticastModel::postData

      inline void MUMulticastModel::processHeader (metadata_t * metadata,
                                                   mu_multicast_msgdata_t  * msgdata,
                                                   size_t       bytes)
      {
        TRACE((stderr, "<%p>:MUMulticastModel::processHeader() msgcount = %d, bytes = %zu\n", this, msgdata->msgcount, bytes));


        // Only one active (receive) connection id at a time
        PAMI_assertf(!_receive_state.active, "connection_id %#X/%#X\n", _receive_state.connection_id, metadata->connection_id);

        _receive_state.connection_id = metadata->connection_id;
        _receive_state.received_length = 0; // no bytes received yet

        // init output to expected/reasonable values (to protect ourselves from misbehaved dispatches)
        _receive_state.expected_length = metadata->sndlen;
        _receive_state.rcvpwq = NULL;
        _receive_state.cb_done.function = NULL;
        _receive_state.cb_done.clientdata = NULL;

        // Invoke the registered dispatch function.
        _dispatch_function(&msgdata->msginfo,                       // Msgdata
                           msgdata->msgcount,                       // Count of msgdata
                           metadata->connection_id,                 // Connection ID of data
                           metadata->root,                          // Sending task/root
                           metadata->sndlen,                        // Length of data sent
                           _dispatch_arg,                            // Opaque dispatch arg
                           &_receive_state.expected_length,         // [out] Length of data to receive
                           (pami_pipeworkqueue_t**) &_receive_state.rcvpwq,// [out] Where to put recv data
                           &_receive_state.cb_done                  // [out] Completion callback to invoke when data received
                          );

        TRACE((stderr, "<%p>:MUMulticastModel::processHeader() after dispatch expected_length %zu, pwq %p\n", this, _receive_state.expected_length, _receive_state.rcvpwq));

        PAMI_assert(_receive_state.expected_length == metadata->sndlen); /// \todo allow partial receives and toss unwanted data

        // No data expected? Then we're done so invoke the receive done callback.
        if (!_receive_state.expected_length)
        {
          if (_receive_state.cb_done.function)
            _receive_state.cb_done.function (_device.getContext(),
                                             _receive_state.cb_done.clientdata,
                                             PAMI_SUCCESS);
          return;
        }

        // Must be expecting some data, so receive is 'active' now.
        _receive_state.active = true;

        // multicast_model_available_buffers_only semantics: If you're receiving data then the pwq must be available
        PAMI_assert(multicast_model_available_buffers_only &&
                   (_receive_state.rcvpwq && _receive_state.rcvpwq->bytesAvailableToProduce() == _receive_state.expected_length));

        _receive_state.buffer = (uint8_t*)_receive_state.rcvpwq->bufferToProduce();

        // Is there payload beyond the msgdata in this packet?  process it now
        unsigned msgsize = sizeof(msgdata->msgcount) + sizeof(msgdata->msgpad) + (msgdata->msgcount * sizeof(msgdata->msginfo));

        if (bytes > msgsize)
        {
          // bump past the msgdata to the remaining payload
          uint8_t* payload = ((uint8_t*)msgdata) + msgsize;
          processData(metadata, payload, bytes - msgsize);
        }

        return;
      }; // PAMI::Device::MU::MUMulticastModel::processHeader

      inline void MUMulticastModel::processData   (metadata_t   *  metadata,
                                                   uint8_t * payload,
                                                   size_t    bytes)
      {
        TRACE((stderr, "<%p>:MUMulticastModel::processData() metadata %p, payload %p, bytes %zu, nleft %zu\n",
               this, metadata, payload, bytes, (_receive_state.expected_length - _receive_state.received_length)));

        DUMP_HEXDATA("MUMulticastModel::processData() payload",(uint32_t*)payload, bytes/sizeof(unsigned));

        PAMI_assertf(_receive_state.active, "connection_id %#X/%#X\n", _receive_state.connection_id, metadata->connection_id);

        // Number of bytes left to copy into the destination buffer
        size_t nleft = _receive_state.expected_length - _receive_state.received_length;

        // Number of bytes left to copy from this packet
        if (nleft > bytes) nleft = bytes;

        if (nleft) // copy data and update receive state
        {
          TRACE((stderr, "<%p>:MUMulticastModel::processData memcpy(%p,%p,%zu)\n", this, _receive_state.buffer, payload, nleft));
          memcpy (_receive_state.buffer, payload, nleft);
          DUMP_HEXDATA("MUMulticastModel::processData() recv buffer",(uint32_t*)_receive_state.buffer,nleft/sizeof(unsigned));
          //_device.read ((uint8_t *)(state->info.data.simple.addr) + nbyte, nleft, cookie);

          // Update the receive state
          _receive_state.buffer += nleft;
          _receive_state.received_length += nleft;
          _receive_state.rcvpwq->produceBytes(nleft);

          if (_receive_state.received_length == _receive_state.expected_length)
          {
            /// \todo handle unwanted data?  stay active and toss it as it arrives

            // No more data packets will be received on this connection.
            _receive_state.active = false;

            // Invoke the receive done callback.
            if (_receive_state.cb_done.function)
              _receive_state.cb_done.function (_device.getContext(),
                                               _receive_state.cb_done.clientdata,
                                               PAMI_SUCCESS);
          }
        }
        else PAMI_abortf("Unwanted data?\n");  /// \todo toss unwanted data?

        return ;
      }; // PAMI::Device::MU::MUMulticastModel::processData

      /// \see PAMI::Device::Interface::RecvFunction_t
      inline
      int MUMulticastModel::dispatch (void   * metadata,
                                      void   * payload,
                                      size_t   bytes,
                                      void   * arg,
                                      void   * cookie)
      {
        metadata_t * m = (metadata_t*)metadata;

        TRACE ((stderr, "<%p>:MUMulticastModel::dispatch(),cookie = %p, root = %d, bytes = %zu/%d, connection id %#X\n", arg, cookie, (m->root), bytes, m->sndlen, m->connection_id));
        DUMP_HEXDATA("MUMulticastModel::dispatch() metadata",(uint32_t*)metadata, 3);
        DUMP_HEXDATA("MUMulticastModel::dispatch() payload",(uint32_t*)payload, bytes/sizeof(unsigned));

        MUMulticastModel * model = (MUMulticastModel *) arg;
        /// \todo I could use two dispatches instead of an if?

        /// \note Only one receive is active at a time due to MU classroute restrictions

        // An active receive?  Must be receiving more data
        if (model->_receive_state.active)
        {
          model->processData(m, (uint8_t*)payload, bytes);
        }
        else  // Not active?  Must be (first) header packet
        {
          model->processHeader(m, (mu_multicast_msgdata_t*)payload, bytes);
        }

        return 0;
      }; // PAMI::Device::MU::MUMulticastModel::dispatch


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
