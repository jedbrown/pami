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
      template <class T_ConnectionManager, class T_Composite, class T_Sysdep, class T_Mcast>
      class FactoryT : public CCMI::Adaptor::Allreduce::Factory<T_Sysdep, T_Mcast, T_ConnectionManager>
      {
      protected:
        T_ConnectionManager     _sconnmgr;

      public:
        virtual ~FactoryT()
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::~FactoryT() ALERT\n",(int)this,T_Composite::name));
        }
        ///
        /// \brief Constructor for allreduce factory implementations.
        ///
        inline FactoryT(T_Sysdep *mapping,
                        T_Mcast  *mof,
//                        CCMI::MultiSend::MulticombineInterface *mf,
                        xmi_mapidtogeometry_fn cb_geometry,
                        ConfigFlags flags) :
          CCMI::Adaptor::Allreduce::Factory<T_Sysdep, T_Mcast, T_ConnectionManager>(mapping,
                                                                                    mof,
//                                                    mf,
                                                                                    cb_geometry,
                                                                                    flags),
          _sconnmgr(mapping)
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::FactoryT() ALERT:\n",(int)this, T_Composite::name));
          TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::%s::FactoryT() mf<%#X>\n",(int)this, T_Composite::name,
                          (int) mof));
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
         xmi_consistency_t           consistency,
         XMI_GEOMETRY_CLASS                 * geometry,
         char                     * srcbuf,
         char                     * dstbuf,
         unsigned                   count,
         xmi_dt                    dtype,
         xmi_op                    op,
         int                        root = -1 )
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::FactoryT::generate() ALERT:\n",(int)this, T_Composite::name));
          TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::%s::FactoryT::generate() %#X, geometry %#X comm %#X\n",(int)this, T_Composite::name,
                          sizeof(*this),(int) geometry, (int) geometry->comm()));

          CCMI_Executor_t *c_request = (CCMI_Executor_t *)geometry->getAllreduceCompositeStorage();

          COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(T_Composite));
          T_Composite *allreduce = new (c_request)
          T_Composite(request,
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

        CCMI::Schedule::Color getOneColor(XMI_GEOMETRY_CLASS * geometry)
        {
          return CCMI::Schedule::NO_COLOR;
        }
        bool Analyze( XMI_GEOMETRY_CLASS * geometry )
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::FactoryT::Analyze() ALERT: %s\n",(int)this, T_Composite::name,
                      T_Composite::analyze(geometry)? "true":"false"));
          return T_Composite::analyze(geometry);
        }

        static inline void _compile_time_assert_ ()
        {
          // Compile time assert
          COMPILE_TIME_ASSERT(sizeof(T_Composite) <= sizeof(CCMI_Executor_t));
        }
      }; // class FactoryT
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
