/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file xmi/core/xmi_dat.c
/// \brief Tables used by XMI
///
/// Since C++ does not yet support designated initializers,
/// this needs to be C code.
///

#include "xmi.h"
#include "math_coremath.h"

/**
 * \brief Translates a XMI_Dt into its size
 */
int xmi_dt_sizes[] = {
[XMI_UNDEFINED_DT] =       0,
[XMI_SIGNED_CHAR] =        sizeof(char),
[XMI_UNSIGNED_CHAR] =      sizeof(unsigned char),
[XMI_SIGNED_SHORT] =       sizeof(short),
[XMI_UNSIGNED_SHORT] =     sizeof(unsigned short),
[XMI_SIGNED_INT] =         sizeof(int),
[XMI_UNSIGNED_INT] =       sizeof(unsigned int),
[XMI_SIGNED_LONG_LONG] =   sizeof(long long),
[XMI_UNSIGNED_LONG_LONG] = sizeof(unsigned long long),
[XMI_FLOAT] =              sizeof(float),
[XMI_DOUBLE] =             sizeof(double),
[XMI_LONG_DOUBLE] =        sizeof(long double),
[XMI_LOGICAL] =            sizeof(unsigned int),
[XMI_SINGLE_COMPLEX] =     2 * sizeof(float),
[XMI_DOUBLE_COMPLEX] =     2 * sizeof(double),
[XMI_LOC_2INT] =           sizeof(int32_int32_t),
[XMI_LOC_SHORT_INT] =      sizeof(int16_int32_t),
[XMI_LOC_FLOAT_INT] =      sizeof(fp32_int32_t),
[XMI_LOC_DOUBLE_INT] =     sizeof(fp64_int32_t),
[XMI_LOC_2FLOAT] =         sizeof(fp32_fp32_t),
[XMI_LOC_2DOUBLE] =        sizeof(fp64_fp64_t),
[XMI_USERDEFINED_DT] =     0,
};

/**
 * \brief Translates a XMI_Dt into its shift factor (bytes)
 *
 * This depends on xmi_dt_sizes[] all being powers of 2.
 * Should probably compute this, and assert, at runtime rather
 * than hard-code it on presumed datatypes.
 */
