/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/fifo/FifoPacket.h
 * \brief ???
 */

#ifndef __components_fifo_FifoPacket_h__
#define __components_fifo_FifoPacket_h__

#include <stdint.h>

#include "Arch.h"

#include "components/fifo/PacketInterface.h"

#ifndef TRACE
#define TRACE(x) // fprintf x
#endif

namespace PAMI
{
  namespace Fifo
  {
    ///
    /// \brief Simple packet implementation
    ///
    /// \note The packet payload size is the entire packet size less the
    ///       packet header size.
    ///
    /// \tparam T_HeaderSize Specifies the number of bytes in the packet header
    /// \tparam T_PacketSize Specifies the number of bytes in the entire packet
    ///
    template <unsigned T_HeaderSize, unsigned T_PacketSize>
    class FifoPacket : public Interface::Packet<FifoPacket <T_HeaderSize, T_PacketSize> >
    {
      public:

        friend class Interface::Packet<FifoPacket <T_HeaderSize, T_PacketSize> >;

        static const size_t header_size  = T_HeaderSize;

        static const size_t payload_size = T_PacketSize - T_HeaderSize;

        inline FifoPacket () :
            Interface::Packet<FifoPacket <T_HeaderSize, T_PacketSize> > ()
        {};


      protected:

        inline void clear_impl ()
        {
          memset ((void *)&_data[0], 0, T_PacketSize);
        }

        inline void * getHeader_impl ()
        {
          return (void *) &_data[0];
        };

        inline void copyHeader_impl (void * dst)
        {
          memcpy (dst, (void *)_data, T_HeaderSize);
        };

        inline void writeHeader_impl (void * src)
        {
          memcpy ((void *)_data, src, T_HeaderSize);
        };

        inline void * getPayload_impl ()
        {
          return (void *) &_data[T_HeaderSize];
        };

        inline void copyPayload_impl (void * addr)
        {
          memcpy ((void *) &_data[T_HeaderSize/sizeof(pami_quad_t)], addr, T_PacketSize - T_HeaderSize);
        };

      private:
        uint8_t _data[T_PacketSize];
    };
  };
};
#undef TRACE_ERR
#endif // __util_fifo_FifoPacket_h__


//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
