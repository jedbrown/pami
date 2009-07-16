/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/CollectiveProtocolFactory.h
 * \brief ???
 */

#ifndef  __ccmi_adaptor_protocol_factory__
#define  __ccmi_adaptor_protocol_factory__

#include "interface/Geometry.h"


namespace CCMI
{
  namespace Adaptor
  {
    class CollectiveProtocolFactory
    {
    public:
      CollectiveProtocolFactory ()
      {
      }
      
      virtual ~CollectiveProtocolFactory ()
      {
      }
      
      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }
      
      /// \brief All protocols determine if a given geometry is adequate
      virtual bool Analyze(CCMI::Adaptor::Geometry *grequest) = 0;

    };
  };
};

#endif
