/**
 * \file components/devices/shmemcoll/asmheader.h
 * \brief ???
 */
#ifndef __components_devices_shmemcoll_asmheader_h__
#define __components_devices_shmemcoll_asmheader_h__

/*#include <firmware/include/fwext/fwext.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <firmware/include/Firmware.h>
#include <hwi/include/bqc/A2_core.h>
#include <hwi/include/bqc/A2_inlines.h>

#include <firmware/include/fwext/fwext_lib.h>


#define L1P_CFG_MMIO     (0x3fde8000000ULL)

#define L1P_CFG_pf_invalidate_all       (0x80000000)
#define L1P_CFG_pf_hit_enable           (0x80000000>>1)
#define L1P_CFG_pf_hit_fwd_l2           (0x80000000>>2)
#define L1P_CFG_pf_dfc_disable          (0x80000000>>3)
#define L1P_CFG_pf_stream_enable        (0x80000000>>5)
#define L1P_CFG_pf_stream_optimistic    (0x80000000>>6)
#define L1P_CFG_pf_stream_est_on_dcbt   (0x80000000>>7)
#define L1P_CFG_pf_stream_depth         (0xE0000000>>9)
#define L1P_CFG_pf_adaptive_reset       (0x80000000>>12)
#define L1P_CFG_pf_adaptive_enable      (0x80000000>>13)
#define L1P_CFG_pf_adaptive_total_depth (0xF8000000>>15)
#define L1P_CFG_pf_list_enable        (0x80000000>>20)
#define L1P_CFG_wc_enable             (0x80000000>>24)
#define L1P_CFG_wc_suppress_if_all_be (0x80000000>>25)
#define L1P_CFG_l1_hit_fwd_l2         (0x80000000>>26)
//(0x4775F0C0ULL)
#define L1P_CFG_MMIO_VAL \
        ( L1P_CFG_pf_hit_enable\
        | L1P_CFG_pf_stream_enable        \
        | L1P_CFG_pf_stream_optimistic    \
        | L1P_CFG_pf_stream_est_on_dcbt   \
        | L1P_CFG_pf_stream_depth         \
        | L1P_CFG_pf_adaptive_enable      \
        | L1P_CFG_pf_adaptive_total_depth \
        | L1P_CFG_pf_list_enable        \
        | L1P_CFG_wc_enable             \
        | L1P_CFG_wc_suppress_if_all_be \
        | L1P_CFG_l1_hit_fwd_l2         )

#define L1P_CFG_MMIO_RESETVAL (L1P_CFG_MMIO_VAL| L1P_CFG_pf_adaptive_reset)
*/
/*
Other inlines
__INLINE__ int StoreConditional32( volatile uint32_t *pVar, uint32_t Val )
__INLINE__ uint32_t LoadReserved32( volatile uint32_t *pVar )
__INLINE__ int StoreConditional( volatile uint64_t *pVar, uint64_t Val )
__INLINE__ uint64_t LoadReserved( volatile uint64_t *pVar )
__INLINE__ void ppc_msync(void) { do { asm volatile ("msync" : : : "memory"); } while(0); }
__INLINE__ void mbar(void)  { do { asm volatile ("mbar"  : : : "memory"); } while(0); }
__INLINE__ void Delay( uint64_t pclks )
__INLINE__ void Delay_ns( uint64_t nsecs, uint64_t core_freq )
__INLINE__ void dcache_invalidate_all( void )     { do { asm volatile ("dccci 0,0" : : : "memory"); } while(0); }
__INLINE__ void dccci( void )                     { do { asm volatile ("dccci 0,0" : : : "memory"); } while(0); }
__INLINE__ void dcache_block_touch( void *vaddr ) { do { asm volatile ("dcbt 0,%0" : : "b" (vaddr) : "memory"); } while(0); }
__INLINE__ void icache_invalidate_all( void )     { do { asm volatile ("iccci 0,0" : : : "memory"); } while(0); }
__INLINE__ void iccci( void )                     { do { asm volatile ("iccci 0,0" : : : "memory"); } while(0); }
__INLINE__ void icache_block_touch( void *vaddr ) { do { asm volatile ("icbt 0,%0" : : "b" (vaddr) : "memory"); } while(0); }
__INLINE__ void Kernel_SetTimeBase( uint64_t pclks )
__INLINE__ uint64_t GetTimeBase( void )
*/



#define VECTOR_LOAD(si,sb,tgt) \
do { \
  asm volatile("qvlfdux %2,%0,%1": "+Ob" (si) : "r" (sb), "i" (tgt) :"memory"); \
} while(0)

/*
#define VECTOR_LOAD(si,sb,tgt) \
do { \
  asm volatile("qvlfdux %2,%0,%1": "+Ob" (si) : "r" (sb), "f" (tgt) :"memory"); \
} while(0)
*/
#define VECT_LOAD_NU(si,sb,tgt) \
do { \
  asm volatile("qvlfdx %2,%0,%1": : "Ob" (si), "r" (sb), "i" (tgt) :"memory"); \
} while(0)

#define VECTOR_STORE(si,sb,tgt) \
do { \
  asm volatile("qvstfdux %2,%0,%1": "+Ob" (si) : "r" (sb), "i" (tgt) :"memory"); \
} while(0)

/*#define VECTOR_STORE(si,sb,tgt) \
do { \
  asm volatile("qvstfdux %2,%0,%1": "+Ob" (si) : "r" (sb), "f" (tgt) :"memory"); \
} while(0)
*/
#define VECT_STORE_NU(si,sb,tgt) \
do { \
  asm volatile("qvstfdx %2,%0,%1": "+Ob" (si) : "r" (sb), "i" (tgt) :"memory"); \
} while(0)

#define VECTOR_STORE_REG(si,sb,tgt) \
do { \
  asm volatile("qvstfdux %2,%0,%1": "+Ob" (si) : "r" (sb), "f" (tgt) :"memory"); \
} while(0)

#if 0
#define VECTOR_SUM(si,sb,tgt) \
do { \
  asm volatile("qvfadd %0,%1,%2": "=r" (tgt) : "i" (si), "i" (sb)); \
} while(0)
#endif

#define VECTOR_SUM(si,sb,tgt) \
do { \
  asm volatile("qvfadd tgt,%0,%1":  : "i" (si), "i" (sb)); \
} while(0)
#if 0
#define VECTOR_SUM(si,sb,tgt) \
do { \
  asm volatile("qvfadd %0,%1,%2": "=f" (tgt) : "f" (si), "f" (sb)); \
} while(0)
#endif

#define VECTOR_SUM_REG(si,sb,tgt) \
do { \
  asm volatile("qvfadd %0,%1,%2": "=f" (tgt) : "i" (si), "f" (sb)); \
} while(0)

#endif
