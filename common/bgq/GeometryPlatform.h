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
/// - Rectangle class route id, independent of any MU class route id's
/// - Phase 1 done flag is = '1' the first time phase 1 was done.  
///   This is necessary for repeated optimize/de-optimize/optimize phases.
#define PAMI_CKEY_PLATEXTENSIONS \
CKEY_RECTANGLE_CLASSROUTEID,\
CKEY_PHASE_1_DONE,

/// \brief The value used to indicate no classroute, diff from not yet optimized
#define PAMI_CR_CKEY_FAIL	((void *)0xbadc0ffee0ddf00d)

#endif // __common_bgq_GeometryPlatform_h__
