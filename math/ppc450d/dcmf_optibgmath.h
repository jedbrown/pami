/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file math/ppc450d/dcmf_optibgmath.h
 * \brief Optimized collective network math routines
 *
 * This file describes any optimized math routines and facilitates
 * plugging them into the core math routine inlines in dcmf_bg_math.h
 *
 * Ordinarely, this file should not be included directly. Only
 * dcmf_bg_math.h should appear in source code #include's.
 *
 * These routines are the ones used by the collective network to
 * process data for use on the tree. These are not public interfaces.
 *
 * Caution, this file should not be changed after building the DCMF
 * libraries.  Application compiles should see the same file as was
 * used for building the product.
 */

#ifndef _dcmf_optibgmath_h_
#define _dcmf_optibgmath_h_

#if defined(__cplusplus)
extern "C" {
#endif /* C++ */

extern void _core_fp32_fp32_maxloc_marshall(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count);
extern void _core_fp32_fp32_maxloc_unmarshall(fp32_fp32_t *dst, const fp32_fp32_t *src, int count);
extern void _core_fp32_fp32_minloc_marshall(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count);
extern void _core_fp32_fp32_minloc_unmarshall(fp32_fp32_t *dst, const fp32_fp32_t *src, int count);
extern void _core_fp32_fp32_post_maxloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count);
extern void _core_fp32_fp32_post_minloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count);
extern void _core_fp32_fp32_pre_maxloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count);
extern void _core_fp32_fp32_pre_minloc(fp32_fp32_t *dst, const fp32_fp32_t *src, int count);
extern void _core_fp32_int32_maxloc_marshall(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count);
extern void _core_fp32_int32_maxloc_unmarshall(fp32_int32_t *dst, const fp32_int32_t *src, int count);
extern void _core_fp32_int32_minloc_marshall(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count);
extern void _core_fp32_int32_minloc_unmarshall(fp32_int32_t *dst, const fp32_int32_t *src, int count);
extern void _core_fp32_int32_post_maxloc(fp32_int32_t *dst, const fp32_int32_t *src, int count);
extern void _core_fp32_int32_post_minloc(fp32_int32_t *dst, const fp32_int32_t *src, int count);
extern void _core_fp32_int32_pre_maxloc(fp32_int32_t *dst, const fp32_int32_t *src, int count);
extern void _core_fp32_int32_pre_minloc(fp32_int32_t *dst, const fp32_int32_t *src, int count);
extern void _core_fp32_max_marshall(float *dst, const float **srcs, int nsrc, int count);
extern void _core_fp32_max_unmarshall(float *dst, const float *src, int count);
extern void _core_fp32_min_marshall(float *dst, const float **srcs, int nsrc, int count);
extern void _core_fp32_min_unmarshall(float *dst, const float *src, int count);
extern void _core_fp32_post_max(float *dst, const float *src, int count);
extern void _core_fp32_post_min(float *dst, const float *src, int count);
extern void _core_fp32_pre_max(float *dst, const float *src, int count);
extern void _core_fp32_pre_min(float *dst, const float *src, int count);
extern void _core_fp64_fp64_maxloc_marshall(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count);
extern void _core_fp64_fp64_maxloc_unmarshall(fp64_fp64_t *dst, const fp64_fp64_t *src, int count);
extern void _core_fp64_fp64_minloc_marshall(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count);
extern void _core_fp64_fp64_minloc_unmarshall(fp64_fp64_t *dst, const fp64_fp64_t *src, int count);
extern void _core_fp64_fp64_post_maxloc(fp64_fp64_t *dst, const fp64_fp64_t *src, int count);
extern void _core_fp64_fp64_post_minloc(fp64_fp64_t *dst, const fp64_fp64_t *src, int count);
extern void _core_fp64_fp64_pre_maxloc(fp64_fp64_t *dst, const fp64_fp64_t *src, int count);
extern void _core_fp64_fp64_pre_minloc(fp64_fp64_t *dst, const fp64_fp64_t *src, int count);
extern void _core_fp64_int32_maxloc_marshall(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count);
extern void _core_fp64_int32_maxloc_unmarshall(fp64_int32_t *dst, const fp64_int32_t *src, int count);
extern void _core_fp64_int32_minloc_marshall(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count);
extern void _core_fp64_int32_minloc_unmarshall(fp64_int32_t *dst, const fp64_int32_t *src, int count);
extern void _core_fp64_int32_post_maxloc(fp64_int32_t *dst, const fp64_int32_t *src, int count);
extern void _core_fp64_int32_post_minloc(fp64_int32_t *dst, const fp64_int32_t *src, int count);
extern void _core_fp64_int32_pre_maxloc(fp64_int32_t *dst, const fp64_int32_t *src, int count);
extern void _core_fp64_int32_pre_minloc(fp64_int32_t *dst, const fp64_int32_t *src, int count);
extern void _core_fp64_max_marshall(double *dst, const double **srcs, int nsrc, int count);
extern void _core_fp64_max_unmarshall(double *dst, const double *src, int count);
extern void _core_fp64_min_marshall(double *dst, const double **srcs, int nsrc, int count);
extern void _core_fp64_min_unmarshall(double *dst, const double *src, int count);
extern void _core_fp64_post_all(double *dst, const double *src, int count);
extern void _core_fp64_post_max(double *dst, const double *src, int count);
extern void _core_fp64_post_min(double *dst, const double *src, int count);
extern void _core_fp64_post_sum(double *dst, const double *src, int count);
extern void _core_fp64_pre_all(double *dst, const double *src, int count);
extern void _core_fp64_pre_max(double *dst, const double *src, int count);
extern void _core_fp64_pre_min(double *dst, const double *src, int count);
extern void _core_fp64_pre_sum(double *dst, const double *src, int count);
extern void _core_int16_band_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_band_unmarshall(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_bor_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_bor_unmarshall(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_bxor_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_bxor_unmarshall(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_int32_maxloc_marshall(uint16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count);
extern void _core_int16_int32_maxloc_unmarshall(int16_int32_t *dst, const uint16_int32_t *src, int count);
extern void _core_int16_int32_minloc_marshall(uint16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count);
extern void _core_int16_int32_minloc_unmarshall(int16_int32_t *dst, const uint16_int32_t *src, int count);
extern void _core_int16_int32_post_maxloc(int16_int32_t *dst, const uint16_int32_t *src, int count);
extern void _core_int16_int32_post_minloc(int16_int32_t *dst, const uint16_int32_t *src, int count);
extern void _core_int16_int32_pre_maxloc(uint16_int32_t *dst, const int16_int32_t *src, int count);
extern void _core_int16_int32_pre_minloc(uint16_int32_t *dst, const int16_int32_t *src, int count);
extern void _core_int16_land_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_land_unmarshall(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_lor_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_lor_unmarshall(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_lxor_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_lxor_unmarshall(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_max_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_max_unmarshall(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_min_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_min_unmarshall(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_post_all(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_post_min(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_pre_all(uint16_t *dst, const int16_t *src, int count);
extern void _core_int16_pre_min(uint16_t *dst, const int16_t *src, int count);
extern void _core_int16_sum_marshall(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_sum_unmarshall(int16_t *dst, const uint16_t *src, int count);
extern void _core_int32_band_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_band_unmarshall(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_bor_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_bor_unmarshall(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_bxor_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_bxor_unmarshall(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_int32_maxloc_marshall(uint32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count);
extern void _core_int32_int32_maxloc_unmarshall(int32_int32_t *dst, const uint32_int32_t *src, int count);
extern void _core_int32_int32_minloc_marshall(uint32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count);
extern void _core_int32_int32_minloc_unmarshall(int32_int32_t *dst, const uint32_int32_t *src, int count);
extern void _core_int32_int32_post_maxloc(int32_int32_t *dst, const uint32_int32_t *src, int count);
extern void _core_int32_int32_post_minloc(int32_int32_t *dst, const uint32_int32_t *src, int count);
extern void _core_int32_int32_pre_maxloc(uint32_int32_t *dst, const int32_int32_t *src, int count);
extern void _core_int32_int32_pre_minloc(uint32_int32_t *dst, const int32_int32_t *src, int count);
extern void _core_int32_land_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_land_unmarshall(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_lor_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_lor_unmarshall(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_lxor_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_lxor_unmarshall(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_max_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_max_unmarshall(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_min_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_min_unmarshall(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_post_all(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_post_min(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_pre_all(uint32_t *dst, const int32_t *src, int count);
extern void _core_int32_pre_min(uint32_t *dst, const int32_t *src, int count);
extern void _core_int32_sum_marshall(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_sum_unmarshall(int32_t *dst, const uint32_t *src, int count);
extern void _core_int64_band_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_band_unmarshall(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_bor_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_bor_unmarshall(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_bxor_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_bxor_unmarshall(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_land_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_land_unmarshall(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_lor_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_lor_unmarshall(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_lxor_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_lxor_unmarshall(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_max_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_max_unmarshall(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_min_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_min_unmarshall(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_post_all(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_post_min(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_pre_all(uint64_t *dst, const int64_t *src, int count);
extern void _core_int64_pre_min(uint64_t *dst, const int64_t *src, int count);
extern void _core_int64_sum_marshall(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_sum_unmarshall(int64_t *dst, const uint64_t *src, int count);
extern void _core_int8_band_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_band_unmarshall(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_bor_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_bor_unmarshall(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_bxor_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_bxor_unmarshall(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_land_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_land_unmarshall(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_lor_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_lor_unmarshall(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_lxor_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_lxor_unmarshall(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_max_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_max_unmarshall(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_min_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_min_unmarshall(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_post_all(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_post_min(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_pre_all(uint8_t *dst, const int8_t *src, int count);
extern void _core_int8_pre_min(uint8_t *dst, const int8_t *src, int count);
extern void _core_int8_sum_marshall(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_sum_unmarshall(int8_t *dst, const uint8_t *src, int count);
extern void _core_uint16_band_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_band_unmarshall(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_bor_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_bor_unmarshall(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_bxor_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_bxor_unmarshall(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_land_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_land_unmarshall(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_lor_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_lor_unmarshall(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_lxor_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_lxor_unmarshall(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_max_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_max_unmarshall(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_min_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_min_unmarshall(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_post_all(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_post_min(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_pre_all(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_pre_min(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_sum_marshall(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_sum_unmarshall(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint32_band_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_band_unmarshall(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_bor_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_bor_unmarshall(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_bxor_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_bxor_unmarshall(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_land_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_land_unmarshall(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_lor_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_lor_unmarshall(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_lxor_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_lxor_unmarshall(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_max_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_max_unmarshall(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_min_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_min_unmarshall(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_post_all(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_post_min(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_pre_all(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_pre_min(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_sum_marshall(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_sum_unmarshall(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint64_band_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_band_unmarshall(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_bor_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_bor_unmarshall(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_bxor_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_bxor_unmarshall(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_land_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_land_unmarshall(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_lor_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_lor_unmarshall(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_lxor_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_lxor_unmarshall(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_max_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_max_unmarshall(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_min_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_min_unmarshall(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_post_all(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_post_min(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_pre_all(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_pre_min(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_sum_marshall(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_sum_unmarshall(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint8_band_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_band_unmarshall(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_bor_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_bor_unmarshall(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_bxor_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_bxor_unmarshall(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_land_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_land_unmarshall(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_lor_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_lor_unmarshall(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_lxor_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_lxor_unmarshall(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_max_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_max_unmarshall(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_min_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_min_unmarshall(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_post_all(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_post_min(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_pre_all(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_pre_min(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_sum_marshall(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_sum_unmarshall(uint8_t *dst, const uint8_t *src, int count);

/* These are non-standard prototypes */
extern void _core_fp64_pre1_2pass(uint16_t *dst_e, uint32_t *dst_m, const double *src, int count);
extern void _core_fp64_pre2_2pass(uint32_t *dst_src_m, uint16_t *src_e, uint16_t *src_ee, int count);
extern void _core_fp64_post_2pass(double *dst, uint16_t *src_e, uint32_t *src_m, int count);


/* Optimized math routines */

extern void _core_fp32_fp32_maxloc_marshall2(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count);
extern void _core_fp32_fp32_minloc_marshall2(fp32_fp32_t *dst, const fp32_fp32_t **srcs, int nsrc, int count);
extern void _core_fp32_fp32_post_maxloc_o(fp32_fp32_t *dst, const fp32_fp32_t *src, int count);
extern void _core_fp32_fp32_post_minloc_o(fp32_fp32_t *dst, const fp32_fp32_t *src, int count);
extern void _core_fp32_fp32_pre_maxloc_o(fp32_fp32_t *dst, const fp32_fp32_t *src, int count);
extern void _core_fp32_fp32_pre_minloc_o(fp32_fp32_t *dst, const fp32_fp32_t *src, int count);
extern void _core_fp32_int32_maxloc_marshall2(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count);
extern void _core_fp32_int32_minloc_marshall2(fp32_int32_t *dst, const fp32_int32_t **srcs, int nsrc, int count);
extern void _core_fp32_int32_post_maxloc_o(fp32_int32_t *dst, const fp32_int32_t *src, int count);
extern void _core_fp32_int32_post_minloc_o(fp32_int32_t *dst, const fp32_int32_t *src, int count);
extern void _core_fp32_int32_pre_maxloc_o(fp32_int32_t *dst, const fp32_int32_t *src, int count);
extern void _core_fp32_int32_pre_minloc_o(fp32_int32_t *dst, const fp32_int32_t *src, int count);
extern void _core_fp32_max_marshall2(float *dst, const float **srcs, int nsrc, int count);
extern void _core_fp32_min_marshall2(float *dst, const float **srcs, int nsrc, int count);
extern void _core_fp32_post_max_o(float *dst, const float *src, int count);
extern void _core_fp32_post_min_o(float *dst, const float *src, int count);
extern void _core_fp32_pre_max_o(float *dst, const float *src, int count);
extern void _core_fp32_pre_min_o(float *dst, const float *src, int count);
extern void _core_fp64_fp64_maxloc_marshall2(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count);
extern void _core_fp64_fp64_minloc_marshall2(fp64_fp64_t *dst, const fp64_fp64_t **srcs, int nsrc, int count);
extern void _core_fp64_fp64_post_maxloc_o(fp64_fp64_t *dst, const fp64_fp64_t *src, int count);
extern void _core_fp64_fp64_post_minloc_o(fp64_fp64_t *dst, const fp64_fp64_t *src, int count);
extern void _core_fp64_fp64_pre_maxloc_o(fp64_fp64_t *dst, const fp64_fp64_t *src, int count);
extern void _core_fp64_fp64_pre_minloc_o(fp64_fp64_t *dst, const fp64_fp64_t *src, int count);
extern void _core_fp64_int32_maxloc_marshall2(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count);
extern void _core_fp64_int32_minloc_marshall2(fp64_int32_t *dst, const fp64_int32_t **srcs, int nsrc, int count);
extern void _core_fp64_int32_post_maxloc_o(fp64_int32_t *dst, const fp64_int32_t *src, int count);
extern void _core_fp64_int32_post_minloc_o(fp64_int32_t *dst, const fp64_int32_t *src, int count);
extern void _core_fp64_int32_pre_maxloc_o(fp64_int32_t *dst, const fp64_int32_t *src, int count);
extern void _core_fp64_int32_pre_minloc_o(fp64_int32_t *dst, const fp64_int32_t *src, int count);
extern void _core_fp64_max_marshall2(double *dst, const double **srcs, int nsrc, int count);
extern void _core_fp64_min_marshall2(double *dst, const double **srcs, int nsrc, int count);
extern void _core_fp64_post_max_o(double *dst, const double *src, int count);
extern void _core_fp64_post_min_o(double *dst, const double *src, int count);
extern void _core_fp64_pre_max_o(double *dst, const double *src, int count);
extern void _core_fp64_pre_min_o(double *dst, const double *src, int count);
extern void _core_int16_band_marshall2(int16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_bor_marshall2(int16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_bxor_marshall2(int16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_int32_maxloc_marshall2(uint16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count);
extern void _core_int16_int32_maxloc_unmarshall_o(int16_int32_t *dst, const uint16_int32_t *src, int count);
extern void _core_int16_int32_minloc_marshall2(uint16_int32_t *dst, const int16_int32_t **srcs, int nsrc, int count);
extern void _core_int16_int32_minloc_unmarshall_o(int16_int32_t *dst, const uint16_int32_t *src, int count);
extern void _core_int16_int32_post_maxloc_o(int16_int32_t *dst, const uint16_int32_t *src, int count);
extern void _core_int16_int32_post_minloc_o(int16_int32_t *dst, const uint16_int32_t *src, int count);
extern void _core_int16_int32_pre_maxloc_o(uint16_int32_t *dst, const int16_int32_t *src, int count);
extern void _core_int16_int32_pre_minloc_o(uint16_int32_t *dst, const int16_int32_t *src, int count);
extern void _core_int16_land_marshall2(int16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_lor_marshall2(int16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_lxor_marshall2(int16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_max_marshall2(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_max_unmarshall_o(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_min_marshall2(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int16_min_unmarshall_o(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_post_all_o(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_post_min_o(int16_t *dst, const uint16_t *src, int count);
extern void _core_int16_pre_all_o(uint16_t *dst, const int16_t *src, int count);
extern void _core_int16_pre_min_o(uint16_t *dst, const int16_t *src, int count);
extern void _core_int16_sum_marshall2(uint16_t *dst, const int16_t **srcs, int nsrc, int count);
extern void _core_int32_band_marshall2(int32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_bor_marshall2(int32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_bxor_marshall2(int32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_int32_maxloc_marshall2(uint32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count);
extern void _core_int32_int32_maxloc_unmarshall_o(int32_int32_t *dst, const uint32_int32_t *src, int count);
extern void _core_int32_int32_minloc_marshall2(uint32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count);
extern void _core_int32_int32_minloc_unmarshall_o(int32_int32_t *dst, const uint32_int32_t *src, int count);
extern void _core_int32_int32_post_maxloc_o(int32_int32_t *dst, const uint32_int32_t *src, int count);
extern void _core_int32_int32_post_minloc_o(int32_int32_t *dst, const uint32_int32_t *src, int count);
extern void _core_int32_int32_pre_maxloc_o(uint32_int32_t *dst, const int32_int32_t *src, int count);
extern void _core_int32_int32_pre_minloc_o(uint32_int32_t *dst, const int32_int32_t *src, int count);
extern void _core_int32_land_marshall2(int32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_lor_marshall2(int32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_lxor_marshall2(int32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_max_marshall2(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_max_unmarshall_o(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_min_marshall2(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int32_min_unmarshall_o(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_post_all_o(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_post_min_o(int32_t *dst, const uint32_t *src, int count);
extern void _core_int32_pre_all_o(uint32_t *dst, const int32_t *src, int count);
extern void _core_int32_pre_min_o(uint32_t *dst, const int32_t *src, int count);
extern void _core_int32_sum_marshall2(uint32_t *dst, const int32_t **srcs, int nsrc, int count);
extern void _core_int64_band_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_bor_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_bxor_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_land_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_lor_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_lxor_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_max_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_max_unmarshall_o(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_min_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int64_min_unmarshall_o(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_post_all_o(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_post_min_o(int64_t *dst, const uint64_t *src, int count);
extern void _core_int64_pre_all_o(uint64_t *dst, const int64_t *src, int count);
extern void _core_int64_pre_min_o(uint64_t *dst, const int64_t *src, int count);
extern void _core_int64_sum_marshall2(uint64_t *dst, const int64_t **srcs, int nsrc, int count);
extern void _core_int8_band_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_bor_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_bxor_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_land_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_lor_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_lxor_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_max_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_max_unmarshall_o(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_min_marshall2(int8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_int8_min_unmarshall_o(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_post_all_o(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_post_min_o(int8_t *dst, const uint8_t *src, int count);
extern void _core_int8_pre_all_o(uint8_t *dst, const int8_t *src, int count);
extern void _core_int8_pre_min_o(uint8_t *dst, const int8_t *src, int count);
extern void _core_int8_sum_marshall2(uint8_t *dst, const int8_t **srcs, int nsrc, int count);
extern void _core_uint16_band_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_bor_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_bxor_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_land_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_lor_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_lxor_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint16_min_unmarshall_o(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_post_min_o(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_pre_min_o(uint16_t *dst, const uint16_t *src, int count);
extern void _core_uint16_sum_marshall2(uint16_t *dst, const uint16_t **srcs, int nsrc, int count);
extern void _core_uint32_band_marshall2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_bor_marshall2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_bxor_marshall2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_land_marshall2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_lor_marshall2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_lxor_marshall2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_max_marshall2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint32_min_unmarshall_o(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_post_min_o(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_pre_min_o(uint32_t *dst, const uint32_t *src, int count);
extern void _core_uint32_sum_marshall2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count);
extern void _core_uint64_band_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_bor_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_bxor_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_land_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_lor_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_lxor_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint64_min_unmarshall_o(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_post_min_o(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_pre_min_o(uint64_t *dst, const uint64_t *src, int count);
extern void _core_uint64_sum_marshall2(uint64_t *dst, const uint64_t **srcs, int nsrc, int count);
extern void _core_uint8_band_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_bor_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_bxor_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_land_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_lor_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_lxor_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_max_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);
extern void _core_uint8_min_unmarshall_o(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_post_min_o(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_pre_min_o(uint8_t *dst, const uint8_t *src, int count);
extern void _core_uint8_sum_marshall2(uint8_t *dst, const uint8_t **srcs, int nsrc, int count);

#if defined(__cplusplus)
}; // extern "C"
#endif /* C++ */

/**
 * N-way (N >= 2) routines.
 *
 * \param[out] dst	Results buffer pointer
 * \param[in] srcs	Pointer to array of pointers to source buffers
 * \param[in] nsrc	Number of source buffers
 * \param[in] count	Number of elements in each buffer
 *
 * If a particular routine has optimized versions, for specific source
 * buffer counts, then the corresponding define here will have
 * statements added in the form:
 *
 *	OPTIMATH_NSRC(dt,op,N,_core_<type>_<oper>N)
 *	...
 *
 * Where "N" is the number of input buffers, "<type>" is the datatype,
 * "<oper>" is the operand mnemonic, "dt" is the DCMF datatype, and "op"
 * is the DCMF operand.  Additionally, there must be a prototype
 * declaration for the optimized routine(s). More than one statement
 * may be present in the macro.
 *
 * Don't forget the backslashes at the end of each line.
 *
 * The code in dcmf_bg_math.h that uses these will
 * be (note: no semicolon after macro):
 *
 * #define OPTIMATH_NSRC(dt,op,n,f) case n: \
 *					f(dst, srcs, nsrc, count);\
 *					break;
 *
 * inline void Core_<type>_<oper>(params...) {
 * 	switch(nsrc) {
 *	OPTIMIZED_<type>_<oper>
 *	default:
 *		_core_<type>_<oper>(params...);
 *		break;
 *	}
 * }
 *
 * These defines will also be used in dcmf_dat.c to build a table
 * of optimized routines, by datatype, operand, and number of inputs.
 * (Note, nsrc will always be at least 2)
 *
 * #define OPTIMATH_NSRC(dt,op,n,f)	[dt][op][n-2] = f,
 *
 * void *dcmf_op_funcs[ndt][nop][nin] = {
 * OPTIMIZED_<type>_<oper>
 * ...
 * };
 *
 * This table is accessed by using the DCMF_OP_FUNCS(dt,op,n) macro
 * (inline).
 */
#define OPTIMIZED_int8_band_marshall
#define OPTIMIZED_int8_bor_marshall
#define OPTIMIZED_int8_bxor_marshall
#define OPTIMIZED_int8_land_marshall
#define OPTIMIZED_int8_lor_marshall
#define OPTIMIZED_int8_lxor_marshall
#define OPTIMIZED_int8_max_marshall
#define OPTIMIZED_int8_min_marshall
#define OPTIMIZED_int8_sum_marshall	\
	OPTIMATH_NSRC(DCMF_SIGNED_CHAR,DCMF_SUM,2,_core_int8_sum_marshall2)
#define OPTIMIZED_uint8_band_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_CHAR,DCMF_BAND,2,_core_uint8_band_marshall2)
#define OPTIMIZED_uint8_bor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_CHAR,DCMF_BOR,2,_core_uint8_bor_marshall2)
#define OPTIMIZED_uint8_bxor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_CHAR,DCMF_BXOR,2,_core_uint8_bxor_marshall2)
#define OPTIMIZED_uint8_land_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_CHAR,DCMF_LAND,2,_core_uint8_land_marshall2)
#define OPTIMIZED_uint8_lor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_CHAR,DCMF_LOR,2,_core_uint8_lor_marshall2)
#define OPTIMIZED_uint8_lxor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_CHAR,DCMF_LXOR,2,_core_uint8_lxor_marshall2)
#define OPTIMIZED_uint8_max_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_CHAR,DCMF_MAX,2,_core_uint8_max_marshall2)
#define OPTIMIZED_uint8_min_marshall
#define OPTIMIZED_uint8_sum_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_CHAR,DCMF_SUM,2,_core_uint8_sum_marshall2)
#define OPTIMIZED_int16_band_marshall
#define OPTIMIZED_int16_bor_marshall
#define OPTIMIZED_int16_bxor_marshall
#define OPTIMIZED_int16_land_marshall
#define OPTIMIZED_int16_lor_marshall
#define OPTIMIZED_int16_lxor_marshall
#define OPTIMIZED_int16_max_marshall			\
	OPTIMATH_NSRC(DCMF_SIGNED_SHORT,DCMF_MAX,2,_core_int16_max_marshall2)
#define OPTIMIZED_int16_int32_maxloc_marshall
#define OPTIMIZED_int16_min_marshall
#define OPTIMIZED_int16_int32_minloc_marshall
#define OPTIMIZED_int16_sum_marshall	\
	OPTIMATH_NSRC(DCMF_SIGNED_SHORT,DCMF_SUM,2,_core_int16_sum_marshall2)
#define OPTIMIZED_uint16_band_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_SHORT,DCMF_BAND,2,_core_uint16_band_marshall2)
#define OPTIMIZED_uint16_bor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_SHORT,DCMF_BOR,2,_core_uint16_bor_marshall2)
#define OPTIMIZED_uint16_bxor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_SHORT,DCMF_BXOR,2,_core_uint16_bxor_marshall2)
#define OPTIMIZED_uint16_land_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_SHORT,DCMF_LAND,2,_core_uint16_land_marshall2)
#define OPTIMIZED_uint16_lor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_SHORT,DCMF_LOR,2,_core_uint16_lor_marshall2)
#define OPTIMIZED_uint16_lxor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_SHORT,DCMF_LXOR,2,_core_uint16_lxor_marshall2)
#define OPTIMIZED_uint16_max_marshall
#define OPTIMIZED_uint16_min_marshall
#define OPTIMIZED_uint16_sum_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_SHORT,DCMF_SUM,2,_core_uint16_sum_marshall2)
#define OPTIMIZED_int32_band_marshall
#define OPTIMIZED_int32_bor_marshall
#define OPTIMIZED_int32_bxor_marshall
#define OPTIMIZED_int32_land_marshall
#define OPTIMIZED_int32_lor_marshall
#define OPTIMIZED_int32_lxor_marshall
#define OPTIMIZED_int32_max_marshall			\
	OPTIMATH_NSRC(DCMF_SIGNED_INT,DCMF_MAX,2,_core_int32_max_marshall2)
#define OPTIMIZED_int32_int32_maxloc_marshall
#define OPTIMIZED_int32_min_marshall
#define OPTIMIZED_int32_int32_minloc_marshall
#define OPTIMIZED_int32_sum_marshall	\
	OPTIMATH_NSRC(DCMF_SIGNED_INT,DCMF_SUM,2,_core_int32_sum_marshall2)
#define OPTIMIZED_uint32_band_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_INT,DCMF_BAND,2,_core_uint32_band_marshall2)
#define OPTIMIZED_uint32_bor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_INT,DCMF_BOR,2,_core_uint32_bor_marshall2)
#define OPTIMIZED_uint32_bxor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_INT,DCMF_BXOR,2,_core_uint32_bxor_marshall2)
#define OPTIMIZED_uint32_land_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_INT,DCMF_LAND,2,_core_uint32_land_marshall2)
#define OPTIMIZED_uint32_lor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_INT,DCMF_LOR,2,_core_uint32_lor_marshall2)
#define OPTIMIZED_uint32_lxor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_INT,DCMF_LXOR,2,_core_uint32_lxor_marshall2)
#define OPTIMIZED_uint32_max_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_INT,DCMF_MAX,2,_core_uint32_max_marshall2)
#define OPTIMIZED_uint32_min_marshall
#define OPTIMIZED_uint32_sum_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_INT,DCMF_SUM,2,_core_uint32_sum_marshall2)
#define OPTIMIZED_int64_band_marshall
#define OPTIMIZED_int64_bor_marshall
#define OPTIMIZED_int64_bxor_marshall
#define OPTIMIZED_int64_land_marshall
#define OPTIMIZED_int64_lor_marshall
#define OPTIMIZED_int64_lxor_marshall
#define OPTIMIZED_int64_max_marshall
#define OPTIMIZED_int64_min_marshall
#define OPTIMIZED_int64_sum_marshall	\
	OPTIMATH_NSRC(DCMF_SIGNED_LONG_LONG,DCMF_SUM,2,_core_int64_sum_marshall2)
#define OPTIMIZED_uint64_band_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_LONG_LONG,DCMF_BAND,2,_core_uint64_band_marshall2)
#define OPTIMIZED_uint64_bor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_LONG_LONG,DCMF_BOR,2,_core_uint64_bor_marshall2)
#define OPTIMIZED_uint64_bxor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_LONG_LONG,DCMF_BXOR,2,_core_uint64_bxor_marshall2)
#define OPTIMIZED_uint64_land_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_LONG_LONG,DCMF_LAND,2,_core_uint64_land_marshall2)
#define OPTIMIZED_uint64_lor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_LONG_LONG,DCMF_LOR,2,_core_uint64_lor_marshall2)
#define OPTIMIZED_uint64_lxor_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_LONG_LONG,DCMF_LXOR,2,_core_uint64_lxor_marshall2)
#define OPTIMIZED_uint64_max_marshall
#define OPTIMIZED_uint64_min_marshall
#define OPTIMIZED_uint64_sum_marshall	\
	OPTIMATH_NSRC(DCMF_UNSIGNED_LONG_LONG,DCMF_SUM,2,_core_uint64_sum_marshall2)
#define OPTIMIZED_fp32_band_marshall
#define OPTIMIZED_fp32_bor_marshall
#define OPTIMIZED_fp32_bxor_marshall
#define OPTIMIZED_fp32_land_marshall
#define OPTIMIZED_fp32_lor_marshall
#define OPTIMIZED_fp32_lxor_marshall
#define OPTIMIZED_fp32_max_marshall
#define OPTIMIZED_fp32_int32_maxloc_marshall
#define OPTIMIZED_fp32_fp32_maxloc_marshall
#define OPTIMIZED_fp32_min_marshall
#define OPTIMIZED_fp32_int32_minloc_marshall
#define OPTIMIZED_fp32_fp32_minloc_marshall
#define OPTIMIZED_fp64_band_marshall
#define OPTIMIZED_fp64_bor_marshall
#define OPTIMIZED_fp64_bxor_marshall
#define OPTIMIZED_fp64_land_marshall
#define OPTIMIZED_fp64_lor_marshall
#define OPTIMIZED_fp64_lxor_marshall
#define OPTIMIZED_fp64_max_marshall
#define OPTIMIZED_fp64_int32_maxloc_marshall
#define OPTIMIZED_fp64_fp64_maxloc_marshall
#define OPTIMIZED_fp64_min_marshall
#define OPTIMIZED_fp64_int32_minloc_marshall
#define OPTIMIZED_fp64_fp64_minloc_marshall

/**
 * The simple, unary, routines.
 *
 * \param[out] dst	Results buffer pointer
 * \param[in] src	Source buffer pointer
 * \param[in] count	Number of elements to process
 *
 * If a particular routine has an optimized version,
 * then the corresponding define here will have
 * statements added in the form:
 *
 *	OPTIMATH_UNARY(dt,op,_core_<type>_<oper>_o)
 *
 * Where "<type>" is the datatype,
 * "<oper>" is the operand mnemonic, "dt" is the DCMF datatype, and "op"
 * is the DCMF operand.  Additionally, there must be a prototype
 * declaration for the optimized routine. Only one statement
 * may be present in the macro.
 *
 * The code in dcmf_bg_math.h that uses these will
 * be (note: no semicolon after macro):
 *
 * #define OPTIMATH_UNARY(dt,op,f) case 1: \
 *					f(dst, src, count); \
 *					break;
 *
 * inline void Core_<type>_<oper>(params...) {
 * 	switch(1) {
 *	OPTIMIZED_<type>_<oper>
 *	default:
 *		_core_<type>_<oper>(params...);
 *		break;
 *	}
 * }
 *
 * These defines will also be used in dcmf_bg_dat.c to build a table
 * of optimized routines, by datatype, operand, and number of inputs.
 * (Note, nsrc will always be at least 2) For unary routines the third
 * subscript will be either "0" (unoptimized) or "1" (optimized, if present).
 *
 * #define OPTIMATH_UNARY(dt,op,f)	[dt][op][1] = f,
 *
 * void *dcmf_pre_op_funcs[ndt][nop][2] = {
 * OPTIMIZED_<type>_<oper>
 * ...
 * [dt][op][0] = _core_<type>_<oper>,	// unoptimized
 * };
 *
 * This table is accessed by using the DCMF_PRE_OP_FUNCS(dt,op,n) macro
 * where "n" is a flag indicating whether optimized or unoptimized routine
 * is to be selected.
 * 
 * Similar tables/functions exist for POST and MARSHALL routines.
 *
 * Note, many of these routines do not relate to DCMF_Op operations and thus
 * cannot be selected by "op". These defines will work in a Core_<type>_<oper>
 * function but not in a table/selector function. Look for DCMF_UNDEFINED_OP.
 * At present, it seems none of the functions are actually used.
 */
#define OPTIMIZED_int8_pre_all(op)
#define OPTIMIZED_int8_post_all(op)
#define OPTIMIZED_int8_pre_min	\
	OPTIMATH_UNARY(DCMF_SIGNED_CHAR,DCMF_MIN,_core_int8_pre_min_o)
#define OPTIMIZED_int8_post_min	\
	OPTIMATH_UNARY(DCMF_SIGNED_CHAR,DCMF_MIN,_core_int8_post_min_o)
#define OPTIMIZED_int8_band_unmarshall
#define OPTIMIZED_int8_bor_unmarshall
#define OPTIMIZED_int8_bxor_unmarshall
#define OPTIMIZED_int8_land_unmarshall
#define OPTIMIZED_int8_lor_unmarshall
#define OPTIMIZED_int8_lxor_unmarshall
#define OPTIMIZED_int8_max_unmarshall	\
	OPTIMATH_UNARY(DCMF_SIGNED_CHAR,DCMF_MAX,_core_int8_max_unmarshall_o)
#define OPTIMIZED_int8_min_unmarshall	\
	OPTIMATH_UNARY(DCMF_SIGNED_CHAR,DCMF_MIN,_core_int8_min_unmarshall_o)
#define OPTIMIZED_int8_sum_unmarshall
#define OPTIMIZED_uint8_pre_all(op)
#define OPTIMIZED_uint8_post_all(op)
#define OPTIMIZED_uint8_pre_min	\
	OPTIMATH_UNARY(DCMF_UNSIGNED_CHAR,DCMF_MIN,_core_uint8_pre_min_o)
#define OPTIMIZED_uint8_post_min	\
	OPTIMATH_UNARY(DCMF_UNSIGNED_CHAR,DCMF_MIN,_core_uint8_post_min_o)
#define OPTIMIZED_uint8_band_unmarshall
#define OPTIMIZED_uint8_bor_unmarshall
#define OPTIMIZED_uint8_bxor_unmarshall
#define OPTIMIZED_uint8_land_unmarshall
#define OPTIMIZED_uint8_lor_unmarshall
#define OPTIMIZED_uint8_lxor_unmarshall
#define OPTIMIZED_uint8_max_unmarshall
#define OPTIMIZED_uint8_min_unmarshall	\
	OPTIMATH_UNARY(DCMF_UNSIGNED_CHAR,DCMF_MIN,_core_uint8_min_unmarshall_o)
#define OPTIMIZED_uint8_sum_unmarshall
#define OPTIMIZED_int16_pre_all(op)	\
	OPTIMATH_UNARY(DCMF_SIGNED_SHORT,op,_core_int16_pre_all_o)
#define OPTIMIZED_int16_post_all(op)	\
	OPTIMATH_UNARY(DCMF_SIGNED_SHORT,op,_core_int16_post_all_o)
#define OPTIMIZED_int16_pre_min		\
	OPTIMATH_UNARY(DCMF_SIGNED_SHORT,DCMF_MIN,_core_int16_pre_min_o)
#define OPTIMIZED_int16_post_min	\
	OPTIMATH_UNARY(DCMF_SIGNED_SHORT,DCMF_MIN,_core_int16_post_min_o)
#define OPTIMIZED_int16_int32_pre_maxloc
#define OPTIMIZED_int16_int32_post_maxloc
#define OPTIMIZED_int16_int32_pre_minloc
#define OPTIMIZED_int16_int32_post_minloc
#define OPTIMIZED_int16_band_unmarshall
#define OPTIMIZED_int16_bor_unmarshall
#define OPTIMIZED_int16_bxor_unmarshall
#define OPTIMIZED_int16_land_unmarshall
#define OPTIMIZED_int16_lor_unmarshall
#define OPTIMIZED_int16_lxor_unmarshall
#define OPTIMIZED_int16_max_unmarshall	\
	OPTIMATH_UNARY(DCMF_SIGNED_SHORT,DCMF_MAX,_core_int16_max_unmarshall_o)
#define OPTIMIZED_int16_int32_maxloc_unmarshall
#define OPTIMIZED_int16_min_unmarshall	\
	OPTIMATH_UNARY(DCMF_SIGNED_SHORT,DCMF_MIN,_core_int16_min_unmarshall_o)
#define OPTIMIZED_int16_int32_minloc_unmarshall
#define OPTIMIZED_int16_sum_unmarshall
#define OPTIMIZED_uint16_pre_all(op)
#define OPTIMIZED_uint16_post_all(op)
#define OPTIMIZED_uint16_pre_min	\
	OPTIMATH_UNARY(DCMF_UNSIGNED_SHORT,DCMF_MIN,_core_uint16_pre_min_o)
#define OPTIMIZED_uint16_post_min	\
	OPTIMATH_UNARY(DCMF_UNSIGNED_SHORT,DCMF_MIN,_core_uint16_post_min_o)
#define OPTIMIZED_uint16_band_unmarshall
#define OPTIMIZED_uint16_bor_unmarshall
#define OPTIMIZED_uint16_bxor_unmarshall
#define OPTIMIZED_uint16_land_unmarshall
#define OPTIMIZED_uint16_lor_unmarshall
#define OPTIMIZED_uint16_lxor_unmarshall
#define OPTIMIZED_uint16_max_unmarshall
#define OPTIMIZED_uint16_min_unmarshall	\
	OPTIMATH_UNARY(DCMF_UNSIGNED_SHORT,DCMF_MIN,_core_uint16_min_unmarshall_o)
#define OPTIMIZED_uint16_sum_unmarshall
#define OPTIMIZED_int32_pre_all(op)
#define OPTIMIZED_int32_post_all(op)
#define OPTIMIZED_int32_pre_min
#define OPTIMIZED_int32_post_min	\
	OPTIMATH_UNARY(DCMF_SIGNED_INT,DCMF_MIN,_core_int32_post_min_o)
#define OPTIMIZED_int32_int32_pre_maxloc
#define OPTIMIZED_int32_int32_post_maxloc
#define OPTIMIZED_int32_int32_pre_minloc
#define OPTIMIZED_int32_int32_post_minloc
#define OPTIMIZED_int32_band_unmarshall
#define OPTIMIZED_int32_bor_unmarshall
#define OPTIMIZED_int32_bxor_unmarshall
#define OPTIMIZED_int32_land_unmarshall
#define OPTIMIZED_int32_lor_unmarshall
#define OPTIMIZED_int32_lxor_unmarshall
#define OPTIMIZED_int32_max_unmarshall
#define OPTIMIZED_int32_int32_maxloc_unmarshall
#define OPTIMIZED_int32_min_unmarshall	\
	OPTIMATH_UNARY(DCMF_SIGNED_INT,DCMF_MIN,_core_int32_min_unmarshall_o)
#define OPTIMIZED_int32_int32_minloc_unmarshall
#define OPTIMIZED_int32_sum_unmarshall
#define OPTIMIZED_uint32_pre_all(op)
#define OPTIMIZED_uint32_post_all(op)
#define OPTIMIZED_uint32_pre_min	\
	OPTIMATH_UNARY(DCMF_UNSIGNED_INT,DCMF_MIN,_core_uint32_pre_min_o)
#define OPTIMIZED_uint32_post_min	\
	OPTIMATH_UNARY(DCMF_UNSIGNED_INT,DCMF_MIN,_core_uint32_post_min_o)
#define OPTIMIZED_uint32_band_unmarshall
#define OPTIMIZED_uint32_bor_unmarshall
#define OPTIMIZED_uint32_bxor_unmarshall
#define OPTIMIZED_uint32_land_unmarshall
#define OPTIMIZED_uint32_lor_unmarshall
#define OPTIMIZED_uint32_lxor_unmarshall
#define OPTIMIZED_uint32_max_unmarshall
#define OPTIMIZED_uint32_min_unmarshall	\
	OPTIMATH_UNARY(DCMF_UNSIGNED_INT,DCMF_MIN,_core_uint32_min_unmarshall_o)
#define OPTIMIZED_uint32_sum_unmarshall
#define OPTIMIZED_int64_pre_all(op)
#define OPTIMIZED_int64_post_all(op)
#define OPTIMIZED_int64_pre_min
#define OPTIMIZED_int64_post_min
#define OPTIMIZED_int64_band_unmarshall
#define OPTIMIZED_int64_bor_unmarshall
#define OPTIMIZED_int64_bxor_unmarshall
#define OPTIMIZED_int64_land_unmarshall
#define OPTIMIZED_int64_lor_unmarshall
#define OPTIMIZED_int64_lxor_unmarshall
#define OPTIMIZED_int64_max_unmarshall
#define OPTIMIZED_int64_min_unmarshall
#define OPTIMIZED_int64_sum_unmarshall
#define OPTIMIZED_uint64_pre_all(op)
#define OPTIMIZED_uint64_post_all(op)
#define OPTIMIZED_uint64_pre_min	\
	OPTIMATH_UNARY(DCMF_UNSIGNED_LONG_LONG,DCMF_MIN,_core_uint64_pre_min_o)
#define OPTIMIZED_uint64_post_min	\
	OPTIMATH_UNARY(DCMF_UNSIGNED_LONG_LONG,DCMF_MIN,_core_uint64_post_min_o)
#define OPTIMIZED_uint64_band_unmarshall
#define OPTIMIZED_uint64_bor_unmarshall
#define OPTIMIZED_uint64_bxor_unmarshall
#define OPTIMIZED_uint64_land_unmarshall
#define OPTIMIZED_uint64_lor_unmarshall
#define OPTIMIZED_uint64_lxor_unmarshall
#define OPTIMIZED_uint64_max_unmarshall
#define OPTIMIZED_uint64_min_unmarshall	\
	OPTIMATH_UNARY(DCMF_UNSIGNED_LONG_LONG,DCMF_MIN,_core_uint64_min_unmarshall_o)
#define OPTIMIZED_uint64_sum_unmarshall
#define OPTIMIZED_fp32_pre_max
#define OPTIMIZED_fp32_post_max
#define OPTIMIZED_fp32_int32_pre_maxloc
#define OPTIMIZED_fp32_int32_post_maxloc
#define OPTIMIZED_fp32_fp32_pre_maxloc
#define OPTIMIZED_fp32_fp32_post_maxloc
#define OPTIMIZED_fp32_pre_min
#define OPTIMIZED_fp32_post_min
#define OPTIMIZED_fp32_int32_pre_minloc
#define OPTIMIZED_fp32_int32_post_minloc
#define OPTIMIZED_fp32_fp32_pre_minloc
#define OPTIMIZED_fp32_fp32_post_minloc
#define OPTIMIZED_fp32_int32_maxloc_unmarshall
#define OPTIMIZED_fp32_fp32_maxloc_unmarshall
#define OPTIMIZED_fp32_min_unmarshall
#define OPTIMIZED_fp32_int32_minloc_unmarshall
#define OPTIMIZED_fp32_fp32_minloc_unmarshall
#define OPTIMIZED_fp32_max_unmarshall
#define OPTIMIZED_fp64_pre_sum
#define OPTIMIZED_fp64_post_sum
#define OPTIMIZED_fp64_pre_max
#define OPTIMIZED_fp64_post_max
#define OPTIMIZED_fp64_pre_all(op)
#define OPTIMIZED_fp64_post_all(op)
#define OPTIMIZED_fp64_int32_pre_maxloc
#define OPTIMIZED_fp64_int32_post_maxloc
#define OPTIMIZED_fp64_fp64_pre_maxloc
#define OPTIMIZED_fp64_fp64_post_maxloc
#define OPTIMIZED_fp64_pre_min
#define OPTIMIZED_fp64_post_min
#define OPTIMIZED_fp64_int32_pre_minloc
#define OPTIMIZED_fp64_int32_post_minloc
#define OPTIMIZED_fp64_fp64_pre_minloc
#define OPTIMIZED_fp64_fp64_post_minloc
#define OPTIMIZED_fp64_max_unmarshall
#define OPTIMIZED_fp64_int32_maxloc_unmarshall
#define OPTIMIZED_fp64_fp64_maxloc_unmarshall
#define OPTIMIZED_fp64_min_unmarshall
#define OPTIMIZED_fp64_int32_minloc_unmarshall
#define OPTIMIZED_fp64_fp64_minloc_unmarshall

#endif /* _dcmf_optibgmath_h_ */
