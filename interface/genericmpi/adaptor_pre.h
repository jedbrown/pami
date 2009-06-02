/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file adaptor/generic/adaptor_pre.h
 * \brief ???
 */


#ifndef   __adaptor_pre_h__
#define   __adaptor_pre_h__

#include "ccmi_collectives.h"

#define ThreadID() 0

    /// The pipeline width must be a multiple of 240 (DMA) and 256 (Tree)
    /// 240 = 15 * 16
    /// 256 =      16 * 16
    /// So, width = 15 * 16 * 16 * x = 3840 * x
    /// Currently x = 4 for a width of 15360.  Adjust as desired.
    const unsigned MIN_PIPELINE_WIDTH = 3840;

    /// FP collectives on Blue Gene/L and Blue Gene/P use a double
    /// pass scheme that processes exponents and mantissas. The
    /// minimum transfer unit for this scheme is 1008 bytes (126
    /// doubles). The LCM of 240 and 1008 is 5040.
    const unsigned MIN_PIPELINE_WIDTH_SUM2P = 5040;

    /// On a torus we can use a smaller pipeline width that can allow
    /// better performance for shorter messages. We use 480 bytes
    /// which is 2 packets 
    const unsigned MIN_PIPELINE_WIDTH_TORUS = 480;

     /// This is the default allreduce min torus pipeline width, set to 1920
     const unsigned ALLREDUCE_MIN_PIPELINE_WIDTH_TORUS = 1920;    

#define __ccmi_quad_defined__
#define __ccmi_error_defined__
#define __ccmi_callback_defined__
#define __ccmi_consistency_defined__
#define __ccmi_op_defined__
#define __ccmi_dt_defined__
#define __ccmi_subtask_defined__
#define __ccmi_recvasynccallback_defined__

#endif
