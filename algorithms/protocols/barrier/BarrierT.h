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
#include "algorithms/protocols/AllSidedCollectiveProtocolFactoryT.h"
#include "algorithms/executor/Barrier.h"
#include "algorithms/protocols/AllSidedCollectiveProtocolFactoryT.h"

extern void registerunexpbarrier(pami_context_t context,
                                 unsigned       comm,
                                 pami_quad_t   &info,
                                 unsigned       peer,
                                 unsigned       algorithm);


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
template < class T_Composite, MetaDataFn get_metadata, class T_Conn, bool T_Unexp=true, PAMI::Geometry::ckeys_t T_Key = PAMI::Geometry::CKEY_BARRIERCOMPOSITE1 >
class BarrierFactoryT : public CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>
{
public:
    BarrierFactoryT(T_Conn                      *cmgr,
                    Interfaces::NativeInterface *native,
                    pami_dispatch_multicast_function cb_head = NULL):
        CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>(cmgr, native, cb_head)
    {
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        _cached_object = NULL;
        _cached_id     = (unsigned) -1;
    }
    virtual Executor::Composite * generate(pami_geometry_t              geometry,
                                           void                      * cmd)

    {
        // Use the cached barrier or generate a new one if the cached barrier doesn't exist
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        PAMI_GEOMETRY_CLASS  *g = ( PAMI_GEOMETRY_CLASS *)geometry;
        Executor::Composite *composite = (Executor::Composite *) g->getKey((size_t)0, /// \todo does NOT support multicontext
                                         T_Key);

        if (!composite)
        {
            composite = CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::generate(geometry, cmd);
            g->setKey((size_t)0, /// \todo does NOT support multicontext
                      T_Key,
                      (void*)composite);
        }

        pami_xfer_t *xfer = (pami_xfer_t *)cmd;
        composite->setDoneCallback(xfer->cb_done, xfer->cookie);
        return composite;
    }

    ///
    ///  \brief Cache the barrier executor for the most recent geometry
    ///  \brief They pami context
    ///  \param[in] id communicator id of the geometry
    ///
    void * getGeometryObject (pami_context_t ctxt, unsigned id)
    {
        if (likely(_cached_object && id  == _cached_id))
            return _cached_object;

        PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) this->getGeometry(ctxt, id);
        _cached_object =  (geometry) ? (geometry->getKey(0, T_Key)) : NULL;
        _cached_id     =  id;

        return _cached_object;
    }

    static void    cb_head   (pami_context_t         ctxt,
                              const pami_quad_t    * info,
                              unsigned              count,
                              unsigned              conn_id,
                              size_t                peer,
                              size_t                sndlen,
                              void                * arg,
                              size_t              * rcvlen,
                              pami_pipeworkqueue_t **recvpwq,
                              PAMI_Callback_t  *     cb_done)
    {
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        CollHeaderData  *cdata = (CollHeaderData *) info;
        BarrierFactoryT *factory = (BarrierFactoryT *) arg;

        *rcvlen    = 0;
        *recvpwq   = 0;
        cb_done->function   = NULL;
        cb_done->clientdata = NULL;

        PAMI_assert (factory != NULL);
        //PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry (ctxt, cdata->_comm);
        void *object = factory->getGeometryObject(ctxt, cdata->_comm);

        if (T_Unexp) {
            if (object == NULL)
            {
                //Geoemtry doesnt exist
                registerunexpbarrier(ctxt,
                                     cdata->_comm,
                                     (pami_quad_t&)*info,
                                     peer,
                                     (unsigned) PAMI::Geometry::GKEY_UEBARRIERCOMPOSITE1);
                return;
            }
        }

        T_Composite *composite = (T_Composite*) object;
        TRACE_INIT((stderr, "<%p>CCMI::Adaptor::Barrier::BarrierFactoryT::cb_head(%d,%p)\n",
                    factory, cdata->_comm, composite));

        //Override poly morphism
        composite->_myexecutor.notifyRecv (peer, *info, NULL, 0);
    }

