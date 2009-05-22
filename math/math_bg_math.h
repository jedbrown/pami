/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file include/math_bg_math.h
 * \brief Prototypes for all the collective network math functions
 *
 * These routines are used when math operations are being performed
 * on the BlueGene/P collective network hardware. The N-way functions
 * perform local math and convert the results for use on the tree
 * with the same operation and datatype. The unary functions are
 * used to perform conversion to/from the tree.
 */


#ifndef _math_bg_math_h_
#define _math_bg_math_h_

#include <stdint.h>
#include "math_coremath.h"

/**
 * \brief Convert a LOC value TO a tree value
 * Convert LOC (MINLOC/MAXLOC) values to collective network
 * values that ensures signs are observed.
 */
#define LOC_INT_TOTREE(i)     ((i) ^ 0x7fffffffUL)
/**
 * \brief Convert a LOC value FROM a tree value
 * Convert LOC (MINLOC/MAXLOC) values to collective network
 * values that ensures signs are observed.
 */
#define LOC_INT_FRTREE(i)     ((i) ^ 0x7fffffffUL)

/* Must be included after math_coremath.h */
#include "common/math_optibgmath.h"

/**
 * \brief Translate operation, datatype, and optimization to math function
 *
 * This table should not be accessed directly. The inline/macro
 * MATH_PRE_OP_FUNCS(dt,op,n) should be used instead. That returns
 * the proper function (pointer) for the given combination of
 * datatype "dt", operand "op", and optimization level "n", taking
 * into account conbinations that are not optimized.
 */
extern void *math_pre_op_funcs[MATH_OP_COUNT][MATH_DT_COUNT][2];
/**
 * \brief Translate operation, datatype, and optimization to math function
 *
 * This table should not be accessed directly. The inline/macro
 * MATH_POST_OP_FUNCS(dt,op,n) should be used instead. That returns
 * the proper function (pointer) for the given combination of
 * datatype "dt", operand "op", and optimization level "n", taking
 * into account conbinations that are not optimized.
 */
extern void *math_post_op_funcs[MATH_OP_COUNT][MATH_DT_COUNT][2];
/**
 * \brief Translate operation, datatype, and number inputs to math function
 *
 * This table should not be accessed directly. The inline/macro
 * MATH_MARSHALL_OP_FUNCS(dt,op,n) should be used instead. That returns
 * the proper function (pointer) for the given combination of
 * datatype "dt", operand "op", and number of inputs "n", taking
 * into account conbinations that are not optimized.
 *
 * currently not used...
 */
extern void *math_marshall_op_funcs[MATH_OP_COUNT][MATH_DT_COUNT][MATH_MAX_NSRC];

/**
 * \brief Return best tree pre-processing routine for datatype and operand.
 *
 * Selects an optimized conversion routine from math_pre_op_funcs[][][] or
 * defaults to the generic, unoptimized, routine if no
 * better one exists.
 *
 * \param dt    Datatype being used
 * \param op    Operand being used
 * \param optim Optimization flag (0/1)
 * \return      Pointer to coremath1 function. NULL if no processing
 * is needed on the tree, (void *)MATH_UNIMPL if the tree does not support
 * the operation on the datatype.
 */
static inline coremath1 MATH_PRE_OP_FUNCS(MATH_Dt dt, MATH_Op op, int optim) {
	return (coremath1)(math_pre_op_funcs[op][dt][1] ?
				math_pre_op_funcs[op][dt][1] :
				math_pre_op_funcs[op][dt][0]);
}

/**
 * \brief Return best tree post-processing routine for datatype and operand.
 *
 * Selects an optimized conversion routine from math_post_op_funcs[][][] or
 * defaults to the generic, unoptimized, routine if no
 * better one exists.
 *
 * \param dt    Datatype being used
 * \param op    Operand being used
 * \param optim Optimization flag (0/1)
 * \return      Pointer to coremath1 function. NULL if no processing
 * is needed on the tree, (void *)MATH_UNIMPL if the tree does not support
 * the operation on the datatype.
 */
static inline coremath1 MATH_POST_OP_FUNCS(MATH_Dt dt, MATH_Op op, int optim) {
	return (coremath1)(math_post_op_funcs[op][dt][1] ?
				math_post_op_funcs[op][dt][1] :
				math_post_op_funcs[op][dt][0]);
}

/**
 * \brief Return best tree math-marshalling routine for datatype and operand.
 *
 * Selects an optimized conversion routine from math_marshall_op_funcs[][][] or
 * defaults to the generic, unoptimized, routine if no
 * better one exists.
 *
 * \param dt    Datatype being used
 * \param op    Operand being used
 * \param nsrc  Number of input buffers
 * \return      Pointer to coremath function. NULL if no processing
 * is needed on the tree, (void *)MATH_UNIMPL if the tree does not support
 * the operation on the datatype.
 */
static inline coremath MATH_MARSHALL_OP_FUNCS(MATH_Dt dt, MATH_Op op, int nsrc) {
	/* assert(nsrc >= 2); */
	int n = nsrc - 1;
	return (coremath)(math_marshall_op_funcs[op][dt][n] ?
				math_marshall_op_funcs[op][dt][n] :
				math_marshall_op_funcs[op][dt][0]);
}

/* Here is how we expand defines in math_optibgmath.h into code: */
#ifndef MATH_NO_OPTIMATH
/** \brief Create a "case" value for use in a switch statement */
#define OPTIMATH_NSRC(dt,op,n,f)	case n: f(dst, srcs, nsrc, count); break;
#else /* MATH_NO_OPTIMATH */
#define OPTIMATH_NSRC(dt,op,n,f)
#endif /* MATH_NO_OPTIMATH */

