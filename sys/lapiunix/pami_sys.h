/**
 * \file sys/lapiunix/pami_sys.h
 * \brief "Default" platform specifc type definitions for IBM's PAMI message layer.
 */

#ifndef __default_pami_sys_h__
#define __default_pami_sys_h__

/* We need 64 bytes memory region for both 32- and 64-bit on PE implementation */
#ifndef __64BIT__
#define PAMI_CLIENT_MEMREGION_SIZE_STATIC (16*sizeof(uintptr_t))
#else
#define PAMI_CLIENT_MEMREGION_SIZE_STATIC (8*sizeof(uintptr_t))
#endif

#define PAMI_WORK_SIZE_STATIC (8*sizeof(uintptr_t))
#define PAMI_REQUEST_NQUADS 512

#endif /* __default_pami_sys_h__ */
