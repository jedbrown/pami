/**
 * \file components/devices/shmemcoll/sum.h
 * \brief ???
 */
#ifndef __components_devices_shmemcoll_sum_h__
#define __components_devices_shmemcoll_sum_h__
#if 0
#include <firmware/include/fwext/fwext.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <firmware/include/Firmware.h>
#include <hwi/include/bqc/A2_core.h>
#include <hwi/include/bqc/A2_inlines.h>

#include <firmware/include/fwext/fwext_lib.h>

#endif
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
