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
      typedef struct __mu_multisync_statedata
      {
        MUInjFifoMessage  message;
      } mu_multisync_statedata_t;
      class MUMultisyncModel : public Interface::MultisyncModel < MUMultisyncModel, sizeof(mu_multisync_statedata_t) >
      {
      protected:

      public:

        /// \see XMI::Device::Interface::MultisyncModel::MultisyncModel
        MUMultisyncModel (xmi_result_t &status, MUCollDevice & device, xmi_client_t client, size_t context);

        /// \see XMI::Device::Interface::MultisyncModel::~MultisyncModel
        ~MUMultisyncModel ();

        static const size_t sizeof_msg                              = sizeof(mu_multisync_statedata_t);
        static const size_t multisync_model_connection_id_max       = (uint32_t) -1; // metadata_t::connection_id \todo 64 bit?

        /// \see XMI::Device::Interface::MultisyncModel::postMultisync
        xmi_result_t postMultisync_impl(uint8_t (&state)[sizeof_msg],
                                        xmi_multisync_t *multisync);

      protected:

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
        xmi_client_t                          _client;
        size_t                                _context;
        xmi_callback_t                        _cb_done;	

      }; // XMI::Device::MU::MUMultisyncModel class

      inline void MUMultisyncModel::initializeDescriptor (MUSPI_DescriptorBase* desc,
                                                          uint64_t payloadPa,
                                                          size_t bytes)
      {
        TRACE((stderr, "<%p>:MUMultisyncModel::initializeDescriptor(%p, %p, %zd)\n", this, desc, (void *)payloadPa, bytes));
        // Clone the model descriptor.
        DUMP_DESCRIPTOR("initializeDescriptor() ..model before clone",&_desc_model);
        _desc_model.clone (*desc);
        DUMP_DESCRIPTOR("initializeDescriptor() ..desc after clone",desc);

        DUMP_DESCRIPTOR("initializeDescriptor() ..before setPayload",desc);
        desc->setPayload (payloadPa, bytes);

        DUMP_DESCRIPTOR("initializeDescriptor() ..done",desc);

      }; // XMI::Device::MU::MUMultisyncModel::initializeDescriptor (MUSPI_DescriptorBase * desc,

      inline xmi_result_t MUMultisyncModel::postMultisync_impl(uint8_t (&state)[MUMultisyncModel::sizeof_msg],
                                                               xmi_multisync_t *multisync)
      {
        XMI_assertf(_cb_done.function == NULL && _cb_done.clientdata == NULL,"Only one global msync at a time");
        _cb_done = multisync->cb_done;

        mu_multisync_statedata_t *state_data = (mu_multisync_statedata_t*)&state;

        TRACE((stderr, "<%p>:MUMultisyncModel::postMultisync_impl() connection_id %#X\n", 
               this, multisync->connection_id));

        // Post the multisync to the device

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

        if(_device.nextInjectionDescriptor (&injfifo,
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

          metadata_t *metadata = (metadata_t*) &hdr->dev.multipkt.metadata;
          DUMP_DESCRIPTOR("MUMultisyncModel::postMsginfo().. before metadata connection_id    ",desc);
          metadata->connection_id = multisync->connection_id;

          // Advance the injection fifo descriptor tail which actually enables
          // the MU hardware to process the descriptor and send the packet
          // on the torus.
          DUMP_DESCRIPTOR("MUMultisyncModel::postMsginfo().. before MUSPI_InjFifoAdvanceDesc()",desc);
          uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (injfifo);

          TRACE((stderr, "<%p>:MUMultisyncModel::postMsginfo().. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld\n", this, sequenceNum));

        }
        else
        {
          TRACE((stderr, "<%p>:MUMultisyncModel::postMsginfo().. nextInjectionDescriptor failed\n", this));
          // Construct a message and post to the device to be processed later.
          new (&state_data->message) MUInjFifoMessage (NULL, NULL, _client, _context);

          // Initialize the descriptor directly in the injection fifo.
          MUSPI_DescriptorBase * desc = state_data->message.getDescriptor ();
          initializeDescriptor (desc, 0, 0);

          // Put the metadata into the network header in the descriptor. (I don't use the extra singlepkt meta bytes).
          MemoryFifoPacketHeader_t * hdr =
          (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);

          metadata_t *metadata = (metadata_t*) &hdr->dev.multipkt.metadata;
          DUMP_DESCRIPTOR("MUMultisyncModel::postMsginfo().. before metadata connection_id    ",desc);
          metadata->connection_id = multisync->connection_id;

          // Point the payload to the connection_id
          state_data->message.setSourceBuffer (&metadata->connection_id, 4);

          DUMP_DESCRIPTOR("MUMultisyncModel::postMsginfo().. before addToSendQ                ",desc);
          // Add this message to the send queue to be processed when there is
          // space available in the injection fifo.
          _device.addToSendQ ((QueueElem *) &state_data->message);
        }

        return XMI_SUCCESS;

      }; // XMI::Device::MU::MUMultisyncModel::postMsginfo
      inline void MUMultisyncModel::processHeader (metadata_t * metadata)
      {
        TRACE((stderr, "<%p>:MUMultisyncModel::processHeader() connection_id = %d\n", this,metadata->connection_id));

        if(_cb_done.function)
          _cb_done.function (XMI_Client_getcontext(_client, _context),
                                           _cb_done.clientdata,
                                           XMI_SUCCESS);
        else 
          TRACE((stderr,"<%p>:MUMultisyncModel::processHeader() WHY BOTHER?  connection_id = %d\n", this,metadata->connection_id));

        return;
      }; // XMI::Device::MU::MUMultisyncModel::processHeader

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
      int MUMultisyncModel::dispatch (void   * metadata,
                                      void   * payload,
                                      size_t   bytes,
                                      void   * arg,
                                      void   * cookie)
      {
        metadata_t * m = (metadata_t*)metadata;

        TRACE ((stderr, "<%p>:MUMultisyncModel::dispatch(), bytes = %zd, connection id %#X\n",arg, bytes, m->connection_id));

        MUMultisyncModel * model = (MUMultisyncModel *) arg;
        model->processHeader(m);
        return 0;
      }; // XMI::Device::MU::MUMultisyncModel::dispatch


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