protected:
    unsigned                             _cached_id;
    void                               * _cached_object;
};


///
/// Barrier Factory All Sided for generate routine
///
template < class T_Composite, MetaDataFn get_metadata, class T_Conn, PAMI::Geometry::ckeys_t T_Key >
class BarrierFactoryAllSidedT : public AllSidedCollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>
{
public:
    BarrierFactoryAllSidedT(T_Conn                      *cmgr,
                            Interfaces::NativeInterface *native):
        AllSidedCollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>(cmgr, native)
    {
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
    }

    virtual Executor::Composite * generate(pami_geometry_t              geometry,
                                           void                       * cmd)
    {
        // Use the cached barrier or generate a new one if the cached barrier doesn't exist
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        PAMI_GEOMETRY_CLASS  *g = ( PAMI_GEOMETRY_CLASS *)geometry;
        /// \todo does NOT support multicontext
        Executor::Composite *composite = (Executor::Composite *) g->getKey((size_t)0,
                                         T_Key);

        if (!composite)
        {
            composite = AllSidedCollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::generate(geometry, cmd);
            g->setKey((size_t)0, /// \todo does NOT support multicontext
                      T_Key,
                      (void*)composite);
        }

        pami_xfer_t *xfer = (pami_xfer_t *)cmd;
        composite->setDoneCallback(xfer->cb_done, xfer->cookie);
        return composite;
    }
};



///
/// Barrier Factory All Sided for generate routine
///
template <class T_Composite, MetaDataFn get_metadata, class T_Conn, int LookupNI=0>
class BarrierFactory2DeviceMsync: public CollectiveProtocolFactory
{
    class collObj
    {
    public:
        collObj(Interfaces::NativeInterface  * nativeL,
                Interfaces::NativeInterface  * nativeG,
                T_Conn                       * cmgr,
                pami_geometry_t                geometry,
                pami_xfer_t                  * cmd,
                pami_event_function            fn,
                void                         * cookie,
                BarrierFactory2DeviceMsync   * factory):
            _factory(factory),
            _user_done_fn(cmd->cb_done),
            _user_cookie(cmd->cookie),
            _obj(nativeL, nativeG,cmgr,geometry,cmd,fn,cookie)
        {
            TRACE_FN_ENTER();
            TRACE_FORMAT("<%p>",this);
            DO_DEBUG((templateName<T_Composite>()));
            TRACE_FN_EXIT();
        }

        BarrierFactory2DeviceMsync         * _factory;
        pami_event_function                  _user_done_fn;
        void                               * _user_cookie;
        T_Composite                                    _obj;
    };


public:
    BarrierFactory2DeviceMsync (T_Conn                   * cmgr,
                                Interfaces::NativeInterface * nativeL,
                                Interfaces::NativeInterface * nativeG):
        CollectiveProtocolFactory(),
        _cmgr(cmgr),
        _nativeL(nativeL),
        _nativeG(nativeG)
    {
        TRACE_FN_ENTER();
        TRACE_FORMAT("<%p>",this);
        TRACE_FN_EXIT();
    }

    virtual ~BarrierFactory2DeviceMsync ()
    {
    }

    /// NOTE: This is required to make "C" programs link successfully with virtual destructors
    void operator delete(void * p)
    {
        CCMI_abort();
    }

    static void done_fn(pami_context_t  context,
                        void          * clientdata,
                        pami_result_t   res)
    {
        TRACE_FN_ENTER();
        collObj *cobj = (collObj *)clientdata;
        TRACE_FORMAT("<%p> cobj %p",cobj->_factory, cobj);
        cobj->_user_done_fn(context, cobj->_user_cookie, res);
        cobj->_factory->_alloc.returnObject(cobj);
        TRACE_FN_EXIT();
    }


