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

#include "Global.h"
#include "algorithms/ccmi.h"
#include "algorithms/executor/Broadcast.h"
#include "algorithms/connmgr/CommSeqConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/CollOpT.h"

namespace CCMI
{
namespace Adaptor
{
namespace Broadcast
{
///
/// \brief Asyc Broadcast Composite. It is single color right now
///

template <class T_Schedule, class T_Conn, SFunc<PAMI_GEOMETRY_CLASS>::ScheduleFn create_schedule>
class AsyncBroadcastT : public CCMI::Executor::Composite
{
protected:
    CCMI::Executor::BroadcastExec<T_Conn>  _executor __attribute__((__aligned__(16)));
    T_Schedule                             _schedule;

public:
    ///
    /// \brief Constructor
    ///
    AsyncBroadcastT ()
    {
    };
    AsyncBroadcastT (Interfaces::NativeInterface   * native,
                     T_Conn                        * cmgr,
                     pami_callback_t                  cb_done,
                     PAMI_GEOMETRY_CLASS            * geometry,
                     unsigned                        root,
                     char                          * src,
                     unsigned                        bytes) :
        Executor::Composite(),
        _executor (native, cmgr, geometry->comm())
    {
        TRACE_ADAPTOR ((stderr, "<%p>Broadcast::AsyncBroadcastT() \n", this));

        _executor.setRoot (root);
        _executor.setBuffers (src, src, bytes);
        _executor.setDoneCallback (cb_done.function, cb_done.clientdata);

        COMPILE_TIME_ASSERT(sizeof(_schedule) >= sizeof(T_Schedule));
        create_schedule(&_schedule, sizeof(_schedule), root, native, geometry);
        _executor.setSchedule (&_schedule, 0);
    }

    CCMI::Executor::BroadcastExec<T_Conn> &executor()
    {
        return _executor;
    }
}; //- AsyncBroadcastT


template <class T_Composite, MetaDataFn get_metadata, class T_Conn,  ConnectionManager::GetKeyFn getKey>
class AsyncBroadcastFactoryT: public CollectiveProtocolFactory
{
protected:
    ///
    /// \brief get geometry from comm id
    ///
    pami_mapidtogeometry_fn      _cb_geometry;

    ///
    /// \brief free memory pool for async broadcast operation objects
    ///
    CCMI::Adaptor::CollOpPoolT<pami_xfer_t,  T_Composite>   _free_pool;

    ///
    /// \brief memory allocator for early arrival descriptors
    ///
    PAMI::MemoryAllocator < sizeof(EADescriptor), 16 > _ead_allocator;

    ///
    /// \brief memory allocator for early arrival buffers
    ///
    PAMI::MemoryAllocator<32768, 16>                 _eab_allocator;

    T_Conn                                        * _cmgr;
    Interfaces::NativeInterface                   * _native;

public:
    AsyncBroadcastFactoryT (T_Conn                      *cmgr,
                            Interfaces::NativeInterface *native):
        CollectiveProtocolFactory(),
        _cmgr(cmgr),
        _native(native)
    {
        native->setMulticastDispatch(cb_async, this);
    }

    virtual ~AsyncBroadcastFactoryT ()
    {
    }

    /// NOTE: This is required to make "C" programs link successfully with virtual destructors
    void operator delete(void * p)
    {
        CCMI_abort();
    }

    //Override the connection manager in this call
    unsigned myGetKey   (unsigned                 root,
                         unsigned                 iconnid,
                         PAMI_GEOMETRY_CLASS    * geometry,
                         T_Conn                ** connmgr)
    {
        return getKey(root,
                      iconnid,
                      geometry,
                      (ConnectionManager::BaseConnectionManager**)connmgr);
    }

    virtual void metadata(pami_metadata_t *mdata)
    {
        TRACE_ADAPTOR((stderr, "<%p>AsyncBroadcastFactoryT::metadata()\n", this));
        DO_DEBUG((templateName<MetaDataFn>()));
        get_metadata(mdata);
    }

    char *allocateBuffer (unsigned size)
    {
        if (size <= 32768)
            return(char *)_eab_allocator.allocateObject();

        char *buf;
        pami_result_t rc;
        rc = __global.heap_mm->memalign((void **)&buf, 0, size);
        return rc == PAMI_SUCCESS ? buf : NULL;
    }

