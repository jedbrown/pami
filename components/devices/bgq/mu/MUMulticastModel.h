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

            typedef struct __attribute__((__packed__)) __mu_multicast_msgdata
              {
                unsigned          msgcount;
                unsigned          msgpad[3];
                xmi_quad_t        msginfo;
              } mu_multicast_msgdata_t __attribute__ ((__aligned__ (16)));

            typedef struct __mu_multicast_statedata
              {

                struct            iovec iov[3];
                size_t            niov;
                uint8_t           msghead[sizeof(mu_multicast_msgdata_t().msgcount) + sizeof(mu_multicast_msgdata_t().msgpad)];
                MUInjFifoMessage  message[2];
              } mu_multicast_statedata_t;

            class MUMulticastModel : public Interface::AMMulticastModel < MUMulticastModel, sizeof(mu_multicast_statedata_t) >
              {

                protected:

                public:

                  /// \see XMI::Device::Interface::MulticastModel::MulticastModel
                  MUMulticastModel (xmi_result_t &status, MUCollDevice & device);

                  /// \see XMI::Device::Interface::MulticastModel::~MulticastModel
                  ~MUMulticastModel ();

//        static const bool   multicast_model_all_sided               = false;
                  static const bool   multicast_model_active_message          = true;
                  static const size_t sizeof_msg                              = sizeof(mu_multicast_statedata_t);
                  static const size_t multicast_model_msgcount_max            = (MUCollDevice::payload_size - (sizeof(mu_multicast_msgdata_t().msgcount) + sizeof(mu_multicast_msgdata_t().msgpad))) / sizeof(mu_multicast_msgdata_t().msginfo);
                  static const size_t multicast_model_bytes_max               = (uint32_t) - 1; // metadata_t::sndlen
                  static const size_t multicast_model_connection_id_max       = (uint32_t) - 1; // metadata_t::connection_id \todo 64 bit?
                  static const bool   multicast_model_available_buffers_only  = true;

                  /// \see XMI::Device::Interface::MulticastModel::registerMcastRecvFunction
                  xmi_result_t registerMcastRecvFunction_impl(int                        dispatch_id,
                                                              xmi_dispatch_multicast_fn  func,
                                                              void                      *arg);
                  /// \see XMI::Device::Interface::MulticastModel::postMulticast
                  xmi_result_t postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                                  xmi_multicast_t *mcast);

                protected:

                  typedef struct __attribute__((__packed__)) _metadata
                    {
                      uint32_t              connection_id;  ///< Collective connection id \todo expand to 64 bit?
                      uint32_t              root;           ///< Root of the collective
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
                    } recv_state_t;

                  inline static int dispatch (void   * metadata,
                                              void   * payload,
                                              size_t   bytes,
                                              void   * arg,
                                              void   * cookie);
                  inline void processData   (metadata_t    *  metadata,
                                             uint8_t * payload,
                                             size_t    bytes);
                  inline void processHeader (metadata_t * metadata,
                                             mu_multicast_msgdata_t  * msgdata,
                                             size_t       bytes);
                  inline void initializeDescriptor (MUSPI_DescriptorBase * desc,
                                                    uint64_t               payloadPa,
                                                    size_t                 bytes);

                  inline bool postPayload(MUInjFifoMessage& message,
                                          unsigned connection_id,
                                          void* payload,
                                          size_t payload_length,
                                          xmi_event_function fn,
                                          void* cookie);
                  inline bool postMsginfo(mu_multicast_statedata_t* state,
                                          MUInjFifoMessage& message,
                                          unsigned connection_id,
                                          size_t total_length,
                                          void* msginfo,
                                          unsigned msgcount,
                                          void* payload,
                                          size_t payload_length,
                                          xmi_event_function fn,
                                          void* cookie);

                private:
                  MUCollDevice                        & _device;
                  MUDescriptorWrapper                   _wrapper_model;
                  MUSPI_CollectiveMemoryFIFODescriptor  _desc_model;
                  xmi_dispatch_multicast_fn             _dispatch_function;
                  void                                * _dispatch_arg;
                  recv_state_t                          _receive_state;

              }; // XMI::Device::MU::MUMulticastModel class

            inline void MUMulticastModel::initializeDescriptor (MUSPI_DescriptorBase* desc,
                                                                uint64_t payloadPa,
                                                                size_t bytes)
            {
              TRACE((stderr, "<%p>:MUMulticastModel::initializeDescriptor(%p, %p, %zd)\n", this, desc, (void *)payloadPa, bytes));
              // Clone the model descriptor.
              DUMP_DESCRIPTOR("initializeDescriptor() ..model before clone", &_desc_model);
              _desc_model.clone (*desc);
              DUMP_DESCRIPTOR("initializeDescriptor() ..desc after clone", desc);

              DUMP_DESCRIPTOR("initializeDescriptor() ..before setPayload", desc);
              desc->setPayload (payloadPa, bytes);

              DUMP_DESCRIPTOR("initializeDescriptor() ..done", desc);

            }; // XMI::Device::MU::MUMulticastModel::initializeDescriptor (MUSPI_DescriptorBase * desc,

            inline xmi_result_t MUMulticastModel::postMulticast_impl(uint8_t (&state)[MUMulticastModel::sizeof_msg],
                                                                     xmi_multicast_t *mcast)
            {
//        XMI_assert(!multicast_model_all_sided || mcast->src != ... not sure what I was trying to assert but think about it...
              mu_multicast_statedata_t *state_data = (mu_multicast_statedata_t*) & state;

              size_t length = mcast->bytes;
              XMI::PipeWorkQueue *pwq = (XMI::PipeWorkQueue *)mcast->src;

              // multicast_model_available_buffers_only semantics: If you're sending data, it must all be ready in the pwq.
              XMI_assert((length == 0) || (multicast_model_available_buffers_only && pwq && pwq->bytesAvailableToConsume() == length));

              XMI_assert(multicast_model_msgcount_max >= mcast->msgcount);

              TRACE((stderr, "<%p>:MUMulticastModel::postMulticast_impl() dispatch %zd, connection_id %#X, msgcount %d/%p, bytes %zd/%p/%p\n",
                     this, mcast->dispatch, mcast->connection_id,
                     mcast->msgcount, mcast->msginfo,
                     mcast->bytes, pwq, pwq ? pwq->bufferToConsume() : NULL));

              // Since this is global multicast, we can use ourself as the target rank (in ctor) for the recFifoId

//      // Use first destination to set our reception fifo
//      XMI::Topology *dstTopology = (XMI::Topology*)mcast->dst_participants;
//      xmi_task_t target_rank = dstTopology->index2Rank(0);
//      size_t addr[BGQ_TDIMS + BGQ_LDIMS];
//      TRACE((stderr, "<%p>:MUMulticastModel::postMulticast_impl() .. %p->getRecFifoIdForDescriptor() target_rank %zd\n", this, &_device, (size_t)target_rank));
//      __global.mapping.task2global ((xmi_task_t)target_rank, addr);
//      TRACE((stderr, "<%p>:MUMulticastModel::postMulticast_impl() .. %p->getRecFifoIdForDescriptor(%zd) target_rank %zd\n", this, &_device, addr[5], (size_t)target_rank));
//      // Assuming p is the recv grp id
//      uint32_t recFifoId = _device.getRecFifoIdForDescriptor(addr[5]);

              void* payload = NULL;

              if (length)
                {
                  payload = (void*)pwq->bufferToConsume();
                  pwq->consumeBytes(length);
                }

              void* msgdata = (void*)mcast->msginfo;

              //calc how big our msginfo needs to be
              size_t msgsize = sizeof(mu_multicast_msgdata_t().msgcount) + sizeof(mu_multicast_msgdata_t().msgpad) + (mcast->msgcount * sizeof(mu_multicast_msgdata_t().msginfo));
              size_t total = msgsize + mcast->bytes;

              // Post the multicast to the device

              if (total < MUCollDevice::payload_size) // pack msginfo and payload into one packet
                {
                  postMsginfo(state_data, state_data->message[0], mcast->connection_id,
                              total, msgdata, mcast->msgcount, payload, length,
                              NULL, NULL);//mcast->cb_done.function, mcast->cb_done.clientdata);
                  // \todo look at dst_participants to decide when to call cb_done? send or receive? semantic question
                }
              else // two messages - the msginfo packet and the payload (multi-)packet
                {
                  postMsginfo(state_data, state_data->message[0], mcast->connection_id,
                              msgsize, msgdata, mcast->msgcount, NULL, 0,
                              NULL, NULL);
                  // \todo look at dst_participants to decide when to call cb_done? send or receive? semantic question
                  postPayload(state_data->message[1], mcast->connection_id,
                              payload, length,
                              NULL, NULL);//mcast->cb_done.function, mcast->cb_done.clientdata);
                  // \todo look at dst_participants to decide when to call cb_done? send or receive? semantic question
                }


              return XMI_SUCCESS;

            }; // XMI::Device::MU::MUMulticastModel::postMulticast_impl

            inline bool MUMulticastModel::postMsginfo(mu_multicast_statedata_t      * state,
                                                      MUInjFifoMessage  & message,
                                                      unsigned            connection_id,
                                                      size_t              total_length,
                                                      void*               msginfo,
                                                      unsigned            msgcount,
                                                      void*               payload,
                                                      size_t              payload_length,
                                                      xmi_event_function  fn,
                                                      void              * cookie)
            {
              TRACE((stderr, "<%p>:MUMulticastModel::postMsginfo()\n", this));

              MUSPI_InjFifo_t    * injfifo;
              MUHWI_Descriptor_t * hwi_desc;
              void               * payloadVa;
              void               * payloadPa;

              if (_device.nextInjectionDescriptor (&injfifo,
                                                   &hwi_desc,
                                                   &payloadVa,
                                                   &payloadPa))
                {
                  TRACE((stderr, "<%p>:MUMulticastModel::postMsginfo().. nextInjectionDescriptor injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", this, injfifo, hwi_desc, payloadVa, payloadPa));
                  MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

                  // Pack the msginfo and payload into the packet payload.
                  uint8_t * data = (uint8_t *) payloadVa;

                  // First the msgcount
                  *(unsigned*)data = msgcount;
                  // Skip the msgcount and padding
                  data += sizeof(mu_multicast_msgdata_t().msgcount) + sizeof(mu_multicast_msgdata_t().msgpad);

                  // Now the msginfo
                  size_t msglength = msgcount * sizeof(mu_multicast_msgdata_t().msginfo);
                  TRACE((stderr, "<%p>:MUMulticastModel::postMsginfo()..msginfo memcpy(%p,%p,%zd)\n", this, data, msginfo, msglength));
                  memcpy (data, msginfo, msglength);
                  data += msglength;

                  // Now the payload
                  TRACE((stderr, "<%p>:MUMulticastModel::postMsginfo()..payload memcpy(%p,%p,%zd)\n", this, data, payload, payload_length));

                  if (payload_length) memcpy (data, payload, payload_length);

                  // Initialize the descriptor directly in the injection fifo.
                  initializeDescriptor (desc, (uint64_t) payloadPa, total_length);


                  // Put the metadata into the network header in the descriptor.
                  MemoryFifoPacketHeader_t * hdr =
                    (MemoryFifoPacketHeader_t *) & desc->PacketHeader;

                  metadata_t *metadata = (metadata_t*) & hdr->dev.multipkt.metadata;

                  DUMP_DESCRIPTOR("MUMulticastModel::postMsginfo().. before metadata connection_id    ", desc);

                  metadata->connection_id = connection_id;

                  DUMP_DESCRIPTOR("MUMulticastModel::postMsginfo().. before metadata root             ", desc);

                  metadata->root          = __global.mapping.task();

                  DUMP_DESCRIPTOR("MUMulticastModel::postMsginfo().. before metadata payload_length   ", desc);

                  metadata->sndlen        = payload_length;

                  // Advance the injection fifo descriptor tail which actually enables
                  // the MU hardware to process the descriptor and send the packet
                  // on the torus.
                  DUMP_DESCRIPTOR("MUMulticastModel::postMsginfo().. before MUSPI_InjFifoAdvanceDesc()", desc);

                  uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (injfifo);

                  TRACE((stderr, "<%p>:MUMulticastModel::postMsginfo().. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld, fn = %p\n", this, sequenceNum, fn));

                  if (fn != NULL)
                    {
#ifndef OPTIMIZE_AGGREGATE_LATENCY
                      // Check if the descriptor is done.
                      uint32_t rc = MUSPI_CheckDescComplete (injfifo, sequenceNum);
                      TRACE((stderr, "<%p>:MUMulticastModel::postMsginfo().. after MUSPI_CheckDescComplete(), rc = %d, fn = %p\n", this, rc, fn));

                      if (rc == 1)
                        {
                          fn (_device.getContext(), cookie, XMI_SUCCESS); // Descriptor is done...notify.
                        }
                      else
#endif
                        {
                          TRACE((stderr, "<%p>:MUMulticastModel::postMsginfo().. descriptor is not done, create message (%p) and add to send queue\n", this, &message));
                          // The descriptor is not done (or was not checked). Save state
                          // information so that the progress of the decriptor can be checked
                          // later and the callback will be invoked when the descriptor is
                          // complete.
                          new (&message) MUInjFifoMessage (fn, cookie, _device.getContext(), sequenceNum);

                          // Queue it.
                          _device.addToDoneQ (message.getWrapper());
                        }
                    }
                }
              else
                {
                  TRACE((stderr, "<%p>:MUMulticastModel::postMsginfo().. nextInjectionDescriptor failed\n", this));
                  // Construct a message and post to the device to be processed later.
                  new (&message) MUInjFifoMessage (fn, cookie, _device.getContext());

                  // Initialize the descriptor directly in the injection fifo.
                  MUSPI_DescriptorBase * desc = message.getDescriptor ();
                  initializeDescriptor (desc, 0, 0);

                  // Enable the "single packet message" bit so that it uses my iov src buffer \todo why?
                  desc->setSoftwareBit (1);

                  // Set the payload to my 3 part iov: msghead, msginfo, payload
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

                  // Put the metadata into the network header in the descriptor. (I don't use the extra singlepkt meta bytes).
                  MemoryFifoPacketHeader_t * hdr =
                    (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);

                  metadata_t *metadata = (metadata_t*) & hdr->dev.multipkt.metadata;
                  DUMP_DESCRIPTOR("MUMulticastModel::postMsginfo().. before metadata connection_id    ", desc);
                  metadata->connection_id = connection_id;
                  DUMP_DESCRIPTOR("MUMulticastModel::postMsginfo().. before metadata root             ", desc);
                  metadata->root          = __global.mapping.task();
                  DUMP_DESCRIPTOR("MUMulticastModel::postMsginfo().. before metadata payload_length   ", desc);
                  metadata->sndlen        = payload_length;

                  DUMP_DESCRIPTOR("MUMulticastModel::postMsginfo().. before addToSendQ                ", desc);
                  // Add this message to the send queue to be processed when there is
                  // space available in the injection fifo.
                  _device.addToSendQ ((QueueElem *) &message);
                }

              return true;

            }; // XMI::Device::MU::MUMulticastModel::postMsginfo

            inline bool MUMulticastModel::postPayload(MUInjFifoMessage &message,
                                                      unsigned connection_id,
                                                      void* payload,
                                                      size_t payload_length,
                                                      xmi_event_function  fn,
                                                      void               *cookie)
            {
              TRACE((stderr, "<%p>:MUMulticastModel::postPayload()\n", this));

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
                  TRACE((stderr, "<%p>:MUMulticastModel::postPayload().. nextInjectionDescriptor injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", this, injfifo, hwi_desc, payloadVa, payloadPa));
                  MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

                  // Initialize the descriptor directly in the injection fifo.
                  initializeDescriptor (desc, paddr, payload_length);

                  // Put the metadata into the network header in the descriptor.
                  MemoryFifoPacketHeader_t * hdr =
                    (MemoryFifoPacketHeader_t *) & desc->PacketHeader;

                  metadata_t *metadata = (metadata_t*) & hdr->dev.multipkt.metadata;
                  DUMP_DESCRIPTOR("MUMulticastModel::postPayload().. before metadata connection_id    ", desc);
                  metadata->connection_id = connection_id;
                  DUMP_DESCRIPTOR("MUMulticastModel::postPayload().. before metadata root             ", desc);
                  metadata->root          = __global.mapping.task();
                  DUMP_DESCRIPTOR("MUMulticastModel::postPayload().. before metadata payload_length   ", desc);
                  metadata->sndlen        = payload_length;

                  // Advance the injection fifo descriptor tail which actually enables
                  // the MU hardware to process the descriptor and send the packet
                  // on the torus.
                  DUMP_DESCRIPTOR("MUMulticastModel::postPayload().. before MUSPI_InjFifoAdvanceDesc()", desc);
                  uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (injfifo);

                  TRACE((stderr, "<%p>:MUMulticastModel::postPayload().. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld, fn = %p\n", this, sequenceNum, fn));

                  if (fn != NULL)
                    {
#ifndef OPTIMIZE_AGGREGATE_LATENCY
                      // Check if the descriptor is done.

                      uint32_t rc = MUSPI_CheckDescComplete (injfifo, sequenceNum);
                      TRACE((stderr, "<%p>:MUMulticastModel::postPayload().. after MUSPI_CheckDescComplete(), rc = %d, fn = %p\n", this, rc, fn));

                      if (rc == 1)
                        {
                          fn (_device.getContext(), cookie, XMI_SUCCESS); // Descriptor is done...notify.
                        }
                      else
#endif
                        {
                          TRACE((stderr, "<%p>:MUMulticastModel::postPayload().. descriptor is not done, create message (%p) and add to send queue\n", this, &message));
                          // The descriptor is not done (or was not checked). Save state
                          // information so that the progress of the decriptor can be checked
                          // later and the callback will be invoked when the descriptor is
                          // complete.
                          new (&message) MUInjFifoMessage (fn, cookie, _device.getContext(), sequenceNum);

                          // Queue it.
                          _device.addToDoneQ (message.getWrapper());
                        }
                    }
                }
              else
                {
                  TRACE((stderr, "<%p>:MUMulticastModel::postPayload().. nextInjectionDescriptor failed\n", this));
                  // Construct a message and post to the device to be processed later.
                  new (&message) MUInjFifoMessage (fn, cookie, _device.getContext());
                  //message.setSourceBuffer (payload, payload_length);

                  // Initialize the descriptor directly in the injection fifo.
                  MUSPI_DescriptorBase * desc = message.getDescriptor ();
                  initializeDescriptor (desc, paddr, payload_length);

                  // Put the metadata into the network header in the descriptor.
                  MemoryFifoPacketHeader_t * hdr =
                    (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);

                  metadata_t *metadata = (metadata_t*) & hdr->dev.multipkt.metadata;
                  DUMP_DESCRIPTOR("MUMulticastModel::postPayload().. before metadata connection_id    ", desc);
                  metadata->connection_id = connection_id;
                  DUMP_DESCRIPTOR("MUMulticastModel::postPayload().. before metadata root             ", desc);
                  metadata->root          = __global.mapping.task();
                  DUMP_DESCRIPTOR("MUMulticastModel::postPayload().. before metadata payload_length   ", desc);
                  metadata->sndlen        = payload_length;

                  DUMP_DESCRIPTOR("MUMulticastModel::postPayload().. before addToSendQ                ", desc);
                  // Add this message to the send queue to be processed when there is
                  // space available in the injection fifo.
                  _device.addToSendQ ((QueueElem *) &message);
                }

              return true;



            }; // XMI::Device::MU::MUMulticastModel::postPayload

            inline void MUMulticastModel::processHeader (metadata_t * metadata,
                                                         mu_multicast_msgdata_t  * msgdata,
                                                         size_t       bytes)
            {
              TRACE((stderr, "<%p>:MUMulticastModel::processHeader() msgcount = %d\n", this, msgdata->msgcount));


              // Only one active (receive) connection id at a time
              XMI_assertf(!_receive_state.active, "connection_id %#X/%#X\n", _receive_state.connection_id, metadata->connection_id);

              _receive_state.active = true;
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

              // Done? Invoke the receive done callback. \todo semantics? no data and cb_done?

              if (!_receive_state.expected_length && _receive_state.cb_done.function)
                _receive_state.cb_done.function (_device.getContext(),
                                                 _receive_state.cb_done.clientdata,
                                                 XMI_SUCCESS);

              // multicast_model_available_buffers_only semantics: If you're receiving data (expected_length > 0) then the pwq must be available
              XMI_assert(multicast_model_available_buffers_only &&
                         ((_receive_state.expected_length && _receive_state.rcvpwq && _receive_state.rcvpwq->bytesAvailableToProduce() == _receive_state.expected_length) ||
                          (_receive_state.expected_length == 0)));

              XMI_assert(_receive_state.expected_length == metadata->sndlen); /// \todo allow partial receives and toss unwanted data

              _receive_state.buffer = _receive_state.expected_length ? (uint8_t*)_receive_state.rcvpwq->bufferToProduce() : NULL;

              // Is there payload beyond the msgdata?  process it
              unsigned msgsize = sizeof(msgdata->msgcount) + sizeof(msgdata->msgpad) + (msgdata->msgcount * sizeof(msgdata->msginfo));

              if (bytes > msgsize)
                {
                  // bump past the msgdata to the remaining payload
                  uint8_t* payload = ((uint8_t*)msgdata) + msgsize;
                  processData(metadata, payload, bytes - msgsize);
                }

              return;
            }; // XMI::Device::MU::MUMulticastModel::processHeader

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
            inline void MUMulticastModel::processData   (metadata_t   *  metadata,
                                                         uint8_t * payload,
                                                         size_t    bytes)
            {
              TRACE((stderr, "<%p>:MUMulticastModel::processData()\n", this));


              XMI_assertf(_receive_state.active, "connection_id %#X/%#X\n", _receive_state.connection_id, metadata->connection_id);

              // Number of bytes left to copy into the destination buffer
              size_t nleft = _receive_state.expected_length - _receive_state.received_length;

              // Number of bytes left to copy from this packet

              if (nleft > bytes) nleft = bytes;

              if (nleft)
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
              else XMI_abort();  /// \todo toss unwanted data?

              return ;
            }; // XMI::Device::MU::MUMulticastModel::processData

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

              if (model->_receive_state.active)
                {
                  model->processData(m, (uint8_t*)payload, bytes);
                }
              else
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






