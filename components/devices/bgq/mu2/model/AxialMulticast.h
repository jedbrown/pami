/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/devices/bgq/mu2/model/AxialMulticast.h
/// \brief Simple axial (line deposit) multicast protocol 
///
/// C++ templates require all source code to be #include'd from a header file.
///
#ifndef __components_devices_bgq_mu2_model_AxialMulticast_h__
#define __components_devices_bgq_mu2_model_AxialMulticast_h__

#include <map>

#include "components/devices/MulticastModel.h"

#include "components/devices/bgq/mu2/model/PacketModelDeposit.h"
#include "components/memory/MemoryAllocator.h"

#include "util/ccmi_debug.h"
#include "util/ccmi_util.h"
#include "components/devices/bgq/mu2/trace.h"

#ifdef CCMI_TRACE_ALL
 #define DO_TRACE_ENTEREXIT 1
 #define DO_TRACE_DEBUG     1
#else
 #define DO_TRACE_ENTEREXIT 0
 #define DO_TRACE_DEBUG     0
#endif


namespace PAMI
{
  namespace Device
  {
    namespace MU
    {

      ///////////////////////////////////////////////////////////////////////////////
      // \class AxialMulticastModel
      // \brief MU Axial (p2p line deposit) Multicast interface
      // \details
      //   - active message model
      //   - uses MU memfifo
      //   - does not fully support PipeWorkQueue (multicast_model_available_buffers_only)
      ///////////////////////////////////////////////////////////////////////////////
      template <bool T_Msgdata_support, bool T_PWQ_support>
      class AxialMulticastModel : public Interface::AMMulticastModel < AxialMulticastModel<T_Msgdata_support, T_PWQ_support>, MU::Context, 4096 /*sizeof(state_data_t)*/ >
      {

      protected:

        typedef uint8_t packet_state_t[PacketModelDeposit<>::packet_model_state_bytes];

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

        // State (request) implementation.  Callers should use uint8_t[MU::AxialMulticastModel::sizeof_msg]
        typedef struct
        {
          packet_state_t          pkt[2];          ///< packet send state memory

          header_metadata_t       header_metadata;
          data_metadata_t         data_metadata;

          size_t                  remaining_length;
          PAMI::PipeWorkQueue   * rcvpwq;
          uint8_t               * buffer;
          size_t                  send_countdown;
          bool                    sendDoneCalled;
          pami_callback_t         cb_done;
          struct                  iovec iov[2];
          AxialMulticastModel<T_Msgdata_support, T_PWQ_support> * model;
        } state_data_t;


      public:

        /// \see PAMI::Device::Interface::MulticastModel::~MulticastModel
        ~AxialMulticastModel ()
        {
        };

        /// \brief Multicast model constants/attributes
        static const bool   multicast_model_active_message          = true;
        static const bool   multicast_model_available_buffers_only  = true;

        static const size_t sizeof_msg                              = 4096 /*sizeof(state_data_t)*/;
        static const size_t packet_model_payload_bytes              = PacketModelDeposit<>::packet_model_payload_bytes;
        static const size_t packet_model_immediate_bytes            = PacketModelDeposit<>::packet_model_immediate_bytes;

        static const size_t multicast_model_msgcount_max            = (packet_model_payload_bytes /*or packet_model_immediate_bytes*/ / sizeof(pami_quad_t));
        static const size_t multicast_model_bytes_max               = (uint32_t) - 1; // protocol_metadata_t::sndlen
        static const size_t multicast_model_connection_id_max       = (uint32_t) - 1; // protocol_metadata_t::connection_id \todo 64 bit?

