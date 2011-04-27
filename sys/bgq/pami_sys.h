/**
 * \file sys/bgq/pami_sys.h
 * \brief Platform specifc type definitions for IBM's PAMI message layer.
 */

#ifndef __bgq_pami_sys_h__
#define __bgq_pami_sys_h__

#define PAMI_CLIENT_MEMREGION_SIZE_STATIC 8
#define PAMI_WORK_SIZE_STATIC (6*sizeof(uintptr_t))
#define PAMI_REQUEST_NQUADS 512

#endif /* __pami_type_h__ */
