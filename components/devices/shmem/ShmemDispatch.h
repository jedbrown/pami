/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemDispatch.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemDispatch_h__
#define __components_devices_shmem_ShmemDispatch_h__

#include <sys/uio.h>

#include <pami.h>

#include "util/fifo/Fifo.h"
#include "util/queue/CircularQueue.h"


#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {
      template < class T_Packet, unsigned T_SetCount = 512 /*8192*/, unsigned T_SetSize = 8 >
      class Dispatch : public PAMI::Fifo::Interface::PacketConsumer< Dispatch<T_Packet, T_SetCount, T_SetSize> >
      {
        protected:

          class UnexpectedPacket : public CircularQueue::Element
          {
            public:

              inline UnexpectedPacket (T_Packet * packet, size_t s = 0) :
                  CircularQueue::Element (),
                  sequence (s)
              {
                id = T_Packet::getDispatch (*packet);
                bytes = T_Packet::payload_size;
                memcpy ((void *) meta, T_Packet::getMetadata (*packet), T_Packet::header_size);
                memcpy ((void *) data, packet->getPayload (), T_Packet::payload_size);
              };

              ///
              /// \see PAMI::Device::Interface::RecvFunction_t
              ///
              static int unexpected (void   * metadata,
                                     void   * payload,
                                     size_t   bytes,
                                     void   * recv_func_parm,
                                     void   * cookie)
              {
                // The metadata is at the front of the packet.
                T_Packet * pkt = (T_Packet *) metadata;

                UnexpectedPacket * uepkt =
                  (UnexpectedPacket *) malloc (sizeof(UnexpectedPacket));
                new ((void *)uepkt) UnexpectedPacket (pkt);

                CircularQueue * q = (CircularQueue *) recv_func_parm;
                q->enqueue ((CircularQueue::Element *) uepkt);

                return 0;
              }

              uint16_t id;
              size_t   sequence;
              uint8_t  meta[T_Packet::header_size];
              uint8_t  data[T_Packet::payload_size];
              size_t   bytes;
          };

        public:

          friend class PAMI::Fifo::Interface::PacketConsumer< Dispatch >;

          inline Dispatch ()
          {
            COMPILE_TIME_ASSERT(T_SetCount*T_SetSize <= 65536); // 65536 == "UINT16_MAX"

            // Initialize the registered receive function array to unexpected().
            size_t i;

            for (i = 0; i < T_SetCount*T_SetSize; i++)
              {
                _function[i]   = UnexpectedPacket::unexpected;
                _clientdata[i] = (void *) & _ueQ[i/T_SetSize];
              }
          };

          inline ~Dispatch () {};

          pami_result_t registerUserDispatch (size_t                      set,
                                              Interface::RecvFunction_t   function,
                                              void                      * clientdata,
                                              uint16_t                  & id)
          {
//fprintf(stderr,"ShmemDispatch::registerUserDispatch(%zu, %p, %p, ...)\n", set, function, clientdata);
            if (set >= T_SetCount) return PAMI_ERROR;


            // Find the next available id for this dispatch set.
            bool found_free_slot = false;
            size_t n = set * T_SetSize + T_SetCount;

            for (id = set * T_SetSize; id < n; id++)
              {
                if (_function[id] == (Interface::RecvFunction_t) UnexpectedPacket::unexpected)
                  {
                    found_free_slot = true;
                    break;
                  }
              }
//fprintf(stderr,"ShmemDispatch::registerUserDispatch(%zu, %p, %p, ...), id = %d\n", set, function, clientdata, id);

            if (!found_free_slot) return PAMI_ERROR;

            _function[id]   = function;
            _clientdata[id] = clientdata;
//fprintf(stderr,"ShmemDispatch::registerUserDispatch(%zu, %p, %p, ...), _function = %p, _clientdata = %p\n", set, function, clientdata, _function, _clientdata);

            // Deliver any unexpected packets for registered dispatch ids. Stop at
            // the first unexpected packet for an un-registered dispatch id.
            UnexpectedPacket * uepkt = NULL;

            while ((uepkt = (UnexpectedPacket *) _ueQ[set].peek()) != NULL)
              {
                if (_function[uepkt->id] != UnexpectedPacket::unexpected)
                  {
                    // Invoke the registered dispatch function
                    TRACE_ERR((stderr, "   (%zu) ShmemDevice::registerRecvFunction() uepkt = %p, uepkt->id = %u\n", __global.mapping.task(), uepkt, uepkt->id));
                    _function[uepkt->id] (uepkt->meta,
                                          uepkt->data,
                                          uepkt->bytes,
                                          _clientdata[uepkt->id],
                                          uepkt->data);

                    // Remove the unexpected packet from the queue and free
                    _ueQ[set].dequeue();
                    free (uepkt);
                  }
                else
                  {
                    // Stop unexpected queue processing.  This maintains packet order
                    // which is required for protocols such as eager.
                    break;
                  }
              }

//fprintf(stderr,"ShmemDispatch::registerUserDispatch(%zu, %p, %p, ...) .. done\n", set, function, clientdata);
            TRACE_ERR((stderr, "<< (%zu) ShmemDevice::registerRecvFunction() => %d\n", __global.mapping.task(), id));
            return PAMI_SUCCESS;
          };

          pami_result_t registerSystemDispatch (Interface::RecvFunction_t   function,
                                                void                      * clientdata,
                                                uint16_t                  & id)
          {
            // Search in reverse order for an unregistered dispatch id and
            // assign this system dispatch function to it.

            bool found_free_slot = false;

            for (id = T_SetSize * T_SetCount - 1; id > 0; id--)
              {
                if (_function[id] == (Interface::RecvFunction_t) UnexpectedPacket::unexpected)
                  {
                    found_free_slot = true;
                    break;
                  }
              }

            if (!found_free_slot) return PAMI_ERROR;

            _function[id]   = function;
            _clientdata[id] = clientdata;

            // Deliver any unexpected packets for registered system dispatch ids.
            UnexpectedPacket * uepkt = NULL;

            while ((uepkt = (UnexpectedPacket *) _system_ueQ.peek()) != NULL)
              {
                if (_function[uepkt->id] != UnexpectedPacket::unexpected)
                  {
                    // Invoke the registered dispatch function
                    _function[uepkt->id] (uepkt->meta,
                                          uepkt->data,
                                          uepkt->bytes,
                                          _clientdata[uepkt->id],
                                          uepkt->data);

                    // Remove the unexpected packet from the queue and free
                    _system_ueQ.dequeue();
                    free (uepkt);
                  }
              }

            return PAMI_SUCCESS;
          };

          void dispatch (uint16_t id, void * metadata, void * payload, size_t bytes)
          {
//fprintf(stderr, "ShmemDispatch::dispatch(%d, %p, %p, %zu)\n", id, metadata, payload, bytes);
//fprintf(stderr, "ShmemDispatch::dispatch(%d, %p, %p, %zu), _function = %p, _clientdata = %p\n", id, metadata, payload, bytes, _function, _clientdata);
//fprintf(stderr, "ShmemDispatch::dispatch(%d, %p, %p, %zu), _function[%d] = %p, _clientdata[%d] = %p\n", id, metadata, payload, bytes, id, _function[id], id, _clientdata[id]);
            _function[id] (metadata, payload, bytes, _clientdata[id], payload);
          };

        protected:

          template <class T_FifoPacket>
          inline bool consume_impl (T_FifoPacket & packet)
          {
//fprintf(stderr, "ShmemDispatch::consume_impl(...)\n");
            uint16_t id = T_Packet::getDispatch (packet);
//fprintf(stderr, "ShmemDispatch::consume_impl(...), id = %d\n", id);
//fprintf(stderr, "ShmemDispatch::consume_impl(...), _function[%d] = %p, _clientdata[%d] = %p\n", id, _function[id], id, _clientdata[id]);
            _function[id] (T_Packet::getMetadata (packet),
                           packet.getPayload (),
                           T_Packet::payload_size,
                           _clientdata[id],
                           packet.getPayload ());

            return true;
          };

          Interface::RecvFunction_t   _function[T_SetCount*T_SetSize];
          void                      * _clientdata[T_SetCount*T_SetSize];

          CircularQueue               _ueQ[T_SetCount];
          CircularQueue               _system_ueQ;

      };
    };
  };
};
#undef TRACE_ERR

#endif // __components_devices_shmem_ShmemDispatch_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