        /// \see PAMI::Device::Interface::MulticastModel::registerMcastRecvFunction
        pami_result_t registerMcastRecvFunction_impl(int                        dispatch_id,
                                                     pami_dispatch_multicast_function func,
                                                     void                      *arg);
        /// \see PAMI::Device::Interface::MulticastModel::postMulticast
        pami_result_t postMulticast_impl(uint8_t (&state)[AxialMulticastModel<T_Msgdata_support, T_PWQ_support>::sizeof_msg],
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

        static
        inline void sendDone(pami_context_t   context,
                             void           * cookie,
                             pami_result_t    result);

      private:
        MU::Context                                & _device;
        pami_task_t                                  _task_id;

        MemoryAllocator < sizeof(state_data_t), 16 > _allocator;

        // Deposit models for the (+) direction on a mesh/torus
        PacketModelDeposit<POSITIVE>                 _header_model_p;
        PacketModelDeposit<POSITIVE>                 _data_model_p;
        // (-) direction models
        PacketModelDeposit<MINUS>                    _header_model_m;
        PacketModelDeposit<MINUS>                    _data_model_m;

        pami_dispatch_multicast_function             _dispatch_function;
        void                                       * _dispatch_arg;

//        T_Connection                                 _connection; ///\todo ConnectionArray isn't appropriate...
        std::map<unsigned, state_data_t *>             _connection;

      public:

        /// \see PAMI::Device::Interface::MulticastModel::MulticastModel
        AxialMulticastModel (MU::Context & device, pami_result_t &status) :
        Interface::AMMulticastModel < AxialMulticastModel<T_Msgdata_support, T_PWQ_support>, MU::Context, 4096 /*sizeof(state_data_t)*/ > (device, status),
        _device (device),
        _task_id(__global.mapping.task()),
        _header_model_p (device),
        _data_model_p (device),
        _header_model_m (device),
        _data_model_m (device),
        _dispatch_function(NULL),
        _dispatch_arg(NULL)
        //        _connection (device)
        {
          TRACE_FN_ENTER();
          // ----------------------------------------------------------------
          // Compile-time assertions
          // ----------------------------------------------------------------

          // This protocol only works with reliable networks.
          COMPILE_TIME_ASSERT(PacketModelDeposit<>::reliable_packet_model == true);

          // This protocol only works with deterministic models.
          COMPILE_TIME_ASSERT(PacketModelDeposit<>::deterministic_packet_model == true);

          // Assert that the size of the packet metadata area is large
          // enough to transfer the eager match information. This is used in the
          // various postMultiPacket() calls to transfer long header and data
          // messages.
          COMPILE_TIME_ASSERT(sizeof(pami_endpoint_t) <= PacketModelDeposit<>::packet_model_multi_metadata_bytes);

          // Assert that the size of the packet payload area is large
          // enough to transfer a single virtual address. This is used in
          // the postPacket() calls to transfer the ack information.
          COMPILE_TIME_ASSERT(sizeof(void *) <= PacketModelDeposit<>::packet_model_payload_bytes);

          // ----------------------------------------------------------------
          // Compile-time assertions (end)
          // ----------------------------------------------------------------
          status = PAMI_SUCCESS;
          TRACE_FN_EXIT();
        }
      private:
        void selectTargets(size_t i, 
                           pami_coord_t *ref, unsigned char *isTorus, pami_coord_t *ll, pami_coord_t *ur, 
                           bool &p, pami_task_t &target_task_p, 
                           bool &m, pami_task_t &target_task_m)
        {
          pami_coord_t target_coords = *ref;                                     
          pami_network      type;                                                
          TRACE_FORMAT("<%zu> isTorus %u "                                       
                       "ref[%zu,%zu,%zu,%zu,%zu]  ",                             
                       i,                                                        
                       isTorus[i],                                               
                       target_coords.u.n_torus.coords[0],                        
                       target_coords.u.n_torus.coords[1],                        
                       target_coords.u.n_torus.coords[2],                        
                       target_coords.u.n_torus.coords[3],                        
                       target_coords.u.n_torus.coords[4]);                       
          if (isTorus[i])
          {
            if (target_coords.u.n_torus.coords[i] == ll->u.n_torus.coords[i])
              target_coords.u.n_torus.coords[i] = ur->u.n_torus.coords[i];
            else
              target_coords.u.n_torus.coords[i]--;                               
            __global.mapping.network2task(&target_coords, &target_task_p, &type);
            p = true; m = false;                                                 
            TRACE_FORMAT("target[%zu,%zu,%zu,%zu,%zu]=%u  ",                       
                         target_coords.u.n_torus.coords[0],                        
                         target_coords.u.n_torus.coords[1],                        
                         target_coords.u.n_torus.coords[2],                        
                         target_coords.u.n_torus.coords[3],                        
                         target_coords.u.n_torus.coords[4],                        
                         target_task_p);                                           
          }
          else
          {
            p =  m = true;                                                       
            target_coords.u.n_torus.coords[i] = ur->u.n_torus.coords[i];         
            __global.mapping.network2task(&target_coords, &target_task_p, &type);
            if (target_coords.u.n_torus.coords[i] == ref->u.n_torus.coords[i])
              p = false;
            TRACE_FORMAT("p=%u target[%zu,%zu,%zu,%zu,%zu]=%u  ",  p,              
                         target_coords.u.n_torus.coords[0],                        
                         target_coords.u.n_torus.coords[1],                        
                         target_coords.u.n_torus.coords[2],                        
                         target_coords.u.n_torus.coords[3],                        
                         target_coords.u.n_torus.coords[4],                        
                         target_task_p);                                           
            target_coords.u.n_torus.coords[i] = ll->u.n_torus.coords[i];         
            __global.mapping.network2task(&target_coords, &target_task_m, &type);
            if (target_coords.u.n_torus.coords[i] == ref->u.n_torus.coords[i])
              m = false;
            TRACE_FORMAT("m=%u target[%zu,%zu,%zu,%zu,%zu]=%u  ",  m,              
                         target_coords.u.n_torus.coords[0],                        
                         target_coords.u.n_torus.coords[1],                        
                         target_coords.u.n_torus.coords[2],                        
                         target_coords.u.n_torus.coords[3],                        
                         target_coords.u.n_torus.coords[4],                        
                         target_task_m);                                           
          }                                                                      
          return;
        }

      };

