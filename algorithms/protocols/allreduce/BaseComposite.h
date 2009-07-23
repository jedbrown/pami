/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/BaseComposite.h
 * \brief CCMI base composite adaptor for allreduce with factory and 
 * restart support
 */


#ifndef __ccmi_adaptor_allreduce_basecomposite_h__
#define __ccmi_adaptor_allreduce_basecomposite_h__

#include "algorithms/composite/Composite.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      class BaseComposite :  public CCMI::Executor::Composite
      {
      protected:

        ///
        /// \brief factory pointer to uniquely identify this suite of
        /// protocols
        ///
        CollectiveProtocolFactory               * _factory;

      public: 

        BaseComposite (CollectiveProtocolFactory *factory) : Composite (), _factory (factory)
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
        virtual unsigned restart   ( CM_CollectiveRequest_t  * request,
                                     CM_Callback_t           & cb_done,
                                     CCMI_Consistency            consistency,
                                     char                      * srcbuf,
                                     char                      * dstbuf,
                                     size_t                      count,
                                     CM_Dt                     dtype,
                                     CM_Op                     op,
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
