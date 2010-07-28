/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/ambcast/AMBroadcastT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_ambcast_AMBroadcastT_h__
#define __algorithms_protocols_ambcast_AMBroadcastT_h__

#include "algorithms/ccmi.h"
#include "algorithms/executor/Broadcast.h"
#include "algorithms/connmgr/RankBasedConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"
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
      template <class T_Schedule, class T_Conn, ScheduleFn create_schedule>
      class AMBroadcastT : public CCMI::Executor::Composite
      {
        protected:
          CCMI::Executor::BroadcastExec<T_Conn>  _executor __attribute__((__aligned__(16)));
          T_Schedule                             _schedule;
          unsigned                               _root;
          Interfaces::NativeInterface           *_native;
          PAMI_GEOMETRY_CLASS                   *_geometry;

        public:
          ///
          /// \brief Constructor
          ///
          AMBroadcastT (Interfaces::NativeInterface              * native,
                        T_Conn                                   * cmgr,
                        pami_geometry_t                            g,
                        pami_xfer_t                              * cmd,
                        pami_event_function                        fn,
                        void                                     * cookie):
              Executor::Composite(),
              _executor (native, cmgr, ((PAMI_GEOMETRY_CLASS *)g)->comm()),
              _root(native->myrank()), //On the initiating node
              _native(native),
              _geometry((PAMI_GEOMETRY_CLASS *)g)
          {
            TRACE_ADAPTOR ((stderr, "<%p>Broadcast::AMBroadcastT() \n", this));
            _executor.setBuffers ((char *)cmd->cmd.xfer_ambroadcast.sndbuf,
                                  (char *)cmd->cmd.xfer_ambroadcast.sndbuf,
                                  cmd->cmd.xfer_ambroadcast.stypecount);
            _executor.setDoneCallback (cmd->cb_done, cmd->cookie);
          }

          void setRoot (unsigned root)
          {
            _root = root;
          }

          void start()
          {
            _executor.setRoot (_root);
            COMPILE_TIME_ASSERT(sizeof(_schedule) >= sizeof(T_Schedule));
            create_schedule(&_schedule, sizeof(_schedule), _root, _native, _geometry);
            _executor.setSchedule(&_schedule, 0);
            _executor.start();
          }

          CCMI::Executor::BroadcastExec<T_Conn> &executor()
          {
            return _executor;
          }
      };

      template <class T, MetaDataFn get_metadata, class C>
      class AMBroadcastFactoryT: public CollectiveProtocolFactoryT<T, get_metadata, C>
      {
        protected:
          pami_dispatch_ambroadcast_fn _cb_ambcast;

        public:
          AMBroadcastFactoryT (C                           *cmgr,
                               Interfaces::NativeInterface *native):
              CollectiveProtocolFactoryT<T, get_metadata, C>(cmgr, native, (pami_dispatch_multicast_fn)cb_head)
          {
          }

          virtual ~AMBroadcastFactoryT ()
          {
          }

          /// NOTE: This is required to make "C" programs link successfully with virtual destructors
          void operator delete(void * p)
          {
            CCMI_abort();
          }

          virtual Executor::Composite * generate(pami_geometry_t              g,
                                                 void                      * cmd)
          {
            TRACE_ADAPTOR((stderr, "AMBroadcastFactoryT::generate()\n"));
            typename CollectiveProtocolFactoryT<T, get_metadata, C>::collObj *cobj =
              (typename CollectiveProtocolFactoryT<T, get_metadata, C>::collObj *)
              CollectiveProtocolFactoryT<T, get_metadata, C>::_alloc.allocateObject();

            new (cobj) typename CollectiveProtocolFactoryT<T, get_metadata, C>::collObj
            (CollectiveProtocolFactoryT<T, get_metadata, C>::_native,  // Native interface
             CollectiveProtocolFactoryT<T, get_metadata, C>::_cmgr,    // Connection Manager
             g,                 // Geometry Object
             (pami_xfer_t*)cmd, // Parameters
             CollectiveProtocolFactoryT<T, get_metadata, C>::done_fn,  // Intercept function
             cobj,              // Intercept cookie
             this);             // Factory

            cobj->_obj.start();
            return NULL;
          }

          virtual void setAsyncInfo (bool                          is_buffered,
                                     pami_dispatch_callback_fn      cb_async,
                                     pami_mapidtogeometry_fn        cb_geometry)
          {
            _cb_ambcast = cb_async.ambroadcast;
          };


          static void   cb_head
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
            AMBroadcastFactoryT *factory = (AMBroadcastFactoryT *) arg;
            CollHeaderData *cdata = (CollHeaderData *) info;

            int comm = cdata->_comm;
            PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) PAMI_GEOMETRY_CLASS::getCachedGeometry(comm);

            if (geometry == NULL)
              {
                geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry (comm);
                PAMI_GEOMETRY_CLASS::updateCachedGeometry(geometry, comm);
              }

            pami_xfer_t broadcast;
            broadcast.algorithm = (size_t) - 1;  ///Not used by the protocols
            broadcast.cmd.xfer_ambroadcast.user_header  = NULL;
            broadcast.cmd.xfer_ambroadcast.headerlen    = 0;
            pami_type_t pt = PAMI_BYTE;
            broadcast.cmd.xfer_ambroadcast.stype = &pt;

            //Assume send datatypes are the same as recv datatypes
            factory->_cb_ambcast
            (NULL,                 // Context: NULL for now, until we can get the context
             cdata->_root,         // Root
             (pami_geometry_t)geometry,  // Geometry
             sndlen,               // sndlen
             NULL,                 // User header, NULL for now
             0,                    // 0 sized for now
             &broadcast.cmd.xfer_ambroadcast.sndbuf,
             &broadcast.cmd.xfer_ambroadcast.stype,
             &broadcast.cmd.xfer_ambroadcast.stypecount,
             &broadcast.cb_done,
             &broadcast.cookie);

            typename CollectiveProtocolFactoryT<T, get_metadata, C>::collObj *cobj =
              (typename CollectiveProtocolFactoryT<T, get_metadata, C>::collObj *)
              factory->CollectiveProtocolFactoryT<T, get_metadata, C>::_alloc.allocateObject();

            new (cobj) typename CollectiveProtocolFactoryT<T, get_metadata, C>::collObj
            (factory->CollectiveProtocolFactoryT<T, get_metadata, C>::_native,  // Native interface
             factory->CollectiveProtocolFactoryT<T, get_metadata, C>::_cmgr,    // Connection Manager
             (pami_geometry_t)geometry,  // Geometry
             &broadcast, // Parameters
             CollectiveProtocolFactoryT<T, get_metadata, C>::done_fn,  // Intercept function
             cobj,              // Intercept cookie
             factory);             // Factory

            //Correct the root to what was passed in
            //T *ambcast = (T *) factory->generate ((pami_geometry_t)geometry, &broadcast);
            cobj->_obj.setRoot (cdata->_root);
            cobj->_obj.start();
            cobj->_obj.executor().notifyRecv(peer, *info, (PAMI::PipeWorkQueue **)rcvpwq, cb_done);

            //We only support sndlen == rcvlen
            * rcvlen  = sndlen;
          }

      }; //- AMBroadcastFactoryT
    };  //- end namespace AMBroadcast
  };  //- end namespace Adaptor
};  //- end CCMI


#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
