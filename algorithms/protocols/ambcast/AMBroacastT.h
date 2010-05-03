/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/AsyncBroadcastT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_broadcast_AsyncBroadcastT_h__
#define __algorithms_protocols_broadcast_AsyncBroadcastT_h__

#include "algorithms/ccmi.h"
#include "algorithms/executor/Broadcast.h"
#include "algorithms/connmgr/CommSeqConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/CollOpT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace AMBroadcast
    {
      ///
      /// \brief Asyc Broadcast Composite. It is single color right now
      ///
      template <clas T_Schedule, class T_Conn> 
	class AMBroadcastT : public CCMI::Executor::Composite
      {
      protected:
        CCMI::Executor::BroadcastExec<T_Conn>  _executor __attribute__((__aligned__(16)));
        T_Schedule                             _schedule;

      public:
        ///
        /// \brief Constructor
        ///
        AMBroadcastT (Interfaces::NativeInterface              * native,
		      C                                        * cmgr,
		      pami_geometry_t                            g,
		      pami_xfer_t                              * cmd,
		      pami_event_function                        fn,
		      void                                     * cookie,
		      CollectiveProtocolFactoryT               * factory):
	Executor::Composite(),
	_executor (native, cmgr, (PAMI_GEOMETRY_CLASS *)g->comm())
        {
          TRACE_ADAPTOR ((stderr, "<%#.8X>Broadcast::AMBroadcastT() \n",(int)this));
	  PAMI_GEOMETRY_CLASS * geometry = (PAMI_GEOMETRY_CLASS *)g;
          _executor.setRoot (cmd->cmd.xfer_ambroadcast.root);
          _executor.setBuffers (cmd->cmd.xfer_ambroadcast.sndbuf,
				cmd->cmd.xfer_ambroadcast.sndbuf,
				cmd->cmd.xfer_ambroadcast.stypecount);
          _executor.setDoneCallback (cmd->cb_done, cmd->cookie);

          COMPILE_TIME_ASSERT(sizeof(_schedule) >= sizeof(T_Schedule));
          create_schedule(&_schedule, sizeof(_schedule), root, native, geometry);
          _executor.setSchedule (&_schedule, 0);
        }

        ///
        /// \brief initialize the schedule based on input geometry.
        /// Template implementation must specialize this function.
        ///
        void  create_schedule(void                      * buf,
                              unsigned                    size,
                              unsigned                    root,
			      Interfaces::NativeInterface * native,
			      PAMI_GEOMETRY_CLASS        * g) {CCMI_abort();};

        CCMI::Executor::BroadcastExec<T_Conn> &executor()
        {
          return _executor;
        }

	static PAMI_Request_t *   cb_head
	  (const pami_quad_t     * info,
	   unsigned               count,
	   unsigned               conn_id,
	   unsigned               peer,
	   unsigned               sndlen,
	   void                 * arg,
	   size_t               * rcvlen,
	   pami_pipeworkqueue_t ** rcvpwq,
	   pami_callback_t       * cb_done)
	{
	  CollectiveProtocolFacotory *factory = (CollectiveProtocolFacotory *) arg;	  
	  CollHeaderData *cdata = (CollHeaderData *) info;

	  int comm = cdata->_comm;
	  PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) PAMI_GEOMETRY_CLASS::getCachedGeometry(comm);
	  if(geometry == NULL)
	  {
	    geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry (comm);
	    PAMI_GEOMETRY_CLASS::updateCachedGeometry(geometry, comm);
	  }
	  
	  pami_xfer_t brodcast;
	  broadcast.algorithm = (unsigned)-1;  ///Not used by the protocols
	  broadcast.cmd.xfer_ambroadcast.user_header  = NULL;
	  broadcast.cmd.xfer_ambroadcast.headerlen    = 0;
	  
	  pami_dispatch_ambroadcast_fn cb_ambcast = factory->getAMBroadcastDispatch();
	  //Assume send datatypes are the same as recv datatypes
          cb_ambcast (NULL,                 // Context: NULL for now, until we can get the context
		      cdata->_root,         // Root
		      (void*)cdata->_comm,  // Geometry
		      sndlen,               // sndlen
		      NULL,                 // User header, NULL for now
		      0,                    // 0 sized for now
		      &broadcast.cmd.xfer_ambroadcast.sndbuf,
		      &broadcast.cmd.xfer_ambroadcast.stype,
		      &broadcast.cmd.xfer_ambroadcast.stypecount,
		      &broadcast.cb_done,
		      &broadcast.cookie);

	  factory->generate (geometry, &xfer);
	  //We only support sndlen == rcvlen
	  * rcvlen  = sndlen;	  
	  return NULL;
	}

      }; //- AMBroadcastT

    };  //- end namespace AMBroadcast
  };  //- end namespace Adaptor
};  //- end CCMI


#endif
