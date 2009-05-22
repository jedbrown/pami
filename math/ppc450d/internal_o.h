/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file math/ppc450d/internal_o.h
 * \brief Prototypes of optimized routines used internally
 */

#ifndef __dcmf_ppc450d_internal_o_h__
#define __dcmf_ppc450d_internal_o_h__

#if defined(__cplusplus)
extern "C" {
#endif /* C++ */

extern void _core_int16_conv_not_o(uint16_t *dst, const int16_t *src, int count);
extern void _core_int16_not_o(int16_t *dst, const int16_t *src, int count);
extern void _core_int16_conv_o(uint16_t *dst, const int16_t *src, int count);
extern void _core_int16_min_conv2(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_prod_conv2(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int32_conv_o(uint32_t *dst, const int32_t *src, int count);
extern void _core_int32_not_o(int32_t *dst, const int32_t *src, int count);
extern void _core_int32_conv_not_o(uint32_t *dst, const int32_t *src, int count);
extern void _core_int32_min_conv2(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_prod_to_tree2(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int64_conv_o(uint64_t *dst, const int64_t *src, int count);
extern void _core_int64_not_o(int64_t *dst, const int64_t *src, int count);
extern void _core_int64_conv_not_o(uint64_t *dst, const int64_t *src, int count);
extern void _core_int8_conv_not_o(uint8_t *dst, const int8_t *src, int count);
extern void _core_int8_not_o(int8_t *dst, const int8_t *src, int count);
extern void _core_int8_conv_o(uint8_t *dst, const int8_t *src, int count);
extern void _core_int8_pack_o(uint8_t *dst, const int8_t *src, int count);
extern void _core_int8_unpack_o(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_min_conv2(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_prod_to_tree2(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_uint16_conv_not_o(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_conv_o(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_not_o(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_pack_o(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_unpack_o(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint32_conv_not_o(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_conv_o(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_not_o(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_pack_o(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_unpack_o(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint64_conv_o(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_not_o(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_conv_not_o(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint8_conv_not_o(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_conv_o(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_not_o(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_pack_o(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_unpack_o(uint8_t *dst, const uint8_t *src, int count);

#if defined(__cplusplus)
};
#endif /* C++ */

#endif /* __dcmf_ppc450d_internal_o_h__ */
