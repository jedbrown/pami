/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/barrier/BarrierT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_barrier_BarrierT_h__
#define __algorithms_protocols_barrier_BarrierT_h__

#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/barrier/BarrierFactory.h"
#include "algorithms/executor/Barrier.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {
      typedef bool (*AnalyzeFn) (PAMI_GEOMETRY_CLASS *g);

      // Barrier Factory for generate routine
      // generate
      //
      template <class T, MetaDataFn get_metadata, class C>
      class BarrierFactoryT : public CollectiveProtocolFactoryT<T, get_metadata, C>
      {
        public:
          BarrierFactoryT(C                           *cmgr,
                          Interfaces::NativeInterface *native,
                          pami_dispatch_multicast_fn    cb_head = NULL):
              CollectiveProtocolFactoryT<T, get_metadata, C>(cmgr, native, cb_head)
          {
            TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
          }
          virtual Executor::Composite * generate(pami_geometry_t              geometry,
                                                 void                      * cmd)

          {
            // Use the cached barrier or generate a new one if the cached barrier doesn't exist
            TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
            PAMI_GEOMETRY_CLASS  *g = ( PAMI_GEOMETRY_CLASS *)geometry;
            Executor::Composite *c = (Executor::Composite *) g->getKey(PAMI::Geometry::PAMI_GKEY_BARRIERCOMPOSITE1);

            if (!c)
              c = CollectiveProtocolFactoryT<T, get_metadata, C>::generate(geometry, cmd);

            return c;
          }
      };

      ///
      /// \brief Binomial barrier
      ///
      template <class T_Schedule, AnalyzeFn afn>
      class BarrierT : public CCMI::Executor::Composite
      {
          ///
          /// \brief The executor for barrier protocol
          ///
          CCMI::Executor::BarrierExec         _myexecutor;
          ///
          /// \brief The schedule for barrier protocol
          ///
          T_Schedule                          _myschedule;

        public:
          ///
          /// \brief Constructor for non-blocking barrier protocols.
          ///
          /// \param[in] mInterface  The multicast Interface
          /// \param[in] geometry    Geometry object
          ///
          BarrierT  (Interfaces::NativeInterface          * mInterface,
                     ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS>     * cmgr,
                     pami_geometry_t                         geometry,
                     void                                 * cmd,
                     pami_event_function                     fn,
                     void                                 * cookie):
              _myexecutor(((PAMI_GEOMETRY_CLASS *)geometry)->nranks(),
                          ((PAMI_GEOMETRY_CLASS *)geometry)->ranks(),
                          ((PAMI_GEOMETRY_CLASS *)geometry)->comm(),
                          0,
                          mInterface),
              _myschedule (__global.mapping.task(), (PAMI::Topology *)((PAMI_GEOMETRY_CLASS *)geometry)->getTopology(0))
          {
            TRACE_INIT((stderr, "<%p>CCMI::Adaptors::Barrier::BarrierT::ctor()\n",
                        this));//, geometry->comm()));
            _myexecutor.setCommSchedule (&_myschedule);
          }

          static bool analyze (PAMI_GEOMETRY_CLASS *geometry)
          {
            return((AnalyzeFn) afn)(geometry);
          }

          virtual void start()
          {
            TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
            _myexecutor.setDoneCallback (_cb_done, _clientdata);
            _myexecutor.start();
          }

	  virtual void   notifyRecv  (unsigned              src,
				      const pami_quad_t   & metadata,
				      PAMI::PipeWorkQueue ** pwq,
				      pami_callback_t      * cb_done,
				      void                 * cookie) 
	  {
	    _myexecutor.notifyRecv (src, metadata, NULL, NULL);
	  }

          static void *   cb_head   (const pami_quad_t    * info,
                                     unsigned              count,
                                     unsigned              conn_id,
                                     unsigned              peer,
                                     unsigned              sndlen,
                                     void                * arg,
                                     size_t              * rcvlen,
                                     pami_pipeworkqueue_t **recvpwq,
                                     PAMI_Callback_t  * cb_done)
          {
            TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
            CollHeaderData  *cdata = (CollHeaderData *) info;
            CollectiveProtocolFactory *factory = (CollectiveProtocolFactory *) arg;

            PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) PAMI_GEOMETRY_CLASS::getCachedGeometry(cdata->_comm);
	    
	    *rcvlen    = 0;
            *recvpwq   = 0;
            cb_done->function    = NULL;
            cb_done->clientdata = NULL;	    

            if (geometry == NULL)
              {
                geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry (cdata->_comm);
		
		if (geometry!= NULL)
		  PAMI_GEOMETRY_CLASS::updateCachedGeometry(geometry, cdata->_comm);
		else {
		  //Geoemtry doesnt exist
		  PAMI_GEOMETRY_CLASS::registerUnexpBarrier(cdata->_comm, (pami_quad_t&)*info, peer, 
							    (unsigned) PAMI::Geometry::PAMI_GKEY_BARRIERCOMPOSITE1);
		  return NULL;
		}		
              }
	    
            PAMI_assert(geometry != NULL);
            BarrierT *composite = (BarrierT*) geometry->getKey(PAMI::Geometry::PAMI_GKEY_BARRIERCOMPOSITE1);
            CCMI_assert (composite != NULL);
            TRACE_INIT((stderr, "<%p>CCMI::Adaptor::Barrier::BarrierFactory::cb_head(%d,%p)\n",
                        factory, cdata->_comm, composite));
	    
            //Override poly morphism
            composite->_myexecutor.notifyRecv (peer, *info, NULL, 0);

            return NULL;
          }
      }; //-BarrierT

//////////////////////////////////////////////////////////////////////////////
    };
  };
};  //namespace CCMI::Adaptor::Barrier

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