    void freeBuffer (unsigned size, char *buf)
    {
        if (size <= 32768)
            return _eab_allocator.returnObject(buf);

        __global.heap_mm->free(buf);
    }

    virtual Executor::Composite * generate(pami_geometry_t              g,
                                           void                      * cmd)
    {
        T_Composite* a_bcast = NULL;
        CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co = NULL;
        pami_broadcast_t *bcast_xfer = &((pami_xfer_t*)cmd)->cmd.xfer_broadcast;

        //CCMI_assert(bcast_xfer->typecount <= 32768);
        PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)g;
        T_Conn *cmgr = _cmgr;
        unsigned key = getKey(bcast_xfer->root,
                              (unsigned) - 1,
                              (PAMI_GEOMETRY_CLASS*)g,
                              (ConnectionManager::BaseConnectionManager**) & cmgr);

        //fprintf (stderr, "%d: Using Key %d\n", _native->myrank(), key);

        if (_native->myrank() == bcast_xfer->root)
        {
            co = _free_pool.allocate(key);
            pami_callback_t  cb_exec_done;
            cb_exec_done.function   = exec_done;
            cb_exec_done.clientdata = co;

            a_bcast = new (co->getComposite())
            T_Composite ( _native,
                          cmgr,
                          cb_exec_done,
                          (PAMI_GEOMETRY_CLASS *)g,
                          bcast_xfer->root,
                          bcast_xfer->buf,
                          bcast_xfer->typecount);

            co->setXfer((pami_xfer_t*)cmd);
            co->setFlag(LocalPosted);
            co->setFactory(this);

            //Use the Key as the connection ID
            if (cmgr == NULL)
                a_bcast->executor().setConnectionID(key);

            a_bcast->executor().start();
        }
        else
        {
            co = (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)geometry->asyncCollectiveUnexpQ().findAndDelete(key);

            /// Try to match in active queue
            if (co)
            {
                CCMI_assert(co->getFlags() & EarlyArrival);

                EADescriptor *ead = (EADescriptor *) co->getEAQ()->peekTail();
                CCMI_assert(ead != NULL);
                CCMI_assert(ead->bytes == bcast_xfer->typecount);
                CCMI_assert(ead->cdata._root == bcast_xfer->root);

                if (ead->flag == EACOMPLETED)
                {
                    if (bcast_xfer->typecount)
                    {
                        char *eab = ead->buf;
                        CCMI_assert(eab != NULL);
                        memcpy (bcast_xfer->buf, eab, bcast_xfer->typecount);
                        freeBuffer(bcast_xfer->typecount, eab);
                        //_eab_allocator.returnObject(eab);
                    }

                    ead->flag = EANODATA;
                    co->getEAQ()->popTail();
                    _ead_allocator.returnObject(ead);

                    if (((pami_xfer_t *)cmd)->cb_done)
                    {
                        ((pami_xfer_t *)cmd)->cb_done(NULL, ((pami_xfer_t *)cmd)->cookie, PAMI_SUCCESS);
                    }

                    _free_pool.free(co);
                }
                else
                {
                    co->setXfer((pami_xfer_t*)cmd);
                    co->setFlag(LocalPosted);
                    co->setFactory(this);

                    a_bcast = co->getComposite();
                }
            }
            /// not found posted CollOp object, create a new one and
            /// queue it in active queue
            else
            {
                co = _free_pool.allocate(key);
                pami_callback_t  cb_exec_done;
                cb_exec_done.function   = exec_done;
                cb_exec_done.clientdata = co;

                a_bcast = new (co->getComposite())
                T_Composite ( _native,
                              cmgr,
                              cb_exec_done,
                              (PAMI_GEOMETRY_CLASS *)g,
                              bcast_xfer->root,
                              bcast_xfer->buf,
                              bcast_xfer->typecount);

                co->setXfer((pami_xfer_t*)cmd);
                co->setFlag(LocalPosted);
                co->setFactory(this);

                //Use the Key as the connection ID
                if (cmgr == NULL)
                    a_bcast->executor().setConnectionID(key);

                geometry->asyncCollectivePostQ().pushTail(co);
            }

            //dev->unlock();
        }

        return NULL; //a_bcast;
    }

