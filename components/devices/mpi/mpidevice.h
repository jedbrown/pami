#ifndef __components_devices_mpi_mpidevice_h__
#define __components_devices_mpi_mpidevice_h__

#include "components/devices/BaseDevice.h"
#include "components/devices/MessageDevice.h"

namespace XMI
{
  namespace Device
  {
    template <class T_SysDep>
    class MPIDevice : public Interface::BaseDevice<MPIDevice<T_SysDep>, T_SysDep>,
                      public Interface::MessageDevice<MPIDevice<T_SysDep> >
    {
    public:
      inline MPIDevice () :
        Interface::BaseDevice<MPIDevice<T_SysDep>, T_SysDep> (),
        Interface::MessageDevice<MPIDevice<T_SysDep> > ()
        {
        };

      // Implement BaseDevice Routines
      
      inline ~MPIDevice () {};
      inline int init_impl (T_SysDep * sysdep)
        {

        };

      inline bool isInit_impl ()
        {

        };
      inline int advance_impl ()
        {

        };

      // Implement MessageDevice Routines
      inline size_t getMessageMetadataSize_impl ()
        {

        }
      inline void   setConnection_impl (int channel, size_t rank, void * arg)
        {

        }
      inline void * getConnection_impl (int channel, size_t rank)
        {

        }

      // Implement Packet Device Routines 
      inline int    readData_impl(void * dst, size_t bytes)
        {

        }
      inline bool   requiresRead_impl()
        {

        }
      inline size_t getPacketMetadataSize_impl()
        {

        }
      inline size_t getPacketPayloadSize_impl()
        {

        }
      
    };
  };
};
#endif // __components_devices_mpi_mpipacketdevice_h__
