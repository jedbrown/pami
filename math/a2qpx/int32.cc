/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/int32.cc
 * \brief Default C math routines for 32 bit signed integer operations.
 */

#include "math_coremath.h"
#include "internal.h"


void _pami_core_int32_band(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_int32_bor(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)|(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_int32_bxor(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)^(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_int32_land(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)&&(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_int32_lor(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)||(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_int32_lxor(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)&&(!b))||((!a)&&(b)))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_int32_max(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(a):(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_int32_min(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) (((a)>(b))?(b):(a))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_int32_prod(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)*(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_int32_sum(int32_t *dst, const int32_t **srcs, int nsrc, int count) {
#define OP(a,b) ((a)+(b))

#define TYPE int32_t
#include "_N_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_int32_int32_maxloc(int32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count) {
        register int n = 0, m, o;
        for (n = 0; n < count; n++) {
                m = 0;  // assume src0 > src1
                for (o = 1; o < nsrc; ++o) {
                        if (srcs[m][n].a < srcs[o][n].a ||
                            (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
                                m = o;
                        }
                }
                dst[n].a = srcs[m][n].a;
                dst[n].b = srcs[m][n].b;
        }
}

void _pami_core_int32_int32_minloc(int32_int32_t *dst, const int32_int32_t **srcs, int nsrc, int count) {
        register int n = 0, m, o;
        for (n = 0; n < count; n++) {
                m = 0;  // assume src0 < src1
                for (o = 1; o < nsrc; ++o) {
                        if (srcs[m][n].a > srcs[o][n].a ||
                            (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
                                m = o;
                        }
                }
                dst[n].a = srcs[m][n].a;
                dst[n].b = srcs[m][n].b;
        }
}

void _pami_core_inti64_int32_maxloc(int64_int32_t *dst, const int64_int32_t **srcs, int nsrc, int count) {
        register int n = 0, m, o;
        for (n = 0; n < count; n++) {
                m = 0;  // assume src0 > src1
                for (o = 1; o < nsrc; ++o) {
                        if (srcs[m][n].a < srcs[o][n].a ||
                            (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
                                m = o;
                        }
                }
                dst[n].a = srcs[m][n].a;
                dst[n].b = srcs[m][n].b;
        }
}

void _pami_core_int64_int32_minloc(int64_int32_t *dst, const int64_int32_t **srcs, int nsrc, int count) {
        register int n = 0, m, o;
        for (n = 0; n < count; n++) {
                m = 0;  // assume src0 < src1
                for (o = 1; o < nsrc; ++o) {
                        if (srcs[m][n].a > srcs[o][n].a ||
                            (srcs[m][n].a == srcs[o][n].a && srcs[m][n].b > srcs[o][n].b)) {
                                m = o;
                        }
                }
                dst[n].a = srcs[m][n].a;
                dst[n].b = srcs[m][n].b;
        }
}