    static void cb_async
    (pami_context_t          ctxt,
     const pami_quad_t     * info,
     unsigned                count,
     unsigned                conn_id,
     size_t                  peer,
     size_t                  sndlen,
     void                  * arg,
     size_t                * rcvlen,
     pami_pipeworkqueue_t ** rcvpwq,
     pami_callback_t       * cb_done)
    {
        AsyncBroadcastFactoryT *factory = (AsyncBroadcastFactoryT *) arg;
        //fprintf(stderr, "%d: <%#.8X>Broadcast::AsyncBroadcastFactoryT::cb_async() connid %d\n",factory->_native->myrank(), (int)factory, conn_id);

        CollHeaderData *cdata = (CollHeaderData *) info;
        T_Composite* a_bcast = NULL;

        int comm = cdata->_comm;
        PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry (ctxt, comm);

        T_Conn *cmgr = factory->_cmgr;
        unsigned key = factory->myGetKey (cdata->_root, conn_id, geometry, &cmgr);
        CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co =
            (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *) geometry->asyncCollectivePostQ().findAndDelete(key);

        if (!co)
        {
            co = factory->_free_pool.allocate(key);
            pami_callback_t cb_exec_done;
            cb_exec_done.function = exec_done;
            cb_exec_done.clientdata = co;

            EADescriptor * ead = (EADescriptor *) factory->_ead_allocator.allocateObject();
            memcpy(&(ead->cdata), cdata, sizeof(cdata));
            ead->flag  = EASTARTED;
            ead->bytes = sndlen;

            if (sndlen)
            {
                ead->buf   = (char *)factory->allocateBuffer(sndlen);//_eab_allocator.allocateObject();
            }

            a_bcast = new (co->getComposite())
            T_Composite ( factory->_native,
                          cmgr,
                          cb_exec_done,
                          geometry,
                          cdata->_root,
                          ead->buf,
                          sndlen);

            co->getEAQ()->pushTail(ead);
            co->setFlag(EarlyArrival);
            co->setFactory (factory);

            //Use the Key as the connection ID
            if (cmgr == NULL)
                a_bcast->executor().setConnectionID(key);

            geometry->asyncCollectiveUnexpQ().pushTail(co);
        }
        else
        {
            // use type count for now, need datatype handling !!!
            // CCMI_assert (co->getXfer()->type != PAMI_TYPE_CONTIGUOUS);
            CCMI_assert (co->getXfer()->cmd.xfer_broadcast.typecount == sndlen);
            a_bcast = (T_Composite *) co->getComposite();
        }

        a_bcast->executor().notifyRecv(peer, *info, (PAMI::PipeWorkQueue **)rcvpwq, cb_done);

        //We only support sndlen == rcvlen
        * rcvlen  = sndlen;

        return;
    }

    static void exec_done (pami_context_t context, void *cd, pami_result_t err)
    {
        CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> * co =
            (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)cd;

        //fprintf (stderr, "%d: exec_done for key %d\n", ((AsyncBroadcastFactoryT *)co->getFactory())->_native->myrank(), co->key());

        unsigned     flag = co->getFlags();

        if (flag & LocalPosted)
        {
            pami_xfer_t *xfer = co->getXfer();
            pami_broadcast_t *bcast_xfer = &co->getXfer()->cmd.xfer_broadcast;

            EADescriptor *ead = (EADescriptor *) co->getEAQ()->popTail();
            AsyncBroadcastFactoryT *factory = (AsyncBroadcastFactoryT *)co->getFactory();

            if (flag & EarlyArrival)
            {
                CCMI_assert(ead != NULL);

                if (bcast_xfer->typecount)
                {
                    char *eab = ead->buf;
                    CCMI_assert(eab != NULL);
                    memcpy (bcast_xfer->buf, eab, bcast_xfer->typecount);
                    factory->freeBuffer(bcast_xfer->typecount, eab); //_eab_allocator.returnObject(eab);
                }

                ead->flag = EANODATA;
                ead->buf  = NULL;
                factory->_ead_allocator.returnObject(ead);
            }
            else
            {
                CCMI_assert(ead == NULL);
            }

            if (xfer->cb_done)
                xfer->cb_done(NULL, xfer->cookie, PAMI_SUCCESS);

            factory->_free_pool.free(co);
        }
        else if (flag & EarlyArrival)
        {
            EADescriptor *ead = (EADescriptor *) co->getEAQ()->peekTail();
            ead->flag = EACOMPLETED;
        }
        else
        {
            CCMI_assert(0);
        }
    }

}; //- Async Composite Factory
};  //- end namespace Broadcast
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
