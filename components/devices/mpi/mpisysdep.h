#ifndef __components_devices_mpi_mpisysdep_h__
#define __components_devices_mpi_mpisysdep_h__

#define mem_sync()    __sync_synchronize()
#define mem_barrier() __sync_synchronize()

#endif // __components_devices_mpi_mpisysdep_h__
