/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/TreeFactoryT.h
 * \brief ???
 */

#ifndef __ccmi_adaptor_allreduce_tree_factoryt_h__
#define __ccmi_adaptor_allreduce_tree_factoryt_h__

#include "./Factory.h"
#include "./TreeCompositeT.h"
#include "./tree/SmpTreeAllreduce.h"
#include "./tree/VnDualShortTreeAllreduce.h"


namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      namespace Tree
      {

        template <class CONNMGR, class COMPOSITE> class FactoryT : public CCMI::Adaptor::Allreduce::Factory<CCMI::TorusCollectiveMapping>
        {
        protected:
          CONNMGR     _sconnmgr;

        public:
          static const unsigned short_thresh=512;
          virtual ~FactoryT()
          {
            TRACE_ALERT((stderr,"<%#.8X>Allreduce::Tree::%s::~FactoryT() ALERT\n",(int)this,COMPOSITE::name));
          }

          FactoryT(CCMI::TorusCollectiveMapping                           * mapping,
                  CCMI::MultiSend::OldMulticastInterface          * mof,
                  CCMI::MultiSend::MulticombineInterface          * mf,
                  CCMI_mapIdToGeometry                           cb_geometry,
                  ConfigFlags                                    flags ) :
          CCMI::Adaptor::Allreduce::Factory<CCMI::TorusCollectiveMapping>(mapping, mof, mf, cb_geometry, flags),
          _sconnmgr(mapping)
          {
            TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::Tree::%s::FactoryT() \n",
                            (int)this, COMPOSITE::name));

            setConnectionManager(&_sconnmgr);
            if(mapping->GetDimLength(CCMI_T_DIM) > 1) // dual/vnm
               CCMI::Adaptor::Allreduce::Tree::VnDualShortTreeAllreduce::init(mapping);
          }


          void operator delete(void * p)
          {
            CCMI_abort();
          }

          bool Analyze(Geometry * geometry)
          {
            return( geometry->isGlobalContext() && geometry->isTree());
          }

          virtual CCMI::Executor::Composite * generate(CM_CollectiveRequest_t * request,
                                                       CM_Callback_t            cb_done,
                                                       CCMI_Consistency           consistency,
                                                       Geometry                 * geometry,
                                                       char                     * srcbuf,
                                                       char                     * dstbuf,
                                                       unsigned                   count,
                                                       CM_Dt                    dtype,
                                                       CM_Op                    op,
                                                       int                        root = -1 )
          {
            TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Tree::%s::FactoryT::generate(), comm %d\n",
                            (int)this, COMPOSITE::name,geometry->comm()));

            int rc = -1;
            rc = CCMI::Adaptor::Allreduce::Tree::checkOp(dtype,op);

            if(rc == -1)
            {
              TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Tree::%s::FactoryT::generate():unsupported op %d, type %d!\n",
                             (int)this, COMPOSITE::name, op, dtype));
              return NULL;
            }

            CCMI_Executor_t *arequest =geometry->getAllreduceCompositeStorage();

            if((_mapping)->GetDimLength(CCMI_T_DIM) == 1) // smp mode
            {
              TRACE_ALERT((stderr,"<%#.8X>Allreduce::Tree::%s::FactoryT::generate() SmpTreeAllreduce ALERT:\n",(int)this, COMPOSITE::name));
              COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(CCMI::Adaptor::Allreduce::Tree::SmpTreeAllreduce));
              CCMI::Adaptor::Allreduce::BaseComposite *allreduce = new (arequest)
              CCMI::Adaptor::Allreduce::Tree::SmpTreeAllreduce ( _minterface, this,  _mapping->rank() );

              allreduce->restart ( request,
                                   cb_done,
                                   consistency,
                                   srcbuf, dstbuf, count,
                                   dtype, op, root );

              geometry->setAllreduceComposite (allreduce);
              return allreduce;
            }
            else if(count <= short_thresh)// && (root == -1)) // short allreduce only
            {
              TRACE_ALERT((stderr,"<%#.8X>Allreduce::Tree::%s::FactoryT::generate() VnDualShortTreeAllreduce ALERT:\n",(int)this, COMPOSITE::name));
              COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(CCMI::Adaptor::Allreduce::Tree::VnDualShortTreeAllreduce));
              CCMI::Adaptor::Allreduce::Tree::VnDualShortTreeAllreduce *allreduce = new (arequest)
                                                          CCMI::Adaptor::Allreduce::Tree::VnDualShortTreeAllreduce ( _minterface, this, _mapping );

              unsigned status =  allreduce->restart( request,
                                  cb_done,
                                  consistency,
                                  srcbuf,
                                  dstbuf,
                                  count,
                                  dtype,
                                  op, 
                                  root );

              if(status == CM_SUCCESS)
              {
                geometry->setAllreduceComposite (allreduce);
                return allreduce;
              }
              else; // Short protocol failed, fall through and try COMPOSITE
            }
            TRACE_ALERT((stderr,"<%#.8X>Allreduce::Tree::%s::FactoryT::generate() ALERT:\n",(int)this, COMPOSITE::name));
            COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(COMPOSITE));
            COMPOSITE *allreduce = new (arequest)
              COMPOSITE(request,
                        _mapping,
                        &_sconnmgr,
                        cb_done,
                        consistency,
                        _moldinterface,
                        geometry,
                        srcbuf,
                        dstbuf,
                        count,
                        dtype,
                        op,
                        _flags,
                        this,
                        root);
            geometry->setAllreduceComposite((Composite *)allreduce);
            allreduce->internal_restart(consistency, cb_done);
            return allreduce;
          }
        private:
          static inline void _compile_time_assert_ ()
          {
            // Compile time assert
            COMPILE_TIME_ASSERT(sizeof(COMPOSITE) <= sizeof(CCMI_Executor_t));
          }

        }; // class Factory
      };
    };
  };
};  //namespace CCMI::Adaptor::Allreduce::Tree


#endif
