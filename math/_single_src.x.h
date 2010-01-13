/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/_single_src.x.h
 * \brief ???
 */
// X-Macro for single source functions. i.e., b=op(a)

#ifdef OP
#ifdef TYPE

register int n=0;
#ifndef NOT_UNROLLED_LOOP
register TYPE rbuffer0, rbuffer1, rbuffer2, rbuffer3;
register TYPE buf0, buf1, buf2, buf3;
for(; n<count-3; n+=4)
  {
    buf0 = src[n+0];
    buf1 = src[n+1];
    buf2 = src[n+2];
    buf3 = src[n+3];

    rbuffer0 = OP(buf0);
    rbuffer1 = OP(buf1);
    rbuffer2 = OP(buf2);
    rbuffer3 = OP(buf3);

    dst[n+0] = rbuffer0;
    dst[n+1] = rbuffer1;
    dst[n+2] = rbuffer2;
    dst[n+3] = rbuffer3;
  }
#endif
for(; n<count; n++)
  {
    dst[n] = OP(src[n]);
  }

#endif
#endif
