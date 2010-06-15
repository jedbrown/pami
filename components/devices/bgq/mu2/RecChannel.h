/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/RecChannel.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu2_RecChannel_h__
#define __components_devices_bgq_mu2_RecChannel_h__

#include <spi/include/mu/RecFifo.h>

#include "components/devices/bgq/mu2/MemoryFifoPacketHeader.h"

#include "components/devices/bgq/mu2/trace.h"
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      ///
      /// \brief The reception channel component encapsulates all resources
      ///        associated with a reception fifo.
      ///
      class RecChannel
      {
        protected:

          /// Total number of dispatch sets
          static const size_t dispatch_set_count = 256;

          /// Number of dispatch functions in a dispatch set
          static const size_t dispatch_set_size  = 16;

          /// System notification dispatch identifier
          static const uint16_t dispatch_system_notify = dispatch_set_count * dispatch_set_size - 1;

          typedef struct
          {
            pami_event_function   fn;
            void                * cookie;
          } notify_t;

          /// \see PAMI::Device::Interface::RecvFunction_t
          static int notify (void   * metadata,
                             void   * payload,
                             size_t   bytes,
                             void   * recv_func_parm,
                             void   * cookie)
          {
            TRACE_FN_ENTER();
            notify_t * n = (notify_t *) payload;

            n->fn (recv_func_parm, // a.k.a. "pami_context_t"
                   n->cookie,
                   PAMI_SUCCESS);

            TRACE_FN_EXIT();
            return 0;
          };


          typedef struct
          {
            Interface::RecvFunction_t   f;
            void                      * p;
          } mu_dispatch_t;

          /// \see PAMI::Device::Interface::RecvFunction_t
          static int error (void   * metadata,
                            void   * payload,
                            size_t   bytes,
                            void   * recv_func_parm,
                            void   * cookie)
          {
            fprintf (stderr, "Error. Dispatch to unregistered id (%zu).\n", (size_t) recv_func_parm);
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return 0;
          };

        public:

          ///
          /// \brief Reception channel constructor
          ///
          inline RecChannel (size_t id = 0) :
              _channel_id (id),
              _channel_cookie (NULL),
              _rfifo (NULL)
          {
            TRACE_FN_ENTER();
            COMPILE_TIME_ASSERT(sizeof(notify_t) <= MemoryFifoPacketHeader::packet_singlepacket_metadata_size);

            // Zero-out the descriptor models before initialization
            memset((void *)&self, 0, sizeof(MUSPI_DescriptorBase));


            // ----------------------------------------------------------------
            // Set the common base descriptor fields
            // ----------------------------------------------------------------
            MUSPI_BaseDescriptorInfoFields_t base;
            memset((void *)&base, 0, sizeof(base));

            base.Pre_Fetch_Only  = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;
            base.Payload_Address = 0;
            base.Message_Length  = 0;
            base.Torus_FIFO_Map  = 0;
            base.Dest.Destination.Destination = 0;

            self.setBaseFields (&base);


            // ----------------------------------------------------------------
            // Set the common point-to-point descriptor fields
            // ----------------------------------------------------------------
            MUSPI_Pt2PtDescriptorInfoFields_t pt2pt;
            memset((void *)&pt2pt, 0, sizeof(pt2pt));

            pt2pt.Hints_ABCD = 0;
            pt2pt.Skip       = 0;
            pt2pt.Misc1 =
              MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
              MUHWI_PACKET_DO_NOT_DEPOSIT |
              MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE;
            pt2pt.Misc2 =
              MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC;

            self.setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
            self.PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;   // huh?
            self.setPt2PtFields (&pt2pt);


            // ----------------------------------------------------------------
            // Set the common memory fifo descriptor fields
            // ----------------------------------------------------------------
            MUSPI_MemoryFIFODescriptorInfoFields_t memfifo;
            memset ((void *)&memfifo, 0, sizeof(memfifo));

            memfifo.Rec_FIFO_Id    = 0;
            memfifo.Rec_Put_Offset = 0;
            memfifo.Interrupt      = MUHWI_DESCRIPTOR_DO_NOT_INTERRUPT_ON_PACKET_ARRIVAL;
            memfifo.SoftwareBit    = 0;

            self.setMemoryFIFOFields (&memfifo);
            self.setMessageUnitPacketType (MUHWI_PACKET_TYPE_FIFO);


            // In loopback we send only on AM
            self.setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);
            self.setHints (MUHWI_PACKET_HINT_AM |
                           MUHWI_PACKET_HINT_B_NONE |
                           MUHWI_PACKET_HINT_C_NONE |
                           MUHWI_PACKET_HINT_D_NONE,
                           MUHWI_PACKET_HINT_E_NONE);

            // Set the payload information.
            self.setPayload (0, 0);

            // ----------------------------------------------------------------
            // Set the "notify" system dispatch identifier
            // ----------------------------------------------------------------
            MemoryFifoPacketHeader * hdr =
              (MemoryFifoPacketHeader *) & self.PacketHeader;
            hdr->setSinglePacket (true);
            hdr->setDispatchId (dispatch_system_notify);

#if (DO_TRACE_DEBUG==1)
            self.dump();
#endif

            TRACE_FN_EXIT();
          };

          ///
          /// \brief Reception channel destructor
          ///
          inline ~RecChannel () {};

          ///
          /// \brief Initialize the reception channel
          ///
          /// The reception channel class does not allocate memory for the MU
          /// reception fifo and other resources. An external entity allocates
          /// these resources and then provides them to the reception channel
          /// object with this initialization method.
          ///
          /// \param[in] f                   An initialized reception fifo
          /// \param[in] channel_cookie      Cookie delivered to all completion
          ///                                events invoked by this channel,
          ///                                a.k.a., "pami_context_t"
          ///
          inline void initialize (uint16_t              rfifo_id,
                                  MUSPI_RecFifo_t     * rfifo,
                                  MUHWI_Destination_t * dest,
                                  void                * channel_cookie)
          {
            TRACE_FN_ENTER();

            _rfifo_id = rfifo_id;
            _rfifo    = rfifo;

            // Initialize the dispatch table. This 'error' function will be
            // replaced with an 'unexpected packet' function and queue.
            size_t i, n = dispatch_set_count * dispatch_set_size;

            for (i = 0; i < n; i++)
              {
                _dispatch[i].f = error;
                _dispatch[i].p = (void *) i;
              }

            // Initialize any mu "system" dispatch functions
            _dispatch[dispatch_system_notify].f = notify;
            _dispatch[dispatch_system_notify].p = channel_cookie; // pami_context_t;

            // ----------------------------------------------------------------
            // Initialize the memory fifo descriptor to route to "self"
            // ----------------------------------------------------------------
            self.setRecFIFOId (rfifo_id);
            self.setDestination (*dest);

            TRACE_FN_EXIT();
          };

          inline void initializeNotifySelfDescriptor (MUSPI_DescriptorBase & desc,
                                                      pami_event_function    fn,
                                                      void                 * cookie)
          {
            TRACE_FN_ENTER();

            // Clone the "self" descriptor
            self.clone (desc);

            // Copy the completion function+cookie into the packet header.
            notify_t * hdr = (notify_t *) & desc.PacketHeader;
            hdr->fn = fn;
            hdr->cookie = cookie;

            TRACE_FN_EXIT();
          };


          ///
          /// \brief Register a packet handler function for a dispatch set
          ///
          /// The function and cookie are invoked ...
          ///
          /// The registration may fail if the requested dispatch set number is
          /// greater than the number of allocated dispatch sets, or if there
          /// are no free dispatch identifiers in the requested dispatch set
          /// due to previous registrations on the requested dispatch set.
          ///
          /// \see MU::RecChannel::dispatch_set_count
          /// \see MU::RecChannel::dispatch_set_size
          ///
          /// \param[in]  set    Dispatch set number
          /// \param[in]  fn     Dispatch function
          /// \param[in]  cookie Dispatch cookie
          /// \param[out] id     Assigned MU dispatch identifier
          ///
          /// \retval true  Successful packet handler registration
          /// \retval false Unsuccessful packet handler registration
          ///
          inline bool registerPacketHandler (size_t                      set,
                                             Interface::RecvFunction_t   fn,
                                             void                      * cookie,
                                             uint16_t                  & id)
          {
            TRACE_FN_ENTER();

            if (set >= dispatch_set_count) return false;

            unsigned i;

            for (i = 0; i < dispatch_set_size; i++)
              {
                id = set * dispatch_set_size + i;

                if (_dispatch[id].f == error)
                  {
                    _dispatch[id].f = fn;
                    _dispatch[id].p = cookie;

                    TRACE_FORMAT("registration success. set = %zu, fn = %p, cookie = %p --> id = %d", set, fn, cookie, id);
                    TRACE_FN_EXIT();
                    return true;
                  }
              }

            TRACE_FORMAT("registration failure. set = %zu, fn = %p, cookie = %p", set, fn, cookie);
            TRACE_FN_EXIT();
            return false;
          };

          inline unsigned advance ()
          {
            TRACE_FN_ENTER();

            unsigned packets = 0;
            uint32_t wrap = 0;
            uint32_t cur_bytes = 0;
            uint32_t total_bytes = 0;
            uint32_t cumulative_bytes = 0;
            MemoryFifoPacketHeader *hdr = NULL;

            MUSPI_RecFifo_t * rfifo = _rfifo;
            TRACE_FORMAT("MUSPI_RecFifo_t * = %p", rfifo);

            while ((total_bytes = MUSPI_getAvailableBytes (rfifo, &wrap)) != 0)
              {
                if (wrap)   //Extra branch over older packet loop
                  {
                    hdr = (MemoryFifoPacketHeader *) MUSPI_getNextPacketWrap (rfifo, &cur_bytes);
                    uint16_t id = 0;
                    void *metadata;
                    hdr->getHeaderInfo (id, &metadata);

                    _dispatch[id].f(metadata, hdr + 1, cur_bytes - 32, _dispatch[id].p, hdr + 1);
                    packets++;

                    TRACE_FORMAT("Received packet wrap of size %d, total bytes %d", cur_bytes, total_bytes);
                  }
                else
                  {
                    cumulative_bytes = 0;

                    while (cumulative_bytes < total_bytes )
                      {
                        hdr = (MemoryFifoPacketHeader *) MUSPI_getNextPacketOptimized (rfifo, &cur_bytes);
                        cumulative_bytes += cur_bytes;
                        uint16_t id = 0;
                        void *metadata;
                        hdr->getHeaderInfo (id, &metadata);
                        TRACE_FORMAT("dispatch = %d, metadata = %p", id, metadata);
                        _dispatch[id].f(metadata, hdr + 1, cur_bytes - 32, _dispatch[id].p, hdr + 1);
                        packets++;
                        //Touch head for next packet
                        TRACE_FORMAT("Received packet of size %d, cumulative bytes = %d, total bytes %d", cur_bytes, cumulative_bytes, total_bytes);
                      }
                  }

                MUSPI_syncRecFifoHwHead (rfifo);
              }

            TRACE_FORMAT("Number of packets received: %d", packets);
            TRACE_FN_EXIT();
            return packets;
          }

          // Memory fifo descriptor initialized to send to the reception
          // fifo in this reception channel
          MUSPI_DescriptorBase   self;

        private:

          size_t            _channel_id;     // a.k.a., MU::Context "fifo number"
          void            * _channel_cookie; // a.k.a., "pami_context_t"
          MUSPI_RecFifo_t * _rfifo;          // The actual reception fifo
          uint16_t          _rfifo_id;       // MUSPI reception fifo id

          mu_dispatch_t     _dispatch[dispatch_set_count * dispatch_set_size];

      }; // class     PAMI::Device::MU::RecChannel
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_RecChannel_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

