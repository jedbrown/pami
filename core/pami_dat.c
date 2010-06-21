/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file core/pami_dat.c
/// \brief Tables used by PAMI
///
/// Since C++ does not yet support designated initializers,
/// this needs to be C code.
///

#include <pami.h>
#include "math/math_coremath.h"


/**
 * \brief Translates a PAMI_Dt into its size
 */
int pami_dt_sizes[] = {
[PAMI_UNDEFINED_DT] =       0,
[PAMI_SIGNED_CHAR] =        sizeof(char),
[PAMI_UNSIGNED_CHAR] =      sizeof(unsigned char),
[PAMI_SIGNED_SHORT] =       sizeof(short),
[PAMI_UNSIGNED_SHORT] =     sizeof(unsigned short),
[PAMI_SIGNED_INT] =         sizeof(int),
[PAMI_UNSIGNED_INT] =       sizeof(unsigned int),
[PAMI_SIGNED_LONG_LONG] =   sizeof(long long),
[PAMI_UNSIGNED_LONG_LONG] = sizeof(unsigned long long),
[PAMI_FLOAT] =              sizeof(float),
[PAMI_DOUBLE] =             sizeof(double),
[PAMI_LONG_DOUBLE] =        sizeof(long double),
[PAMI_LOGICAL] =            sizeof(unsigned int),
[PAMI_SINGLE_COMPLEX] =     2 * sizeof(float),
[PAMI_DOUBLE_COMPLEX] =     2 * sizeof(double),
[PAMI_LOC_2INT] =           sizeof(int32_int32_t),
[PAMI_LOC_SHORT_INT] =      sizeof(int16_int32_t),
[PAMI_LOC_FLOAT_INT] =      sizeof(fp32_int32_t),
[PAMI_LOC_DOUBLE_INT] =     sizeof(fp64_int32_t),
[PAMI_LOC_2FLOAT] =         sizeof(fp32_fp32_t),
[PAMI_LOC_2DOUBLE] =        sizeof(fp64_fp64_t),
[PAMI_USERDEFINED_DT] =     0,
};

/**
 * \brief Translates a PAMI_Dt into its shift factor (bytes)
 *
 * This depends on pami_dt_sizes[] all being powers of 2.
 * Should probably compute this, and assert, at runtime rather
 * than hard-code it on presumed datatypes.
 */
int pami_dt_shift[] = {
[PAMI_UNDEFINED_DT] =       -1,
[PAMI_SIGNED_CHAR] =        0, /* sizeof(char) = 1 */
[PAMI_UNSIGNED_CHAR] =      0, /* sizeof(unsigned char) = 1 */
[PAMI_SIGNED_SHORT] =       1, /* sizeof(short) = 2 */
[PAMI_UNSIGNED_SHORT] =     1, /* sizeof(unsigned short) = 2 */
[PAMI_SIGNED_INT] =         2, /* sizeof(int) = 4 */
[PAMI_UNSIGNED_INT] =       2, /* sizeof(unsigned int) = 4 */
[PAMI_SIGNED_LONG_LONG] =   3, /* sizeof(long long) = 8 */
[PAMI_UNSIGNED_LONG_LONG] = 3, /* sizeof(unsigned long long) = 8 */
[PAMI_FLOAT] =              2, /* sizeof(float) = 4 */
[PAMI_DOUBLE] =             3, /* sizeof(double) = 8 */
[PAMI_LONG_DOUBLE] =        4, /* sizeof(long double) = 16 */
[PAMI_LOGICAL] =            2, /* sizeof(unsigned int) = 4 */
[PAMI_SINGLE_COMPLEX] =     3, /* 2 * sizeof(float) = 8 */
[PAMI_DOUBLE_COMPLEX] =     4, /* 2 * sizeof(double) = 16 */
[PAMI_LOC_2INT] =           3, /* sizeof(int32_int32_t) = 8 */
[PAMI_LOC_SHORT_INT] =      3, /* sizeof(int16_int32_t) = 8 */
[PAMI_LOC_FLOAT_INT] =      3, /* sizeof(fp32_int32_t) = 8 */
[PAMI_LOC_DOUBLE_INT] =     4, /* sizeof(fp64_int32_t) = 16 */
[PAMI_LOC_2FLOAT] =         3, /* sizeof(fp32_fp32_t) = 8 */
[PAMI_LOC_2DOUBLE] =        4, /* sizeof(fp64_fp64_t) = 16 */
[PAMI_USERDEFINED_DT] =     -1,
};

