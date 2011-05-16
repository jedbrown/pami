/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/UnexpectedPacketQueue.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu2_UnexpectedPacketQueue_h__
#define __components_devices_bgq_mu2_UnexpectedPacketQueue_h__

#include "util/queue/Queue.h"

#include "util/trace.h"
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      template <typename T>
      class UnexpectedPacketQueue : public Queue
      {
        protected:

          class UnexpectedPacket : public Queue::Element
          {
            public:

              UnexpectedPacket (uint8_t  * metadata,
                                uint8_t  * payload,
                                size_t     bytes,
                                uint16_t   id) :
                  Queue::Element (),
                  _metadata (metadata),
                  _payload (payload),
                  _bytes (bytes),
                  _id (id)
              {
              };

              ~UnexpectedPacket () {};

              bool invoke (Device::Interface::RecvFunction_t fn, void * recv_func_parm, T id)
              {
                if (_id != id)
                  return false;

                fn (_metadata, _payload, _bytes, recv_func_parm, _payload);

                return true;
              };

            protected:

              uint8_t  * _metadata;
              uint8_t  * _payload;
              size_t     _bytes;
              uint16_t   _id;
          };

        public:

          UnexpectedPacketQueue (T dispatch_count, size_t maximum_metadata_bytes) :
              Queue (),
              _maximum_metadata_bytes (maximum_metadata_bytes)
          {
            UnexpectedPacketQueue ** queue = (UnexpectedPacketQueue **) malloc (sizeof(T) * dispatch_count + sizeof (UnexpectedPacketQueue *));
            *queue = this;

            uintptr_t ptr = (uintptr_t) queue;
            _id = (T *) (ptr + sizeof(uintptr_t));

            unsigned i;

            for (i = 0; i < dispatch_count; i++)
              {
                _id[i] = i;
              }
          };

          ~UnexpectedPacketQueue () {};

          void * getCookie (size_t id)
          {
            return (void *) & _id[id];
          };


          void dispatch (uint16_t id, Device::Interface::RecvFunction_t fn, void * recv_func_parm)
          {
            UnexpectedPacket * packet =
              (UnexpectedPacket *) peek ();

            while (packet != NULL)
              {
                UnexpectedPacket * current = packet;
                packet = (UnexpectedPacket *) packet->next();

                if (current->invoke (fn, recv_func_parm, id))
                  {
                    remove (current);
                    free (current);
                  }
              }
          };

          //protected:

          /// \see PAMI::Device::Interface::RecvFunction_t
          static int dispatch_fn (void   * metadata,
                                  void   * payload,
                                  size_t   bytes,
                                  void   * recv_func_parm,
                                  void   * cookie)
          {
            uint16_t * id = (uint16_t *) recv_func_parm;
            UnexpectedPacketQueue * queue = (UnexpectedPacketQueue *) (id - *id - 1);

            UnexpectedPacket * packet = (UnexpectedPacket *) malloc (sizeof(UnexpectedPacket) + bytes + queue->_maximum_metadata_bytes);

            uint8_t * tmp_payload  = (uint8_t *) (packet + 1);
            memcpy (tmp_payload, payload, bytes);

            uint8_t * tmp_metadata = (uint8_t *) (tmp_payload + bytes);
            memcpy (tmp_metadata, metadata, queue->_maximum_metadata_bytes);

            new (packet) UnexpectedPacket (tmp_metadata, tmp_payload, bytes, *id);

            queue->enqueue (packet);

            fprintf (stderr, "Warning. Dispatch to unregistered id (%d).\n", *id);
            return 0;
          };

          uint16_t              * _id;
          unsigned                _maximum_metadata_bytes;

      };   // class PAMI::Device::MU::UnexpectedPacketQueue
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_UnexpectedPacketQueue_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
