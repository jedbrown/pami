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

#include "components/devices/BaseDevice.h"
#include "components/devices/MessageDevice.h"
#include "components/devices/shmem/ShmemBaseDevice.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    template <class T_SysDep, class T_Fifo, class T_Packet>
    class ShmemPacketDevice : public ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>,
                              public Interface::BaseDevice<ShmemPacketDevice<T_SysDep, T_Fifo, T_Packet>, T_SysDep>,
                              public Interface::MessageDevice<ShmemPacketDevice<T_SysDep, T_Fifo, T_Packet> >
    {
      public:
        inline ShmemPacketDevice () :
            ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet> (),
            Interface::BaseDevice<ShmemPacketDevice<T_SysDep, T_Fifo, T_Packet>, T_SysDep> (),
            Interface::MessageDevice<ShmemPacketDevice<T_SysDep, T_Fifo, T_Packet> > ()
        {
        };

        inline ~ShmemPacketDevice () {};

        /// \see XMI::Device::Interface::BaseDevice::init()
        inline int init_impl (T_SysDep * sysdep)
        {
          TRACE_ERR((stderr, "(%zd) ShmemPacketDevice::init_impl (), this = %p\n", __global.mapping.task(), this));
          return this->init_internal (sysdep);
        };

        /// \see XMI::Device::Interface::BaseDevice::advance()
        inline int advance_impl ()
        {
          return this->advance_internal ();
        };
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
