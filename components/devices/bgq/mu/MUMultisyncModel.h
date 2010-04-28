/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUMultisyncModel.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu_MUMultisyncModel_h__
#define __components_devices_bgq_mu_MUMultisyncModel_h__

#include <hwi/include/bqc/MU_PacketHeader.h>

#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/CollectiveMemoryFIFODescriptorXX.h>
#include <spi/include/kernel/memory.h>

#include "components/devices/MultisyncModel.h"

#include "components/devices/bgq/mu/MUCollDevice.h"
#include "components/devices/bgq/mu/msg/InjFifoMessage.h"
#include "components/devices/bgq/mu/Dispatch.h"
#include "components/devices/bgq/mu/MUDescriptorWrapper.h"

#include <pami.h>
#include "Global.h"
#include "PipeWorkQueue.h"
#include "Topology.h"

#ifdef TRACE
  #undef TRACE
#endif
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
      // Receive state
      //typedef struct msync_recv_state
      class msync_recv_state_t : public Queue::Element
      {
      public:
        inline msync_recv_state_t() :
        Queue::Element ()
        {
        };
        unsigned                             connection_id;
        pami_callback_t                       cb_done;
      };

      // State (request) implementation.  Caller should use uint8_t[MUMultisync::sizeof_msg]
      typedef struct __mu_multisync_statedata
      {
        InjFifoMessage          message;
        msync_recv_state_t      receive_state;
      } mu_multisync_statedata_t;

      ///////////////////////////////////////////////////////////////////////////////
      // \class MUMultisyncModel
      // \brief MU collective device Multisync interface
      // \details
      //   - all sided
      //   - uses MU memfifo
      //   - global (uses global class route)
      //   - one destination task per node
      ///////////////////////////////////////////////////////////////////////////////
      class MUMultisyncModel : public Interface::MultisyncModel < MUMultisyncModel, MUCollDevice, sizeof(mu_multisync_statedata_t) >
      {

      protected:

      public:

        /// \see PAMI::Device::Interface::MultisyncModel::MultisyncModel
        MUMultisyncModel (MUCollDevice & device, pami_result_t &status);

        /// \see PAMI::Device::Interface::MultisyncModel::~MultisyncModel
        ~MUMultisyncModel ();

        static const size_t sizeof_msg                              = sizeof(mu_multisync_statedata_t);
        static const size_t multisync_model_connection_id_max       = (uint32_t) - 1; // metadata_t::connection_id \todo 64 bit?

        /// \see PAMI::Device::Interface::MultisyncModel::postMultisync
        pami_result_t postMultisync_impl(uint8_t (&state)[sizeof_msg],
                                        pami_multisync_t *multisync);

      protected:

        // Metadata passed in the packet header
        typedef struct __attribute__((__packed__)) _metadata
        {
          uint32_t              connection_id;  ///< Collective connection id \todo expand to 64 bit?
        } metadata_t;

        inline static int dispatch (void   * metadata,
                                    void   * payload,
                                    size_t   bytes,
                                    void   * arg,
                                    void   * cookie);
        inline void processHeader (metadata_t * metadata);
        inline void initializeDescriptor (MUSPI_DescriptorBase * desc,
                                          uint64_t               payloadPa,
                                          size_t                 bytes);

      private:
        MUCollDevice                        & _device;
        MUDescriptorWrapper                   _wrapper_model;
        MUSPI_CollectiveMemoryFIFODescriptor  _desc_model;
        Queue                                 _recvQ;
        //std::map<unsigned,msync_recv_state_t*, std::less<unsigned>, __gnu_cxx::malloc_allocator<std::pair<const unsigned,msync_recv_state_t*> > >      _recvQ;            // _recvQ[connection_id]

      }; // PAMI::Device::MU::MUMultisyncModel class

      inline void MUMultisyncModel::initializeDescriptor (MUSPI_DescriptorBase* desc,
                                                          uint64_t payloadPa,
                                                          size_t bytes)
      {
        TRACE((stderr, "<%p>:MUMultisyncModel::initializeDescriptor(%p, %p, %zu)\n", this, desc, (void *)payloadPa, bytes));
        // Clone the model descriptor.
        _desc_model.clone (*desc);
        desc->setPayload (payloadPa, bytes);

        DUMP_DESCRIPTOR("initializeDescriptor() ..done", desc);

      }; // PAMI::Device::MU::MUMultisyncModel::initializeDescriptor (MUSPI_DescriptorBase * desc,

      inline pami_result_t MUMultisyncModel::postMultisync_impl(uint8_t (&state)[MUMultisyncModel::sizeof_msg],
                                                               pami_multisync_t *multisync)
      {
        mu_multisync_statedata_t *state_data = (mu_multisync_statedata_t*) & state;

        state_data->receive_state.cb_done       = multisync->cb_done;
        state_data->receive_state.connection_id = multisync->connection_id;
        _recvQ.pushTail(& state_data->receive_state);

        TRACE((stderr, "<%p>:MUMultisyncModel::postMultisync_impl() connection_id %#X\n",
               this, multisync->connection_id));

        // Post the multisync to the device

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

        if (_device.nextInjectionDescriptor (&injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
        {
          TRACE((stderr, "<%p>:MUMultisyncModel::postMsginfo().. nextInjectionDescriptor injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", this, injfifo, hwi_desc, payloadVa, payloadPa));
          MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

          // Initialize the descriptor directly in the injection fifo.
          // (we don't really care what the data is at payloadPa for the allreduce)
          initializeDescriptor (desc, (uint64_t) payloadPa, 4);

          // Put the metadata into the network header in the descriptor.
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & desc->PacketHeader;

          metadata_t *metadata = (metadata_t*) & hdr->dev.singlepkt.metadata;
          metadata->connection_id = multisync->connection_id;

          // Advance the injection fifo descriptor tail which actually enables
          // the MU hardware to process the descriptor and send the packet
          // on the torus.
          DUMP_DESCRIPTOR("MUMultisyncModel::postMsginfo().. before MUSPI_InjFifoAdvanceDesc()", desc);
          MUSPI_InjFifoAdvanceDesc (injfifo);
        }
        else
        {
          TRACE((stderr, "<%p>:MUMultisyncModel::postMsginfo().. nextInjectionDescriptor failed\n", this));
          // Construct a message and post to the device to be processed later.
          new (&state_data->message) InjFifoMessage (NULL, NULL, _device.getContext());

          // Initialize the descriptor directly in the injection fifo.
          MUSPI_DescriptorBase * desc = state_data->message.getDescriptor ();
          initializeDescriptor (desc, 0, 0);

          // Put the metadata into the network header in the descriptor.
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);

          metadata_t *metadata = (metadata_t*) & hdr->dev.singlepkt.metadata;
          metadata->connection_id = multisync->connection_id;

          // Point the payload to the connection_id
          //state_data->message.setSourceBuffer (&metadata->connection_id, 4);
          Kernel_MemoryRegion_t kmr;
          Kernel_CreateMemoryRegion (&kmr, &metadata->connection_id, 4);
          desc->setPayload((uint64_t) kmr.BasePa, 4);

          DUMP_DESCRIPTOR("MUMultisyncModel::postMsginfo().. before addToSendQ                ", desc);
          // Add this message to the send queue to be processed when there is
          // space available in the injection fifo.
          _device.addToSendQ ((PAMI::Queue::Element *) &state_data->message);
        }

        return PAMI_SUCCESS;

      }; // PAMI::Device::MU::MUMultisyncModel::postMsginfo

      inline void MUMultisyncModel::processHeader (metadata_t * metadata)
      {
        TRACE((stderr, "<%p>:MUMultisyncModel::processHeader() connection_id = %d\n", this, metadata->connection_id));

        msync_recv_state_t* receive_state = (msync_recv_state_t*)_recvQ.peekHead();
        // probably the head, but (unlikely) search if it isn't
        while(receive_state && receive_state->connection_id != metadata->connection_id)
          receive_state = (msync_recv_state_t*)_recvQ.nextElem(receive_state);
        PAMI_assert(receive_state); // all-sided and sync'd by MU so this shouldn't be unexpected data

        if (receive_state->cb_done.function)
          receive_state->cb_done.function (_device.getContext(),
                                           receive_state->cb_done.clientdata,
                                           PAMI_SUCCESS);
        else
          TRACE((stderr, "<%p>:MUMultisyncModel::processHeader() WHY BOTHER?  connection_id = %d\n", this, metadata->connection_id));

        _recvQ.deleteElem(receive_state);

        return;
      }; // PAMI::Device::MU::MUMultisyncModel::processHeader

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
      int MUMultisyncModel::dispatch (void   * metadata,
                                      void   * payload,
                                      size_t   bytes,
                                      void   * arg,
                                      void   * cookie)
      {
        metadata_t * m = (metadata_t*)metadata;

        TRACE ((stderr, "<%p>:MUMultisyncModel::dispatch(), bytes = %zu, connection id %#X\n", arg, bytes, m->connection_id));
        DUMP_HEXDATA("MUMultisyncModel::dispatch()",(uint32_t*)metadata, 3);

        MUMultisyncModel * model = (MUMultisyncModel *) arg;
        model->processHeader(m);
        return 0;
      }; // PAMI::Device::MU::MUMultisyncModel::dispatch


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