    virtual Executor::Composite * generate(pami_geometry_t             geometry,
                                           void                      * cmd)
    {
        TRACE_FN_ENTER();


        // This should compile out if native interfaces are scoped
        // globally.
        if(LookupNI)
        {
            PAMI_GEOMETRY_CLASS *g = (PAMI_GEOMETRY_CLASS*)geometry;
            _nativeL = _ni_local_map[g->comm()];
            _nativeG = _ni_global_map[g->comm()];
        }

        collObj *cobj = (collObj*)  _alloc.allocateObject();
        TRACE_FORMAT("<%p> cobj %p",this, cobj);
        new(cobj) collObj(_nativeL,         // Native interface
                          _nativeG,         // Native interface
                          _cmgr,            // Connection Manager
                          geometry,         // Geometry Object
                          (pami_xfer_t*)cmd,// Parameters
                          done_fn,          // Intercept function
                          cobj,             // Intercept cookie
                          this);            // Factory
        TRACE_FN_EXIT();
        return(Executor::Composite *)(&cobj->_obj);
    }

    virtual void metadata(pami_metadata_t *mdata)
    {
        TRACE_FN_ENTER();
        TRACE_FORMAT("mdata=%p",mdata);
        get_metadata(mdata);
        TRACE_FN_EXIT();
    }

    inline void setNI(pami_geometry_t geometry,
                      Interfaces::NativeInterface *nativeL,
                      Interfaces::NativeInterface *nativeG)
    {
        PAMI_assert(LookupNI == 1); // no local master?
        PAMI_GEOMETRY_CLASS *g = (PAMI_GEOMETRY_CLASS*)geometry;
        _ni_local_map[g->comm()]  = nativeL;
        _ni_global_map[g->comm()] = nativeG;
        _nativeL = NULL;
        _nativeG = NULL;
    }

private:
    T_Conn                                     * _cmgr;
    Interfaces::NativeInterface                * _nativeL;
    Interfaces::NativeInterface                * _nativeG;
    PAMI::MemoryAllocator<sizeof(collObj), 16>   _alloc;
    std::map<size_t,Interfaces::NativeInterface *>   _ni_local_map;
    std::map<size_t,Interfaces::NativeInterface *>   _ni_global_map;
};


///
/// \brief barrier template
///
template < class T_Schedule, AnalyzeFn afn, PAMI::Geometry::topologyIndex_t T_Geometry_Index = PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX, PAMI::Geometry::ckeys_t T_Key = PAMI::Geometry::CKEY_BARRIERCOMPOSITE1 >
class BarrierT : public CCMI::Executor::Composite
{
public:
    ///
    /// \brief The executor for barrier protocol
    ///
    CCMI::Executor::BarrierExec         _myexecutor;
    ///
    /// \brief The schedule for barrier protocol
    ///
    T_Schedule                          _myschedule;

    ///
    /// \brief Constructor for non-blocking barrier protocols.
    ///
    /// \param[in] mInterface  The multicast Interface
    /// \param[in] geometry    Geometry object
    ///
    BarrierT  (Interfaces::NativeInterface          * mInterface,
               ConnectionManager::SimpleConnMgr     * cmgr,
               pami_geometry_t                         geometry,
               void                                 * cmd,
               pami_event_function                     fn,
               void                                 * cookie):
        _myexecutor(((PAMI_GEOMETRY_CLASS *)geometry)->comm(),
                    0,                                        // connection id?
                    mInterface),
        _myschedule (__global.mapping.task(), (PAMI::Topology *)((PAMI_GEOMETRY_CLASS *)geometry)->getTopology(T_Geometry_Index))
    {
        TRACE_INIT((stderr, "<%p>CCMI::Adaptors::Barrier::BarrierT::ctor()\n",
                    this));//, geometry->comm()));
        _myexecutor.setCommSchedule (&_myschedule);
    }

    CCMI::Executor::BarrierExec *getExecutor() {
        return &_myexecutor;
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
