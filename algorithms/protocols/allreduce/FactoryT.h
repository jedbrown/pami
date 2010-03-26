/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/FactoryT.h
 * \brief  CCMI factory for sync [all]reduce compositeT
  */

#ifndef __algorithms_protocols_allreduce_FactoryT_h__
#define __algorithms_protocols_allreduce_FactoryT_h__

#include "algorithms/protocols/allreduce/Factory.h"

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
      typedef void      (*MetaDataFn)   (pami_metadata_t *m);
      template <class T_ConnectionManager, class T_Composite, class T_Sysdep, class T_Mcast, MetaDataFn get_metadata>
      class FactoryT : public CCMI::Adaptor::Allreduce::Factory<T_Sysdep, T_Mcast, T_ConnectionManager>
      {
      protected:
        T_ConnectionManager     _sconnmgr;

      public:
        virtual ~FactoryT()
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::%s::~FactoryT() ALERT\n",this,T_Composite::name));
        }
        ///
        /// \brief Constructor for allreduce factory implementations.
        ///
        inline FactoryT(T_Sysdep *mapping,
                        T_Mcast  *mof,
//                        CCMI::MultiSend::MulticombineInterface *mf,
                        pami_mapidtogeometry_fn cb_geometry,
                        ConfigFlags flags) :
          CCMI::Adaptor::Allreduce::Factory<T_Sysdep, T_Mcast, T_ConnectionManager>(mapping,
                                                                                    mof,
//                                                    mf,
                                                                                    cb_geometry,
                                                                                    flags),
          _sconnmgr(mapping)
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::%s::FactoryT() ALERT:\n",this, T_Composite::name));
          TRACE_ADAPTOR ((stderr, "<%p>Allreduce::%s::FactoryT() mf<%#X>\n",this, T_Composite::name,
                          (int) mof));
          setConnectionManager(&_sconnmgr);
        }

        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
          CCMI_abort();
        }


        class collObj
         {
        public:
          collObj(pami_xfer_t *xfer):
            _xfer(*xfer),
            _rsize(sizeof(_req)),
            _user_done_fn(xfer->cb_done),
            _user_cookie(xfer->cookie)
            {
              _xfer.cb_done = alloc_done_fn;
              _xfer.cookie  = this;
            }
          PAMI_CollectiveRequest_t      _req[1];
          pami_xfer_t                   _xfer;
          int                          _rsize;
          pami_event_function           _user_done_fn;
          void                       * _user_cookie;
        };

        static void alloc_done_fn( pami_context_t   context,
                                   void          * cookie,
                                   pami_result_t    result )
          {
            collObj *cObj = (collObj*)cookie;
            cObj->_user_done_fn(context,cObj->_user_cookie,result);
            free(cObj);
          }

        virtual Executor::Composite * generate(pami_geometry_t              geometry,
                                               void                      * cmd)

          {
            collObj *obj = (collObj*)malloc(sizeof(*obj));
            new(obj) collObj((pami_xfer_t*)cmd);
            PAMI_Callback_t cb_done;
            cb_done.function   = obj->_xfer.cb_done;
            cb_done.clientdata = obj->_xfer.cookie;
            return this->generate(&obj->_req[0],
                                  cb_done,
                                  PAMI_MATCH_CONSISTENCY,
                                  (PAMI_GEOMETRY_CLASS *)geometry,
                                  obj->_xfer.cmd.xfer_allreduce.sndbuf,
                                  obj->_xfer.cmd.xfer_allreduce.rcvbuf,
                                  obj->_xfer.cmd.xfer_allreduce.stypecount,
                                  obj->_xfer.cmd.xfer_allreduce.dt,
                                  obj->_xfer.cmd.xfer_allreduce.op);
          }


        virtual void metadata(pami_metadata_t *mdata)
          {
            get_metadata(mdata);
          }

        ///
        /// \brief Generate a non-blocking allreduce message.
        ///
        virtual CCMI::Executor::Composite * generate
        (PAMI_CollectiveRequest_t * request,
         PAMI_Callback_t            cb_done,
         pami_consistency_t           consistency,
         PAMI_GEOMETRY_CLASS                 * geometry,
         char                     * srcbuf,
         char                     * dstbuf,
         unsigned                   count,
         pami_dt                    dtype,
         pami_op                    op,
         int                        root = -1 )
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::%s::FactoryT::generate() ALERT:\n",this, T_Composite::name));
          TRACE_ADAPTOR ((stderr, "<%p>Allreduce::%s::FactoryT::generate() %#X, geometry %#X comm %#X\n",this, T_Composite::name,
                          sizeof(*this),(int) geometry, (int) geometry->comm()));

          T_Composite *arcomposite = (T_Composite*)geometry->getAllreduceComposite();
          if(arcomposite != NULL && arcomposite->getFactory() == this)
              {
                pami_result_t status = (pami_result_t)
                  arcomposite->restart(request,
                                       cb_done,
                                       PAMI_MATCH_CONSISTENCY,
                                       srcbuf,
                                       dstbuf,
                                       count,
                                       dtype,
                                       op);
                if(status == PAMI_SUCCESS) geometry->setAllreduceComposite(arcomposite);
                return NULL;
              }
          else
              {
          CCMI_Executor_t *c_request = (CCMI_Executor_t *)geometry->getAllreduceCompositeStorage();
          COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(T_Composite));
                arcomposite = new (c_request)
          T_Composite(request,
                    this->_mapping, &this->_sconnmgr, cb_done,
                    consistency, this->_moldinterface, geometry,
                    srcbuf, dstbuf, 0, count, dtype, op,
                    this->_flags, this,
                    root,
                    getOneColor(geometry)
                    );

                geometry->setAllreduceComposite (arcomposite);
                arcomposite->startBarrier (consistency);
              }
          return NULL;
        }

        CCMI::Schedule::Color getOneColor(PAMI_GEOMETRY_CLASS * geometry)
        {
          return CCMI::Schedule::NO_COLOR;
        }
        bool Analyze( PAMI_GEOMETRY_CLASS * geometry )
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::%s::FactoryT::Analyze() ALERT: %s\n",this, T_Composite::name,
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
