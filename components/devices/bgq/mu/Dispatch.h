/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/Dispatch.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu_Dispatch_h__
#define __components_devices_bgq_mu_Dispatch_h__

#include <hwi/include/bqc/MU_PacketHeader.h>

#include "components/devices/PacketInterface.h"

namespace XMI
{
  namespace Device
  {
    namespace MU
    {
      typedef union MemoryFifoPacketHeader
      {
        MUHWI_PacketHeader_t hwi;
        struct
        {
          uint8_t reserved[13];
          uint8_t reserved0  : 2;
          uint8_t issingle   : 1;
          uint8_t singleonly : 5;
          union
          {
            struct
            {
              uint8_t metadata[16];
            } singlepkt;
            struct
            {
              uint8_t reserved[4];
              uint8_t metadata[12];
            } multipkt;
          };
          uint16_t dispatch_id;
        } dev;
      } MemoryFifoPacketHeader_t;


      typedef struct dispatch
      {
        Interface::RecvFunction_t   f;
        void                      * p;
      } dispatch_t;

    };   // XMI::Device::MU namespace
  };     // XMI::Device namespace
};       // XMI namespace

#endif

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