#if defined(__cplusplus)
extern "C"
{
#endif

/**** N-way routines ****/

/**
 * \brief Optimized bitwise AND conversion to tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion to tree operation on \a count signed-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int8_band_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int8_band_marshall
	default:
		_core_int8_band_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion to tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion to tree operation on \a count signed-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int8_bor_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int8_bor_marshall
	default:
		_core_int8_bor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion to tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion to tree operation on \a count signed-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int8_bxor_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int8_bxor_marshall
	default:
		_core_int8_bxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion to tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion to tree operation on \a count signed-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int8_land_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int8_land_marshall
	default:
		_core_int8_land_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion to tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion to tree operation on \a count signed-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int8_lor_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int8_lor_marshall
	default:
		_core_int8_lor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion to tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion to tree operation on \a count signed-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int8_lxor_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int8_lxor_marshall
	default:
		_core_int8_lxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion to tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAX conversion to tree operation on \a count signed-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int8_max_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int8_max_marshall
	default:
		_core_int8_max_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion to tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MIN conversion to tree operation on \a count signed-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int8_min_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int8_min_marshall
	default:
		_core_int8_min_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion to tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized addition conversion to tree operation on \a count signed-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int8_sum_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int8_sum_marshall
	default:
		_core_int8_sum_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion to tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion to tree operation on \a count unsigned-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint8_band_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint8_band_marshall
	default:
		_core_uint8_band_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion to tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion to tree operation on \a count unsigned-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint8_bor_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint8_bor_marshall
	default:
		_core_uint8_bor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion to tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion to tree operation on \a count unsigned-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint8_bxor_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint8_bxor_marshall
	default:
		_core_uint8_bxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion to tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion to tree operation on \a count unsigned-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint8_land_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint8_land_marshall
	default:
		_core_uint8_land_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion to tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion to tree operation on \a count unsigned-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint8_lor_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint8_lor_marshall
	default:
		_core_uint8_lor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion to tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion to tree operation on \a count unsigned-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint8_lxor_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint8_lxor_marshall
	default:
		_core_uint8_lxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion to tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAX conversion to tree operation on \a count unsigned-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint8_max_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint8_max_marshall
	default:
		_core_uint8_max_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion to tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MIN conversion to tree operation on \a count unsigned-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint8_min_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint8_min_marshall
	default:
		_core_uint8_min_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion to tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized addition conversion to tree operation on \a count unsigned-char
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint8_sum_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint8_sum_marshall
	default:
		_core_uint8_sum_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion to tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion to tree operation on \a count signed-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int16_band_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int16_band_marshall
	default:
		_core_int16_band_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion to tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion to tree operation on \a count signed-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int16_bor_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int16_bor_marshall
	default:
		_core_int16_bor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion to tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion to tree operation on \a count signed-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int16_bxor_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int16_bxor_marshall
	default:
		_core_int16_bxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion to tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion to tree operation on \a count signed-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int16_land_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int16_land_marshall
	default:
		_core_int16_land_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion to tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion to tree operation on \a count signed-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int16_lor_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int16_lor_marshall
	default:
		_core_int16_lor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion to tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion to tree operation on \a count signed-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int16_lxor_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int16_lxor_marshall
	default:
		_core_int16_lxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion to tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAX conversion to tree operation on \a count signed-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int16_max_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int16_max_marshall
	default:
		_core_int16_max_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAXLOC conversion to tree on signed-short/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAXLOC conversion to tree operation on \a count signed-short/signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int16_int32_maxloc_marshall(uint16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int16_int32_maxloc_marshall
	default:
		_core_int16_int32_maxloc_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion to tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MIN conversion to tree operation on \a count signed-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int16_min_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int16_min_marshall
	default:
		_core_int16_min_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MINLOC conversion to tree on signed-short/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MINLOC conversion to tree operation on \a count signed-short/signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int16_int32_minloc_marshall(uint16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int16_int32_minloc_marshall
	default:
		_core_int16_int32_minloc_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion to tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized addition conversion to tree operation on \a count signed-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int16_sum_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int16_sum_marshall
	default:
		_core_int16_sum_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion to tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion to tree operation on \a count unsigned-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint16_band_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint16_band_marshall
	default:
		_core_uint16_band_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion to tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion to tree operation on \a count unsigned-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint16_bor_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint16_bor_marshall
	default:
		_core_uint16_bor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion to tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion to tree operation on \a count unsigned-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint16_bxor_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint16_bxor_marshall
	default:
		_core_uint16_bxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion to tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion to tree operation on \a count unsigned-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint16_land_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint16_land_marshall
	default:
		_core_uint16_land_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion to tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion to tree operation on \a count unsigned-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint16_lor_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint16_lor_marshall
	default:
		_core_uint16_lor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion to tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion to tree operation on \a count unsigned-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint16_lxor_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint16_lxor_marshall
	default:
		_core_uint16_lxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion to tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAX conversion to tree operation on \a count unsigned-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint16_max_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint16_max_marshall
	default:
		_core_uint16_max_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion to tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MIN conversion to tree operation on \a count unsigned-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint16_min_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint16_min_marshall
	default:
		_core_uint16_min_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion to tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized addition conversion to tree operation on \a count unsigned-short
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint16_sum_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint16_sum_marshall
	default:
		_core_uint16_sum_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion to tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion to tree operation on \a count signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int32_band_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int32_band_marshall
	default:
		_core_int32_band_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion to tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion to tree operation on \a count signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int32_bor_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int32_bor_marshall
	default:
		_core_int32_bor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion to tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion to tree operation on \a count signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int32_bxor_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int32_bxor_marshall
	default:
		_core_int32_bxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion to tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion to tree operation on \a count signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int32_land_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int32_land_marshall
	default:
		_core_int32_land_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion to tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion to tree operation on \a count signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int32_lor_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int32_lor_marshall
	default:
		_core_int32_lor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion to tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion to tree operation on \a count signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int32_lxor_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int32_lxor_marshall
	default:
		_core_int32_lxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion to tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAX conversion to tree operation on \a count signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int32_max_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int32_max_marshall
	default:
		_core_int32_max_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAXLOC conversion to tree on signed-int/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAXLOC conversion to tree operation on \a count signed-int/signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int32_int32_maxloc_marshall(uint32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int32_int32_maxloc_marshall
	default:
		_core_int32_int32_maxloc_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion to tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MIN conversion to tree operation on \a count signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int32_min_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int32_min_marshall
	default:
		_core_int32_min_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MINLOC conversion to tree on signed-int/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MINLOC conversion to tree operation on \a count signed-int/signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int32_int32_minloc_marshall(uint32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int32_int32_minloc_marshall
	default:
		_core_int32_int32_minloc_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion to tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized addition conversion to tree operation on \a count signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int32_sum_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int32_sum_marshall
	default:
		_core_int32_sum_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion to tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion to tree operation on \a count unsigned-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint32_band_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint32_band_marshall
	default:
		_core_uint32_band_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion to tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion to tree operation on \a count unsigned-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint32_bor_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint32_bor_marshall
	default:
		_core_uint32_bor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion to tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion to tree operation on \a count unsigned-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint32_bxor_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint32_bxor_marshall
	default:
		_core_uint32_bxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion to tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion to tree operation on \a count unsigned-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint32_land_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint32_land_marshall
	default:
		_core_uint32_land_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion to tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion to tree operation on \a count unsigned-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint32_lor_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint32_lor_marshall
	default:
		_core_uint32_lor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion to tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion to tree operation on \a count unsigned-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint32_lxor_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint32_lxor_marshall
	default:
		_core_uint32_lxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion to tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAX conversion to tree operation on \a count unsigned-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint32_max_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint32_max_marshall
	default:
		_core_uint32_max_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion to tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MIN conversion to tree operation on \a count unsigned-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint32_min_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint32_min_marshall
	default:
		_core_uint32_min_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion to tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized addition conversion to tree operation on \a count unsigned-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint32_sum_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint32_sum_marshall
	default:
		_core_uint32_sum_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion to tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion to tree operation on \a count signed-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int64_band_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int64_band_marshall
	default:
		_core_int64_band_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion to tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion to tree operation on \a count signed-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int64_bor_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int64_bor_marshall
	default:
		_core_int64_bor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion to tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion to tree operation on \a count signed-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int64_bxor_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int64_bxor_marshall
	default:
		_core_int64_bxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion to tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion to tree operation on \a count signed-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int64_land_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int64_land_marshall
	default:
		_core_int64_land_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion to tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion to tree operation on \a count signed-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int64_lor_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int64_lor_marshall
	default:
		_core_int64_lor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion to tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion to tree operation on \a count signed-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int64_lxor_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int64_lxor_marshall
	default:
		_core_int64_lxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion to tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAX conversion to tree operation on \a count signed-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int64_max_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int64_max_marshall
	default:
		_core_int64_max_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion to tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MIN conversion to tree operation on \a count signed-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int64_min_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int64_min_marshall
	default:
		_core_int64_min_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion to tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized addition conversion to tree operation on \a count signed-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_int64_sum_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_int64_sum_marshall
	default:
		_core_int64_sum_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion to tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion to tree operation on \a count unsigned-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint64_band_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint64_band_marshall
	default:
		_core_uint64_band_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion to tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion to tree operation on \a count unsigned-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint64_bor_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint64_bor_marshall
	default:
		_core_uint64_bor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion to tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion to tree operation on \a count unsigned-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint64_bxor_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint64_bxor_marshall
	default:
		_core_uint64_bxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion to tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion to tree operation on \a count unsigned-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint64_land_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint64_land_marshall
	default:
		_core_uint64_land_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion to tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion to tree operation on \a count unsigned-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint64_lor_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint64_lor_marshall
	default:
		_core_uint64_lor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion to tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion to tree operation on \a count unsigned-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint64_lxor_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint64_lxor_marshall
	default:
		_core_uint64_lxor_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion to tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAX conversion to tree operation on \a count unsigned-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint64_max_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint64_max_marshall
	default:
		_core_uint64_max_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion to tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MIN conversion to tree operation on \a count unsigned-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint64_min_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint64_min_marshall
	default:
		_core_uint64_min_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion to tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized addition conversion to tree operation on \a count unsigned-long-long
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_uint64_sum_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_uint64_sum_marshall
	default:
		_core_uint64_sum_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion to tree on float datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAX conversion to tree operation on \a count float
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_fp32_max_marshall(float *dst, const float **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_fp32_max_marshall
	default:
		_core_fp32_max_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAXLOC conversion to tree on float/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAXLOC conversion to tree operation on \a count float/signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_fp32_int32_maxloc_marshall(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_fp32_int32_maxloc_marshall
	default:
		_core_fp32_int32_maxloc_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAXLOC conversion to tree on float/float datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAXLOC conversion to tree operation on \a count float/float
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_fp32_fp32_maxloc_marshall(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_fp32_fp32_maxloc_marshall
	default:
		_core_fp32_fp32_maxloc_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion to tree on float datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MIN conversion to tree operation on \a count float
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_fp32_min_marshall(float *dst, const float **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_fp32_min_marshall
	default:
		_core_fp32_min_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MINLOC conversion to tree on float/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MINLOC conversion to tree operation on \a count float/signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_fp32_int32_minloc_marshall(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_fp32_int32_minloc_marshall
	default:
		_core_fp32_int32_minloc_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MINLOC conversion to tree on float/float datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MINLOC conversion to tree operation on \a count float/float
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_fp32_fp32_minloc_marshall(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_fp32_fp32_minloc_marshall
	default:
		_core_fp32_fp32_minloc_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion to tree on double datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAX conversion to tree operation on \a count double
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_fp64_max_marshall(double *dst, const double **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_fp64_max_marshall
	default:
		_core_fp64_max_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAXLOC conversion to tree on double/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAXLOC conversion to tree operation on \a count double/signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_fp64_int32_maxloc_marshall(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_fp64_int32_maxloc_marshall
	default:
		_core_fp64_int32_maxloc_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MAXLOC conversion to tree on double/double datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MAXLOC conversion to tree operation on \a count double/double
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_fp64_fp64_maxloc_marshall(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_fp64_fp64_maxloc_marshall
	default:
		_core_fp64_fp64_maxloc_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion to tree on double datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MIN conversion to tree operation on \a count double
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_fp64_min_marshall(double *dst, const double **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_fp64_min_marshall
	default:
		_core_fp64_min_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MINLOC conversion to tree on double/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MINLOC conversion to tree operation on \a count double/signed-int
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_fp64_int32_minloc_marshall(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_fp64_int32_minloc_marshall
	default:
		_core_fp64_int32_minloc_marshall(dst, srcs, nsrc, count);
		break;
	}
}

/**
 * \brief Optimized MINLOC conversion to tree on double/double datatypes.
 *
 * \param dst The destination buffer.
 * \param srcs The source buffers.
 * \param nsrc The number of source buffers.
 * \param count The number of elements.
 *
 * Optimized MINLOC conversion to tree operation on \a count double/double
 * elements of the source buffers, results in destination buffer.
 */
static inline void Core_fp64_fp64_minloc_marshall(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count) {
	switch(nsrc) {
	OPTIMIZED_fp64_fp64_minloc_marshall
	default:
		_core_fp64_fp64_minloc_marshall(dst, srcs, nsrc, count);
		break;
	}
}

#undef OPTIMATH_NSRC

/**** Unary routines ****/

/* Here is how we expand defines in math_optibgmath.h into code: */
#ifndef MATH_NO_OPTIMATH
/** \brief Create a "case" value for use in a switch statement */
#define OPTIMATH_UNARY(dt,op,f)	case 1: f(dst, src, count); break;
#else /* MATH_NO_OPTIMATH */
#define OPTIMATH_UNARY(dt,op,f)
#endif /* MATH_NO_OPTIMATH */

/**
 * \brief Optimized pre-processing for default on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for default operation on \a count signed-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int8_pre_all(uint8_t *dst, const int8_t *src, int count) {
	switch(1) {
	OPTIMIZED_int8_pre_all(MATH_NOOP)
	default:
		_core_int8_pre_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for default on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for default operation on \a count signed-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int8_post_all(int8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_int8_post_all(MATH_NOOP)
	default:
		_core_int8_post_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MIN on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MIN operation on \a count signed-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int8_pre_min(uint8_t *dst, const int8_t *src, int count) {
	switch(1) {
	OPTIMIZED_int8_pre_min
	default:
		_core_int8_pre_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MIN on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MIN operation on \a count signed-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int8_post_min(int8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_int8_post_min
	default:
		_core_int8_post_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion from tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion from tree operation on \a count signed-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int8_band_unmarshall(int8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_int8_band_unmarshall
	default:
		_core_int8_band_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion from tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion from tree operation on \a count signed-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int8_bor_unmarshall(int8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_int8_bor_unmarshall
	default:
		_core_int8_bor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion from tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion from tree operation on \a count signed-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int8_bxor_unmarshall(int8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_int8_bxor_unmarshall
	default:
		_core_int8_bxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion from tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion from tree operation on \a count signed-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int8_land_unmarshall(int8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_int8_land_unmarshall
	default:
		_core_int8_land_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion from tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion from tree operation on \a count signed-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int8_lor_unmarshall(int8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_int8_lor_unmarshall
	default:
		_core_int8_lor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion from tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion from tree operation on \a count signed-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int8_lxor_unmarshall(int8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_int8_lxor_unmarshall
	default:
		_core_int8_lxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion from tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAX conversion from tree operation on \a count signed-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int8_max_unmarshall(int8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_int8_max_unmarshall
	default:
		_core_int8_max_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion from tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MIN conversion from tree operation on \a count signed-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int8_min_unmarshall(int8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_int8_min_unmarshall
	default:
		_core_int8_min_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion from tree on signed-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized addition conversion from tree operation on \a count signed-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int8_sum_unmarshall(int8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_int8_sum_unmarshall
	default:
		_core_int8_sum_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for default on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for default operation on \a count unsigned-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint8_pre_all(uint8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint8_pre_all(MATH_NOOP)
	default:
		_core_uint8_pre_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for default on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for default operation on \a count unsigned-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint8_post_all(uint8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint8_post_all(MATH_NOOP)
	default:
		_core_uint8_post_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MIN on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MIN operation on \a count unsigned-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint8_pre_min(uint8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint8_pre_min
	default:
		_core_uint8_pre_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MIN on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MIN operation on \a count unsigned-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint8_post_min(uint8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint8_post_min
	default:
		_core_uint8_post_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion from tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion from tree operation on \a count unsigned-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint8_band_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint8_band_unmarshall
	default:
		_core_uint8_band_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion from tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion from tree operation on \a count unsigned-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint8_bor_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint8_bor_unmarshall
	default:
		_core_uint8_bor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion from tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion from tree operation on \a count unsigned-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint8_bxor_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint8_bxor_unmarshall
	default:
		_core_uint8_bxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion from tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion from tree operation on \a count unsigned-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint8_land_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint8_land_unmarshall
	default:
		_core_uint8_land_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion from tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion from tree operation on \a count unsigned-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint8_lor_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint8_lor_unmarshall
	default:
		_core_uint8_lor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion from tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion from tree operation on \a count unsigned-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint8_lxor_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint8_lxor_unmarshall
	default:
		_core_uint8_lxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion from tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAX conversion from tree operation on \a count unsigned-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint8_max_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint8_max_unmarshall
	default:
		_core_uint8_max_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion from tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MIN conversion from tree operation on \a count unsigned-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint8_min_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint8_min_unmarshall
	default:
		_core_uint8_min_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion from tree on unsigned-char datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized addition conversion from tree operation on \a count unsigned-char
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint8_sum_unmarshall(uint8_t *dst, const uint8_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint8_sum_unmarshall
	default:
		_core_uint8_sum_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for default on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for default operation on \a count signed-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_pre_all(uint16_t *dst, const int16_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_pre_all(MATH_NOOP)
	default:
		_core_int16_pre_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for default on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for default operation on \a count signed-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_post_all(int16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_post_all(MATH_NOOP)
	default:
		_core_int16_post_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MIN on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MIN operation on \a count signed-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_pre_min(uint16_t *dst, const int16_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_pre_min
	default:
		_core_int16_pre_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MIN on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MIN operation on \a count signed-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_post_min(int16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_post_min
	default:
		_core_int16_post_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MAXLOC on signed-short/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MAXLOC operation on \a count signed-short/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_int32_pre_maxloc(uint16_int32_t *dst, const int16_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_int32_pre_maxloc
	default:
		_core_int16_int32_pre_maxloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MAXLOC on signed-short/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MAXLOC operation on \a count signed-short/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_int32_post_maxloc(int16_int32_t *dst, const uint16_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_int32_post_maxloc
	default:
		_core_int16_int32_post_maxloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MINLOC on signed-short/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MINLOC operation on \a count signed-short/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_int32_pre_minloc(uint16_int32_t *dst, const int16_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_int32_pre_minloc
	default:
		_core_int16_int32_pre_minloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MINLOC on signed-short/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MINLOC operation on \a count signed-short/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_int32_post_minloc(int16_int32_t *dst, const uint16_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_int32_post_minloc
	default:
		_core_int16_int32_post_minloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion from tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion from tree operation on \a count signed-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_band_unmarshall(int16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_band_unmarshall
	default:
		_core_int16_band_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion from tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion from tree operation on \a count signed-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_bor_unmarshall(int16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_bor_unmarshall
	default:
		_core_int16_bor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion from tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion from tree operation on \a count signed-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_bxor_unmarshall(int16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_bxor_unmarshall
	default:
		_core_int16_bxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion from tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion from tree operation on \a count signed-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_land_unmarshall(int16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_land_unmarshall
	default:
		_core_int16_land_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion from tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion from tree operation on \a count signed-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_lor_unmarshall(int16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_lor_unmarshall
	default:
		_core_int16_lor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion from tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion from tree operation on \a count signed-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_lxor_unmarshall(int16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_lxor_unmarshall
	default:
		_core_int16_lxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion from tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAX conversion from tree operation on \a count signed-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_max_unmarshall(int16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_max_unmarshall
	default:
		_core_int16_max_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAXLOC conversion from tree on signed-short/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAXLOC conversion from tree operation on \a count signed-short/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_int32_maxloc_unmarshall(int16_int32_t *dst, const uint16_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_int32_maxloc_unmarshall
	default:
		_core_int16_int32_maxloc_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion from tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MIN conversion from tree operation on \a count signed-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_min_unmarshall(int16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_min_unmarshall
	default:
		_core_int16_min_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MINLOC conversion from tree on signed-short/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MINLOC conversion from tree operation on \a count signed-short/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_int32_minloc_unmarshall(int16_int32_t *dst, const uint16_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_int32_minloc_unmarshall
	default:
		_core_int16_int32_minloc_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion from tree on signed-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized addition conversion from tree operation on \a count signed-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int16_sum_unmarshall(int16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_int16_sum_unmarshall
	default:
		_core_int16_sum_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for default on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for default operation on \a count unsigned-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint16_pre_all(uint16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint16_pre_all(MATH_NOOP)
	default:
		_core_uint16_pre_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for default on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for default operation on \a count unsigned-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint16_post_all(uint16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint16_post_all(MATH_NOOP)
	default:
		_core_uint16_post_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MIN on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MIN operation on \a count unsigned-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint16_pre_min(uint16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint16_pre_min
	default:
		_core_uint16_pre_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MIN on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MIN operation on \a count unsigned-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint16_post_min(uint16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint16_post_min
	default:
		_core_uint16_post_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion from tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion from tree operation on \a count unsigned-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint16_band_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint16_band_unmarshall
	default:
		_core_uint16_band_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion from tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion from tree operation on \a count unsigned-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint16_bor_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint16_bor_unmarshall
	default:
		_core_uint16_bor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion from tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion from tree operation on \a count unsigned-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint16_bxor_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint16_bxor_unmarshall
	default:
		_core_uint16_bxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion from tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion from tree operation on \a count unsigned-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint16_land_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint16_land_unmarshall
	default:
		_core_uint16_land_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion from tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion from tree operation on \a count unsigned-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint16_lor_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint16_lor_unmarshall
	default:
		_core_uint16_lor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion from tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion from tree operation on \a count unsigned-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint16_lxor_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint16_lxor_unmarshall
	default:
		_core_uint16_lxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion from tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAX conversion from tree operation on \a count unsigned-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint16_max_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint16_max_unmarshall
	default:
		_core_uint16_max_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion from tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MIN conversion from tree operation on \a count unsigned-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint16_min_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint16_min_unmarshall
	default:
		_core_uint16_min_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion from tree on unsigned-short datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized addition conversion from tree operation on \a count unsigned-short
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint16_sum_unmarshall(uint16_t *dst, const uint16_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint16_sum_unmarshall
	default:
		_core_uint16_sum_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for default on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for default operation on \a count signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_pre_all(uint32_t *dst, const int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_pre_all(MATH_NOOP)
	default:
		_core_int32_pre_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for default on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for default operation on \a count signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_post_all(int32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_post_all(MATH_NOOP)
	default:
		_core_int32_post_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MIN on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MIN operation on \a count signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_pre_min(uint32_t *dst, const int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_pre_min
	default:
		_core_int32_pre_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MIN on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MIN operation on \a count signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_post_min(int32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_post_min
	default:
		_core_int32_post_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MAXLOC on signed-int/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MAXLOC operation on \a count signed-int/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_int32_pre_maxloc(uint32_int32_t *dst, const int32_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_int32_pre_maxloc
	default:
		_core_int32_int32_pre_maxloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MAXLOC on signed-int/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MAXLOC operation on \a count signed-int/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_int32_post_maxloc(int32_int32_t *dst, const uint32_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_int32_post_maxloc
	default:
		_core_int32_int32_post_maxloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MINLOC on signed-int/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MINLOC operation on \a count signed-int/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_int32_pre_minloc(uint32_int32_t *dst, const int32_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_int32_pre_minloc
	default:
		_core_int32_int32_pre_minloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MINLOC on signed-int/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MINLOC operation on \a count signed-int/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_int32_post_minloc(int32_int32_t *dst, const uint32_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_int32_post_minloc
	default:
		_core_int32_int32_post_minloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion from tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion from tree operation on \a count signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_band_unmarshall(int32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_band_unmarshall
	default:
		_core_int32_band_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion from tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion from tree operation on \a count signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_bor_unmarshall(int32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_bor_unmarshall
	default:
		_core_int32_bor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion from tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion from tree operation on \a count signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_bxor_unmarshall(int32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_bxor_unmarshall
	default:
		_core_int32_bxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion from tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion from tree operation on \a count signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_land_unmarshall(int32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_land_unmarshall
	default:
		_core_int32_land_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion from tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion from tree operation on \a count signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_lor_unmarshall(int32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_lor_unmarshall
	default:
		_core_int32_lor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion from tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion from tree operation on \a count signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_lxor_unmarshall(int32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_lxor_unmarshall
	default:
		_core_int32_lxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion from tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAX conversion from tree operation on \a count signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_max_unmarshall(int32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_max_unmarshall
	default:
		_core_int32_max_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAXLOC conversion from tree on signed-int/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAXLOC conversion from tree operation on \a count signed-int/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_int32_maxloc_unmarshall(int32_int32_t *dst, const uint32_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_int32_maxloc_unmarshall
	default:
		_core_int32_int32_maxloc_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion from tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MIN conversion from tree operation on \a count signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_min_unmarshall(int32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_min_unmarshall
	default:
		_core_int32_min_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MINLOC conversion from tree on signed-int/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MINLOC conversion from tree operation on \a count signed-int/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_int32_minloc_unmarshall(int32_int32_t *dst, const uint32_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_int32_minloc_unmarshall
	default:
		_core_int32_int32_minloc_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion from tree on signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized addition conversion from tree operation on \a count signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int32_sum_unmarshall(int32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_int32_sum_unmarshall
	default:
		_core_int32_sum_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for default on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for default operation on \a count unsigned-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint32_pre_all(uint32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint32_pre_all(MATH_NOOP)
	default:
		_core_uint32_pre_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for default on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for default operation on \a count unsigned-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint32_post_all(uint32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint32_post_all(MATH_NOOP)
	default:
		_core_uint32_post_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MIN on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MIN operation on \a count unsigned-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint32_pre_min(uint32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint32_pre_min
	default:
		_core_uint32_pre_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MIN on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MIN operation on \a count unsigned-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint32_post_min(uint32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint32_post_min
	default:
		_core_uint32_post_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion from tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion from tree operation on \a count unsigned-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint32_band_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint32_band_unmarshall
	default:
		_core_uint32_band_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion from tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion from tree operation on \a count unsigned-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint32_bor_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint32_bor_unmarshall
	default:
		_core_uint32_bor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion from tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion from tree operation on \a count unsigned-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint32_bxor_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint32_bxor_unmarshall
	default:
		_core_uint32_bxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion from tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion from tree operation on \a count unsigned-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint32_land_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint32_land_unmarshall
	default:
		_core_uint32_land_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion from tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion from tree operation on \a count unsigned-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint32_lor_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint32_lor_unmarshall
	default:
		_core_uint32_lor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion from tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion from tree operation on \a count unsigned-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint32_lxor_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint32_lxor_unmarshall
	default:
		_core_uint32_lxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion from tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAX conversion from tree operation on \a count unsigned-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint32_max_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint32_max_unmarshall
	default:
		_core_uint32_max_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion from tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MIN conversion from tree operation on \a count unsigned-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint32_min_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint32_min_unmarshall
	default:
		_core_uint32_min_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion from tree on unsigned-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized addition conversion from tree operation on \a count unsigned-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint32_sum_unmarshall(uint32_t *dst, const uint32_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint32_sum_unmarshall
	default:
		_core_uint32_sum_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for default on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for default operation on \a count signed-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int64_pre_all(uint64_t *dst, const int64_t *src, int count) {
	switch(1) {
	OPTIMIZED_int64_pre_all(MATH_NOOP)
	default:
		_core_int64_pre_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for default on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for default operation on \a count signed-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int64_post_all(int64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_int64_post_all(MATH_NOOP)
	default:
		_core_int64_post_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MIN on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MIN operation on \a count signed-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int64_pre_min(uint64_t *dst, const int64_t *src, int count) {
	switch(1) {
	OPTIMIZED_int64_pre_min
	default:
		_core_int64_pre_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MIN on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MIN operation on \a count signed-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int64_post_min(int64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_int64_post_min
	default:
		_core_int64_post_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion from tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion from tree operation on \a count signed-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int64_band_unmarshall(int64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_int64_band_unmarshall
	default:
		_core_int64_band_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion from tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion from tree operation on \a count signed-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int64_bor_unmarshall(int64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_int64_bor_unmarshall
	default:
		_core_int64_bor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion from tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion from tree operation on \a count signed-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int64_bxor_unmarshall(int64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_int64_bxor_unmarshall
	default:
		_core_int64_bxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion from tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion from tree operation on \a count signed-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int64_land_unmarshall(int64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_int64_land_unmarshall
	default:
		_core_int64_land_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion from tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion from tree operation on \a count signed-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int64_lor_unmarshall(int64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_int64_lor_unmarshall
	default:
		_core_int64_lor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion from tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion from tree operation on \a count signed-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int64_lxor_unmarshall(int64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_int64_lxor_unmarshall
	default:
		_core_int64_lxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion from tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAX conversion from tree operation on \a count signed-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int64_max_unmarshall(int64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_int64_max_unmarshall
	default:
		_core_int64_max_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion from tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MIN conversion from tree operation on \a count signed-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int64_min_unmarshall(int64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_int64_min_unmarshall
	default:
		_core_int64_min_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion from tree on signed-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized addition conversion from tree operation on \a count signed-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_int64_sum_unmarshall(int64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_int64_sum_unmarshall
	default:
		_core_int64_sum_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for default on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for default operation on \a count unsigned-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint64_pre_all(uint64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint64_pre_all(MATH_NOOP)
	default:
		_core_uint64_pre_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for default on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for default operation on \a count unsigned-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint64_post_all(uint64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint64_post_all(MATH_NOOP)
	default:
		_core_uint64_post_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MIN on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MIN operation on \a count unsigned-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint64_pre_min(uint64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint64_pre_min
	default:
		_core_uint64_pre_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MIN on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MIN operation on \a count unsigned-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint64_post_min(uint64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint64_post_min
	default:
		_core_uint64_post_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise AND conversion from tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise AND conversion from tree operation on \a count unsigned-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint64_band_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint64_band_unmarshall
	default:
		_core_uint64_band_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise OR conversion from tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise OR conversion from tree operation on \a count unsigned-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint64_bor_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint64_bor_unmarshall
	default:
		_core_uint64_bor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized bitwise XOR conversion from tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized bitwise XOR conversion from tree operation on \a count unsigned-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint64_bxor_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint64_bxor_unmarshall
	default:
		_core_uint64_bxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical AND conversion from tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical AND conversion from tree operation on \a count unsigned-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint64_land_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint64_land_unmarshall
	default:
		_core_uint64_land_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical OR conversion from tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical OR conversion from tree operation on \a count unsigned-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint64_lor_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint64_lor_unmarshall
	default:
		_core_uint64_lor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized logical XOR conversion from tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized logical XOR conversion from tree operation on \a count unsigned-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint64_lxor_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint64_lxor_unmarshall
	default:
		_core_uint64_lxor_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion from tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAX conversion from tree operation on \a count unsigned-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint64_max_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint64_max_unmarshall
	default:
		_core_uint64_max_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion from tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MIN conversion from tree operation on \a count unsigned-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint64_min_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint64_min_unmarshall
	default:
		_core_uint64_min_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized addition conversion from tree on unsigned-long-long datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized addition conversion from tree operation on \a count unsigned-long-long
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_uint64_sum_unmarshall(uint64_t *dst, const uint64_t *src, int count) {
	switch(1) {
	OPTIMIZED_uint64_sum_unmarshall
	default:
		_core_uint64_sum_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MAX on float datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MAX operation on \a count float
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_pre_max(float *dst, const float *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_pre_max
	default:
		_core_fp32_pre_max(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MAX on float datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MAX operation on \a count float
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_post_max(float *dst, const float *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_post_max
	default:
		_core_fp32_post_max(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MAXLOC on float/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MAXLOC operation on \a count float/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_int32_pre_maxloc(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_int32_pre_maxloc
	default:
		_core_fp32_int32_pre_maxloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MAXLOC on float/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MAXLOC operation on \a count float/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_int32_post_maxloc(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_int32_post_maxloc
	default:
		_core_fp32_int32_post_maxloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MAXLOC on float/float datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MAXLOC operation on \a count float/float
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_fp32_pre_maxloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_fp32_pre_maxloc
	default:
		_core_fp32_fp32_pre_maxloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MAXLOC on float/float datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MAXLOC operation on \a count float/float
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_fp32_post_maxloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_fp32_post_maxloc
	default:
		_core_fp32_fp32_post_maxloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MIN on float datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MIN operation on \a count float
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_pre_min(float *dst, const float *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_pre_min
	default:
		_core_fp32_pre_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MIN on float datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MIN operation on \a count float
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_post_min(float *dst, const float *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_post_min
	default:
		_core_fp32_post_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MINLOC on float/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MINLOC operation on \a count float/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_int32_pre_minloc(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_int32_pre_minloc
	default:
		_core_fp32_int32_pre_minloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MINLOC on float/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MINLOC operation on \a count float/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_int32_post_minloc(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_int32_post_minloc
	default:
		_core_fp32_int32_post_minloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MINLOC on float/float datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MINLOC operation on \a count float/float
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_fp32_pre_minloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_fp32_pre_minloc
	default:
		_core_fp32_fp32_pre_minloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MINLOC on float/float datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MINLOC operation on \a count float/float
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_fp32_post_minloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_fp32_post_minloc
	default:
		_core_fp32_fp32_post_minloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAXLOC conversion from tree on float/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAXLOC conversion from tree operation on \a count float/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_int32_maxloc_unmarshall(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_int32_maxloc_unmarshall
	default:
		_core_fp32_int32_maxloc_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAXLOC conversion from tree on float/float datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAXLOC conversion from tree operation on \a count float/float
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_fp32_maxloc_unmarshall(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_fp32_maxloc_unmarshall
	default:
		_core_fp32_fp32_maxloc_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion from tree on float datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MIN conversion from tree operation on \a count float
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_min_unmarshall(float *dst, const float *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_min_unmarshall
	default:
		_core_fp32_min_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MINLOC conversion from tree on float/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MINLOC conversion from tree operation on \a count float/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_int32_minloc_unmarshall(fp32_int32_t *dst, const fp32_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_int32_minloc_unmarshall
	default:
		_core_fp32_int32_minloc_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MINLOC conversion from tree on float/float datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MINLOC conversion from tree operation on \a count float/float
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_fp32_minloc_unmarshall(fp32_fp32_t *dst, const fp32_fp32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_fp32_minloc_unmarshall
	default:
		_core_fp32_fp32_minloc_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion from tree on float datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAX conversion from tree operation on \a count float
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp32_max_unmarshall(float *dst, const float *src, int count) {
	switch(1) {
	OPTIMIZED_fp32_max_unmarshall
	default:
		_core_fp32_max_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MAX on double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MAX operation on \a count double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_pre_max(double *dst, const double *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_pre_max
	default:
		_core_fp64_pre_max(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MAX on double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MAX operation on \a count double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_post_max(double *dst, const double *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_post_max
	default:
		_core_fp64_post_max(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for default on double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for default operation on \a count double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_pre_all(double *dst, const double *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_pre_all(MATH_NOOP)
	default:
		_core_fp64_pre_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for default on double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for default operation on \a count double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_post_all(double *dst, const double *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_post_all(MATH_NOOP)
	default:
		_core_fp64_post_all(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MAXLOC on double/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MAXLOC operation on \a count double/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_int32_pre_maxloc(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_int32_pre_maxloc
	default:
		_core_fp64_int32_pre_maxloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MAXLOC on double/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MAXLOC operation on \a count double/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_int32_post_maxloc(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_int32_post_maxloc
	default:
		_core_fp64_int32_post_maxloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MAXLOC on double/double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MAXLOC operation on \a count double/double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_fp64_pre_maxloc(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_fp64_pre_maxloc
	default:
		_core_fp64_fp64_pre_maxloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MAXLOC on double/double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MAXLOC operation on \a count double/double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_fp64_post_maxloc(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_fp64_post_maxloc
	default:
		_core_fp64_fp64_post_maxloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MIN on double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MIN operation on \a count double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_pre_min(double *dst, const double *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_pre_min
	default:
		_core_fp64_pre_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MIN on double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MIN operation on \a count double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_post_min(double *dst, const double *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_post_min
	default:
		_core_fp64_post_min(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MINLOC on double/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MINLOC operation on \a count double/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_int32_pre_minloc(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_int32_pre_minloc
	default:
		_core_fp64_int32_pre_minloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MINLOC on double/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MINLOC operation on \a count double/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_int32_post_minloc(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_int32_post_minloc
	default:
		_core_fp64_int32_post_minloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized pre-processing for MINLOC on double/double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized pre-processing for MINLOC operation on \a count double/double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_fp64_pre_minloc(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_fp64_pre_minloc
	default:
		_core_fp64_fp64_pre_minloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized post-processing for MINLOC on double/double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized post-processing for MINLOC operation on \a count double/double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_fp64_post_minloc(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_fp64_post_minloc
	default:
		_core_fp64_fp64_post_minloc(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAX conversion from tree on double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAX conversion from tree operation on \a count double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_max_unmarshall(double *dst, const double *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_max_unmarshall
	default:
		_core_fp64_max_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAXLOC conversion from tree on double/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAXLOC conversion from tree operation on \a count double/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_int32_maxloc_unmarshall(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_int32_maxloc_unmarshall
	default:
		_core_fp64_int32_maxloc_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MAXLOC conversion from tree on double/double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MAXLOC conversion from tree operation on \a count double/double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_fp64_maxloc_unmarshall(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_fp64_maxloc_unmarshall
	default:
		_core_fp64_fp64_maxloc_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MIN conversion from tree on double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MIN conversion from tree operation on \a count double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_min_unmarshall(double *dst, const double *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_min_unmarshall
	default:
		_core_fp64_min_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MINLOC conversion from tree on double/signed-int datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MINLOC conversion from tree operation on \a count double/signed-int
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_int32_minloc_unmarshall(fp64_int32_t *dst, const fp64_int32_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_int32_minloc_unmarshall
	default:
		_core_fp64_int32_minloc_unmarshall(dst, src, count);
		break;
	}
}

/**
 * \brief Optimized MINLOC conversion from tree on double/double datatypes.
 *
 * \param dst The destination buffer.
 * \param src The source buffer.
 * \param count The number of elements.
 *
 * Optimized MINLOC conversion from tree operation on \a count double/double
 * elements of the source buffer, results in destination buffer.
 */
static inline void Core_fp64_fp64_minloc_unmarshall(fp64_fp64_t *dst, const fp64_fp64_t *src, int count) {
	switch(1) {
	OPTIMIZED_fp64_fp64_minloc_unmarshall
	default:
		_core_fp64_fp64_minloc_unmarshall(dst, src, count);
		break;
	}
}

#undef OPTIMATH_UNARY

#if defined(__cplusplus)
}; // extern "C"
#endif

#endif /* _math_bg_math_h_ */