      template <bool T_Msgdata_support, bool T_PWQ_support>
      inline pami_result_t AxialMulticastModel<T_Msgdata_support, T_PWQ_support>::postMulticast_impl(uint8_t (&state)[AxialMulticastModel::sizeof_msg],
                                                                                                     pami_multicast_t *mcast,
                                                                                                     void             *devinfo)
      {
        TRACE_FN_ENTER();

        TRACE_FORMAT( "connection_id %#X, src_participants %p, dst_participants %p", mcast->connection_id, mcast->src_participants, mcast->dst_participants);

        // Get the source data buffer/length and validate (assert) inputs
        size_t length = mcast->bytes;
        PAMI::PipeWorkQueue *pwq = (PAMI::PipeWorkQueue *)mcast->src;

        if (T_PWQ_support == false)
        {
          // If you're sending data, it must all be ready in the pwq.
/// \todo ?          PAMI_assert((length == 0) || (pwq && pwq->bytesAvailableToConsume() == length));
        }
        else
        {
          PAMI_abortf("T_PWQ_support not supported yet");
        }

        if (T_Msgdata_support == false)
        {
          // No msgdata expected
          PAMI_assert(mcast->msgcount == 0);
        }

        TRACE_FORMAT( "dispatch %zu, connection_id %#X, msgcount %d/%p, ndst %zu, bytes %zu/%zu pwq %p buffer %p",
                      mcast->dispatch, mcast->connection_id,
                      mcast->msgcount, mcast->msginfo,
                      mcast->bytes, 
                      mcast->dst_participants? ((PAMI::Topology*)mcast->dst_participants)->size():(size_t)-1, 
                      pwq ? pwq->bytesAvailableToConsume():(size_t)-1,
                      pwq, pwq ? pwq->bufferToConsume() : NULL);

        state_data_t *state_data = (state_data_t*) & state;

        void* payload = NULL;

        state_data->header_metadata.connection_id = mcast->connection_id;
        state_data->header_metadata.root          = _task_id;
        state_data->header_metadata.sndlen        = length;
        state_data->header_metadata.msgcount      = mcast->msgcount;
        state_data->model                         = this;

        state_data->data_metadata.connection_id   = mcast->connection_id;

        state_data->cb_done = mcast->cb_done;

        /// \todo allow root to receive data too? unclear multicast semantics...
        state_data->remaining_length = 0;  // root doesn't copy any data
        state_data->rcvpwq = NULL;         //(PAMI::PipeWorkQueue*)mcomb->results;
        state_data->buffer = NULL;              //state_data->rcvpwq?(uint8_t*)state_data->rcvpwq->bufferToProduce():NULL;

        state_data->sendDoneCalled                = false;
//      state_data->send_countdown                = BGQ_TDIMS*2 ;
        state_data->send_countdown                = __global.mapping.globalDims()*2; // assume sending all + and - dimensions.

        // Set the connection state
//        PAMI_assert(_connection.get(mcast->connection_id) == NULL);
//        _connection.set(mcast->connection_id, (void *)state_data);
        PAMI_assert(_connection[mcast->connection_id] == NULL);
        _connection[mcast->connection_id] = state_data;

        PAMI::Topology* dst_topology = (PAMI::Topology*)mcast->dst_participants;

        pami_task_t * ranklist = NULL;
        unsigned char risTorus[PAMI_MAX_DIMS]={};
        if ((dst_topology->type() != PAMI_AXIAL_TOPOLOGY))// && (dst_topology->size() > 1))
        {
          PAMI::Topology* src_topology = (PAMI::Topology*)mcast->src_participants;
          pami_coord_t rref;
          pami_network rtype = PAMI_N_TORUS_NETWORK;                                               
          DO_DEBUG(for (unsigned j = 0; dst_topology && (j < dst_topology->size()); ++j) {pami_coord_t tref;
                     TRACE_FORMAT( "destinations[%u]=%zu, size %zu, %u, [%zu,%zu,%zu,%zu,%zu]", 
                                   j, (size_t)dst_topology->index2Rank(j), dst_topology->size(),
                                   __global.mapping.task2network((size_t)dst_topology->index2Rank(j), &tref,  rtype), 
                                   tref.u.n_torus.coords[0],                                 
                                   tref.u.n_torus.coords[1],                                 
                                   tref.u.n_torus.coords[2],                                 
                                   tref.u.n_torus.coords[3],                                 
                                   tref.u.n_torus.coords[4]
                                 )});
          
          DO_DEBUG(for (unsigned j = 0;src_topology && (j < src_topology->size()); ++j) {pami_coord_t tref;
                   TRACE_FORMAT( "sources[%u]=%zu, size %zu, %u, [%zu,%zu,%zu,%zu,%zu]", 
                                 j, (size_t)src_topology->index2Rank(j), src_topology->size(),
                                 __global.mapping.task2network((size_t)src_topology->index2Rank(j), &tref,  rtype), 
                                 tref.u.n_torus.coords[0],                                 
                                 tref.u.n_torus.coords[1],                                 
                                 tref.u.n_torus.coords[2],                                 
                                 tref.u.n_torus.coords[3],                                 
                                 tref.u.n_torus.coords[4]
                               )});

          // Probably need to try to make our own axial topology for now...
          __global.mapping.task2network(_task_id, &rref,  rtype);    

          PAMI::Topology coord_topology;
// aborts          src_topology->unionTopology(&coord_topology, dst_topology);
          size_t tsize = (src_topology?src_topology->size():0)+(dst_topology?dst_topology->size():0);
          ranklist = (pami_task_t*)malloc(tsize);
          for (size_t i = 0 ; src_topology && i < src_topology->size(); ++i)
          {
            ranklist[i] = src_topology->index2Rank(i);
          }
          for (size_t j=0,i = src_topology?src_topology->size():0 ; dst_topology && j < dst_topology->size(); ++i,++j)
          {
            ranklist[i] = dst_topology->index2Rank(j);
          }
          new (&coord_topology) Topology(ranklist, tsize);

          DO_DEBUG(for (unsigned j = 0; j < coord_topology.size(); ++j) {pami_coord_t tref;
                   TRACE_FORMAT( "coord_topology[%u]=%zu, size %zu, %u, [%zu,%zu,%zu,%zu,%zu]", 
                                 j, (size_t) coord_topology.index2Rank(j),  coord_topology.size(),
                                 __global.mapping.task2network((size_t) coord_topology.index2Rank(j), &tref,  rtype), 
                                 tref.u.n_torus.coords[0],                                 
                                 tref.u.n_torus.coords[1],                                 
                                 tref.u.n_torus.coords[2],                                 
                                 tref.u.n_torus.coords[3],                                 
                                 tref.u.n_torus.coords[4]
                               )});

          coord_topology.convertTopology(PAMI_COORD_TOPOLOGY);
          PAMI_assert(coord_topology.type() == PAMI_COORD_TOPOLOGY);

          pami_coord_t rll;
          pami_coord_t rur;

          coord_topology.rectSeg(&rll, &rur, risTorus);
          dst_topology = new PAMI::Topology(&rll, &rur, &rref, risTorus);
          DO_DEBUG(for (unsigned j = 0; j < dst_topology->size(); ++j) {pami_coord_t tref;
                   TRACE_FORMAT( "axial_topology[%u]=%zu, size %zu, %u, [%zu,%zu,%zu,%zu,%zu]", 
                                 j, (size_t) dst_topology->index2Rank(j),  dst_topology->size(),
                                 __global.mapping.task2network((size_t) dst_topology->index2Rank(j), &tref,  rtype), 
                                 tref.u.n_torus.coords[0],                                 
                                 tref.u.n_torus.coords[1],                                 
                                 tref.u.n_torus.coords[2],                                 
                                 tref.u.n_torus.coords[3],                                 
                                 tref.u.n_torus.coords[4]
                               )});
        }

        /// \todo eventually assert that our input is axial instead of converting above
        PAMI_assert((dst_topology->type() == PAMI_AXIAL_TOPOLOGY));// || (dst_topology->size() == 1));
        pami_coord_t *ll=NULL;
        pami_coord_t *ur=NULL;
        pami_coord_t *ref=NULL;
        unsigned char *isTorus=NULL;

#if ASSERT_LEVEL > 0
        pami_result_t result = dst_topology->axial(&ll, &ur, &ref, &isTorus);
        PAMI_assert(result == PAMI_SUCCESS);
#else
        dst_topology->axial(&ll, &ur, &ref, &isTorus);
#endif
        TRACE_FORMAT("ll[%zu,%zu,%zu,%zu,%zu], ur[%zu,%zu,%zu,%zu,%zu]",                             
                     ll->u.n_torus.coords[0],                                 
                     ll->u.n_torus.coords[1],                                 
                     ll->u.n_torus.coords[2],                                 
                     ll->u.n_torus.coords[3],                                 
                     ll->u.n_torus.coords[4],                                
                     ur->u.n_torus.coords[0],                                 
                     ur->u.n_torus.coords[1],                                 
                     ur->u.n_torus.coords[2],                                 
                     ur->u.n_torus.coords[3],                                 
                     ur->u.n_torus.coords[4]);                                

        pami_task_t   target_task_p=0, target_task_m=0; //target tasks (+) and (-) directions
        size_t        target_offset = mcast->context;
        bool p, m; // send (+), send (-)
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
            if (length <= packet_model_payload_bytes /*or packet_model_immediate_bytes*/) // one packet
            {
              for (size_t i=0; i < __global.mapping.globalDims(); ++i)
              {
                /// Calculate (+) and (-) targets for this network dimension
                selectTargets(i, ref,  isTorus, ll, ur, p, target_task_p, m, target_task_m);

                // Send + and/or - directions as determined in selectTargets()
                if (p)
                  _header_model_p.postPacket(state_data->pkt[0],
                                             sendDone,
                                             state_data,
                                             target_task_p, target_offset,
                                             &state_data->header_metadata,
                                             sizeof(header_metadata_t),
                                             payload,
                                             length);
                else state_data->send_countdown--; // no (+) destination so no done needed

                if (m)
                  _header_model_m.postPacket(state_data->pkt[0],
                                             sendDone,
                                             state_data,
                                             target_task_m, target_offset,
                                             &state_data->header_metadata,
                                             sizeof(header_metadata_t),
                                             payload,
                                             length);
                else state_data->send_countdown--; // no (-) destination so no done needed
              }
            }
            else  // > one packet of payload
            {
              for (size_t i=0; i < __global.mapping.globalDims(); ++i)
              {
                /// Calculate (+) and (-) targets for this network dimension
                selectTargets(i, ref,  isTorus, ll, ur, p, target_task_p, m, target_task_m);

                // Send + and/or - directions as determined in selectTargets()
                if (p)
                {
                  _header_model_p.postPacket (state_data->pkt[0],
                                              NULL,
                                              NULL,
                                              target_task_p, target_offset,
                                              &state_data->header_metadata,
                                              sizeof(header_metadata_t),
                                              payload,
                                              packet_model_payload_bytes /*or packet_model_immediate_bytes*/);
                  _data_model_p.postMultiPacket (state_data->pkt[1],
                                                 sendDone,
                                                 state_data,
                                                 target_task_p, target_offset,
                                                 &state_data->data_metadata,
                                                 sizeof(data_metadata_t),
                                                 ((char*)payload) + packet_model_payload_bytes /*or packet_model_immediate_bytes*/,
                                                 length - packet_model_payload_bytes /*or packet_model_immediate_bytes*/);
                }
                else state_data->send_countdown--; // no (+) destination so no done needed

                if (m)
                {
                  _header_model_m.postPacket (state_data->pkt[0],
                                              NULL,
                                              NULL,
                                              target_task_p, target_offset,
                                              &state_data->header_metadata,
                                              sizeof(header_metadata_t),
                                              payload,
                                              packet_model_payload_bytes /*or packet_model_immediate_bytes*/);
                  _data_model_m.postMultiPacket (state_data->pkt[1],
                                                 sendDone,
                                                 state_data,
                                                 target_task_p, target_offset,
                                                 &state_data->data_metadata,
                                                 sizeof(data_metadata_t),
                                                 ((char*)payload) + packet_model_payload_bytes /*or packet_model_immediate_bytes*/,
                                                 length - packet_model_payload_bytes /*or packet_model_immediate_bytes*/);
                }
                else state_data->send_countdown--; // no (-) destination so no done needed
              }
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
            if (total <= packet_model_payload_bytes /*or packet_model_immediate_bytes*/) // one packet
            {
              // pack msginfo and payload into one (single) packet

              state_data->iov[0].iov_base = msgdata;
              state_data->iov[0].iov_len  = msglength;
              state_data->iov[1].iov_base = payload;
              state_data->iov[1].iov_len  = length;

              for (size_t i=0; i < __global.mapping.globalDims(); ++i)
              {
                /// Calculate (+) and (-) targets for this network dimension
                selectTargets(i, ref,  isTorus, ll, ur, p, target_task_p, m, target_task_m);

                // Send + and/or - directions as determined in selectTargets()
                if (p)
                  _header_model_p.postPacket (state_data->pkt[0],
                                              sendDone,
                                              state_data,
                                              target_task_p, target_offset,
                                              &state_data->header_metadata,
                                              sizeof(header_metadata_t),
                                              state_data->iov);
                else state_data->send_countdown--; // no (+) destination so no done needed

                if (m)
                  _header_model_m.postPacket (state_data->pkt[0],
                                              sendDone,
                                              state_data,
                                              target_task_p, target_offset,
                                              &state_data->header_metadata,
                                              sizeof(header_metadata_t),
                                              state_data->iov);
                else state_data->send_countdown--; // no (-) destination so no done needed
              }
            }
            else // > one packet
            {
              // first packet contains msgdata
              state_data->iov[0].iov_base = msgdata;
              state_data->iov[0].iov_len  = msglength;
              state_data->iov[1].iov_base = payload;
              state_data->iov[1].iov_len  = MIN(length, (packet_model_payload_bytes /*or packet_model_immediate_bytes*/ - msglength));
              for (size_t i=0; i < __global.mapping.globalDims(); ++i)
              {
                /// Calculate (+) and (-) targets for this network dimension
                selectTargets(i, ref,  isTorus, ll, ur, p, target_task_p, m, target_task_m);

                // Send + and/or - directions as determined in selectTargets()
                if (p)
                {
                  _header_model_p.postPacket (state_data->pkt[0],
                                              NULL,
                                              NULL,
                                              target_task_p, target_offset,
                                              &state_data->header_metadata,
                                              sizeof(header_metadata_t),
                                              state_data->iov);

                  _data_model_p.postMultiPacket (state_data->pkt[1],
                                                 sendDone,
                                                 state_data,
                                                 target_task_p, target_offset,
                                                 &state_data->data_metadata,
                                                 sizeof(data_metadata_t),
                                                 (char*)payload + state_data->iov[1].iov_len,
                                                 length - state_data->iov[1].iov_len);
                }
                else state_data->send_countdown--; // no (+) destination so no done needed

                if (m)
                {
                  _header_model_m.postPacket (state_data->pkt[0],
                                              NULL,
                                              NULL,
                                              target_task_p, target_offset,
                                              &state_data->header_metadata,
                                              sizeof(header_metadata_t),
                                              state_data->iov);

                  _data_model_m.postMultiPacket (state_data->pkt[1],
                                                 sendDone,
                                                 state_data,
                                                 target_task_p, target_offset,
                                                 &state_data->data_metadata,
                                                 sizeof(data_metadata_t),
                                                 (char*)payload + state_data->iov[1].iov_len,
                                                 length - state_data->iov[1].iov_len);
                }
                else state_data->send_countdown--; // no (-) destination so no done needed
              }
            }

          } // T_Msgdata_support==true
        } // T_PWQ_support==false

