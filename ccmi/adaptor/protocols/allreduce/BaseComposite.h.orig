/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file collectives/bgp/protocols/allreduce/BaseComposite.h
 * \brief CCMI base composite adaptor for allreduce with factory and 
 * restart support
 */


#ifndef __ccmi_collectives_allreduce_basecomposite_h__
#define __ccmi_collectives_allreduce_basecomposite_h__

#include "../../../interfaces/Composite.h"
#include "../ProtocolFactory.h"

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
        ProtocolFactory               * _factory;

      public: 

        BaseComposite (ProtocolFactory *factory) : Composite (), _factory (factory)
        {
        } 

        ProtocolFactory * getFactory ()
        {
          return _factory;
        }

        ///
        /// \brief At this level we only support single color
        /// collectives
        ///
        virtual unsigned restart   ( CCMI_CollectiveRequest_t  * request,
                                     CCMI_Callback_t           & cb_done,
                                     CCMI_Consistency            consistency,
                                     char                      * srcbuf,
                                     char                      * dstbuf,
                                     unsigned                    count,
                                     CCMI_Dt                     dtype,
                                     CCMI_Op                     op,
                                     int                         root=-1) = 0;  
        virtual ~BaseComposite()
        {
        }

        void operator delete (void *p)
        {
          CCMI_assert (0);
        }
      };

    };
  };
}; // namespace CCMI::Adaptor::Allreduce

#endif
