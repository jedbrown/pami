/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/CollectiveProtocolFactory.h
 * \brief ???
 */

#ifndef __algorithms_protocols_CollectiveProtocolFactory_h__
#define __algorithms_protocols_CollectiveProtocolFactory_h__

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
      virtual bool Analyze(XMI_GEOMETRY_CLASS *grequest) = 0;

    };
  };
};

#endif