        TRACE_FORMAT( "dispatch %zu, connection_id %#X exit",
                      mcast->dispatch, mcast->connection_id);
        if (ranklist != NULL)
        {
          free(ranklist);
          delete dst_topology;
        }
        // send could have completed before we had calculated the correct send_countdown so maybe call done now.
        if ((state_data->send_countdown == 0) && !state_data->sendDoneCalled)
        {
          uint32_t connection_id = state_data->header_metadata.connection_id;
          TRACE_FORMAT( "done cb_done function %p, clientdata %p", state_data->cb_done.function, state_data->cb_done.clientdata);
          _connection.erase(connection_id);
          //_connection.clear(header->connection_id);
          state_data->sendDoneCalled = true;
          // Invoke the receive done callback.
          if (state_data->cb_done.function)
            state_data->cb_done.function (0,//_device.getContext(), ///\todo why does this assert?
                                          state_data->cb_done.clientdata,
                                          PAMI_SUCCESS);

        }

        TRACE_FN_EXIT();
        return PAMI_SUCCESS;

      }; // AxialMulticastModel::postMulticast_impl


      ///
      /// \brief Multicast header dispatch.
      ///
      ///
      /// \see PAMI::Device::Interface::RecvFunction_t
      ///
      template <bool T_Msgdata_support, bool T_PWQ_support>
      int AxialMulticastModel<T_Msgdata_support, T_PWQ_support>::dispatch_header (void   * metadata,
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

        AxialMulticastModel<T_Msgdata_support, T_PWQ_support> * model = (AxialMulticastModel<T_Msgdata_support, T_PWQ_support> *) arg;

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
          model->_dispatch_function(model->_device.getContext(),
				    (pami_quad_t*)msg,                              // Msgdata
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

          TRACE_FORMAT( "after dispatch remaining_length %zu, pwq %p", state_data->remaining_length, state_data->rcvpwq);

        }

        TRACE_FORMAT( "cookie = %p, root = %d, bytes = %zu remaining = %zd, sndlen = %d, connection id %u/%#X", cookie, (m->root), bytes,  state_data->remaining_length, m->sndlen, m->connection_id, m->connection_id);

        if (T_PWQ_support == false)
        {
          // If you're receiving data, it must all be ready in the pwq.
          PAMI_assertf(((state_data->rcvpwq == NULL) && (state_data->remaining_length == 0)) || (state_data->rcvpwq && (state_data->rcvpwq->bytesAvailableToProduce() >= state_data->remaining_length)), "state_data->rcvpwq->bytesAvailableToProduce() %zd,state_data->remaining_length %zd", state_data->rcvpwq ? state_data->rcvpwq->bytesAvailableToProduce() : -1, state_data->remaining_length);
        }

        model->processData(state_data, (uint8_t*) payload, bytes);

        TRACE_FN_EXIT();
        return 0;
      }; // PAMI::Device::MU::AxialMulticastModel::dispatch_header

