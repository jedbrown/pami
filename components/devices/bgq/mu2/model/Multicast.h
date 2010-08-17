/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/devices/bgq/mu2/model/Multicast.h
/// \brief Simple multicast protocol
///
/// C++ templates require all source code to be #include'd from a header file.
///
#ifndef __components_devices_bgq_mu2_model_Multicast_h__
#define __components_devices_bgq_mu2_model_Multicast_h__

#include <map>

#include "components/devices/MulticastModel.h"

#include "components/devices/bgq/mu2/model/BroadcastPacketModel.h"
#include "components/memory/MemoryAllocator.h"

#include "components/devices/bgq/mu2/trace.h"
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {

      ///////////////////////////////////////////////////////////////////////////////
      // \class MulticastModel
      // \brief MU collective Multicast interface
      // \details
      //   - active message model
      //   - uses MU memfifo
      //   - one destination task per node
      //   - does not fully support PipeWorkQueue (multicast_model_available_buffers_only)
      ///////////////////////////////////////////////////////////////////////////////
      template <bool T_Msgdata_support, bool T_PWQ_support>
      class MulticastModel : public Interface::AMMulticastModel < MulticastModel<T_Msgdata_support, T_PWQ_support>, MU::Context, 4096 /*sizeof(state_data_t)*/ >
      {

        protected:

          typedef uint8_t packet_state_t[BroadcastPacketModel::packet_model_state_bytes];

          // Metadata passed in the (single-packet) header
          typedef struct __attribute__((__packed__))
          {
            uint32_t              connection_id;  ///< Collective connection id
            uint32_t              root;           ///< Root of the collective
            uint32_t              sndlen;         ///< Number of bytes of application data
            uint32_t              msgcount;       ///< Count of msgdata quads
          } header_metadata_t;

          // Metadata passed in the (multi-packet) data packet(s)
          typedef struct __attribute__((__packed__))
          {
            uint32_t              connection_id;  ///< Collective connection id
          } data_metadata_t;

          // State (request) implementation.  Callers should use uint8_t[MU::MulticastModel::sizeof_msg]
          typedef struct
          {
            packet_state_t          pkt[2];          ///< packet send state memory

            header_metadata_t       header_metadata;
            data_metadata_t         data_metadata;

            size_t                  remaining_length;
            PAMI::PipeWorkQueue   * rcvpwq;
            uint8_t               * buffer;
            pami_callback_t         cb_done;
            struct                  iovec iov[2];
          } state_data_t;


        public:

          /// \see PAMI::Device::Interface::MulticastModel::~MulticastModel
          ~MulticastModel () {};

          /// \brief Multicast model constants/attributes
//      static const bool   multicast_model_all_sided               = false;
          static const bool   multicast_model_active_message          = true;
          static const bool   multicast_model_available_buffers_only  = true;

          static const size_t sizeof_msg                              = 4096 /*sizeof(state_data_t)*/;
          static const size_t packet_model_payload_bytes              = BroadcastPacketModel::packet_model_payload_bytes;
          static const size_t packet_model_immediate_max              = BroadcastPacketModel::packet_model_immediate_max;

          static const size_t multicast_model_msgcount_max            = (packet_model_payload_bytes /*or packet_model_immediate_max*/ / sizeof(pami_quad_t));
          static const size_t multicast_model_bytes_max               = (uint32_t) - 1; // protocol_metadata_t::sndlen
          static const size_t multicast_model_connection_id_max       = (uint32_t) - 1; // protocol_metadata_t::connection_id \todo 64 bit?

          /// \see PAMI::Device::Interface::MulticastModel::registerMcastRecvFunction
          pami_result_t registerMcastRecvFunction_impl(int                        dispatch_id,
                                                       pami_dispatch_multicast_fn  func,
                                                       void                      *arg);
          /// \see PAMI::Device::Interface::MulticastModel::postMulticast
          pami_result_t postMulticast_impl(uint8_t (&state)[MulticastModel<T_Msgdata_support, T_PWQ_support>::sizeof_msg],
                                           pami_multicast_t *mcast,
                                           void             *devinfo = NULL);

        protected:
          /// \brief MU dispatch function
          inline static int dispatch_header (void   * metadata,
                                             void   * payload,
                                             size_t   bytes,
                                             void   * arg,
                                             void   * cookie);
          inline static int dispatch_data (void   * metadata,
                                           void   * payload,
                                           size_t   bytes,
                                           void   * arg,
                                           void   * cookie);
          /// \brief Process user data packets
          inline void processData   (state_data_t *state_data,
                                     uint8_t      *payload,
                                     size_t        bytes);

          inline state_data_t * allocateState ()
          {
            return(state_data_t *) _allocator.allocateObject ();
          }

          inline void freeState (state_data_t * object)
          {
            _allocator.returnObject ((void *) object);
          }


        private:
          MU::Context                                & _device;
          pami_task_t                                  _task_id;

          MemoryAllocator < sizeof(state_data_t), 16 > _allocator;

          BroadcastPacketModel                         _header_model;
          BroadcastPacketModel                         _data_model;

          pami_dispatch_multicast_fn                   _dispatch_function;
          void                                       * _dispatch_arg;

//        T_Connection                                 _connection; ///\todo ConnectionArray isn't appropriate...
          std::map<unsigned, state_data_t *>             _connection;

        public:

          /// \see PAMI::Device::Interface::MulticastModel::MulticastModel
          MulticastModel (MU::Context & device, pami_result_t &status) :
              Interface::AMMulticastModel < MulticastModel<T_Msgdata_support, T_PWQ_support>, MU::Context, 4096 /*sizeof(state_data_t)*/ > (device, status),
              _device (device),
              _task_id(__global.mapping.task()),
              _header_model (device),
              _data_model (device),
              _dispatch_function(NULL),
              _dispatch_arg(NULL)
              //        _connection (device)
          {
            TRACE_FN_ENTER();
            // ----------------------------------------------------------------
            // Compile-time assertions
            // ----------------------------------------------------------------

            // This protocol only works with reliable networks.
            COMPILE_TIME_ASSERT(BroadcastPacketModel::reliable_packet_model == true);

            // This protocol only works with deterministic models.
            COMPILE_TIME_ASSERT(BroadcastPacketModel::deterministic_packet_model == true);

            // Assert that the size of the packet metadata area is large
            // enough to transfer the eager match information. This is used in the
            // various postMultiPacket() calls to transfer long header and data
            // messages.
            COMPILE_TIME_ASSERT(sizeof(pami_endpoint_t) <= BroadcastPacketModel::packet_model_multi_metadata_bytes);

            // Assert that the size of the packet payload area is large
            // enough to transfer a single virtual address. This is used in
            // the postPacket() calls to transfer the ack information.
            COMPILE_TIME_ASSERT(sizeof(void *) <= BroadcastPacketModel::packet_model_payload_bytes);

            // ----------------------------------------------------------------
            // Compile-time assertions (end)
            // ----------------------------------------------------------------
            status = PAMI_SUCCESS;
            TRACE_FN_EXIT();
          }
      };

      template <bool T_Msgdata_support, bool T_PWQ_support>
      inline pami_result_t MulticastModel<T_Msgdata_support, T_PWQ_support>::postMulticast_impl(uint8_t (&state)[MulticastModel::sizeof_msg],
          pami_multicast_t *mcast,
          void             *devinfo)
      {
        TRACE_FN_ENTER();

#ifdef ENABLE_MU_CLASSROUTES
        uint32_t classRoute = (uint32_t)(size_t)devinfo; // convert platform independent void* to bgq uint32_t classroute
#else
        uint32_t classRoute = 0;
#endif

        TRACE_FORMAT( "connection_id %#X, class route %#X\n", mcast->connection_id, classRoute);

        // Get the source data buffer/length and validate (assert) inputs
        size_t length = mcast->bytes;
        PAMI::PipeWorkQueue *pwq = (PAMI::PipeWorkQueue *)mcast->src;

        if (T_PWQ_support == false)
          {
            // If you're sending data, it must all be ready in the pwq.
            PAMI_assert((length == 0) || (pwq && pwq->bytesAvailableToConsume() == length));
          }
        else
          {
            PAMI_abortf("T_PWQ_support not supported yet\n");
          }

        if (T_Msgdata_support == false)
          {
            // No msgdata expected
            PAMI_assert(mcast->msgcount == 0);
          }

        TRACE_FORMAT( "dispatch %zu, connection_id %#X, msgcount %d/%p, bytes %zu/%p/%p\n",
                      mcast->dispatch, mcast->connection_id,
                      mcast->msgcount, mcast->msginfo,
                      mcast->bytes, pwq, pwq ? pwq->bufferToConsume() : NULL);

        state_data_t *state_data = (state_data_t*) & state;

        void* payload = NULL;

        state_data->header_metadata.connection_id = mcast->connection_id;
        state_data->header_metadata.root          = _task_id;
        state_data->header_metadata.sndlen        = length;
        state_data->header_metadata.msgcount      = mcast->msgcount;

        state_data->data_metadata.connection_id   = mcast->connection_id;

        state_data->cb_done = mcast->cb_done;

        /// \todo allow root to receive data too? unclear multicast semantics...
        state_data->remaining_length = 0;  // root doesn't copy any data
        state_data->rcvpwq = NULL;         //(PAMI::PipeWorkQueue*)mcomb->results;
        state_data->buffer = NULL;              //state_data->rcvpwq?(uint8_t*)state_data->rcvpwq->bufferToProduce():NULL;

        // Set the connection state
//        PAMI_assert(_connection.get(mcast->connection_id) == NULL);
//        _connection.set(mcast->connection_id, (void *)state_data);
        PAMI_assert(_connection[mcast->connection_id] == NULL);
        _connection[mcast->connection_id] = state_data;

        if (T_PWQ_support == false)
          {
            if (length)
              {
                payload = (void*)pwq->bufferToConsume();
                pwq->consumeBytes(length);
              }

            if (T_Msgdata_support == false)
              {
                // Post the multicast to the device in one or more packets
                if (length <= packet_model_payload_bytes /*or packet_model_immediate_max*/) // one packet
                  {
                    _header_model.postCollectivePacket (state_data->pkt[0],
                                                        NULL,
                                                        NULL,
                                                        classRoute,
                                                        &state_data->header_metadata,
                                                        sizeof(header_metadata_t),
                                                        payload,
                                                        length);
                  }
                else  // > one packet of payload
                  {
                    _header_model.postCollectivePacket (state_data->pkt[0],
                                                        NULL,
                                                        NULL,
                                                        classRoute,
                                                        &state_data->header_metadata,
                                                        sizeof(header_metadata_t),
                                                        payload,
                                                        packet_model_payload_bytes /*or packet_model_immediate_max*/);
                    _data_model.postMultiCollectivePacket (state_data->pkt[1],
                                                           NULL,
                                                           NULL,
                                                           classRoute,
                                                           &state_data->data_metadata,
                                                           sizeof(data_metadata_t),
                                                           ((char*)payload) + packet_model_payload_bytes /*or packet_model_immediate_max*/,
                                                           length - packet_model_payload_bytes /*or packet_model_immediate_max*/);
                  }
              } // T_Msgdata_support==false
            else //T_Msgdata_support==true
              {
                // Get the msginfo buffer/length and validate (assert) inputs
                void* msgdata = (void*)mcast->msginfo;

                PAMI_assert(multicast_model_msgcount_max >= mcast->msgcount);

                size_t msglength = mcast->msgcount * sizeof(pami_quad_t);
                size_t total = length + msglength;

                // Post the multicast to the device in one or more packets
                if (total <= packet_model_payload_bytes /*or packet_model_immediate_max*/) // one packet
                  {
                    // pack msginfo and payload into one (single) packet

                    state_data->iov[0].iov_base = msgdata;
                    state_data->iov[0].iov_len  = msglength;
                    state_data->iov[1].iov_base = payload;
                    state_data->iov[1].iov_len  = length;

                    _header_model.postCollectivePacket (state_data->pkt[0],
                                                        NULL,
                                                        NULL,
                                                        classRoute,
                                                        &state_data->header_metadata,
                                                        sizeof(header_metadata_t),
                                                        state_data->iov);
                  }
                else // > one packet
                  {
                    // first packet contains msgdata
                    state_data->iov[0].iov_base = msgdata;
                    state_data->iov[0].iov_len  = msglength;
                    state_data->iov[1].iov_base = payload;
                    state_data->iov[1].iov_len  = MIN(length, (packet_model_payload_bytes /*or packet_model_immediate_max*/ - msglength));
                    _header_model.postCollectivePacket (state_data->pkt[0],
                                                        NULL,
                                                        NULL,
                                                        classRoute,
                                                        &state_data->header_metadata,
                                                        sizeof(header_metadata_t),
                                                        state_data->iov);

                    _data_model.postMultiCollectivePacket (state_data->pkt[1],
                                                           NULL,
                                                           NULL,
                                                           classRoute,
                                                           &state_data->data_metadata,
                                                           sizeof(data_metadata_t),
                                                           (char*)payload + state_data->iov[1].iov_len,
                                                           length - state_data->iov[1].iov_len);
                  }

              } // T_Msgdata_support==true
          } // T_PWQ_support==false

        TRACE_FORMAT( "dispatch %zu, connection_id %#X exit\n",
                      mcast->dispatch, mcast->connection_id);

        TRACE_FN_EXIT();
        return PAMI_SUCCESS;

      }; // MulticastModel::postMulticast_impl


      ///
      /// \brief Multicast header dispatch.
      ///
      ///
      /// \see PAMI::Device::Interface::RecvFunction_t
      ///
      template <bool T_Msgdata_support, bool T_PWQ_support>
      int MulticastModel<T_Msgdata_support, T_PWQ_support>::dispatch_header (void   * metadata,
                                                                             void   * payload,
                                                                             size_t   bytes,
                                                                             void   * arg,
                                                                             void   * cookie)
      {
        TRACE_FN_ENTER();
        TRACE_HEXDATA(metadata, 16);
        TRACE_HEXDATA(payload, 32);

        header_metadata_t * m = (header_metadata_t *)metadata;
        char * p;
        char * msg = p = (char*) payload;

        MulticastModel<T_Msgdata_support, T_PWQ_support> * model = (MulticastModel<T_Msgdata_support, T_PWQ_support> *) arg;

        size_t data_length = m->sndlen;

        if (T_Msgdata_support == true)
          {
            // Adjust the payload past the msgdata
            size_t msglength = m->msgcount * sizeof(pami_quad_t);
            data_length -= msglength;
            bytes       -= msglength;
            p += msglength;
          }

        // Allocate/retrieve a state object!
        state_data_t *state_data;

        // The root already has a state object, retrieve it
        if (m->root == model->_task_id)
          {
            state_data = model->_connection[m->connection_id]; //model->_connection.get(m->connection_id);
            PAMI_assert(state_data);
          }
        else //allocate one
          {
            state_data = model->allocateState ();
            state_data->header_metadata.connection_id   = m->connection_id;
            state_data->header_metadata.msgcount        = m->msgcount;
            state_data->header_metadata.root            = m->root;
            state_data->header_metadata.sndlen          = m->sndlen;

            // Set the connection state
//          PAMI_assert(model->_connection.get(connection_id) == NULL);
//          model->_connection.set(connection_id, (void *)state_data);
            PAMI_assert(model->_connection[m->connection_id] == NULL);
            model->_connection[m->connection_id] = state_data;

            state_data->rcvpwq = NULL;
            state_data->cb_done.function = NULL;
            state_data->cb_done.clientdata = NULL;

            // Invoke the registered dispatch function.
            model->_dispatch_function((pami_quad_t*)msg,                              // Msgdata
                                      m->msgcount,                      // Count of msgdata
                                      m->connection_id,                 // Connection ID of data
                                      m->root,                          // Sending task/root
                                      m->sndlen,                        // Length of data sent
                                      model->_dispatch_arg,                    // Opaque dispatch arg
                                      &state_data->remaining_length,          // [out] Length of data to receive
                                      (pami_pipeworkqueue_t**) &state_data->rcvpwq,// [out] Where to put recv data
                                      &state_data->cb_done                   // [out] Completion callback to invoke when data received
                                     );
            state_data->buffer = state_data->rcvpwq ? (uint8_t*)state_data->rcvpwq->bufferToProduce() : NULL;

            TRACE_FORMAT( "after dispatch remaining_length %zu, pwq %p\n", state_data->remaining_length, state_data->rcvpwq);

          }

        TRACE_FORMAT( "cookie = %p, root = %d, bytes = %zu remaining = %zd, sndlen = %d, connection id %u/%#X\n", cookie, (m->root), bytes,  state_data->remaining_length, m->sndlen, m->connection_id, m->connection_id);

        if (T_PWQ_support == false)
          {
            // If you're receiving data, it must all be ready in the pwq.
            PAMI_assertf(((state_data->rcvpwq == NULL) && (state_data->remaining_length == 0)) || (state_data->rcvpwq && (state_data->rcvpwq->bytesAvailableToProduce() >= state_data->remaining_length)), "state_data->rcvpwq->bytesAvailableToProduce() %zd,state_data->remaining_length %zd\n", state_data->rcvpwq ? state_data->rcvpwq->bytesAvailableToProduce() : -1, state_data->remaining_length);
          }

        model->processData(state_data, (uint8_t*) payload, bytes);

        TRACE_FN_EXIT();
        return 0;
      }; // PAMI::Device::MU::MulticastModel::dispatch_header

      ///
      /// \brief Multicast data (after header) dispatch.
      ///
      ///
      /// \see PAMI::Device::Interface::RecvFunction_t
      ///
      template <bool T_Msgdata_support, bool T_PWQ_support>
      int MulticastModel<T_Msgdata_support, T_PWQ_support>::dispatch_data   (void   * metadata,
                                                                             void   * payload,
                                                                             size_t   bytes,
                                                                             void   * arg,
                                                                             void   * cookie)
      {
        TRACE_FN_ENTER();
        TRACE_HEXDATA(metadata, 16);
        TRACE_HEXDATA(payload, 32);

        data_metadata_t * m = (data_metadata_t *)metadata;

        MulticastModel<T_Msgdata_support, T_PWQ_support> * model = (MulticastModel<T_Msgdata_support, T_PWQ_support> *) arg;

        TRACE_FORMAT( "cookie = %p, bytes = %zu, connection id %u/%#X\n", cookie, bytes, m->connection_id, m->connection_id);

        // Retrieve a state object
        state_data_t *state_data = model->_connection[m->connection_id]; //model->_connection.get(connection_id);
        PAMI_assert(state_data);

        model->processData(state_data, (uint8_t*) payload, bytes);

        TRACE_FN_EXIT();
        return 0;
      }; // PAMI::Device::MU::MulticastModel::dispatch_data

      template <bool T_Msgdata_support, bool T_PWQ_support>
      inline void MulticastModel<T_Msgdata_support, T_PWQ_support>::processData   (state_data_t * state_data,
                                                                                   uint8_t * payload,
                                                                                   size_t    bytes)
      {
        TRACE_FN_ENTER();
        header_metadata_t   *  header = &state_data->header_metadata;
        TRACE_FORMAT( "state_data %p, connection id %u, payload %p, bytes %zu, remaining length %zu, sndlen %u\n",
                      state_data, header->connection_id, payload, bytes, state_data->remaining_length, header->sndlen);


        // Number of bytes left to copy into the destination buffer
        size_t nleft = state_data->remaining_length;

        // Number of bytes left to copy from this packet
        if (nleft > bytes) nleft = bytes;

        if (nleft) // copy data and update receive state_data
          {
            TRACE_FORMAT( "memcpy(%p,%p,%zu)\n", state_data->buffer, payload, nleft);
            memcpy (state_data->buffer, payload, nleft);

            // Update the receive state_data
            state_data->buffer += nleft;
            state_data->remaining_length -= nleft;
            state_data->rcvpwq->produceBytes(nleft);
          }
        else ;  /// toss unwanted data?

        // Decrement the original bytes sent by the bytes just received...
        // this is different from expected length because the dispatch might
        // not receive all data sent (tossing some).
        header->sndlen -= MIN(header->sndlen, (uint32_t)bytes);

        if (header->sndlen == 0)
          {
            TRACE_FORMAT( "done cb_done function %p, clientdata %p\n", state_data->cb_done.function, state_data->cb_done.clientdata);
            _connection.erase(header->connection_id);
            //_connection.clear(header->connection_id);

            // Invoke the receive done callback.
            if (state_data->cb_done.function)
              state_data->cb_done.function (0,//_device.getContext(), ///\todo why does this assert?
                                            state_data->cb_done.clientdata,
                                            PAMI_SUCCESS);

            if (_task_id != header->root) freeState(state_data);
          }

        TRACE_FN_EXIT();
        return ;
      }; // PAMI::Device::MU::MulticastModel::processData


      template <bool T_Msgdata_support, bool T_PWQ_support>
      pami_result_t MulticastModel<T_Msgdata_support, T_PWQ_support>::registerMcastRecvFunction_impl(int                         dispatch_id,
          pami_dispatch_multicast_fn  func,
          void                       *arg)
      {
        TRACE_FN_ENTER();
        TRACE_FORMAT( "id %d, func %p, arg %p\n", dispatch_id, func, arg);
        PAMI_assert(multicast_model_active_message);

        _dispatch_function = func;
        _dispatch_arg = arg;

        TRACE_FORMAT( "register data model dispatch %d\n", dispatch_id);
        pami_result_t status = _data_model.init (dispatch_id,
                                                 dispatch_data, this,
                                                 NULL, NULL);
        TRACE_FORMAT( "data model status = %d\n", status);

        if (status == PAMI_SUCCESS)
          {
            TRACE_FORMAT( "register header  model dispatch %d\n", dispatch_id);
            status = _header_model.init (dispatch_id,
                                         dispatch_header, this,
                                         NULL, NULL);
            TRACE_FORMAT( "header model status = %d\n", status);
          }

        TRACE_FN_EXIT();
        return status;
      };
    };
  };
};

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_model_Multicast_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
