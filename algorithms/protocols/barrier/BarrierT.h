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
      typedef bool (*AnalyzeFn) (XMI_GEOMETRY_CLASS *g);

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
		   ConnectionManager::SimpleConnMgr<XMI_SYSDEP_CLASS>     * cmgr,
                   xmi_geometry_t                         geometry,
		   void                                 * cmd) :
	_myexecutor(((XMI_GEOMETRY_CLASS *)geometry)->nranks(),
		    ((XMI_GEOMETRY_CLASS *)geometry)->ranks(),
		    ((XMI_GEOMETRY_CLASS *)geometry)->comm(),
		    0,
		    mInterface),
	  _myschedule (__global.mapping.task(), (XMI::Topology *)((XMI_GEOMETRY_CLASS *)geometry)->getTopology(0))
	{
          TRACE_INIT((stderr,"<%#.8X>CCMI::Adaptors::Barrier::BarrierT::ctor(%X)\n",
                     (int)this, geometry->comm()));
          _myexecutor.setCommSchedule (&_myschedule);
        }

        static bool analyze (XMI_GEOMETRY_CLASS *geometry)
        {
          return((AnalyzeFn) afn)(geometry);
        }

	virtual void start() {
	  _myexecutor.setDoneCallback (_cb_done, _clientdata);
	  _myexecutor.start();
	}

	static void *   cb_head   (const xmi_quad_t    * info,
				   unsigned              count,
				   unsigned              conn_id,
				   unsigned              peer,
				   unsigned              sndlen,
				   void                * arg,
				   size_t              * rcvlen,
				   xmi_pipeworkqueue_t **recvpwq,
				   XMI_Callback_t  * cb_done)
	{
	  CollHeaderData  *cdata = (CollHeaderData *) info;
	  CollectiveProtocolFactory *factory = (CollectiveProtocolFactory *) arg;

	  XMI_GEOMETRY_CLASS *geometry = (XMI_GEOMETRY_CLASS *) XMI_GEOMETRY_CLASS::getCachedGeometry(cdata->_comm);
	  if(geometry == NULL)
	  {
	    geometry = (XMI_GEOMETRY_CLASS *) factory->getGeometry (cdata->_comm);
	    XMI_GEOMETRY_CLASS::updateCachedGeometry(geometry, cdata->_comm);
	  }
	  assert(geometry != NULL);
	  BarrierT *composite = (BarrierT*) geometry->getKey(XMI::Geometry::XMI_GKEY_BARRIERCOMPOSITE1);
	  CCMI_assert (composite != NULL);
	  TRACE_INIT((stderr,"<%#.8X>CCMI::Adaptor::Barrier::BarrierFactory::cb_head(%d,%x)\n",
		      (int)factory,cdata->_comm,(int)executor));

	  //Override poly morphism
	  composite->_myexecutor.notifyRecv (peer, *info, NULL, 0);

	  *rcvlen    = 0;
	  *recvpwq   = 0;
	  cb_done->function    = NULL;
	  cb_done->clientdata = NULL;

	  return NULL;
	}
      }; //-BarrierT

//////////////////////////////////////////////////////////////////////////////
    };
  };
};  //namespace CCMI::Adaptor::Barrier

#endif
