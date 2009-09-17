/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/FactoryT.h
 * \brief  CCMI factory for sync [all]reduce compositeT
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
      template <class CONNMGR, class COMPOSITE, class MAP> class FactoryT : public CCMI::Adaptor::Allreduce::Factory<MAP>
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
        inline FactoryT(MAP *mapping,
                        CCMI::MultiSend::OldMulticastInterface *mof,
                        CCMI::MultiSend::MulticombineInterface *mf,
                        CCMI_mapIdToGeometry cb_geometry,
                        ConfigFlags flags) :
        CCMI::Adaptor::Allreduce::Factory<MAP>(mapping, mof, mf, cb_geometry, flags),
        _sconnmgr(mapping)
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::FactoryT() ALERT:\n",(int)this, COMPOSITE::name));
          TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::%s::FactoryT() mf<%#X>\n",(int)this, COMPOSITE::name,
                          (int) mf));
          setConnectionManager(&_sconnmgr);
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
        (XMI_CollectiveRequest_t * request,
         XMI_Callback_t            cb_done,
         CCMI_Consistency           consistency,
         Geometry                 * geometry,
         char                     * srcbuf,
         char                     * dstbuf,
         unsigned                   count,
         XMI_Dt                    dtype,
         XMI_Op                    op,
         int                        root = -1 )
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::FactoryT::generate() ALERT:\n",(int)this, COMPOSITE::name));
          TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::%s::FactoryT::generate() %#X, geometry %#X comm %#X\n",(int)this, COMPOSITE::name,
                          sizeof(*this),(int) geometry, (int) geometry->comm()));

          CCMI_Executor_t *c_request = geometry->getAllreduceCompositeStorage();

          COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(COMPOSITE));
          COMPOSITE *allreduce = new (c_request)
          COMPOSITE(request,
                    this->_mapping, &this->_sconnmgr, cb_done,
                    consistency, this->_moldinterface, geometry,
                    srcbuf, dstbuf, 0, count, dtype, op,
                    this->_flags, this,
                    root,
                    getOneColor(geometry)
                    );

          geometry->setAllreduceComposite (allreduce);
          allreduce->startBarrier (consistency);

          return allreduce;
        }

        CCMI::Schedule::Color getOneColor(Geometry * geometry)
        {
          return CCMI::Schedule::NO_COLOR;
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
