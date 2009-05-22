/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file math/internal.h
 * \brief Prototypes of routines used internally
 */

#ifndef __math_math_internal_h__
#define __math_math_internal_h__

#if defined(__cplusplus)
extern "C" {
#endif /* C++ */

extern void _core_int16_conv_not(uint16_t *dst, const int16_t *src, int count);
extern void _core_int16_conv(uint16_t *dst, const int16_t *src, int count);
extern void _core_int16_min_conv(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_not(int16_t *dst, const int16_t *src, int count);
extern void _core_int16_prod_conv(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int32_conv_not(uint32_t *dst, const int32_t *src, int count);
extern void _core_int32_conv(uint32_t *dst, const int32_t *src, int count);
extern void _core_int32_min_conv(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_not(int32_t *dst, const int32_t *src, int count);
extern void _core_int32_prod_to_tree(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int64_conv_not(uint64_t *dst, const int64_t *src, int count);
extern void _core_int64_conv(uint64_t *dst, const int64_t *src, int count);
extern void _core_int64_not(int64_t *dst, const int64_t *src, int count);
extern void _core_int8_conv_not(uint8_t *dst, const int8_t *src, int count); 
extern void _core_int8_conv(uint8_t *dst, const int8_t *src, int count); 
extern void _core_int8_min_conv(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_not(int8_t *dst, const int8_t *src, int count); 
extern void _core_int8_pack(uint8_t *dst, const int8_t *src, int count);
extern void _core_int8_prod_to_tree(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_unpack(int8_t *dst, const uint8_t *src, int count);
extern void _core_uint16_conv_not(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_conv(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_not(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_pack(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_unpack(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint32_conv_not(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_conv(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_not(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_pack(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_unpack(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint64_conv_not(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_conv(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_not(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_pack(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_unpack(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint8_conv_not(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_conv(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_not(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_pack(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_unpack(uint8_t *dst, const uint8_t *src, int count);

#if defined(__cplusplus)
};
#endif /* C++ */

#endif /* __dcmf_math_internal_h__ */
