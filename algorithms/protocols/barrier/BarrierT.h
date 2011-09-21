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
#include "algorithms/protocols/AllSidedCollectiveProtocolFactoryNCOT.h"

#include "util/trace.h"

#ifdef CCMI_TRACE_ALL
  #define DO_TRACE_ENTEREXIT 1
  #define DO_TRACE_DEBUG     1
#else
  #define DO_TRACE_ENTEREXIT 0
  #define DO_TRACE_DEBUG     0
#endif


#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

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
template < class T_Composite, MetaDataFn get_metadata, class T_Conn, bool T_Unexp, PAMI::Geometry::ckeys_t T_Key >
class BarrierFactoryT : public CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>
{
public:
    BarrierFactoryT(T_Conn                      *cmgr,
                    Interfaces::NativeInterface *native,
                    pami_dispatch_multicast_function cb_head = NULL):
        CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>(cmgr, native, cb_head)
    {
        TRACE_FN_ENTER();
        TRACE_FORMAT( "%p",this);
        _cached_object = NULL;
        _cached_id     = (unsigned) -1;
        TRACE_FN_EXIT();
    }
    ~BarrierFactoryT()
    {
        TRACE_FN_ENTER();
        TRACE_FORMAT( "%p",this);
        TRACE_FN_EXIT();
    }
    virtual Executor::Composite * generate(pami_geometry_t              geometry,
                                           void                      * cmd)

    {
        // Use the cached barrier or generate a new one if the cached barrier doesn't exist
        TRACE_FN_ENTER();
        PAMI_GEOMETRY_CLASS  *g = ( PAMI_GEOMETRY_CLASS *)geometry;
        Executor::Composite *composite = (Executor::Composite *) g->getKey((size_t)0, /// \todo does NOT support multicontext
                                         T_Key);

        TRACE_ERR((stderr,"(%8.8u)<%p>generate composite %p geometry %p, T_Key %u\n",Kernel_ProcessorID(),this,composite, geometry, T_Key));
        if (!composite)
        {
            composite = CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::generate(geometry, cmd);
            TRACE_FORMAT( "<%p> composite %p",this,composite);
            g->setKey((size_t)0, /// \todo does NOT support multicontext
                      T_Key,
                      (void*)composite);
            TRACE_ERR((stderr,"(%8.8u)<%p>generate composite %p geometry %p, T_Key %u\n",Kernel_ProcessorID(),this,composite, geometry, T_Key));
        }

        pami_xfer_t *xfer = (pami_xfer_t *)cmd;
        composite->setDoneCallback(xfer->cb_done, xfer->cookie);
        pami_metadata_t mdata;
        this->metadata(&mdata);
        TRACE_ERR((stderr,"(%8.8u)<%p>generate composite %p geometry %p, T_Key %u, name %s\n",Kernel_ProcessorID(),this,composite, geometry, T_Key, mdata.name));
        TRACE_FN_EXIT();
        return composite;
    }
    static void cleanup_done_fn(pami_context_t  context,
                          void           *clientdata,
                          pami_result_t   res)
    {
      TRACE_FN_ENTER();
      T_Composite *obj = (T_Composite *)clientdata;
      typename CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::collObj *cobj = (typename CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::collObj *)obj->getCollObj();
      cobj->~collObj();
      cobj->_factory->_alloc.returnObject(cobj);
      TRACE_FN_EXIT();
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

        //  Here we set the cached id back to the normal value
        //  of -1 because the user may re-use geometry ids, and we may errantly
        //  cache the id.  Set this to id if the user will not re-use
        //  geometry id's in a create/destroy cycle.
        // _cached_id     =  id;
        _cached_id     =  -1;

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
        TRACE_FN_ENTER();
        CollHeaderData  *cdata = (CollHeaderData *) info;
        BarrierFactoryT *factory = (BarrierFactoryT *) arg;
        TRACE_FORMAT( "<%p>cdata %p",factory,cdata);

        *rcvlen    = 0;
        *recvpwq   = 0;
        cb_done->function   = NULL;
        cb_done->clientdata = NULL;

        PAMI_assert (factory != NULL);
        //PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry (ctxt, cdata->_comm);
        void *object = factory->getGeometryObject(ctxt, cdata->_comm);
        TRACE_ERR((stderr,"(%8.8u)<%p>cb_head composite %p, T_Unexp %u, T_Key %u\n",Kernel_ProcessorID(),factory, object, T_Unexp, T_Key));
        if (T_Unexp) {
            if (object == NULL)
            {
                //Geometry doesn't exist
                registerunexpbarrier(ctxt,
                                     cdata->_comm,
                                     (pami_quad_t&)*info,
                                     peer,
                                     (unsigned) PAMI::Geometry::GKEY_UEBARRIERCOMPOSITE1);
                TRACE_FN_EXIT();
                return;
            }
        }

        T_Composite *composite = (T_Composite*) object;
        TRACE_FORMAT("<%p>CCMI::Adaptor::Barrier::BarrierFactoryT::cb_head(%d,%p)",
                    factory, cdata->_comm, composite);

        //Override poly morphism
        PAMI_assert(ctxt != NULL);
        PAMI_assert(composite != NULL);
        composite->_myexecutor.setContext(ctxt);
        composite->_myexecutor.notifyRecv (peer, *info, NULL, 0);
        TRACE_FN_EXIT();
    }

protected:
    unsigned                             _cached_id;
    void                               * _cached_object;
};


