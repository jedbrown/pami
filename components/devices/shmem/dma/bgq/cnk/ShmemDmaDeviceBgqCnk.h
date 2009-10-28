/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file devices/prod/cdi/shmem/dma/bgp/cnk/ShmemDmaDeviceBgqCnk.h
 * \brief ???
 */

#ifndef __components_devices_shmem_dmadevicebgqcnk_h__
#define __components_devices_shmem_dmadevicebgqcnk_h__

#include "components/devices/BaseDevice.h"
#include "components/devices/MessageDevice.h"
#include "components/devices/shmem/ShmemBaseDevice.h"

namespace XMI 
{
  namespace Device 
  {
    template <class T_Fifo, class T_Packet>
    class ShmemDmaDeviceBgqCnk : 
					public ShmemBaseDevice<T_Fifo, T_Packet>, 
					public Interface::BaseDevice<ShmemDmaDeviceBgqCnk<T_Fifo, T_Packet>, XMI::SysDep>, 
					public Interface::MessageDevice<ShmemDmaDeviceBgqCnk<T_Fifo, T_Packet> >
    {
      public:
		  inline ShmemDmaDeviceBgqCnk () :
            ShmemBaseDevice<T_Fifo, T_Packet> (),
            Interface::BaseDevice<ShmemDmaDeviceBgqCnk< T_Fifo, T_Packet>, XMI::SysDep> (),
            Interface::MessageDevice<ShmemDmaDeviceBgqCnk<T_Fifo, T_Packet> > ()
        { };

        inline ~ShmemDmaDeviceBgqCnk () {};

        /// \see XMI::Device::Interface::BaseDevice::init()
        inline int init_impl (XMI::SysDep * sysdep )
        {
          //TRACE_ERR((stderr, "(%zd) ShmemPacketDevice::init_impl (), this = %p\n", sysdep->mapping.task(), this));
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

#endif /* __dcmf_cdi_device_shmem_dma_bgp_cnk_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
