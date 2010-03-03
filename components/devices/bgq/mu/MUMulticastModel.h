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
      ///////////////////////////////////////////////////////////////////////////////
      // Some structures that needed to be defined outside the class
      ///////////////////////////////////////////////////////////////////////////////
      // Structure used to pass user's msgdata with alignment and padding.
      typedef struct __attribute__((__packed__)) __mu_multicast_msgdata
      {
        unsigned          msgcount;
        unsigned          msgpad[3];
        xmi_quad_t        msginfo;
      } mu_multicast_msgdata_t __attribute__ ((__aligned__ (16)));

      // State (request) implementation.  Caller should use uint8_t[MUMulticast::sizeof_msg]
      typedef struct __mu_multicast_statedata
      {
        struct            iovec iov[3];
        size_t            niov;
        uint8_t           msghead[sizeof(mu_multicast_msgdata_t().msgcount) + sizeof(mu_multicast_msgdata_t().msgpad)];
        MUInjFifoMessage  message[2];
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
      class MUMulticastModel : public Interface::AMMulticastModel < MUMulticastModel, sizeof(mu_multicast_statedata_t) >
      {

      protected:

      public:

        /// \see XMI::Device::Interface::MulticastModel::MulticastModel
        MUMulticastModel (xmi_result_t &status, MUCollDevice & device);

        /// \see XMI::Device::Interface::MulticastModel::~MulticastModel
        ~MUMulticastModel ();

        /// \brief Multicast model constants/attributes
//      static const bool   multicast_model_all_sided               = false;
        static const bool   multicast_model_active_message          = true;
        static const bool   multicast_model_available_buffers_only  = true;

        static const size_t sizeof_msg                              = sizeof(mu_multicast_statedata_t);
        static const size_t multicast_model_msgcount_max            = (MUCollDevice::payload_size - (sizeof(mu_multicast_msgdata_t().msgcount) + sizeof(mu_multicast_msgdata_t().msgpad))) / sizeof(mu_multicast_msgdata_t().msginfo);
        static const size_t multicast_model_bytes_max               = (uint32_t) - 1; // metadata_t::sndlen
        static const size_t multicast_model_connection_id_max       = (uint32_t) - 1; // metadata_t::connection_id \todo 64 bit?

        /// \see XMI::Device::Interface::MulticastModel::registerMcastRecvFunction
        xmi_result_t registerMcastRecvFunction_impl(int                        dispatch_id,
                                                    xmi_dispatch_multicast_fn  func,
                                                    void                      *arg);
        /// \see XMI::Device::Interface::MulticastModel::postMulticast
        xmi_result_t postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                        xmi_multicast_t *mcast);

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
          XMI::PipeWorkQueue                 * rcvpwq;
          uint8_t                            * buffer;
          xmi_callback_t                       cb_done;
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
                               MUInjFifoMessage& message,
                               unsigned connection_id,
                               size_t total_length,
                               void* msginfo,
                               unsigned msgcount,
                               void* optional_payload = NULL,
                               size_t optional_payload_length = 0);
        /// \brief  post a packet with user data
        inline bool postData(MUInjFifoMessage& message,
                             unsigned connection_id,
                             void* payload,
                             size_t payload_length);

      private:
        MUCollDevice                        & _device;
        MUDescriptorWrapper                   _wrapper_model;
        MUSPI_CollectiveMemoryFIFODescriptor  _desc_model;
        xmi_dispatch_multicast_fn             _dispatch_function;
        void                                * _dispatch_arg;
        // We only need one receive state because we only support one active collective at a time
        mcast_recv_state_t                          _receive_state;

      }; // XMI::Device::MU::MUMulticastModel class

      ///////////////////////////////////////////////////////////////////////////////
      // Inline implementations
      ///////////////////////////////////////////////////////////////////////////////

      inline xmi_result_t MUMulticastModel::postMulticast_impl(uint8_t (&state)[MUMulticastModel::sizeof_msg],
                                                               xmi_multicast_t *mcast)
      {
//      XMI_assert(!multicast_model_all_sided || mcast->src != ... not sure what I was trying to assert but think about it...
        mu_multicast_statedata_t *state_data = (mu_multicast_statedata_t*) & state;

        // Get the source data buffer/length and validate (assert) inputs
        size_t length = mcast->bytes;
        XMI::PipeWorkQueue *pwq = (XMI::PipeWorkQueue *)mcast->src;

        // multicast_model_available_buffers_only semantics: If you're sending data, it must all be ready in the pwq.
        XMI_assert((length == 0) || (multicast_model_available_buffers_only && pwq && pwq->bytesAvailableToConsume() == length));

        TRACE((stderr, "<%p>:MUMulticastModel::postMulticast_impl() dispatch %zd, connection_id %#X, msgcount %d/%p, bytes %zd/%p/%p\n",
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

        XMI_assert(multicast_model_msgcount_max >= mcast->msgcount);

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
                     msgsize, msgdata, mcast->msgcount);
          postData(state_data->message[1], mcast->connection_id,
                   payload, length);
        }

        TRACE((stderr, "<%p>:MUMulticastModel::postMulticast_impl() dispatch %zd, connection_id %#X exit\n",
               this, mcast->dispatch, mcast->connection_id));

        return XMI_SUCCESS;

      }; // XMI::Device::MU::MUMulticastModel::postMulticast_impl

      inline void MUMulticastModel::initializeDescriptor (MUSPI_DescriptorBase* desc,
                                                          uint64_t payloadPa,
                                                          size_t bytes)
      {
        TRACE((stderr, "<%p>:MUMulticastModel::initializeDescriptor(%p, %p, %zd)\n", this, desc, (void *)payloadPa, bytes));

        // Clone the model descriptor.
        _desc_model.clone (*desc);

        desc->setPayload (payloadPa, bytes);
        DUMP_DESCRIPTOR("initializeDescriptor() ..done", desc);

      }; // XMI::Device::MU::MUMulticastModel::initializeDescriptor (MUSPI_DescriptorBase * desc,

      inline bool MUMulticastModel::postHeader(mu_multicast_statedata_t      * state,
                                               MUInjFifoMessage  & message,
                                               unsigned            connection_id,
                                               size_t              total_length,
                                               void*               msginfo,
                                               unsigned            msgcount,
                                               void*               payload,
                                               size_t              payload_length)
      {
        TRACE((stderr, "<%p>:MUMulticastModel::postHeader()\n", this));

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

        // Check if we can inject directly to to fifo
        if (_device.nextInjectionDescriptor (&injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
        {
          TRACE((stderr, "<%p>:MUMulticastModel::postHeader().. nextInjectionDescriptor injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", this, injfifo, hwi_desc, payloadVa, payloadPa));
          MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

          // Pack the msginfo and (optional) payload into the packet payload.
          uint8_t * data = (uint8_t *) payloadVa;

          // First the msgcount
          *(unsigned*)data = msgcount;
          // Skip the msgcount and padding
          data += sizeof(mu_multicast_msgdata_t().msgcount) + sizeof(mu_multicast_msgdata_t().msgpad);

          // Now the msginfo
          size_t msglength = msgcount * sizeof(mu_multicast_msgdata_t().msginfo);
          TRACE((stderr, "<%p>:MUMulticastModel::postHeader()..msginfo memcpy(%p,%p,%zd)\n", this, data, msginfo, msglength));
          memcpy (data, msginfo, msglength);
          data += msglength;

          // Now the (optional) payload
          TRACE((stderr, "<%p>:MUMulticastModel::postHeader()..payload memcpy(%p,%p,%zd)\n", this, data, payload, payload_length));

          if (payload_length) memcpy (data, payload, payload_length);

          // Initialize the descriptor directly in the injection fifo.
          initializeDescriptor (desc, (uint64_t) payloadPa, total_length);

          // Put the metadata into the network header in the descriptor.
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & desc->PacketHeader;

          metadata_t *metadata = (metadata_t*) & hdr->dev.multipkt.metadata;

          metadata->connection_id = connection_id;
          metadata->root          = __global.mapping.task();
          metadata->sndlen        = payload_length;

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
          new (&message) MUInjFifoMessage ();

          // Initialize the descriptor in the message
          MUSPI_DescriptorBase * desc = message.getDescriptor ();
          initializeDescriptor (desc, 0, 0);

          // Enable the "single packet message" bit so that it uses my iov src buffer
          desc->setSoftwareBit (1);

          // Set the payload to my 3 part iov: msghead, msginfo, (optional) payload
          *(unsigned*)state->msghead = msgcount;

          state->niov = 1;
          state->iov[0].iov_base = state->msghead;
          state->iov[0].iov_len = sizeof(state->msghead);

          if (msgcount) /// \todo if! then set a bit in metadata and don't send any msghead?
          {
            state->iov[1].iov_base = msginfo;
            state->iov[1].iov_len = msgcount * sizeof(mu_multicast_msgdata_t().msginfo);
            state->niov++;
          }

          if (payload_length)
          {
            state->iov[state->niov].iov_base = payload;
            state->iov[state->niov].iov_len = payload_length;
            state->niov++;
          }

          message.setSourceBuffer (state->iov, state->niov);

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
          _device.addToSendQ ((QueueElem *) &message);
        }

        TRACE((stderr, "<%p>:MUMulticastModel::postHeader() exit\n", this));

        return true;

      }; // XMI::Device::MU::MUMulticastModel::postHeader

      inline bool MUMulticastModel::postData(MUInjFifoMessage &message,
                                             unsigned connection_id,
                                             void* payload,
                                             size_t payload_length)
      {
        TRACE((stderr, "<%p>:MUMulticastModel::postData()\n", this));

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

        if (_device.nextInjectionDescriptor (&injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
        {
          TRACE((stderr, "<%p>:MUMulticastModel::postData().. nextInjectionDescriptor injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", this, injfifo, hwi_desc, payloadVa, payloadPa));
          MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

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

          // Construct a message and post to the device to be processed later.
          new (&message) MUInjFifoMessage ();

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
          _device.addToSendQ ((QueueElem *) &message);
        }

        return true;



      }; // XMI::Device::MU::MUMulticastModel::postData

      inline void MUMulticastModel::processHeader (metadata_t * metadata,
                                                   mu_multicast_msgdata_t  * msgdata,
                                                   size_t       bytes)
      {
        TRACE((stderr, "<%p>:MUMulticastModel::processHeader() msgcount = %d\n", this, msgdata->msgcount));


        // Only one active (receive) connection id at a time
        XMI_assertf(!_receive_state.active, "connection_id %#X/%#X\n", _receive_state.connection_id, metadata->connection_id);

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
                           (xmi_pipeworkqueue_t**) &_receive_state.rcvpwq,// [out] Where to put recv data
                           &_receive_state.cb_done                  // [out] Completion callback to invoke when data received
                          );

        TRACE((stderr, "<%p>:MUMulticastModel::processHeader() after dispatch expected_length %zd, pwq %p\n", this, _receive_state.expected_length, _receive_state.rcvpwq));

        XMI_assert(_receive_state.expected_length == metadata->sndlen); /// \todo allow partial receives and toss unwanted data

        // No data expected? Then we're done so invoke the receive done callback.
        if (!_receive_state.expected_length)
        {
          if (_receive_state.cb_done.function)
            _receive_state.cb_done.function (_device.getContext(),
                                             _receive_state.cb_done.clientdata,
                                             XMI_SUCCESS);
          return;
        }

        // Must be expecting some data, so receive is 'active' now.
        _receive_state.active = true;

        // multicast_model_available_buffers_only semantics: If you're receiving data then the pwq must be available
        XMI_assert(multicast_model_available_buffers_only &&
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
      }; // XMI::Device::MU::MUMulticastModel::processHeader

      inline void MUMulticastModel::processData   (metadata_t   *  metadata,
                                                   uint8_t * payload,
                                                   size_t    bytes)
      {
        TRACE((stderr, "<%p>:MUMulticastModel::processData() metadata %p, payload %p, bytes %zd, nleft %zd\n",
               this, metadata, payload, bytes, (_receive_state.expected_length - _receive_state.received_length)));


        XMI_assertf(_receive_state.active, "connection_id %#X/%#X\n", _receive_state.connection_id, metadata->connection_id);

        // Number of bytes left to copy into the destination buffer
        size_t nleft = _receive_state.expected_length - _receive_state.received_length;

        // Number of bytes left to copy from this packet
        if (nleft > bytes) nleft = bytes;

        if (nleft) // copy data and update receive state
        {
          TRACE((stderr, "<%p>:MUMulticastModel::processData memcpy(%p,%p,%zd)\n", this, _receive_state.buffer, payload, nleft));
          memcpy (_receive_state.buffer, payload, nleft);
          //_device.read ((uint8_t *)(state->info.data.simple.addr) + nbyte, nleft, cookie);

          // Update the receive state
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
                                               XMI_SUCCESS);
          }
        }
        else XMI_abortf("Unwanted data?\n");  /// \todo toss unwanted data?

        return ;
      }; // XMI::Device::MU::MUMulticastModel::processData

      /// \see XMI::Device::Interface::RecvFunction_t
      inline
      int MUMulticastModel::dispatch (void   * metadata,
                                      void   * payload,
                                      size_t   bytes,
                                      void   * arg,
                                      void   * cookie)
      {
        metadata_t * m = (metadata_t*)metadata;

        TRACE ((stderr, "<%p>:MUMulticastModel::dispatch(), root = %d, bytes = %zd/%d, connection id %#X\n", arg, (m->root), bytes, m->sndlen, m->connection_id));

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
      }; // XMI::Device::MU::MUMulticastModel::dispatch


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