///
/// Barrier Factory All Sided for generate routine
///
template < class T_Composite, MetaDataFn get_metadata, class T_Conn, PAMI::Geometry::ckeys_t T_Key >
class BarrierFactoryAllSidedT : public AllSidedCollectiveProtocolFactoryNCOT<T_Composite, get_metadata, T_Conn>
{
public:
    BarrierFactoryAllSidedT(T_Conn                      *cmgr,
                            Interfaces::NativeInterface *native):
        AllSidedCollectiveProtocolFactoryNCOT<T_Composite, get_metadata, T_Conn>(cmgr, native)
    {
        TRACE_FN_ENTER();
        TRACE_FORMAT( "%p",this);
        TRACE_FN_EXIT();
    }
    BarrierFactoryAllSidedT(T_Conn                       *cmgr,
                            Interfaces::NativeInterface **native):
        AllSidedCollectiveProtocolFactoryNCOT<T_Composite, get_metadata, T_Conn>(cmgr, native)
    {
        TRACE_FN_ENTER();
        TRACE_FORMAT( "%p",this);
        TRACE_FN_EXIT();
    }

    virtual Executor::Composite * generate(pami_geometry_t              geometry,
                                           void                       * cmd)
    {
        // Use the cached barrier or generate a new one if the cached barrier doesn't exist
        TRACE_FN_ENTER();
        TRACE_FORMAT( "%p",this);
        PAMI_GEOMETRY_CLASS  *g = ( PAMI_GEOMETRY_CLASS *)geometry;
        /// \todo does NOT support multicontext
        Executor::Composite *composite = (Executor::Composite *) g->getKey((size_t)0,
                                         T_Key);

        if (!composite)
        {
            composite = AllSidedCollectiveProtocolFactoryNCOT<T_Composite, get_metadata, T_Conn>::generate(geometry, cmd);
            g->setKey((size_t)0, /// \todo does NOT support multicontext
                      T_Key,
                      (void*)composite);
        }

        pami_xfer_t *xfer = (pami_xfer_t *)cmd;
        composite->setDoneCallback(xfer->cb_done, xfer->cookie);
        TRACE_FN_EXIT();
        return composite;
    }
};



///
/// \brief barrier template
///
template < class T_Schedule, AnalyzeFn afn, PAMI::Geometry::topologyIndex_t T_Geometry_Index, PAMI::Geometry::ckeys_t T_Key >
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
        TRACE_FN_ENTER();
        TRACE_FORMAT( "%p",this);
        _myexecutor.setCommSchedule (&_myschedule);
        this->_collObj = cookie;//SSS: cookie is the collObj that contains this composite. I need to set it here so I can free it later.
        TRACE_FN_EXIT();
    }
    ~BarrierT()
    {
        TRACE_FN_ENTER();
        TRACE_FORMAT( "%p",this);
        TRACE_FN_EXIT();
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
        TRACE_FN_ENTER();
        TRACE_FORMAT( "%p %p %p",this, _cb_done, _clientdata);
        _myexecutor.setDoneCallback (_cb_done, _clientdata);
        _myexecutor.start();
        TRACE_FN_EXIT();
    }

    virtual void   notifyRecv  (unsigned              src,
                                const pami_quad_t   & metadata,
                                PAMI::PipeWorkQueue ** pwq,
                                pami_callback_t      * cb_done,
                                void                 * cookie)
    {
        _myexecutor.notifyRecv (src, metadata, NULL, NULL);
    }

    inline void * getCollObj()
    {
      TRACE_FN_ENTER();
      TRACE_FORMAT("<%p> %p",this,_collObj);
      TRACE_FN_EXIT();
      return _collObj;
    }

    void   *_collObj;

}; //-BarrierT

//////////////////////////////////////////////////////////////////////////////
};
};
};  //namespace CCMI::Adaptor::Barrier

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
