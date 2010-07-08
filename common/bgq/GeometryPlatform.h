/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file common/bgq/GeometryPlatform.h
/// BG/Q platform extensions to Geometry class

#ifndef __common_bgq_GeometryPlatform_h__
#define __common_bgq_GeometryPlatform_h__

/// \brief Extensions to enum keys_t
#define PAMI_GKEY_PLATEXTENSIONS       \
       PAMI_GKEY_CLASSROUTE,

/// \brief The value used to indicate no classroute, diff from not yet optimized
#define PAMI_CR_GKEY_FAIL	((void *)0xbadc0ffee0ddf00d)

#endif // __common_bgq_GeometryPlatform_h__
