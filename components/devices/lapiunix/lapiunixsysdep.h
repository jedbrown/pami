/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapiunix/lapiunixsysdep.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_lapiunixsysdep_h__
#define __components_devices_lapiunix_lapiunixsysdep_h__

#define mem_sync()    __sync_synchronize()
#define mem_barrier() __sync_synchronize()

#endif // __components_devices_lapiunix_lapiunixsysdep_h__
