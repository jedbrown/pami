/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/devices/bgp/collective_network/cn_bg_dat.c
/// \brief Tables used by DCMF
///

#include "sys/xmi.h"
#include "math/bgp/collective_network/xmi_bg_math.h"

#ifndef XMI_NO_OPTIMATH
#define OPTIMATH_NSRC(dt,op,n,f)	[op][dt][n-1] = f,
#define OPTIMATH_UNARY(dt,op,f)		[op][dt][1] = f,
#else /* XMI_NO_OPTIMATH */
#define OPTIMATH_NSRC(dt,op,n,f)
#define OPTIMATH_UNARY(dt,op,f)
#endif /* XMI_NO_OPTIMATH */

/**
 * \brief Translate operation, datatype, and optimization to math function
 *
 * This table should not be accessed directly. The inline/macro
 * XMI_PRE_OP_FUNCS(dt,op,n) should be used instead. That returns
 * the proper function (pointer) for the given combination of
 * datatype "dt", operand "op", and optimization level "n", taking
 * into account conbinations that are not optimized.
 */
void *xmi_pre_op_funcs[XMI_OP_COUNT][XMI_DT_COUNT][2] = {

OPTIMIZED_int16_pre_all(XMI_MAX)
OPTIMIZED_int32_pre_all(XMI_MAX)
OPTIMIZED_int64_pre_all(XMI_MAX)
OPTIMIZED_uint64_pre_all(XMI_MAX)
OPTIMIZED_fp32_pre_max
OPTIMIZED_fp64_pre_max

OPTIMIZED_int16_pre_min
OPTIMIZED_uint16_pre_min
OPTIMIZED_int32_pre_min
OPTIMIZED_uint32_pre_min
OPTIMIZED_int64_pre_min
OPTIMIZED_uint64_pre_min
OPTIMIZED_fp32_pre_min
OPTIMIZED_fp64_pre_min

OPTIMIZED_fp64_pre_sum

OPTIMIZED_int32_int32_pre_maxloc
OPTIMIZED_int16_int32_pre_maxloc
OPTIMIZED_fp32_int32_pre_maxloc
OPTIMIZED_fp64_int32_pre_maxloc
OPTIMIZED_fp32_fp32_pre_maxloc
OPTIMIZED_fp64_fp64_pre_maxloc

OPTIMIZED_int32_int32_pre_minloc
OPTIMIZED_int16_int32_pre_minloc
OPTIMIZED_fp32_int32_pre_minloc
OPTIMIZED_fp64_int32_pre_minloc
OPTIMIZED_fp32_fp32_pre_minloc
OPTIMIZED_fp64_fp64_pre_minloc

/*
 * Default, generic routines are "n == 0".
 * If an entry does not exist here, that combination will be silently
 * passed to the tree. Use "(void *)XMI_UNIMPL" for unsupported operations/types.
 */
[XMI_MAX][XMI_SIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_MAX][XMI_UNSIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_MAX][XMI_SIGNED_SHORT][0] =	_xmi_core_int16_pre_all,
[XMI_MAX][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_MAX][XMI_SIGNED_INT][0] =	_xmi_core_int32_pre_all,
[XMI_MAX][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_MAX][XMI_SIGNED_LONG_LONG][0] =	_xmi_core_int64_pre_all,
[XMI_MAX][XMI_UNSIGNED_LONG_LONG][0] =_xmi_core_uint64_pre_all,
[XMI_MAX][XMI_FLOAT][0] =		_xmi_core_fp32_pre_max,
[XMI_MAX][XMI_DOUBLE][0] =		_xmi_core_fp64_pre_max,
[XMI_MAX][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_MAX][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_MAX][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_MAX][XMI_LOGICAL][0] =		(void *)XMI_UNIMPL,

[XMI_MIN][XMI_SIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_MIN][XMI_UNSIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_MIN][XMI_SIGNED_SHORT][0] =	_xmi_core_int16_pre_min,
[XMI_MIN][XMI_UNSIGNED_SHORT][0] =	_xmi_core_uint16_pre_min,
[XMI_MIN][XMI_SIGNED_INT][0] =	_xmi_core_int32_pre_min,
[XMI_MIN][XMI_UNSIGNED_INT][0] =	_xmi_core_uint32_pre_min,
[XMI_MIN][XMI_SIGNED_LONG_LONG][0] =	_xmi_core_int64_pre_min,
[XMI_MIN][XMI_UNSIGNED_LONG_LONG][0] =_xmi_core_uint64_pre_min,
[XMI_MIN][XMI_FLOAT][0] =		_xmi_core_fp32_pre_min,
[XMI_MIN][XMI_DOUBLE][0] =		_xmi_core_fp64_pre_min,
[XMI_MIN][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_MIN][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_MIN][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_MIN][XMI_LOGICAL][0] =		(void *)XMI_UNIMPL,

[XMI_SUM][XMI_SIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_SUM][XMI_UNSIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_SUM][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_SUM][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_SUM][XMI_SIGNED_INT][0] =	NULL,
[XMI_SUM][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_SUM][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_SUM][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_SUM][XMI_FLOAT][0] =		(void *)XMI_UNIMPL,
[XMI_SUM][XMI_DOUBLE][0] =		_xmi_core_fp64_pre_sum,
[XMI_SUM][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_SUM][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_SUM][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_SUM][XMI_LOGICAL][0] =		(void *)XMI_UNIMPL,

[XMI_PROD][XMI_SIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_UNSIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_SIGNED_SHORT][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_UNSIGNED_SHORT][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_SIGNED_INT][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_UNSIGNED_INT][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_SIGNED_LONG_LONG][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_UNSIGNED_LONG_LONG][0] =(void *)XMI_UNIMPL,
[XMI_PROD][XMI_FLOAT][0] =		(void *)XMI_UNIMPL,
[XMI_PROD][XMI_DOUBLE][0] =		(void *)XMI_UNIMPL,
[XMI_PROD][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_LOGICAL][0] =		(void *)XMI_UNIMPL,

[XMI_LAND][XMI_SIGNED_CHAR][0] =	NULL,
[XMI_LAND][XMI_UNSIGNED_CHAR][0] =	NULL,
[XMI_LAND][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_LAND][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_LAND][XMI_SIGNED_INT][0] =	NULL,
[XMI_LAND][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_LAND][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_LAND][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_LAND][XMI_FLOAT][0] =		NULL,
[XMI_LAND][XMI_DOUBLE][0] =		NULL,
[XMI_LAND][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_LAND][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_LAND][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_LAND][XMI_LOGICAL][0] =		NULL,

[XMI_LOR][XMI_SIGNED_CHAR][0] =	NULL,
[XMI_LOR][XMI_UNSIGNED_CHAR][0] =	NULL,
[XMI_LOR][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_LOR][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_LOR][XMI_SIGNED_INT][0] =	NULL,
[XMI_LOR][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_LOR][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_LOR][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_LOR][XMI_FLOAT][0] =		NULL,
[XMI_LOR][XMI_DOUBLE][0] =		NULL,
[XMI_LOR][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_LOR][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_LOR][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_LOR][XMI_LOGICAL][0] =		NULL,

[XMI_LXOR][XMI_SIGNED_CHAR][0] =	NULL,
[XMI_LXOR][XMI_UNSIGNED_CHAR][0] =	NULL,
[XMI_LXOR][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_LXOR][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_LXOR][XMI_SIGNED_INT][0] =	NULL,
[XMI_LXOR][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_LXOR][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_LXOR][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_LXOR][XMI_FLOAT][0] =		NULL,
[XMI_LXOR][XMI_DOUBLE][0] =		NULL,
[XMI_LXOR][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_LXOR][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_LXOR][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_LXOR][XMI_LOGICAL][0] =		NULL,

[XMI_BAND][XMI_SIGNED_CHAR][0] =	NULL,
[XMI_BAND][XMI_UNSIGNED_CHAR][0] =	NULL,
[XMI_BAND][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_BAND][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_BAND][XMI_SIGNED_INT][0] =	NULL,
[XMI_BAND][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_BAND][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_BAND][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_BAND][XMI_FLOAT][0] =		NULL,
[XMI_BAND][XMI_DOUBLE][0] =		NULL,
[XMI_BAND][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_BAND][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_BAND][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_BAND][XMI_LOGICAL][0] =		NULL,

[XMI_BOR][XMI_SIGNED_CHAR][0] =	NULL,
[XMI_BOR][XMI_UNSIGNED_CHAR][0] =	NULL,
[XMI_BOR][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_BOR][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_BOR][XMI_SIGNED_INT][0] =	NULL,
[XMI_BOR][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_BOR][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_BOR][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_BOR][XMI_FLOAT][0] =		NULL,
[XMI_BOR][XMI_DOUBLE][0] =		NULL,
[XMI_BOR][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_BOR][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_BOR][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_BOR][XMI_LOGICAL][0] =		NULL,

[XMI_BXOR][XMI_SIGNED_CHAR][0] =	NULL,
[XMI_BXOR][XMI_UNSIGNED_CHAR][0] =	NULL,
[XMI_BXOR][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_BXOR][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_BXOR][XMI_SIGNED_INT][0] =	NULL,
[XMI_BXOR][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_BXOR][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_BXOR][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_BXOR][XMI_FLOAT][0] =		NULL,
[XMI_BXOR][XMI_DOUBLE][0] =		NULL,
[XMI_BXOR][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_BXOR][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_BXOR][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_BXOR][XMI_LOGICAL][0] =		NULL,

[XMI_MAXLOC][XMI_LOC_2INT][0] =	_xmi_core_int32_int32_pre_maxloc,
[XMI_MAXLOC][XMI_LOC_SHORT_INT][0] =	_xmi_core_int16_int32_pre_maxloc,
[XMI_MAXLOC][XMI_LOC_FLOAT_INT][0] =	_xmi_core_fp32_int32_pre_maxloc,
[XMI_MAXLOC][XMI_LOC_DOUBLE_INT][0] =	_xmi_core_fp64_int32_pre_maxloc,
[XMI_MAXLOC][XMI_LOC_2FLOAT][0] =	_xmi_core_fp32_fp32_pre_maxloc,
[XMI_MAXLOC][XMI_LOC_2DOUBLE][0] =	_xmi_core_fp64_fp64_pre_maxloc,

[XMI_MINLOC][XMI_LOC_2INT][0] =	_xmi_core_int32_int32_pre_minloc,
[XMI_MINLOC][XMI_LOC_SHORT_INT][0] =	_xmi_core_int16_int32_pre_minloc,
[XMI_MINLOC][XMI_LOC_FLOAT_INT][0] =	_xmi_core_fp32_int32_pre_minloc,
[XMI_MINLOC][XMI_LOC_DOUBLE_INT][0] =	_xmi_core_fp64_int32_pre_minloc,
[XMI_MINLOC][XMI_LOC_2FLOAT][0] =	_xmi_core_fp32_fp32_pre_minloc,
[XMI_MINLOC][XMI_LOC_2DOUBLE][0] =	_xmi_core_fp64_fp64_pre_minloc,

};

