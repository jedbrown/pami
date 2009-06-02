/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2005, 2007.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */
#include <assert.h>

#include "../include/pgasrt.h"

#if defined(__cplusplus)
extern "C" {
#endif


DECL_STATIC_MUTEX(__xlupc_rmw_fxp_mutex);
DECL_STATIC_MUTEX(__xlupc_rmw_flp_mutex);

/* ------------------------------------------------------------------------ */
/** \brief atomic read modify update for fixed point types                  */
/* ------------------------------------------------------------------------ */

#define PGASRT_DEFINE_LOCAL_FXP_UPDATE_MUTEX(name_ext,type)                   \
void                                                                         \
__pgasrt_update_##name_ext(type *array_elt, type *val, __pgasrt_ops_t op)    \
{                                                                            \
   MUTEX_LOCK(&__xlupc_rmw_fxp_mutex);                                       \
   switch(op) {                                                              \
     case PGASRT_OP_ADD:  *array_elt += *(type *)val; break;                 \
     case PGASRT_OP_MUL:  *array_elt *= *(type *)val; break;                 \
     case PGASRT_OP_DIV:  *array_elt /= *(type *)val; break;                 \
     case PGASRT_OP_AND:  *array_elt &= *(type *)val; break;                 \
     case PGASRT_OP_OR:   *array_elt |= *(type *)val; break;                 \
     case PGASRT_OP_XOR:  *array_elt ^= *(type *)val; break;                 \
     case PGASRT_OP_MAX:  if(*(type *)val > *array_elt)                      \
                             *array_elt = *(type *)val;                      \
                          break;                                             \
     case PGASRT_OP_MIN:  if(*(type *)val < *array_elt)                      \
                             *array_elt = *(type *)val;                      \
                          break;                                             \
     default: assert(((void)"Unknown operator for update", 0)); break;       \
   }                                                                         \
   MUTEX_UNLOCK(&__xlupc_rmw_fxp_mutex);                                     \
}

/* ------------------------------------------------------------------------ */
/** atomic read modify update for floating point types                      */
/* ------------------------------------------------------------------------ */

#define PGASRT_DEFINE_LOCAL_FLP_UPDATE_MUTEX(name_ext,type)                   \
void                                                                         \
__pgasrt_update_##name_ext(type *array_elt, type *val, __pgasrt_ops_t op)    \
{                                                                            \
   MUTEX_LOCK(&__xlupc_rmw_flp_mutex); /* should be changed to larx/stcx */  \
    switch(op) {                                                             \
     case PGASRT_OP_ADD:  *array_elt += *(type *)val; break;                 \
     case PGASRT_OP_MUL:  *array_elt *= *(type *)val; break;                 \
     case PGASRT_OP_DIV:  *array_elt /= *(type *)val; break;                 \
     case PGASRT_OP_MAX:  if(*(type *)val > *array_elt)                      \
                             *array_elt = *(type *)val;                      \
                          break;                                             \
     case PGASRT_OP_MIN:  if(*(type *)val < *array_elt)                      \
                             *array_elt = *(type *)val;                      \
                          break;                                             \
     case PGASRT_OP_AND:                                                     \
     case PGASRT_OP_OR:                                                      \
     case PGASRT_OP_XOR: assert(((void)"Invalid update op for floats",0));   \
                             break;                                          \
     default: assert(((void)"Unknown operator for update", 0)); break;       \
    }                                                                        \
   MUTEX_UNLOCK(&__xlupc_rmw_flp_mutex);                                     \
}

/* ------------------------------------------------------------------------ */
/** \brief optimized update for 32 bit integers                             */
/* ------------------------------------------------------------------------ */

#define PGASRT_DEFINE_LOCAL_FXP_UPDATE_LWARX(name_ext,type)                  \
void                                                                        \
__pgasrt_update_##name_ext(type *array_elt, type *val, __pgasrt_ops_t op)   \
{                                                                           \
  type old, temp;                                                           \
  int rc = 0;                                                               \
  do {                                                                      \
    old = (type)__lwarx((int *)array_elt);                                  \
    switch(op) {                                                            \
    case PGASRT_OP_ADD: temp = old + *val; break;                           \
    case PGASRT_OP_MUL: temp = old * *val; break;                           \
    case PGASRT_OP_DIV: temp = old / *val; break;                           \
    case PGASRT_OP_AND: temp = old & *val; break;                           \
    case PGASRT_OP_OR:  temp = old | *val; break;                           \
    case PGASRT_OP_XOR: temp = old ^ *val; break;                           \
    case PGASRT_OP_MAX: temp = (*val > old) ? *val : old; break;            \
    case PGASRT_OP_MIN: temp = (*val < old) ? *val : old; break;            \
    }                                                                       \
  }  while( !__stwcx((int *)array_elt, temp) );                             \
}

/* ------------------------------------------------------------------------ */
/** \brief optimized update for 64 bit integers                             */
/* ------------------------------------------------------------------------ */

#define PGASRT_DEFINE_LOCAL_FXP_UPDATE_LDARX(name_ext,type)                  \
void                                                                        \
__pgasrt_update_##name_ext(type *array_elt, type *val, __pgasrt_ops_t op)   \
{                                                                           \
  type old, temp;                                                           \
  int rc = 0;                                                               \
  do {                                                                      \
    old = (type)__ldarx((int *)array_elt);                                  \
    switch(op) {                                                            \
    case PGASRT_OP_ADD: temp = old + *val; break;                           \
    case PGASRT_OP_MUL: temp = old * *val; break;                           \
    case PGASRT_OP_DIV: temp = old / *val; break;                           \
    case PGASRT_OP_AND: temp = old & *val; break;                           \
    case PGASRT_OP_OR:  temp = old | *val; break;                           \
    case PGASRT_OP_XOR: temp = old ^ *val; break;                           \
    case PGASRT_OP_MAX: temp = (*val > old) ? *val : old; break;            \
    case PGASRT_OP_MIN: temp = (*val < old) ? *val : old; break;            \
    }                                                                       \
  }  while( !__stdcx((int *)array_elt, temp) );                             \
}

/* ************************************************************************* */
/*            subword or float -- supported only through mutex               */
/* ************************************************************************* */

PGASRT_DEFINE_LOCAL_FXP_UPDATE_MUTEX(byte, unsigned char);
PGASRT_DEFINE_LOCAL_FXP_UPDATE_MUTEX(chr, char);
PGASRT_DEFINE_LOCAL_FXP_UPDATE_MUTEX(srt, short);
PGASRT_DEFINE_LOCAL_FXP_UPDATE_MUTEX(hwrd, unsigned short);
PGASRT_DEFINE_LOCAL_FLP_UPDATE_MUTEX(dbl, double);
PGASRT_DEFINE_LOCAL_FLP_UPDATE_MUTEX(flt, float);

/* ************************************************************************* */
/*              32 bit and 64 bit atomic updates                             */
/* ************************************************************************* */

#if defined (__GNUC__) || defined (__PGASRT_DISTRIBUTED) || 1
/* no larx/stcx support in gcc or on distributed (BG/L) */
PGASRT_DEFINE_LOCAL_FXP_UPDATE_MUTEX(int, int);
PGASRT_DEFINE_LOCAL_FXP_UPDATE_MUTEX(word, unsigned int);
PGASRT_DEFINE_LOCAL_FXP_UPDATE_MUTEX(llg, long long);
PGASRT_DEFINE_LOCAL_FXP_UPDATE_MUTEX(dwrd, unsigned long long);
#else /* XLC */
#include <builtins.h>
/* 32 bit atomic updates */
PGASRT_DEFINE_LOCAL_FXP_UPDATE_LWARX(int, int);
PGASRT_DEFINE_LOCAL_FXP_UPDATE_LWARX(word, unsigned int);
#if defined(__PPC64__) || defined(__64BIT__)
/* 64 bit atomic updates */
PGASRT_DEFINE_LOCAL_FXP_UPDATE_LDARX(llg, long long);
PGASRT_DEFINE_LOCAL_FXP_UPDATE_LDARX(dwrd, unsigned long long);
#else /* 32 bit */
/* PPC does not support atomic 64 bit updates in 32 bit mode */
PGASRT_DEFINE_LOCAL_FXP_UPDATE_MUTEX(llg, long long);
PGASRT_DEFINE_LOCAL_FXP_UPDATE_MUTEX(dwrd, unsigned long long);
#endif /* __PPC64__ */
#endif /* XLC */

/* ************************************************************************* */
/*                  implementation of generalized update                     */
/* ************************************************************************* */

void __pgasrt_update (__pgasrt_local_addr_t          d, 
		      __pgasrt_local_addr_t          s,
		      __pgasrt_dtypes_t             type,
		      __pgasrt_ops_t               op)
{
  switch (type)
    {
    case PGASRT_DT_byte:
      __pgasrt_update_byte((unsigned char *)d, (unsigned char *)s, op);
      break;
    case PGASRT_DT_chr:
      __pgasrt_update_chr((char *)d, (char *)s, op);
      break;
    case PGASRT_DT_srt:
      __pgasrt_update_srt((short *)d, (short *)s, op);
      break;
    case PGASRT_DT_hwrd:
      __pgasrt_update_hwrd((unsigned short *)d, (unsigned short *)s, op);
      break;
    case PGASRT_DT_int:
      __pgasrt_update_int((int *)d, (int *)s, op);
      break;
    case PGASRT_DT_word:
      __pgasrt_update_word((unsigned int *)d, (unsigned int *)s, op);
      break;
    case PGASRT_DT_llg:
      __pgasrt_update_llg((long long *)d, (long long *)s, op);
      break;
    case PGASRT_DT_dwrd:
      __pgasrt_update_dwrd((unsigned long long *)d,(unsigned long long *)s,op);
      break;
    case PGASRT_DT_dbl:
      __pgasrt_update_dbl((double *)d, (double *)s, op);
      break;
    case PGASRT_DT_flt:
      __pgasrt_update_flt((float *)d, (float *)s, op);
      break;
    case PGASRT_DT_dblint:
      __pgasrt_fatalerror (-1, "dblint datatype not supported for updates");
      break;
    }
}

#if defined(__cplusplus)
}
#endif
