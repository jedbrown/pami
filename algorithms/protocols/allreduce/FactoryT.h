/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file ccmi/adaptor/protocols/allreduce/FactoryT.h
 * \brief  CCMI factory for sync [all]reduce compositeT 
 * \todo should the mapping be a template parameter?  Mapping vs Mapping 
  */

#ifndef __ccmi_adaptor_allreduce_factoryt_h__
#define __ccmi_adaptor_allreduce_factoryt_h__

#include "./Factory.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      /// class FactoryT
      ///
      /// \brief Factory template class for CompositeT
      ///
      /// This factory will generate a CompositeT [all]reduce.
      /// 
      template <class CONNMGR, class COMPOSITE> class FactoryT : public CCMI::Adaptor::Allreduce::Factory
      {
      protected:
        CONNMGR     _sconnmgr;

      public:
        virtual ~FactoryT()
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::~FactoryT() ALERT\n",(int)this,COMPOSITE::name));
        }
        ///
        /// \brief Constructor for allreduce factory implementations.
        ///
        inline FactoryT(CCMI::Mapping *mapping, 
                        CCMI::MultiSend::MulticastInterface *mf, 
                        CCMI_mapIdToGeometry cb_geometry,
                        ConfigFlags flags) :
        CCMI::Adaptor::Allreduce::Factory(mapping, mf, cb_geometry, flags), 
        _sconnmgr(mapping)
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::FactoryT() ALERT:\n",(int)this, COMPOSITE::name));
          TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::%s::FactoryT() mf<%#X>\n",(int)this, COMPOSITE::name,
                          (int) mf));
          setConnectionManager(&_sconnmgr);
          mf->setCallback (cb_receiveHead, this);
        }

        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
          CCMI_abort();
        }

        ///
        /// \brief Generate a non-blocking allreduce message.
        ///
        virtual CCMI::Executor::Composite * generate
        (CCMI_CollectiveRequest_t * request,
         CCMI_Callback_t            cb_done,
         CCMI_Consistency           consistency,
         Geometry                 * geometry,
         char                     * srcbuf,
         char                     * dstbuf,
         unsigned                   count,
         CCMI_Dt                    dtype,
         CCMI_Op                    op,
         int                        root = -1 )
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::FactoryT::generate() ALERT:\n",(int)this, COMPOSITE::name));
          TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::%s::FactoryT::generate() %#X, geometry %#X comm %#X\n",(int)this, COMPOSITE::name, 
                          sizeof(*this),(int) geometry, (int) geometry->comm()));

          CCMI_Executor_t *c_request = geometry->getAllreduceCompositeStorage();

          COMPOSITE *allreduce = 
          new (c_request, sizeof(CCMI_Executor_t))
          COMPOSITE(request,
                    (CCMI::Mapping *)_mapping, &_sconnmgr, cb_done,
                    consistency, _minterface, geometry,
                    srcbuf, dstbuf, 0, count, dtype, op,
                    _flags, this,
                    root
                    );

          geometry->setAllreduceComposite (allreduce);      
          allreduce->startBarrier (consistency);

          return allreduce;
        }

        bool Analyze( Geometry * geometry )
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::FactoryT::Analyze() ALERT: %s\n",(int)this, COMPOSITE::name,
                      COMPOSITE::analyze(geometry)? "true":"false"));
          return COMPOSITE::analyze(geometry);
        }

        static inline void _compile_time_assert_ ()
        {
          // Compile time assert
          COMPILE_TIME_ASSERT(sizeof(COMPOSITE) <= sizeof(CCMI_Executor_t));
        }
      }; // class FactoryT
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