      ///
      /// \brief Multicast data (after header) dispatch.
      ///
      ///
      /// \see PAMI::Device::Interface::RecvFunction_t
      ///
      template <bool T_Msgdata_support, bool T_PWQ_support>
      int AxialMulticastModel<T_Msgdata_support, T_PWQ_support>::dispatch_data   (void   * metadata,
                                                                                  void   * payload,
                                                                                  size_t   bytes,
                                                                                  void   * arg,
                                                                                  void   * cookie)
      {
        TRACE_FN_ENTER();
        TRACE_HEXDATA(metadata, 16);
        TRACE_HEXDATA(payload, 32);

        data_metadata_t * m = (data_metadata_t *)metadata;

        AxialMulticastModel<T_Msgdata_support, T_PWQ_support> * model = (AxialMulticastModel<T_Msgdata_support, T_PWQ_support> *) arg;

        TRACE_FORMAT( "cookie = %p, bytes = %zu, connection id %u/%#X", cookie, bytes, m->connection_id, m->connection_id);

        // Retrieve a state object
        state_data_t *state_data = model->_connection[m->connection_id]; //model->_connection.get(connection_id);
        PAMI_assert(state_data);

        model->processData(state_data, (uint8_t*) payload, bytes);

        TRACE_FN_EXIT();
        return 0;
      }; // PAMI::Device::MU::AxialMulticastModel::dispatch_data