int xmi_dt_shift[] = {
[XMI_UNDEFINED_DT] =       -1,
[XMI_SIGNED_CHAR] =        0, /* sizeof(char) = 1 */
[XMI_UNSIGNED_CHAR] =      0, /* sizeof(unsigned char) = 1 */
[XMI_SIGNED_SHORT] =       1, /* sizeof(short) = 2 */
[XMI_UNSIGNED_SHORT] =     1, /* sizeof(unsigned short) = 2 */
[XMI_SIGNED_INT] =         2, /* sizeof(int) = 4 */
[XMI_UNSIGNED_INT] =       2, /* sizeof(unsigned int) = 4 */
[XMI_SIGNED_LONG_LONG] =   3, /* sizeof(long long) = 8 */
[XMI_UNSIGNED_LONG_LONG] = 3, /* sizeof(unsigned long long) = 8 */
[XMI_FLOAT] =              2, /* sizeof(float) = 4 */
[XMI_DOUBLE] =             3, /* sizeof(double) = 8 */
[XMI_LONG_DOUBLE] =        4, /* sizeof(long double) = 16 */
[XMI_LOGICAL] =            2, /* sizeof(unsigned int) = 4 */
[XMI_SINGLE_COMPLEX] =     3, /* 2 * sizeof(float) = 8 */
[XMI_DOUBLE_COMPLEX] =     4, /* 2 * sizeof(double) = 16 */
[XMI_LOC_2INT] =           3, /* sizeof(int32_int32_t) = 8 */
[XMI_LOC_SHORT_INT] =      3, /* sizeof(int16_int32_t) = 8 */
[XMI_LOC_FLOAT_INT] =      3, /* sizeof(fp32_int32_t) = 8 */
[XMI_LOC_DOUBLE_INT] =     4, /* sizeof(fp64_int32_t) = 16 */
[XMI_LOC_2FLOAT] =         3, /* sizeof(fp32_fp32_t) = 8 */
[XMI_LOC_2DOUBLE] =        4, /* sizeof(fp64_fp64_t) = 16 */
[XMI_USERDEFINED_DT] =     -1,
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
void *math_op_funcs[XMI_OP_COUNT][XMI_DT_COUNT][MATH_MAX_NSRC] = {

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
[XMI_MAX][XMI_SIGNED_CHAR][0] =	_core_int8_max,
[XMI_MAX][XMI_UNSIGNED_CHAR][0] =	_core_uint8_max,
[XMI_MAX][XMI_SIGNED_SHORT][0] =	_core_int16_max,
[XMI_MAX][XMI_UNSIGNED_SHORT][0] =	_core_uint16_max,
[XMI_MAX][XMI_SIGNED_INT][0] =	_core_int32_max,
[XMI_MAX][XMI_UNSIGNED_INT][0] =	_core_uint32_max,
[XMI_MAX][XMI_SIGNED_LONG_LONG][0] =	_core_int64_max,
[XMI_MAX][XMI_UNSIGNED_LONG_LONG][0] =_core_uint64_max,
[XMI_MAX][XMI_FLOAT][0] =		_core_fp32_max,
[XMI_MAX][XMI_DOUBLE][0] =		_core_fp64_max,
[XMI_MAX][XMI_LONG_DOUBLE][0] =	_core_fp128_max,

[XMI_MIN][XMI_SIGNED_CHAR][0] =	_core_int8_min,
[XMI_MIN][XMI_UNSIGNED_CHAR][0] =	_core_uint8_min,
[XMI_MIN][XMI_SIGNED_SHORT][0] =	_core_int16_min,
[XMI_MIN][XMI_UNSIGNED_SHORT][0] =	_core_uint16_min,
[XMI_MIN][XMI_SIGNED_INT][0] =	_core_int32_min,
[XMI_MIN][XMI_UNSIGNED_INT][0] =	_core_uint32_min,
[XMI_MIN][XMI_SIGNED_LONG_LONG][0] =	_core_int64_min,
[XMI_MIN][XMI_UNSIGNED_LONG_LONG][0] =_core_uint64_min,
[XMI_MIN][XMI_FLOAT][0] =		_core_fp32_min,
[XMI_MIN][XMI_DOUBLE][0] =		_core_fp64_min,
[XMI_MIN][XMI_LONG_DOUBLE][0] =	_core_fp128_min,

[XMI_SUM][XMI_SIGNED_CHAR][0] =	_core_int8_sum,
[XMI_SUM][XMI_UNSIGNED_CHAR][0] =	_core_uint8_sum,
[XMI_SUM][XMI_SIGNED_SHORT][0] =	_core_int16_sum,
[XMI_SUM][XMI_UNSIGNED_SHORT][0] =	_core_uint16_sum,
[XMI_SUM][XMI_SIGNED_INT][0] =	_core_int32_sum,
[XMI_SUM][XMI_UNSIGNED_INT][0] =	_core_uint32_sum,
[XMI_SUM][XMI_SIGNED_LONG_LONG][0] =	_core_int64_sum,
[XMI_SUM][XMI_UNSIGNED_LONG_LONG][0] =_core_uint64_sum,
[XMI_SUM][XMI_FLOAT][0] =		_core_fp32_sum,
[XMI_SUM][XMI_DOUBLE][0] =		_core_fp64_sum,
[XMI_SUM][XMI_LONG_DOUBLE][0] =	_core_fp128_sum,

[XMI_PROD][XMI_SIGNED_CHAR][0] =	_core_int8_prod,
[XMI_PROD][XMI_UNSIGNED_CHAR][0] =	_core_uint8_prod,
[XMI_PROD][XMI_SIGNED_SHORT][0] =	_core_int16_prod,
[XMI_PROD][XMI_UNSIGNED_SHORT][0] =	_core_uint16_prod,
[XMI_PROD][XMI_SIGNED_INT][0] =	_core_int32_prod,
[XMI_PROD][XMI_UNSIGNED_INT][0] =	_core_uint32_prod,
[XMI_PROD][XMI_SIGNED_LONG_LONG][0] =	_core_int64_prod,
[XMI_PROD][XMI_UNSIGNED_LONG_LONG][0] =_core_uint64_prod,
[XMI_PROD][XMI_FLOAT][0] =		_core_fp32_prod,
[XMI_PROD][XMI_DOUBLE][0] =		_core_fp64_prod,
[XMI_PROD][XMI_LONG_DOUBLE][0] =	_core_fp128_prod,

[XMI_LAND][XMI_SIGNED_CHAR][0] =	_core_int8_land,
[XMI_LAND][XMI_UNSIGNED_CHAR][0] =	_core_uint8_land,
[XMI_LAND][XMI_SIGNED_SHORT][0] =	_core_int16_land,
[XMI_LAND][XMI_UNSIGNED_SHORT][0] =	_core_uint16_land,
[XMI_LAND][XMI_SIGNED_INT][0] =	_core_int32_land,
[XMI_LAND][XMI_UNSIGNED_INT][0] =	_core_uint32_land,
[XMI_LAND][XMI_SIGNED_LONG_LONG][0] =	_core_int64_land,
[XMI_LAND][XMI_UNSIGNED_LONG_LONG][0] =_core_uint64_land,

[XMI_LOR][XMI_SIGNED_CHAR][0] =	_core_int8_lor,
[XMI_LOR][XMI_UNSIGNED_CHAR][0] =	_core_uint8_lor,
[XMI_LOR][XMI_SIGNED_SHORT][0] =	_core_int16_lor,
[XMI_LOR][XMI_UNSIGNED_SHORT][0] =	_core_uint16_lor,
[XMI_LOR][XMI_SIGNED_INT][0] =	_core_int32_lor,
[XMI_LOR][XMI_UNSIGNED_INT][0] =	_core_uint32_lor,
[XMI_LOR][XMI_SIGNED_LONG_LONG][0] =	_core_int64_lor,
[XMI_LOR][XMI_UNSIGNED_LONG_LONG][0] =_core_uint64_lor,

[XMI_LXOR][XMI_SIGNED_CHAR][0] =	_core_int8_lxor,
[XMI_LXOR][XMI_UNSIGNED_CHAR][0] =	_core_uint8_lxor,
[XMI_LXOR][XMI_SIGNED_SHORT][0] =	_core_int16_lxor,
[XMI_LXOR][XMI_UNSIGNED_SHORT][0] =	_core_uint16_lxor,
[XMI_LXOR][XMI_SIGNED_INT][0] =	_core_int32_lxor,
[XMI_LXOR][XMI_UNSIGNED_INT][0] =	_core_uint32_lxor,
[XMI_LXOR][XMI_SIGNED_LONG_LONG][0] =	_core_int64_lxor,
[XMI_LXOR][XMI_UNSIGNED_LONG_LONG][0] =_core_uint64_lxor,

[XMI_BAND][XMI_SIGNED_CHAR][0] =	_core_int8_band,
[XMI_BAND][XMI_UNSIGNED_CHAR][0] =	_core_uint8_band,
[XMI_BAND][XMI_SIGNED_SHORT][0] =	_core_int16_band,
[XMI_BAND][XMI_UNSIGNED_SHORT][0] =	_core_uint16_band,
[XMI_BAND][XMI_SIGNED_INT][0] =	_core_int32_band,
[XMI_BAND][XMI_UNSIGNED_INT][0] =	_core_uint32_band,
[XMI_BAND][XMI_SIGNED_LONG_LONG][0] =	_core_int64_band,
[XMI_BAND][XMI_UNSIGNED_LONG_LONG][0] =_core_uint64_band,

[XMI_BOR][XMI_SIGNED_CHAR][0] =	_core_int8_bor,
[XMI_BOR][XMI_UNSIGNED_CHAR][0] =	_core_uint8_bor,
[XMI_BOR][XMI_SIGNED_SHORT][0] =	_core_int16_bor,
[XMI_BOR][XMI_UNSIGNED_SHORT][0] =	_core_uint16_bor,
[XMI_BOR][XMI_SIGNED_INT][0] =	_core_int32_bor,
[XMI_BOR][XMI_UNSIGNED_INT][0] =	_core_uint32_bor,
[XMI_BOR][XMI_SIGNED_LONG_LONG][0] =	_core_int64_bor,
[XMI_BOR][XMI_UNSIGNED_LONG_LONG][0] =_core_uint64_bor,

[XMI_BXOR][XMI_SIGNED_CHAR][0] =	_core_int8_bxor,
[XMI_BXOR][XMI_UNSIGNED_CHAR][0] =	_core_uint8_bxor,
[XMI_BXOR][XMI_SIGNED_SHORT][0] =	_core_int16_bxor,
[XMI_BXOR][XMI_UNSIGNED_SHORT][0] =	_core_uint16_bxor,
[XMI_BXOR][XMI_SIGNED_INT][0] =	_core_int32_bxor,
[XMI_BXOR][XMI_UNSIGNED_INT][0] =	_core_uint32_bxor,
[XMI_BXOR][XMI_SIGNED_LONG_LONG][0] =	_core_int64_bxor,
[XMI_BXOR][XMI_UNSIGNED_LONG_LONG][0] =_core_uint64_bxor,

[XMI_MAXLOC][XMI_LOC_2INT][0] =	_core_int32_int32_maxloc,
[XMI_MAXLOC][XMI_LOC_SHORT_INT][0] =	_core_int16_int32_maxloc,
[XMI_MAXLOC][XMI_LOC_FLOAT_INT][0] =	_core_fp32_int32_maxloc,
[XMI_MAXLOC][XMI_LOC_DOUBLE_INT][0] =	_core_fp64_int32_maxloc,
[XMI_MAXLOC][XMI_LOC_2FLOAT][0] =	_core_fp32_fp32_maxloc,
[XMI_MAXLOC][XMI_LOC_2DOUBLE][0] =	_core_fp64_fp64_maxloc,

[XMI_MINLOC][XMI_LOC_2INT][0] =	_core_int32_int32_minloc,
[XMI_MINLOC][XMI_LOC_SHORT_INT][0] =	_core_int16_int32_minloc,
[XMI_MINLOC][XMI_LOC_FLOAT_INT][0] =	_core_fp32_int32_minloc,
[XMI_MINLOC][XMI_LOC_DOUBLE_INT][0] =	_core_fp64_int32_minloc,
[XMI_MINLOC][XMI_LOC_2FLOAT][0] =	_core_fp32_fp32_minloc,
[XMI_MINLOC][XMI_LOC_2DOUBLE][0] =	_core_fp64_fp64_minloc,

};
#undef OPTIMATH_NSRC
