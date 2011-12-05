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
template <class T_Schedule, class T_Conn, SFunc<PAMI_GEOMETRY_CLASS>::ScheduleFn create_schedule>
class AMBroadcastT : public CCMI::Executor::Composite
{
protected:
    CCMI::Executor::BroadcastExec<T_Conn>  _executor __attribute__((__aligned__(16)));
    T_Schedule                             _schedule;
    pami_endpoint_t                        _root;
    Interfaces::NativeInterface           *_native;
    PAMI_GEOMETRY_CLASS                   *_geometry;
    pami_context_t                         _ctxt;
    size_t                                 _ctxt_id;
public:
    ///
    /// \brief Constructor
    ///
    AMBroadcastT (pami_context_t                             ctxt,
                  size_t                                     ctxt_id,
                  Interfaces::NativeInterface              * native,
                  T_Conn                                   * cmgr,
                  pami_geometry_t                            g,
                  pami_xfer_t                              * cmd,
                  pami_event_function                        fn,
                  void                                     * cookie):
        Executor::Composite(),
        _executor (native, cmgr, ((PAMI_GEOMETRY_CLASS *)g)->comm()),
        _root(native->endpoint()), //On the initiating node
        _native(native),
        _geometry((PAMI_GEOMETRY_CLASS *)g),
        _ctxt(ctxt),
        _ctxt_id(ctxt_id)
    {
        TRACE_ADAPTOR ((stderr, "<%p>Broadcast::AMBroadcastT() \n", this));
        _executor.setRoot (_root);
        _executor.setDoneCallback (cmd->cb_done, cmd->cookie);
    }

    void setRoot (unsigned root)
    {
        _root = root;
        _executor.setRoot (_root);
    }

    void  setBuffers (char *src, char *dst, int len, TypeCode * stype, TypeCode * rtype)
    {
        _executor.setBuffers (src,dst,len,stype,rtype);
    }

    void start()
    {
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

template <class T_Composite, MetaDataFn get_metadata, class T_Conn>
class AMBroadcastFactoryT: public CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>
{
protected:
    pami_dispatch_ambroadcast_function _cb_ambcast;

public:
    AMBroadcastFactoryT (pami_context_t               ctxt,
                         size_t                       ctxt_id,
                         pami_mapidtogeometry_fn      cb_geometry,
                         T_Conn                      *cmgr,
                         Interfaces::NativeInterface *native):
      CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>(ctxt,ctxt_id,cb_geometry,cmgr, native, cb_head)
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

        pami_xfer_t* pcmd = (pami_xfer_t*)cmd;

        typename CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::collObj *cobj =
            (typename CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::collObj *)
            CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::_alloc.allocateObject();

        new (cobj) typename CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::collObj
          (this->_context,
           this->_context_id,
           CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::_native,  // Native interface
         CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::_cmgr,    // Connection Manager
         g,                 // Geometry Object
         pcmd, // Parameters
         CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::done_fn,  // Intercept function
         cobj,              // Intercept cookie
         this);             // Factory

        TypeCode * stype = (TypeCode *)pcmd->cmd.xfer_ambroadcast.stype;
        cobj->_obj.setBuffers ((char *)pcmd->cmd.xfer_ambroadcast.sndbuf,
                              (char *)pcmd->cmd.xfer_ambroadcast.sndbuf,
                               pcmd->cmd.xfer_ambroadcast.stypecount * stype->GetDataSize(),
                               stype,stype);
        cobj->_obj.start();
        return NULL;
    }

    virtual void setAsyncInfo (bool                          is_buffered,
                               pami_dispatch_callback_function cb_async,
                               pami_mapidtogeometry_fn        cb_geometry)
    {
        _cb_ambcast = cb_async.ambroadcast;
    };


    static void   cb_head
    (pami_context_t         ctxt,
     const pami_quad_t     * info,
     unsigned               count,
     unsigned               conn_id,
     size_t                 peer,
     size_t                 sndlen,
     void                 * arg,
     size_t               * rcvlen,
     pami_pipeworkqueue_t ** rcvpwq,
     pami_callback_t       * cb_done)
    {
        AMBroadcastFactoryT *factory = (AMBroadcastFactoryT *) arg;
        CollHeaderData *cdata = (CollHeaderData *) info;

        int comm = cdata->_comm;
        PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry (ctxt, comm);

        pami_xfer_t broadcast;
        broadcast.algorithm = (size_t) - 1;  ///Not used by the protocols
        broadcast.cmd.xfer_ambroadcast.user_header  = NULL;
        broadcast.cmd.xfer_ambroadcast.headerlen    = 0;
        pami_type_t pt = PAMI_TYPE_BYTE;
        broadcast.cmd.xfer_ambroadcast.stype = pt;

        pami_recv_t recv = {0};
        factory->_cb_ambcast
          (ctxt,                      // context
           NULL,                      // user cookie:  todo, NULL for now
           NULL,                      // User header, NULL for now
           0,                         // 0 sized for now
           NULL,                      // Pipe ADDR, NULL for now
           sndlen,                    // sndlen
           cdata->_root,              // origin (root)
           (pami_geometry_t)geometry, // Geometry
           &recv);                    // recv info

        // Todo:  data_fn, offset handling
        broadcast.cmd.xfer_ambroadcast.sndbuf     = recv.addr;
        broadcast.cmd.xfer_ambroadcast.stype      = recv.type;
        broadcast.cmd.xfer_ambroadcast.stypecount = sndlen;
        broadcast.cb_done                         = recv.local_fn;
        broadcast.cookie                          = recv.cookie;

        TypeCode * stype = (TypeCode *)broadcast.cmd.xfer_ambroadcast.stype;

        typename CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::collObj *cobj =
            (typename CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::collObj *)
            factory->CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::_alloc.allocateObject();
        //FIXME:: pass proper context/context id for endpoint
        new (cobj) typename CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::collObj
          (ctxt,0,factory->CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::_native,  // Native interface
         factory->CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::_cmgr,    // Connection Manager
         (pami_geometry_t)geometry,  // Geometry
         &broadcast, // Parameters
         CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::done_fn,  // Intercept function
         cobj,              // Intercept cookie
         factory);           // Factory 

        //Correct the root to what was passed in
        cobj->_obj.setRoot (cdata->_root);
        cobj->_obj.setBuffers ((char *)recv.addr,(char *)recv.addr, sndlen*stype->GetDataSize(), stype, stype);
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
