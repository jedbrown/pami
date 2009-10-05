/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file util/fifo/FifoPacket.h
 * \brief ???
 */

#ifndef __util_fifo_fifopacket_h__
#define __util_fifo_fifopacket_h__

#include <stdint.h>

#include "Packet.h"

#ifndef TRACE
#define TRACE(x)
#endif

namespace XMI
{
    namespace Fifo
    {
      template <unsigned T_HeaderSize, unsigned T_PacketSize>
      class FifoPacket : public Packet<FifoPacket <T_HeaderSize, T_PacketSize> >
      {
        public:
          inline FifoPacket () :
              Packet<FifoPacket <T_HeaderSize, T_PacketSize> > ()
          {};

          inline void clear_impl ()
          {
            memset ((void *)&_data[0], 0, T_PacketSize);
          }

          inline void * getHeader_impl ()
          {
            return (void *) &_data[0];
          };

          inline void copyHeader_impl (void * addr)
          {
            uint8_t * hdr = (uint8_t *) addr;
            unsigned i;

            for (i = 0; i < T_HeaderSize; i++) hdr[i] = _data[i];
          };

          inline void * getPayload ()
          {
            return (void *) &_data[T_HeaderSize];
          };

          inline void copyPayload_impl (void * addr)
          {
            uint8_t * dst = (uint8_t *) addr;
            uint8_t * payload = &_data[T_HeaderSize];
            unsigned i;

            for (i = 0; i < (T_PacketSize - T_HeaderSize); i++) dst[i] = payload[i];
          };

          static const size_t headerSize_impl  = T_HeaderSize;
          static const size_t payloadSize_impl = T_PacketSize - T_HeaderSize;
        private:
          uint8_t _data[T_PacketSize];
      };
  };
};
#undef TRACE_ERR
#endif // __util_fifo_fifopacket_h__


//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
