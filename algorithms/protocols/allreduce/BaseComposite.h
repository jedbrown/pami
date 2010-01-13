/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/BaseComposite.h
 * \brief CCMI base composite adaptor for allreduce with factory and
 * restart support
 */


#ifndef __algorithms_protocols_allreduce_BaseComposite_h__
#define __algorithms_protocols_allreduce_BaseComposite_h__

#include "algorithms/composite/Composite.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      class BaseComposite :  public CCMI::Executor::OldComposite
      {
      protected:

        ///
        /// \brief factory pointer to uniquely identify this suite of
        /// protocols
        ///
        CollectiveProtocolFactory               * _factory;

      public:

        BaseComposite (CollectiveProtocolFactory *factory) : CCMI::Executor::OldComposite (), _factory (factory)
        {
        }

        CollectiveProtocolFactory * getFactory ()
        {
          return _factory;
        }

        ///
        /// \brief At this level we only support single color
        /// collectives
        ///
        virtual unsigned restart   ( XMI_CollectiveRequest_t  * request,
                                     XMI_Callback_t           & cb_done,
                                     xmi_consistency_t          consistency,
                                     char                      * srcbuf,
                                     char                      * dstbuf,
                                     size_t                      count,
                                     xmi_dt                      dtype,
                                     xmi_op                      op,
                                     size_t                      root = (size_t)-1) = 0;
        virtual ~BaseComposite()
        {
        }

        void operator delete (void *p)
        {
          CCMI_abort();
        }
      };

    };
  };
}; // namespace CCMI::Adaptor::Allreduce

#endif
