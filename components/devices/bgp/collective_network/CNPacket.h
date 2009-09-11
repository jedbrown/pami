/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgp/collective_network/CNPacket.h
 * \brief Morphing of collective network constants/structs into C++ classes
 */
#ifndef __components_devices_bgp_cnpacket_h__
#define __components_devices_bgp_cnpacket_h__

#if defined(__bgp__)

/* -------------------------------------------------------------------------- */
/*                                                    BG/P tree packet header */
/* -------------------------------------------------------------------------- */
#include "spi/bgp_SPI.h"
#include "spi/bgp_collective_spi.h"

namespace XMI {
namespace Device {
namespace BGP {

      enum { COMBINE_OP_NONE=_BGP_TR_OP_NONE, COMBINE_OP_OR =_BGP_TR_OP_OR,
             COMBINE_OP_AND =_BGP_TR_OP_AND,  COMBINE_OP_XOR=_BGP_TR_OP_XOR,
             COMBINE_OP_MAX =_BGP_TR_OP_MAX,  COMBINE_OP_ADD=_BGP_TR_OP_ADD };

      struct CNPacket {
        _BGP_TreeHwHdr _hh;  /**< hardware header */

        /* constructors */
        CNPacket (const CNPacket &model) { _hh = model._hh; }

        CNPacket (unsigned cls, unsigned opcode, unsigned opsize, unsigned tag) {
          _hh.CtvHdr.Class    = cls;
          _hh.CtvHdr.Ptp      = 0;
          _hh.CtvHdr.Irq      = 0;
          _hh.CtvHdr.OpCode   = opcode;
          _hh.CtvHdr.OpSize   = opsize;
          _hh.CtvHdr.Tag      = tag;
          _hh.CtvHdr.CsumMode = _BGP_TR_CSUM_NONE;
        }

        CNPacket (unsigned cls, unsigned p2paddr) {
          _hh.PtpHdr.Class     = cls;
          _hh.PtpHdr.Ptp       = 1;
          _hh.PtpHdr.Irq       = 0;
          _hh.PtpHdr.PtpTarget = p2paddr;
          _hh.PtpHdr.CsumMode  = _BGP_TR_CSUM_NONE;
        }


        /* accessor functions */
        unsigned pclass () const { return _hh.PtpHdr.Class; }
        unsigned p2p    () const { return _hh.PtpHdr.Ptp; }
        unsigned irq    () const { return _hh.PtpHdr.Irq; }
        unsigned opcode () const { return _hh.CtvHdr.OpCode; }
        unsigned opsize () const { return _hh.CtvHdr.OpSize; }
        unsigned tag    () const { return _hh.CtvHdr.Tag; }
        unsigned p2paddr() const { return _hh.PtpHdr.PtpTarget; }
      }; /* CNPacket */

}; /* BGP */
}; /* Device */
}; /* XMI */

#endif /* defined(__bgp__) */

#endif /* __components_devices_bgp_cnpacket_h__ */
