/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/AsyncFactoryT.h
 * \brief  CCMI factory for async [all]reduce compositeT
 *
 * \todo AsyncFactoryT and FactoryT are very similar and could be combined.
 * \todo should the mapping be a template parameter?  CollectiveMapping vs CollectiveMapping
 */

#ifndef __algorithms_protocols_allreduce_AsyncFactoryT_h__
#define __algorithms_protocols_allreduce_AsyncFactoryT_h__

#include "algorithms/protocols/allreduce/AsyncFactory.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      /// class AsyncFactoryT
      ///
      /// \brief Factory template class for CompositeT
      ///
      /// This factory will generate a CompositeT [all]reduce.
      ///
      template <class T_ConnectionManager, class T_Composite, class T_Sysdep, class T_Mcast>
      class AsyncFactoryT : public CCMI::Adaptor::Allreduce::AsyncFactory<T_Sysdep, T_Mcast, T_ConnectionManager>
      {
      protected:
        T_ConnectionManager     _sconnmgr;
//      static PAMI_Request_t *   cb_asyncReceiveHead(const PAMIQuad    * info,
//                                                    unsigned          count,
//                                                    unsigned          peer,
//                                                    unsigned          sndlen,
//                                                    unsigned          conn_id,
//                                                    void            * arg,
//                                                    unsigned        * rcvlen,
//                                                    char           ** rcvbuf,
//                                                    unsigned        * pipewidth,
//                                                    PAMI_Callback_t * cb_done)
//      {
//        return CCMI::Adaptor::Allreduce::AsyncFactory::cb_receiveHead(info,
//                                                                          count,
//                                                                          peer,
//                                                                          sndlen,
//                                                                          conn_id,
//                                                                          arg,
//                                                                          rcvlen,
//                                                                          rcvbuf,
//                                                                          pipewidth,
//                                                                          cb_done);
//      }

      public:
        virtual ~AsyncFactoryT()
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::%s::~AsyncFactoryT() ALERT\n",this,T_Composite::name));
        }
        ///
        /// \brief Constructor for allreduce factory implementations.
        ///
        inline AsyncFactoryT(T_Sysdep *mapping,
                             T_Mcast *mf,
                             pami_mapidtogeometry_fn cb_geometry,
                             ConfigFlags flags) :
          CCMI::Adaptor::Allreduce::AsyncFactory<T_Sysdep, T_Mcast, T_ConnectionManager>(mapping, mf, cb_geometry, flags),
        _sconnmgr(mapping)
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::%s::AsyncFactoryT() ALERT:\n",this, T_Composite::name));
          TRACE_ADAPTOR ((stderr, "<%p>Allreduce::%s::AsyncFactoryT() mf<%#X>\n",this, T_Composite::name,
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
          TRACE_ALERT((stderr,"<%p>Allreduce::%s::AsyncFactoryT::generate() ALERT:\n",this, T_Composite::name));
          TRACE_ADAPTOR ((stderr, "<%p>Allreduce::%s::AsyncFactoryT::generate() %#X, geometry %#X comm %#X\n",this, T_Composite::name,
                          sizeof(*this),(int) geometry, (int) geometry->comm()));

          //CCMI_assert(geometry->getAsyncAllreduceMode());
          CCMI_Executor_t *c_request = geometry->getAllreduceCompositeStorage();

          COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(T_Composite));
          T_Composite *allreduce = new (c_request)
          T_Composite(request,
                    this->_mapping, &this->_sconnmgr, cb_done,
                    consistency, this->_minterface, geometry,
                    srcbuf, dstbuf, 0, count, dtype, op,
                    this->_flags, this,
                    geometry->getAllreduceIteration(),
                    root,
                    getOneColor(geometry)
                   );

          geometry->setAllreduceComposite (allreduce);
          allreduce->start();

          return allreduce;
        }

        ///
        /// \brief Generate a non-blocking allreduce message.
        ///
        virtual CCMI::Executor::Composite * generateAsync
        (PAMI_GEOMETRY_CLASS                 * geometry,
         unsigned                   count,
         pami_dt                    dtype,
         pami_op                    op,
         unsigned                   iteration,
         int                        root = -1 )
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::%s::AsyncFactoryT::generateAsync() ALERT:\n",this, T_Composite::name));
          TRACE_ADAPTOR ((stderr, "<%p>Allreduce::%s::AsyncFactoryT::generateAsync() %#X,"
                          " geometry %#X comm %#X iteration %#X\n",this, T_Composite::name,
                          sizeof(*this),(int) geometry, (int) geometry->comm(), iteration));

          PAMI_Callback_t temp_cb_done = {CCMI::Adaptor::Allreduce::temp_done_callback, NULL};

          //CCMI_assert(geometry->getAsyncAllreduceMode());
          CCMI_Executor_t *c_request = geometry->getAllreduceCompositeStorage(iteration);

          COMPILE_TIME_ASSERT(sizeof(CCMI_Executor_t) >= sizeof(T_Composite));
          T_Composite *allreduce = new (c_request)
          T_Composite((PAMI_CollectiveRequest_t*)NULL, // restart will reset this
                    this->_mapping, &this->_sconnmgr,
                    temp_cb_done, // bogus temporary cb, restart will reset it.
                    (pami_consistency_t) PAMI_MATCH_CONSISTENCY, // restart may reset this
                    this->_minterface,
                    geometry,
                    NULL, // restart will reset src buffer
                    NULL, // restart will reset dst buffer
                    0, count, dtype, op,
                    this->_flags, this, iteration,
                    root,
                    getOneColor(geometry)
                   );

          geometry->setAllreduceComposite(allreduce,iteration);

          return allreduce;
        }

        // Template implementation should specialize this function if they want a color
        CCMI::Schedule::Color getOneColor(PAMI_GEOMETRY_CLASS * geometry)
        {
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::%s::AsyncFactoryT::getOneColor() NO_COLOR\n",this, T_Composite::name));
          return CCMI::Schedule::NO_COLOR;
        }
        bool Analyze( PAMI_GEOMETRY_CLASS * geometry )
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::%s::AsyncFactoryT::Analyze() ALERT: %s\n",this, T_Composite::name,
                       T_Composite::analyze(geometry)? "true":"false"));
          return T_Composite::analyze(geometry);
        }

        static inline void _compile_time_assert_ ()
        {
          // Compile time assert
          COMPILE_TIME_ASSERT(sizeof(T_Composite) <= sizeof(CCMI_Executor_t));
        }
      }; // class AsyncFactoryT
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
