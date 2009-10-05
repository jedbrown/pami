/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemSysDep.h
 * \brief ???
 *
 * \todo Remove this old crusty code. It should really be part of the
 *       build/configure system or some other sysdep component. The
 *       most important code here is the mem_sync() and mem_barrier()
 *       macros. These can also be implemented in a different way.
 */

#ifndef __components_devices_shmem_shmemsysdep_h__
#define __components_devices_shmem_shmemsysdep_h__

#ifdef __bgp__

/// The device supports a rendezvous style protocol for shaddr
//#define SUPPORT_SHADDR
#include <bpcore/ppc450_inlines.h>
#include <bpcore/bgp_atomic_ops.h>
#include <spi/kernel_interface.h>
#define mem_sync()	_bgp_msync()
#define mem_barrier()	_bgp_mbar()
typedef _BGP_Atomic atomic_t;

#else

#undef SUPPORT_SHADDR
#define mem_sync()    __sync_synchronize()
#define mem_barrier() __sync_synchronize()
#endif

#endif // __components_devices_shmem_shmemsysdep_h__
