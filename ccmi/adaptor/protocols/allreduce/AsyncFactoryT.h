/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file ccmi/adaptor/protocols/allreduce/AsyncFactoryT.h
 * \brief  CCMI factory for async [all]reduce compositeT 
 *  
 * \todo AsyncFactoryT and FactoryT are very similar and could be combined. 
 * \todo should the mapping be a template parameter?  Mapping vs Mapping 
 */

#ifndef __ccmi_adaptor_allreduce_asyncfactoryt_h__
#define __ccmi_adaptor_allreduce_asyncfactoryt_h__

#include "protocols/allreduce/AsyncFactory.h"

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
      template <class CONNMGR, class COMPOSITE> class AsyncFactoryT : public CCMI::Adaptor::Allreduce::AsyncFactory
      {
      protected:
        CONNMGR     _sconnmgr;
//      static CCMI_Request_t *   cb_asyncReceiveHead(const CCMIQuad    * info,
//                                                    unsigned          count,
//                                                    unsigned          peer,
//                                                    unsigned          sndlen,
//                                                    unsigned          conn_id,
//                                                    void            * arg,
//                                                    unsigned        * rcvlen,
//                                                    char           ** rcvbuf,
//                                                    unsigned        * pipewidth,
//                                                    CCMI_Callback_t * cb_done)
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
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::~AsyncFactoryT() ALERT\n",(int)this,COMPOSITE::name));
        }
        ///
        /// \brief Constructor for allreduce factory implementations.
        ///
        inline AsyncFactoryT(CCMI::Mapping *mapping, 
                             CCMI::MultiSend::MulticastInterface *mf, 
                             CCMI_mapIdToGeometry cb_geometry,
                             ConfigFlags flags) :
        CCMI::Adaptor::Allreduce::AsyncFactory(mapping, mf, cb_geometry, flags), 
        _sconnmgr(mapping)
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::AsyncFactoryT() ALERT:\n",(int)this, COMPOSITE::name));
          TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::%s::AsyncFactoryT() mf<%#X>\n",(int)this, COMPOSITE::name,
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
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::AsyncFactoryT::generate() ALERT:\n",(int)this, COMPOSITE::name));
          TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::%s::AsyncFactoryT::generate() %#X, geometry %#X comm %#X\n",(int)this, COMPOSITE::name, 
                          sizeof(*this),(int) geometry, (int) geometry->comm()));

          //CCMI_assert(geometry->getAsyncAllreduceMode());
          CCMI_Executor_t *c_request = geometry->getAllreduceCompositeStorage();

          COMPOSITE *allreduce = 
          new (c_request, sizeof(CCMI_Executor_t))
          COMPOSITE(request,
                    (CCMI::Mapping *)_mapping, &_sconnmgr, cb_done,
                    consistency, _minterface, geometry,
                    srcbuf, dstbuf, 0, count, dtype, op,
                    _flags, this, 
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
        (Geometry                 * geometry,
         unsigned                   count,
         CCMI_Dt                    dtype,
         CCMI_Op                    op,
         unsigned                   iteration,
         int                        root = -1 )
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::AsyncFactoryT::generateAsync() ALERT:\n",(int)this, COMPOSITE::name));
          TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::%s::AsyncFactoryT::generateAsync() %#X,"
                          " geometry %#X comm %#X iteration %#X\n",(int)this, COMPOSITE::name, 
                          sizeof(*this),(int) geometry, (int) geometry->comm(), iteration));

          CCMI_Callback_t temp_cb_done = {CCMI::Adaptor::Allreduce::temp_done_callback, NULL};

          //CCMI_assert(geometry->getAsyncAllreduceMode());
          CCMI_Executor_t *c_request = geometry->getAllreduceCompositeStorage(iteration);

          COMPOSITE *allreduce = 
          new (c_request, sizeof(CCMI_Executor_t))
          COMPOSITE((CCMI_CollectiveRequest_t*)NULL, // restart will reset this
                    (CCMI::Mapping *)_mapping, &_sconnmgr, 
                    temp_cb_done, // bogus temporary cb, restart will reset it.
                    (CCMI_Consistency) CCMI_MATCH_CONSISTENCY, // restart may reset this
                    _minterface, 
                    geometry,
                    NULL, // restart will reset src buffer
                    NULL, // restart will reset dst buffer
                    0, count, dtype, op,
                    _flags, this, iteration,
                    root,
                    getOneColor(geometry)
                   );

          geometry->setAllreduceComposite(allreduce,iteration);      

          return allreduce;
        }

        // Template implementation should specialize this function if they want a color
        CCMI::Schedule::Color getOneColor(Geometry * geometry)
        {
          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::%s::AsyncFactoryT::getOneColor() NO_COLOR\n",(int)this, COMPOSITE::name));
          return CCMI::Schedule::NO_COLOR;
        }
        bool Analyze( Geometry * geometry )
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::AsyncFactoryT::Analyze() ALERT: %s\n",(int)this, COMPOSITE::name,
                       COMPOSITE::analyze(geometry)? "true":"false"));
          return COMPOSITE::analyze(geometry);
        }

        static inline void _compile_time_assert_ ()
        {
          // Compile time assert
          COMPILE_TIME_ASSERT(sizeof(COMPOSITE) <= sizeof(CCMI_Executor_t));
        }
      }; // class AsyncFactoryT
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
