/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/Metadata.h
 * \brief ???
 */

#ifndef __algorithms_geometry_Metadata_h__
#define __algorithms_geometry_Metadata_h__

#include <stdint.h>

namespace PAMI
{
  namespace Geometry
  {
    
    // This is a simple wrapper class around the C style pami metadata struct
    // This is to guarantee that some value has been set for the metadata
    // fields for every collective
    // This sets the fields to "always works" and "not hw accelerated"
    // Any code that uses this must set the appropriate fields
    class Metadata: public pami_metadata_t
    {
    public:
      inline Metadata(const char *in_name)
        {
          // Copy the name field into the metadata struct
          strncpy(this->name, in_name, sizeof(this->name));
          
          // Misc info
          this->version                           = 1;
          this->check_fn                          = NULL;
          this->range_lo                          = 0;
          this->range_hi                          = 2>>31;

          // Correctness Check
          this->check_correct.bitmask_correct     = 0;
          this->check_correct.values.mustquery    = 0;
          this->check_correct.values.nonlocal     = 0;
          this->check_correct.values.sendminalign = 1;
          this->check_correct.values.recvminalign = 1;
          this->check_correct.values.alldt        = 1;
          this->check_correct.values.allop        = 1;
          this->check_correct.values.contigsflags = 1;
          this->check_correct.values.contigrflags = 1;
          this->check_correct.values.continsflags = 0;
          this->check_correct.values.continrflags = 0;

          // Performance Checks
          this->check_perf.bitmask_perf           = 0;
          this->check_perf.values.hw_accel        = 0;
          this->range_lo_perf                     = 0;
          this->range_hi_perf                     = 2>>31;
        }
    };
  };
};




#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