      template <bool T_Msgdata_support, bool T_PWQ_support>
      inline void AxialMulticastModel<T_Msgdata_support, T_PWQ_support>::processData   (state_data_t * state_data,
                                                                                        uint8_t * payload,
                                                                                        size_t    bytes)
      {
        TRACE_FN_ENTER();
        header_metadata_t   *  header = &state_data->header_metadata;
        TRACE_FORMAT( "state_data %p, connection id %u, payload %p, bytes %zu, remaining length %zu, sndlen %u",
                      state_data, header->connection_id, payload, bytes, state_data->remaining_length, header->sndlen);


        // Number of bytes left to copy into the destination buffer
        size_t nleft = state_data->remaining_length;

        // Number of bytes left to copy from this packet
        if (nleft > bytes) nleft = bytes;

        if (nleft) // copy data and update receive state_data
        {
          TRACE_FORMAT( "memcpy(%p,%p,%zu)", state_data->buffer, payload, nleft);
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
          TRACE_FORMAT( "done cb_done function %p, clientdata %p", state_data->cb_done.function, state_data->cb_done.clientdata);
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
      }; // PAMI::Device::MU::AxialMulticastModel::processData


      ///
      /// \brief Send done callback.
      ///
      /// This callback will invoke the application completion
      /// callback function when all sends are done.
      ///
      template <bool T_Msgdata_support, bool T_PWQ_support>
      inline void AxialMulticastModel<T_Msgdata_support, T_PWQ_support>::sendDone(pami_context_t   context,
                                                                                  void           * cookie,
                                                                                  pami_result_t    result)
      {
        TRACE_FN_ENTER();
        typename AxialMulticastModel<T_Msgdata_support, T_PWQ_support>::state_data_t * state = (typename AxialMulticastModel<T_Msgdata_support, T_PWQ_support>::state_data_t *) cookie;
        uint32_t connection_id = state->header_metadata.connection_id;

        TRACE_FORMAT( "state_data %p, connection id %u, send_countdown %zu",
                      state, connection_id, state->send_countdown);

        if (--state->send_countdown == 0)
        {
          TRACE_FORMAT( "done cb_done function %p, clientdata %p", state->cb_done.function, state->cb_done.clientdata);
          AxialMulticastModel<T_Msgdata_support, T_PWQ_support> * model = state->model;
          model->_connection.erase(connection_id);
          //_connection.clear(header->connection_id);
          state->sendDoneCalled = true;
          // Invoke the receive done callback.
          if (state->cb_done.function)
            state->cb_done.function (0,//_device.getContext(), ///\todo why does this assert?
                                     state->cb_done.clientdata,
                                     PAMI_SUCCESS);

        }

        TRACE_FN_EXIT();
        return ;
      }; // PAMI::Device::MU::AxialMulticastModel::sendDone


      template <bool T_Msgdata_support, bool T_PWQ_support>
      pami_result_t AxialMulticastModel<T_Msgdata_support, T_PWQ_support>::registerMcastRecvFunction_impl(int                         dispatch_id,
                                                                                                          pami_dispatch_multicast_function func,
                                                                                                          void                       *arg)
      {
        TRACE_FN_ENTER();
        TRACE_FORMAT( "id %d, func %p, arg %p", dispatch_id, func, arg);
        PAMI_assert(multicast_model_active_message);

        _dispatch_function = func;
        _dispatch_arg = arg;
        pami_result_t status = PAMI_SUCCESS;
        TRACE_FORMAT( "register data model dispatch %d", dispatch_id);
        status = _data_model_p.init (dispatch_id,
                                     dispatch_data, this,
                                     NULL, NULL);
        TRACE_FORMAT( "data model status = %d", status);
        status = _data_model_m.init (dispatch_id,
                                     dispatch_data, this,
                                     NULL, NULL);
        TRACE_FORMAT( "data model status = %d", status);

        if (status == PAMI_SUCCESS)
        {
          TRACE_FORMAT( "register header  model dispatch %d", dispatch_id);
          status = _header_model_p.init (dispatch_id,
                                         dispatch_header, this,
                                         NULL, NULL);
          TRACE_FORMAT( "header model status = %d", status);
          status = _header_model_m.init (dispatch_id,
                                         dispatch_header, this,
                                         NULL, NULL);
          TRACE_FORMAT( "header model status = %d", status);
        }
        PAMI_assert(status == PAMI_SUCCESS);
        TRACE_FN_EXIT();
        return status;
      };
    };
  };
};


#endif // __components_devices_bgq_mu2_model_AxialMulticast_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
