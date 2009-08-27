/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemPacketDevice.h
 * \brief ???
 */

#ifndef __components_devices_shmem_shmempacketdevice_h__
#define __components_devices_shmem_shmempacketdevice_h__

#include "ShmemBaseDevice.h"
#include "../BaseDevice.h"
#include "../MessageDevice.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)  // fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    template <class T_Fifo, class T_Packet>
    class ShmemPacketDevice : public ShmemBaseDevice<T_Fifo, T_Packet>,
                              public Interface::BaseDevice<ShmemPacketDevice<T_Fifo, T_Packet> >,
                              public Interface::MessageDevice<ShmemPacketDevice<T_Fifo, T_Packet> >
    {
      public:
        inline ShmemPacketDevice () :
            ShmemBaseDevice<T_Fifo, T_Packet> (),
            Message::Device<ShmemPacketDevice<T_Fifo, T_Packet> > ()
        {
        };

        inline ~ShmemPacketDevice () {};

        /// \see XMI::Device::Interface::BaseDevice::init()
        inline int init_impl (SysDep & sysdep)
        {
          return ShmemBaseDevice<T_Fifo, T_Packet>::init_internal (sysdep);
        }

        /// \see XMI::Device::Interface::BaseDevice::advance()
        inline int advance_impl ()
        {
          return ShmemBaseDevice<T_Fifo, T_Packet>::advance_internal ();
        }
    };
  };
};
#undef TRACE_ERR
#endif // __components_devices_shmem_shmempacketdevice_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