/**
 * \brief Translate operation, datatype, and optimization to math function
 *
 * This table should not be accessed directly. The inline/macro
 * XMI_POST_OP_FUNCS(dt,op,n) should be used instead. That returns
 * the proper function (pointer) for the given combination of
 * datatype "dt", operand "op", and optimization level "n", taking
 * into account conbinations that are not optimized.
 */
void *xmi_post_op_funcs[XMI_OP_COUNT][XMI_DT_COUNT][2] = {

OPTIMIZED_int16_post_all(XMI_MAX)
OPTIMIZED_int32_post_all(XMI_MAX)
OPTIMIZED_int64_post_all(XMI_MAX)
OPTIMIZED_uint64_post_all(XMI_MAX)
OPTIMIZED_fp32_post_max
OPTIMIZED_fp64_post_max

OPTIMIZED_int16_post_min
OPTIMIZED_uint16_post_min
OPTIMIZED_int32_post_min
OPTIMIZED_uint32_post_min
OPTIMIZED_int64_post_min
OPTIMIZED_uint64_post_min
OPTIMIZED_fp32_post_min
OPTIMIZED_fp64_post_min

OPTIMIZED_fp64_post_sum

OPTIMIZED_int32_int32_post_maxloc
OPTIMIZED_int16_int32_post_maxloc
OPTIMIZED_fp32_int32_post_maxloc
OPTIMIZED_fp64_int32_post_maxloc
OPTIMIZED_fp32_fp32_post_maxloc
OPTIMIZED_fp64_fp64_post_maxloc

OPTIMIZED_int32_int32_post_minloc
OPTIMIZED_int16_int32_post_minloc
OPTIMIZED_fp32_int32_post_minloc
OPTIMIZED_fp64_int32_post_minloc
OPTIMIZED_fp32_fp32_post_minloc
OPTIMIZED_fp64_fp64_post_minloc

/*
 * Default, generic routines are "n == 0".
 * If an entry does not exist here, that combination will be silently
 * passed to the tree. Use "(void *)XMI_UNIMPL" for unsupported operations/types.
 */
[XMI_MAX][XMI_SIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_MAX][XMI_UNSIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_MAX][XMI_SIGNED_SHORT][0] =	_xmi_core_int16_post_all,
[XMI_MAX][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_MAX][XMI_SIGNED_INT][0] =	_xmi_core_int32_post_all,
[XMI_MAX][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_MAX][XMI_SIGNED_LONG_LONG][0] =	_xmi_core_int64_post_all,
[XMI_MAX][XMI_UNSIGNED_LONG_LONG][0] =_xmi_core_uint64_post_all,
[XMI_MAX][XMI_FLOAT][0] =		_xmi_core_fp32_post_max,
[XMI_MAX][XMI_DOUBLE][0] =		_xmi_core_fp64_post_max,
[XMI_MAX][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_MAX][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_MAX][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_MAX][XMI_LOGICAL][0] =		(void *)XMI_UNIMPL,

[XMI_MIN][XMI_SIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_MIN][XMI_UNSIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_MIN][XMI_SIGNED_SHORT][0] =	_xmi_core_int16_post_min,
[XMI_MIN][XMI_UNSIGNED_SHORT][0] =	_xmi_core_uint16_post_min,
[XMI_MIN][XMI_SIGNED_INT][0] =	_xmi_core_int32_post_min,
[XMI_MIN][XMI_UNSIGNED_INT][0] =	_xmi_core_uint32_post_min,
[XMI_MIN][XMI_SIGNED_LONG_LONG][0] =	_xmi_core_int64_post_min,
[XMI_MIN][XMI_UNSIGNED_LONG_LONG][0] =_xmi_core_uint64_post_min,
[XMI_MIN][XMI_FLOAT][0] =		_xmi_core_fp32_post_min,
[XMI_MIN][XMI_DOUBLE][0] =		_xmi_core_fp64_post_min,
[XMI_MIN][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_MIN][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_MIN][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_MIN][XMI_LOGICAL][0] =		(void *)XMI_UNIMPL,

[XMI_SUM][XMI_SIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_SUM][XMI_UNSIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_SUM][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_SUM][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_SUM][XMI_SIGNED_INT][0] =	NULL,
[XMI_SUM][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_SUM][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_SUM][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_SUM][XMI_FLOAT][0] =		(void *)XMI_UNIMPL,
[XMI_SUM][XMI_DOUBLE][0] =		_xmi_core_fp64_post_sum,
[XMI_SUM][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_SUM][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_SUM][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_SUM][XMI_LOGICAL][0] =		(void *)XMI_UNIMPL,

[XMI_PROD][XMI_SIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_UNSIGNED_CHAR][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_SIGNED_SHORT][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_UNSIGNED_SHORT][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_SIGNED_INT][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_UNSIGNED_INT][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_SIGNED_LONG_LONG][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_UNSIGNED_LONG_LONG][0] =(void *)XMI_UNIMPL,
[XMI_PROD][XMI_FLOAT][0] =		(void *)XMI_UNIMPL,
[XMI_PROD][XMI_DOUBLE][0] =		(void *)XMI_UNIMPL,
[XMI_PROD][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_PROD][XMI_LOGICAL][0] =		(void *)XMI_UNIMPL,

[XMI_LAND][XMI_SIGNED_CHAR][0] =	NULL,
[XMI_LAND][XMI_UNSIGNED_CHAR][0] =	NULL,
[XMI_LAND][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_LAND][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_LAND][XMI_SIGNED_INT][0] =	NULL,
[XMI_LAND][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_LAND][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_LAND][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_LAND][XMI_FLOAT][0] =		NULL,
[XMI_LAND][XMI_DOUBLE][0] =		NULL,
[XMI_LAND][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_LAND][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_LAND][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_LAND][XMI_LOGICAL][0] =		NULL,

[XMI_LOR][XMI_SIGNED_CHAR][0] =	NULL,
[XMI_LOR][XMI_UNSIGNED_CHAR][0] =	NULL,
[XMI_LOR][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_LOR][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_LOR][XMI_SIGNED_INT][0] =	NULL,
[XMI_LOR][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_LOR][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_LOR][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_LOR][XMI_FLOAT][0] =		NULL,
[XMI_LOR][XMI_DOUBLE][0] =		NULL,
[XMI_LOR][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_LOR][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_LOR][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_LOR][XMI_LOGICAL][0] =		NULL,

[XMI_LXOR][XMI_SIGNED_CHAR][0] =	NULL,
[XMI_LXOR][XMI_UNSIGNED_CHAR][0] =	NULL,
[XMI_LXOR][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_LXOR][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_LXOR][XMI_SIGNED_INT][0] =	NULL,
[XMI_LXOR][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_LXOR][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_LXOR][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_LXOR][XMI_FLOAT][0] =		NULL,
[XMI_LXOR][XMI_DOUBLE][0] =		NULL,
[XMI_LXOR][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_LXOR][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_LXOR][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_LXOR][XMI_LOGICAL][0] =		NULL,

[XMI_BAND][XMI_SIGNED_CHAR][0] =	NULL,
[XMI_BAND][XMI_UNSIGNED_CHAR][0] =	NULL,
[XMI_BAND][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_BAND][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_BAND][XMI_SIGNED_INT][0] =	NULL,
[XMI_BAND][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_BAND][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_BAND][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_BAND][XMI_FLOAT][0] =		NULL,
[XMI_BAND][XMI_DOUBLE][0] =		NULL,
[XMI_BAND][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_BAND][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_BAND][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_BAND][XMI_LOGICAL][0] =		NULL,

[XMI_BOR][XMI_SIGNED_CHAR][0] =	NULL,
[XMI_BOR][XMI_UNSIGNED_CHAR][0] =	NULL,
[XMI_BOR][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_BOR][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_BOR][XMI_SIGNED_INT][0] =	NULL,
[XMI_BOR][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_BOR][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_BOR][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_BOR][XMI_FLOAT][0] =		NULL,
[XMI_BOR][XMI_DOUBLE][0] =		NULL,
[XMI_BOR][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_BOR][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_BOR][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_BOR][XMI_LOGICAL][0] =		NULL,

[XMI_BXOR][XMI_SIGNED_CHAR][0] =	NULL,
[XMI_BXOR][XMI_UNSIGNED_CHAR][0] =	NULL,
[XMI_BXOR][XMI_SIGNED_SHORT][0] =	NULL,
[XMI_BXOR][XMI_UNSIGNED_SHORT][0] =	NULL,
[XMI_BXOR][XMI_SIGNED_INT][0] =	NULL,
[XMI_BXOR][XMI_UNSIGNED_INT][0] =	NULL,
[XMI_BXOR][XMI_SIGNED_LONG_LONG][0] =	NULL,
[XMI_BXOR][XMI_UNSIGNED_LONG_LONG][0] =NULL,
[XMI_BXOR][XMI_FLOAT][0] =		NULL,
[XMI_BXOR][XMI_DOUBLE][0] =		NULL,
[XMI_BXOR][XMI_LONG_DOUBLE][0] =	(void *)XMI_UNIMPL,
[XMI_BXOR][XMI_SINGLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_BXOR][XMI_DOUBLE_COMPLEX][0] =	(void *)XMI_UNIMPL,
[XMI_BXOR][XMI_LOGICAL][0] =		NULL,

[XMI_MAXLOC][XMI_LOC_2INT][0] =	_xmi_core_int32_int32_post_maxloc,
[XMI_MAXLOC][XMI_LOC_SHORT_INT][0] =	_xmi_core_int16_int32_post_maxloc,
[XMI_MAXLOC][XMI_LOC_FLOAT_INT][0] =	_xmi_core_fp32_int32_post_maxloc,
[XMI_MAXLOC][XMI_LOC_DOUBLE_INT][0] =	_xmi_core_fp64_int32_post_maxloc,
[XMI_MAXLOC][XMI_LOC_2FLOAT][0] =	_xmi_core_fp32_fp32_post_maxloc,
[XMI_MAXLOC][XMI_LOC_2DOUBLE][0] =	_xmi_core_fp64_fp64_post_maxloc,

[XMI_MINLOC][XMI_LOC_2INT][0] =	_xmi_core_int32_int32_post_minloc,
[XMI_MINLOC][XMI_LOC_SHORT_INT][0] =	_xmi_core_int16_int32_post_minloc,
[XMI_MINLOC][XMI_LOC_FLOAT_INT][0] =	_xmi_core_fp32_int32_post_minloc,
[XMI_MINLOC][XMI_LOC_DOUBLE_INT][0] =	_xmi_core_fp64_int32_post_minloc,
[XMI_MINLOC][XMI_LOC_2FLOAT][0] =	_xmi_core_fp32_fp32_post_minloc,
[XMI_MINLOC][XMI_LOC_2DOUBLE][0] =	_xmi_core_fp64_fp64_post_minloc,

};

#undef OPTIMATH_NSRC
#undef OPTIMATH_UNARY
