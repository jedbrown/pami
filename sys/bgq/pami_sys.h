/**
 * \file sys/bgq/pami_sys.h
 * \brief Platform specifc type definitions for IBM's PAMI message layer.
 *
 * \defgroup bgq_pami_sys BG/Q specifc interface definitions.
 *
 * \{
 */

#ifndef __bgq_pami_sys_h__pre__
#define __bgq_pami_sys_h__pre__

/**
 * \defgroup bgq_pami_sys_pre pami.h pre-include definitions
 */

#define PAMI_CLIENT_MEMREGION_SIZE_STATIC (sizeof(uintptr_t))
#define PAMI_WORK_SIZE_STATIC (6*sizeof(uintptr_t))
#define PAMI_REQUEST_NQUADS 512

/** \} */ /* end of "bgq_pami_sys_pre" group */

#else
#ifndef __bgq_pami_sys_h__post__
#define __bgq_pami_sys_h__post__

/**
 * \defgroup bgq_pami_sys_post pami.h post-include definitions
 */
#define PAMI_ENDPOINT_NULL 0
#define PAMI_Endpoint_create(__client,__task,__offset,__endpoint) ({ *(__endpoint) = (((__offset) << 23) | (__task)); PAMI_SUCCESS; })
#define PAMI_Endpoint_query(__endpoint,__task,__offset) ({ *(__task) = (__endpoint) & 0x007fffff; *(__offset) = ((__endpoint) >> 23) & 0x03f; PAMI_SUCCESS; })

/** \} */ /* end of "bgq_pami_sys_post" group */

#endif /* __bgq_pami_sys_h__post__ */
#endif /* __bgq_pami_sys_h__pre__ */

/** \} */ /* end of "bgq_pami_sys" group */
