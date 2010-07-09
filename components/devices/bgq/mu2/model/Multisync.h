/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/devices/bgq/mu2/model/Multisync.h
/// \brief Simple Multisync protocol
///
/// C++ templates require all source code to be #include'd from a header file.
///
#ifndef __components_devices_bgq_mu2_model_Multisync_h__
#define __components_devices_bgq_mu2_model_Multisync_h__

#include <map>

#include "components/devices/MultisyncModel.h"

#include "components/devices/bgq/mu2/model/AllreducePacketModel.h"
#include "components/memory/MemoryAllocator.h"

#include "components/devices/bgq/mu2/trace.h"
#define DO_TRACE_ENTEREXIT 1
#define DO_TRACE_DEBUG     1

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {

      ///////////////////////////////////////////////////////////////////////////////
      // \class MultisyncModel
      // \brief MU collective Multisync interface
      // \details
      //   - active message model
      //   - uses MU memfifo
      //   - global (uses global class route)
      //   - one destination task per node
      //   - does not fully support PipeWorkQueue (Multisync_model_available_buffers_only)
      ///////////////////////////////////////////////////////////////////////////////
      template <bool T_Msgdata_support, bool T_PWQ_support>
      class MultisyncModel : public Interface::MultisyncModel < MultisyncModel<T_Msgdata_support,T_PWQ_support>, MU::Context, 2048 /*sizeof(state_data_t)*/ >
      {

      protected:

        typedef uint8_t packet_state_t[AllreducePacketModel::packet_model_state_bytes];

        // Metadata passed in the (single-packet) header
        typedef struct __attribute__((__packed__)) 
        {
          uint32_t              connection_id;  ///< Collective connection id
          uint32_t              debug;                                           
        } header_metadata_t;

        // State (request) implementation.  Callers should use uint8_t[MU::MultisyncModel::sizeof_msg]
        typedef struct 
        {
          packet_state_t          pkt;          ///< packet send state memory

         header_metadata_t       header_metadata;

          pami_callback_t         cb_done;
        } state_data_t;


      public:

        /// \see PAMI::Device::Interface::MultisyncModel::~MultisyncModel
        ~MultisyncModel () {};

        /// \brief Multisync model constants/attributes  
//      static const bool   Multisync_model_all_sided               = false;
        static const bool   Multisync_model_active_message          = true;
        static const bool   Multisync_model_available_buffers_only  = true;

        static const size_t sizeof_msg                              = 2048 /*sizeof(state_data_t)*/;
        static const size_t packet_model_payload_bytes              = AllreducePacketModel::packet_model_payload_bytes;
        static const size_t packet_model_immediate_max              = AllreducePacketModel::packet_model_immediate_max;

        static const size_t Multisync_model_msgcount_max            = (packet_model_payload_bytes /*or packet_model_immediate_max*/ / sizeof(pami_quad_t));
        static const size_t Multisync_model_bytes_max               = (uint32_t) - 1; // protocol_metadata_t::sndlen
        static const size_t Multisync_model_connection_id_max       = (uint32_t) - 1; // protocol_metadata_t::connection_id \todo 64 bit?

        /// \see PAMI::Device::Interface::MultisyncModel::postMultisync
        pami_result_t postMultisync_impl(uint8_t (&state)[MultisyncModel<T_Msgdata_support,T_PWQ_support>::sizeof_msg],
                                         pami_multisync_t *msync);

      protected:
        static unsigned getNextDispatch()
        {
          static unsigned _id = 0x81;
          TRACE_FN_ENTER();
          TRACE_FORMAT("%u\n", (_id+1)&&0xFF);
          TRACE_FN_EXIT();
          return ++_id&&0xFF;
        }
        /// \brief MU dispatch function
        inline static int dispatch_header (void   * metadata,
                                           void   * payload,
                                           size_t   bytes,
                                           void   * arg,
                                           void   * cookie);

      private:
        MU::Context                                & _device;
        pami_task_t                                  _task_id;

        unsigned                                     _route; /// \todo get route from geometry? not member data

        AllreducePacketModel                         _header_model;

//        T_Connection                                 _connection; ///\todo ConnectionArray isn't appropriate...
        std::map<unsigned,state_data_t *>             _connection;
        unsigned _debug;

      public:

        /// \see PAMI::Device::Interface::MultisyncModel::MultisyncModel
        MultisyncModel (MU::Context & device, pami_result_t &status) :
        Interface::MultisyncModel < MultisyncModel<T_Msgdata_support,T_PWQ_support>, MU::Context, 2048 /*sizeof(state_data_t)*/ > (device, status),
        _device (device),
        _task_id(__global.mapping.task()),
        _route(0), /// \todo class route support
        _header_model (device),
          _debug(0x11*(_task_id+1))
        //        _connection (device)
        {
          TRACE_FN_ENTER();
          // ----------------------------------------------------------------
          // Compile-time assertions
          // ----------------------------------------------------------------

          // This protocol only works with reliable networks.
          COMPILE_TIME_ASSERT(AllreducePacketModel::reliable_packet_model == true);

          // This protocol only works with deterministic models.
          COMPILE_TIME_ASSERT(AllreducePacketModel::deterministic_packet_model == true);

          // Assert that the size of the packet metadata area is large
          // enough to transfer the eager match information. This is used in the
          // various postMultiPacket() calls to transfer long header and data
          // messages.
          COMPILE_TIME_ASSERT(sizeof(pami_endpoint_t) <= AllreducePacketModel::packet_model_multi_metadata_bytes);

          // Assert that the size of the packet payload area is large
          // enough to transfer a single virtual address. This is used in
          // the postPacket() calls to transfer the ack information.
          COMPILE_TIME_ASSERT(sizeof(void *) <= AllreducePacketModel::packet_model_payload_bytes);

          // ----------------------------------------------------------------
          // Compile-time assertions (end)
          // ----------------------------------------------------------------
          status = _header_model.init (getNextDispatch(),
                                       dispatch_header, this,
                                       NULL,NULL);

          status = PAMI_SUCCESS;
          TRACE_FN_EXIT();
        }
      };

      template <bool T_Msgdata_support, bool T_PWQ_support>
      inline pami_result_t MultisyncModel<T_Msgdata_support,T_PWQ_support>::postMultisync_impl(uint8_t (&state)[MultisyncModel::sizeof_msg],
                                                                                               pami_multisync_t *msync)
      {
        TRACE_FN_ENTER();

        TRACE_FORMAT( "connection_id %#X\n", msync->connection_id);

        state_data_t *state_data = (state_data_t*) & state;

        state_data->header_metadata.connection_id = msync->connection_id;
        state_data->header_metadata.debug = _debug++;

        state_data->cb_done = msync->cb_done;

        // Set the connection state
//        PAMI_assert(_connection.get(msync->connection_id) == NULL);
//        _connection.set(msync->connection_id, (void *)state);
        PAMI_assert(_connection[msync->connection_id] == NULL);
        _connection[msync->connection_id] = state_data;

        _header_model.postCollectivePacket (state_data->pkt,
                                            NULL,
                                            NULL,
                                            0, //_route,
                                            &state_data->header_metadata,
                                            sizeof(header_metadata_t),
                                            (void*)NULL, 
                                            0);

        TRACE_FORMAT( "connection_id %#X, debug %#X exit\n", msync->connection_id, state_data->header_metadata.debug);

        TRACE_FN_EXIT();
        return PAMI_SUCCESS;

      }; // MultisyncModel::postMultisync_impl


      ///
      /// \brief Multisync header dispatch.
      ///
      ///
      /// \see PAMI::Device::Interface::RecvFunction_t
      ///
      template <bool T_Msgdata_support, bool T_PWQ_support>
      int MultisyncModel<T_Msgdata_support,T_PWQ_support>::dispatch_header (void   * metadata,
                                                                                   void   * payload,
                                                                                   size_t   bytes,
                                                                                   void   * arg,
                                                                                   void   * cookie)
      {
        TRACE_FN_ENTER();
        TRACE_HEXDATA(metadata,16);
        TRACE_HEXDATA(payload,32);

        header_metadata_t * m = (header_metadata_t *)metadata;

        MultisyncModel<T_Msgdata_support,T_PWQ_support> * model = (MultisyncModel<T_Msgdata_support,T_PWQ_support> *) arg;

        // Retrieve a state object!
        state_data_t *state;

        TRACE_FORMAT( "connection_id %#X  debug %#X\n", m->connection_id, m->debug);

        state = model->_connection[m->connection_id]; //model->_connection.get(m->connection_id);
        state->header_metadata.debug = -1;
        PAMI_assert(state);

        model->_connection.erase(state->header_metadata.connection_id);
        //_connection.clear(state->header_metadata.connection_id);

        // Invoke the receive done callback.
        if (state->cb_done.function)
          state->cb_done.function (0,//model->_device.getContext(), ///\todo why does this assert?
                                     state->cb_done.clientdata,
                                     PAMI_SUCCESS);
        TRACE_FN_EXIT();
        return 0;
      }; // PAMI::Device::MU::MultisyncModel::dispatch_header


    };
  };
};

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_model_Multisync_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
