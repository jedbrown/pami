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

#ifndef __util_fifo_FifoPacket_h__
#define __util_fifo_FifoPacket_h__

#include <stdint.h>

#include "Arch.h"

#include "util/fifo/Packet.h"

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
          {
            COMPILE_TIME_ASSERT(T_HeaderSize%sizeof(xmi_quad_t)==0);
            COMPILE_TIME_ASSERT(T_PacketSize%sizeof(xmi_quad_t)==0);
          };

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
            if(likely((size_t)addr & 0x0f == 0))
              Type<xmi_quad_t>::copy<T_HeaderSize>((xmi_quad_t *) addr, _data);
            else
              Type<size_t>::copy<T_HeaderSize>((size_t *) addr, (size_t *) _data);
          };

          inline void * getPayload ()
          {
            return (void *) &_data[T_HeaderSize/sizeof(xmi_quad_t)];
          };

          inline void copyPayload_impl (void * addr)
          {
            if(likely((size_t)addr & 0x0f == 0))
              Type<xmi_quad_t>::copy<T_PacketSize-T_HeaderSize> ((xmi_quad_t *) addr, &_data[T_HeaderSize]);
            else
              Type<size_t>::copy<T_PacketSize-T_HeaderSize> ((size_t *) addr, (size_t *) &_data[T_HeaderSize]);
          };

          static const size_t headerSize_impl  = T_HeaderSize;
          static const size_t payloadSize_impl = T_PacketSize - T_HeaderSize;
        private:
          xmi_quad_t _data[T_PacketSize/sizeof(xmi_quad_t)];
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