/**
 * \brief Select a math function for operation, datatype, and number of inputs
 * This table sohuld not be accessed directly. The inline/macro
 * MATH_OP_FUNCS(dt,op,n) should be used instead. That returns
 * the proper function (pointer) for the given combination of
 * datatype "dt", operand "op", and number of inputs "n", taking
 * into account conbinations that are not optimized.
 */
#ifndef MATH_NO_OPTIMATH
#define OPTIMATH_NSRC(dt,op,n,f)	[op][dt][n-1] = f,
#else /* MATH_NO_OPTIMATH */
#define OPTIMATH_NSRC(dt,op,n,f)
#endif /* MATH_NO_OPTIMATH */
void *math_op_funcs[PAMI_OP_COUNT][PAMI_DT_COUNT][MATH_MAX_NSRC] = {

OPTIMIZED_int8_max
OPTIMIZED_uint8_max
OPTIMIZED_int16_max
OPTIMIZED_uint16_max
OPTIMIZED_int32_max
OPTIMIZED_uint32_max
OPTIMIZED_int64_max
OPTIMIZED_uint64_max
OPTIMIZED_fp32_max
OPTIMIZED_fp64_max
OPTIMIZED_fp128_max

OPTIMIZED_int8_min
OPTIMIZED_uint8_min
OPTIMIZED_int16_min
OPTIMIZED_uint16_min
OPTIMIZED_int32_min
OPTIMIZED_uint32_min
OPTIMIZED_int64_min
OPTIMIZED_uint64_min
OPTIMIZED_fp32_min
OPTIMIZED_fp64_min
OPTIMIZED_fp128_min

OPTIMIZED_int8_sum
OPTIMIZED_uint8_sum
OPTIMIZED_int16_sum
OPTIMIZED_uint16_sum
OPTIMIZED_int32_sum
OPTIMIZED_uint32_sum
OPTIMIZED_int64_sum
OPTIMIZED_uint64_sum
OPTIMIZED_fp32_sum
OPTIMIZED_fp64_sum
OPTIMIZED_fp128_sum

OPTIMIZED_int8_prod
OPTIMIZED_uint8_prod
OPTIMIZED_int16_prod
OPTIMIZED_uint16_prod
OPTIMIZED_int32_prod
OPTIMIZED_uint32_prod
OPTIMIZED_int64_prod
OPTIMIZED_uint64_prod
OPTIMIZED_fp32_prod
OPTIMIZED_fp64_prod
OPTIMIZED_fp128_prod

OPTIMIZED_int8_land
OPTIMIZED_uint8_land
OPTIMIZED_int16_land
OPTIMIZED_uint16_land
OPTIMIZED_int32_land
OPTIMIZED_uint32_land
OPTIMIZED_int64_land
OPTIMIZED_uint64_land

OPTIMIZED_int8_lor
OPTIMIZED_uint8_lor
OPTIMIZED_int16_lor
OPTIMIZED_uint16_lor
OPTIMIZED_int32_lor
OPTIMIZED_uint32_lor
OPTIMIZED_int64_lor
OPTIMIZED_uint64_lor

OPTIMIZED_int8_lxor
OPTIMIZED_uint8_lxor
OPTIMIZED_int16_lxor
OPTIMIZED_uint16_lxor
OPTIMIZED_int32_lxor
OPTIMIZED_uint32_lxor
OPTIMIZED_int64_lxor
OPTIMIZED_uint64_lxor

OPTIMIZED_int8_band
OPTIMIZED_uint8_band
OPTIMIZED_int16_band
OPTIMIZED_uint16_band
OPTIMIZED_int32_band
OPTIMIZED_uint32_band
OPTIMIZED_int64_band
OPTIMIZED_uint64_band

OPTIMIZED_int8_bor
OPTIMIZED_uint8_bor
OPTIMIZED_int16_bor
OPTIMIZED_uint16_bor
OPTIMIZED_int32_bor
OPTIMIZED_uint32_bor
OPTIMIZED_int64_bor
OPTIMIZED_uint64_bor

OPTIMIZED_int8_bxor
OPTIMIZED_uint8_bxor
OPTIMIZED_int16_bxor
OPTIMIZED_uint16_bxor
OPTIMIZED_int32_bxor
OPTIMIZED_uint32_bxor
OPTIMIZED_int64_bxor
OPTIMIZED_uint64_bxor

OPTIMIZED_int32_int32_maxloc
OPTIMIZED_int16_int32_maxloc
OPTIMIZED_fp32_int32_maxloc
OPTIMIZED_fp64_int32_maxloc
OPTIMIZED_fp32_fp32_maxloc
OPTIMIZED_fp64_fp64_maxloc

OPTIMIZED_int32_int32_minloc
OPTIMIZED_int16_int32_minloc
OPTIMIZED_fp32_int32_minloc
OPTIMIZED_fp64_int32_minloc
OPTIMIZED_fp32_fp32_minloc
OPTIMIZED_fp64_fp64_minloc

/*
 * Default, generic N-way, routines are "nsrc == 0".
 * There must be real functions for every one of these.
 */
[PAMI_MAX][PAMI_SIGNED_CHAR][0] =	(void*)_pami_core_int8_max,
[PAMI_MAX][PAMI_UNSIGNED_CHAR][0] =	(void*)_pami_core_uint8_max,
[PAMI_MAX][PAMI_SIGNED_SHORT][0] =	(void*)_pami_core_int16_max,
[PAMI_MAX][PAMI_UNSIGNED_SHORT][0] =	(void*)_pami_core_uint16_max,
[PAMI_MAX][PAMI_SIGNED_INT][0] =	(void*)_pami_core_int32_max,
[PAMI_MAX][PAMI_UNSIGNED_INT][0] =	(void*)_pami_core_uint32_max,
[PAMI_MAX][PAMI_SIGNED_LONG_LONG][0] =	(void*)_pami_core_int64_max,
[PAMI_MAX][PAMI_UNSIGNED_LONG_LONG][0] =(void*)_pami_core_uint64_max,
[PAMI_MAX][PAMI_FLOAT][0] =		(void*)_pami_core_fp32_max,
[PAMI_MAX][PAMI_DOUBLE][0] =		(void*)_pami_core_fp64_max,
[PAMI_MAX][PAMI_LONG_DOUBLE][0] =	(void*)_pami_core_fp128_max,

[PAMI_MIN][PAMI_SIGNED_CHAR][0] =	(void*)_pami_core_int8_min,
[PAMI_MIN][PAMI_UNSIGNED_CHAR][0] =	(void*)_pami_core_uint8_min,
[PAMI_MIN][PAMI_SIGNED_SHORT][0] =	(void*)_pami_core_int16_min,
[PAMI_MIN][PAMI_UNSIGNED_SHORT][0] =	(void*)_pami_core_uint16_min,
[PAMI_MIN][PAMI_SIGNED_INT][0] =	(void*)_pami_core_int32_min,
[PAMI_MIN][PAMI_UNSIGNED_INT][0] =	(void*)_pami_core_uint32_min,
[PAMI_MIN][PAMI_SIGNED_LONG_LONG][0] =	(void*)_pami_core_int64_min,
[PAMI_MIN][PAMI_UNSIGNED_LONG_LONG][0] =(void*)_pami_core_uint64_min,
[PAMI_MIN][PAMI_FLOAT][0] =		(void*)_pami_core_fp32_min,
[PAMI_MIN][PAMI_DOUBLE][0] =		(void*)_pami_core_fp64_min,
[PAMI_MIN][PAMI_LONG_DOUBLE][0] =	(void*)_pami_core_fp128_min,

[PAMI_SUM][PAMI_SIGNED_CHAR][0] =	(void*)_pami_core_int8_sum,
[PAMI_SUM][PAMI_UNSIGNED_CHAR][0] =	(void*)_pami_core_uint8_sum,
[PAMI_SUM][PAMI_SIGNED_SHORT][0] =	(void*)_pami_core_int16_sum,
[PAMI_SUM][PAMI_UNSIGNED_SHORT][0] =	(void*)_pami_core_uint16_sum,
[PAMI_SUM][PAMI_SIGNED_INT][0] =	(void*)_pami_core_int32_sum,
[PAMI_SUM][PAMI_UNSIGNED_INT][0] =	(void*)_pami_core_uint32_sum,
[PAMI_SUM][PAMI_SIGNED_LONG_LONG][0] =	(void*)_pami_core_int64_sum,
[PAMI_SUM][PAMI_UNSIGNED_LONG_LONG][0] =(void*)_pami_core_uint64_sum,
[PAMI_SUM][PAMI_FLOAT][0] =		(void*)_pami_core_fp32_sum,
[PAMI_SUM][PAMI_DOUBLE][0] =		(void*)_pami_core_fp64_sum,
[PAMI_SUM][PAMI_LONG_DOUBLE][0] =	(void*)_pami_core_fp128_sum,

[PAMI_PROD][PAMI_SIGNED_CHAR][0] =	(void*)_pami_core_int8_prod,
[PAMI_PROD][PAMI_UNSIGNED_CHAR][0] =	(void*)_pami_core_uint8_prod,
[PAMI_PROD][PAMI_SIGNED_SHORT][0] =	(void*)_pami_core_int16_prod,
[PAMI_PROD][PAMI_UNSIGNED_SHORT][0] =	(void*)_pami_core_uint16_prod,
[PAMI_PROD][PAMI_SIGNED_INT][0] =	(void*)_pami_core_int32_prod,
[PAMI_PROD][PAMI_UNSIGNED_INT][0] =	(void*)_pami_core_uint32_prod,
[PAMI_PROD][PAMI_SIGNED_LONG_LONG][0] =	(void*)_pami_core_int64_prod,
[PAMI_PROD][PAMI_UNSIGNED_LONG_LONG][0] =(void*)_pami_core_uint64_prod,
[PAMI_PROD][PAMI_FLOAT][0] =		(void*)_pami_core_fp32_prod,
[PAMI_PROD][PAMI_DOUBLE][0] =		(void*)_pami_core_fp64_prod,
[PAMI_PROD][PAMI_LONG_DOUBLE][0] =	(void*)_pami_core_fp128_prod,

[PAMI_LAND][PAMI_SIGNED_CHAR][0] =	(void*)_pami_core_int8_land,
[PAMI_LAND][PAMI_UNSIGNED_CHAR][0] =	(void*)_pami_core_uint8_land,
[PAMI_LAND][PAMI_SIGNED_SHORT][0] =	(void*)_pami_core_int16_land,
[PAMI_LAND][PAMI_UNSIGNED_SHORT][0] =	(void*)_pami_core_uint16_land,
[PAMI_LAND][PAMI_SIGNED_INT][0] =	(void*)_pami_core_int32_land,
[PAMI_LAND][PAMI_UNSIGNED_INT][0] =	(void*)_pami_core_uint32_land,
[PAMI_LAND][PAMI_SIGNED_LONG_LONG][0] =	(void*)_pami_core_int64_land,
[PAMI_LAND][PAMI_UNSIGNED_LONG_LONG][0] =(void*)_pami_core_uint64_land,
[PAMI_LAND][PAMI_FLOAT][0] =		(void*)_pami_core_fp32_land,
[PAMI_LAND][PAMI_DOUBLE][0] =		(void*)_pami_core_fp64_land,

[PAMI_LOR][PAMI_SIGNED_CHAR][0] =	(void*)_pami_core_int8_lor,
[PAMI_LOR][PAMI_UNSIGNED_CHAR][0] =	(void*)_pami_core_uint8_lor,
[PAMI_LOR][PAMI_SIGNED_SHORT][0] =	(void*)_pami_core_int16_lor,
[PAMI_LOR][PAMI_UNSIGNED_SHORT][0] =	(void*)_pami_core_uint16_lor,
[PAMI_LOR][PAMI_SIGNED_INT][0] =	(void*)_pami_core_int32_lor,
[PAMI_LOR][PAMI_UNSIGNED_INT][0] =	(void*)_pami_core_uint32_lor,
[PAMI_LOR][PAMI_SIGNED_LONG_LONG][0] =	(void*)_pami_core_int64_lor,
[PAMI_LOR][PAMI_UNSIGNED_LONG_LONG][0] =(void*)_pami_core_uint64_lor,
[PAMI_LOR][PAMI_FLOAT][0] =		(void*)_pami_core_fp32_lor,
[PAMI_LOR][PAMI_DOUBLE][0] =		(void*)_pami_core_fp64_lor,

[PAMI_LXOR][PAMI_SIGNED_CHAR][0] =	(void*)_pami_core_int8_lxor,
[PAMI_LXOR][PAMI_UNSIGNED_CHAR][0] =	(void*)_pami_core_uint8_lxor,
[PAMI_LXOR][PAMI_SIGNED_SHORT][0] =	(void*)_pami_core_int16_lxor,
[PAMI_LXOR][PAMI_UNSIGNED_SHORT][0] =	(void*)_pami_core_uint16_lxor,
[PAMI_LXOR][PAMI_SIGNED_INT][0] =	(void*)_pami_core_int32_lxor,
[PAMI_LXOR][PAMI_UNSIGNED_INT][0] =	(void*)_pami_core_uint32_lxor,
[PAMI_LXOR][PAMI_SIGNED_LONG_LONG][0] =	(void*)_pami_core_int64_lxor,
[PAMI_LXOR][PAMI_UNSIGNED_LONG_LONG][0] =(void*)_pami_core_uint64_lxor,
[PAMI_LXOR][PAMI_FLOAT][0] =		(void*)_pami_core_fp32_lxor,
[PAMI_LXOR][PAMI_DOUBLE][0] =		(void*)_pami_core_fp64_lxor,

[PAMI_BAND][PAMI_SIGNED_CHAR][0] =	(void*)_pami_core_int8_band,
[PAMI_BAND][PAMI_UNSIGNED_CHAR][0] =	(void*)_pami_core_uint8_band,
[PAMI_BAND][PAMI_SIGNED_SHORT][0] =	(void*)_pami_core_int16_band,
[PAMI_BAND][PAMI_UNSIGNED_SHORT][0] =	(void*)_pami_core_uint16_band,
[PAMI_BAND][PAMI_SIGNED_INT][0] =	(void*)_pami_core_int32_band,
[PAMI_BAND][PAMI_UNSIGNED_INT][0] =	(void*)_pami_core_uint32_band,
[PAMI_BAND][PAMI_SIGNED_LONG_LONG][0] =	(void*)_pami_core_int64_band,
[PAMI_BAND][PAMI_UNSIGNED_LONG_LONG][0] =(void*)_pami_core_uint64_band,
[PAMI_BAND][PAMI_FLOAT][0] =		(void*)_pami_core_fp32_band,
[PAMI_BAND][PAMI_DOUBLE][0] =		(void*)_pami_core_fp64_band,

[PAMI_BOR][PAMI_SIGNED_CHAR][0] =	(void*)_pami_core_int8_bor,
[PAMI_BOR][PAMI_UNSIGNED_CHAR][0] =	(void*)_pami_core_uint8_bor,
[PAMI_BOR][PAMI_SIGNED_SHORT][0] =	(void*)_pami_core_int16_bor,
[PAMI_BOR][PAMI_UNSIGNED_SHORT][0] =	(void*)_pami_core_uint16_bor,
[PAMI_BOR][PAMI_SIGNED_INT][0] =	(void*)_pami_core_int32_bor,
[PAMI_BOR][PAMI_UNSIGNED_INT][0] =	(void*)_pami_core_uint32_bor,
[PAMI_BOR][PAMI_SIGNED_LONG_LONG][0] =	(void*)_pami_core_int64_bor,
[PAMI_BOR][PAMI_UNSIGNED_LONG_LONG][0] =(void*)_pami_core_uint64_bor,
[PAMI_BOR][PAMI_FLOAT][0] =		(void*)_pami_core_fp32_bor,
[PAMI_BOR][PAMI_DOUBLE][0] =		(void*)_pami_core_fp64_bor,

[PAMI_BXOR][PAMI_SIGNED_CHAR][0] =	(void*)_pami_core_int8_bxor,
[PAMI_BXOR][PAMI_UNSIGNED_CHAR][0] =	(void*)_pami_core_uint8_bxor,
[PAMI_BXOR][PAMI_SIGNED_SHORT][0] =	(void*)_pami_core_int16_bxor,
[PAMI_BXOR][PAMI_UNSIGNED_SHORT][0] =	(void*)_pami_core_uint16_bxor,
[PAMI_BXOR][PAMI_SIGNED_INT][0] =	(void*)_pami_core_int32_bxor,
[PAMI_BXOR][PAMI_UNSIGNED_INT][0] =	(void*)_pami_core_uint32_bxor,
[PAMI_BXOR][PAMI_SIGNED_LONG_LONG][0] =	(void*)_pami_core_int64_bxor,
[PAMI_BXOR][PAMI_UNSIGNED_LONG_LONG][0] =(void*)_pami_core_uint64_bxor,
[PAMI_BXOR][PAMI_FLOAT][0] =		(void*)_pami_core_fp32_bxor,
[PAMI_BXOR][PAMI_DOUBLE][0] =		(void*)_pami_core_fp64_bxor,

[PAMI_MAXLOC][PAMI_LOC_2INT][0] =	(void*)_pami_core_int32_int32_maxloc,
[PAMI_MAXLOC][PAMI_LOC_SHORT_INT][0] =	(void*)_pami_core_int16_int32_maxloc,
[PAMI_MAXLOC][PAMI_LOC_FLOAT_INT][0] =	(void*)_pami_core_fp32_int32_maxloc,
[PAMI_MAXLOC][PAMI_LOC_DOUBLE_INT][0] =	(void*)_pami_core_fp64_int32_maxloc,
[PAMI_MAXLOC][PAMI_LOC_2FLOAT][0] =	(void*)_pami_core_fp32_fp32_maxloc,
[PAMI_MAXLOC][PAMI_LOC_2DOUBLE][0] =	(void*)_pami_core_fp64_fp64_maxloc,

[PAMI_MINLOC][PAMI_LOC_2INT][0] =	(void*)_pami_core_int32_int32_minloc,
[PAMI_MINLOC][PAMI_LOC_SHORT_INT][0] =	(void*)_pami_core_int16_int32_minloc,
[PAMI_MINLOC][PAMI_LOC_FLOAT_INT][0] =	(void*)_pami_core_fp32_int32_minloc,
[PAMI_MINLOC][PAMI_LOC_DOUBLE_INT][0] =	(void*)_pami_core_fp64_int32_minloc,
[PAMI_MINLOC][PAMI_LOC_2FLOAT][0] =	(void*)_pami_core_fp32_fp32_minloc,
[PAMI_MINLOC][PAMI_LOC_2DOUBLE][0] =	(void*)_pami_core_fp64_fp64_minloc,

};


#undef OPTIMATH_NSRC
