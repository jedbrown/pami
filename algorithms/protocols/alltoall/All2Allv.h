/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/alltoall/All2Allv.h
 * \brief Simple composite based on multicast
 */
#ifndef __algorithms_protocols_alltoall_All2Allv_h__
#define __algorithms_protocols_alltoall_All2Allv_h__

#include "Global.h"
#include "algorithms/composite/Composite.h"
#include "util/ccmi_util.h"
#include "components/devices/ManytomanyModel.h"

namespace CCMI
{
  namespace Adaptor
  {
    template<typename M_Int>
    class VType
    {
    public:
      static PAMI::M2MType type;
    };
    template<>
    PAMI::M2MType VType<int>::type=PAMI::M2M_VECTOR_INT;
    template<>
    PAMI::M2MType VType<size_t>::type=PAMI::M2M_VECTOR_LONG;

    template <typename T_Int>
    class All2AllvProtocolT: public CCMI::Executor::Composite
    {

      // Join int/size_t into one T_Int xfer struct
      typedef struct
      {
        char                    * sndbuf;
        pami_type_t               stype;
        T_Int                   * stypecounts;
        T_Int                   * sdispls;
        char                    * rcvbuf;
        pami_type_t               rtype;
        T_Int                   * rtypecounts;
        T_Int                   * rdispls;
      } xfer_union_t;
    protected:
      Interfaces::NativeInterface *_native;
      PAMI_GEOMETRY_CLASS   * _geometry;
      pami_manytomanybuf_t    _send;
      pami_manytomanybuf_t    _recv;
      pami_manytomany_t       _m2m_info;
      pami_callback_t         _my_cb_done;
      pami_callback_t         _app_cb_done;
      T_Int                  * _sendinit;
      T_Int                  * _recvinit;
      void                   *_initbuf;
      unsigned                _donecount;
      PAMI::M2MPipeWorkQueueT<T_Int, 0>  _sendpwq;
      PAMI::M2MPipeWorkQueueT<T_Int, 0>  _recvpwq;
      //CollHeaderData          _metadata;
    public:
      All2AllvProtocolT<T_Int>()
      {
      };
      All2AllvProtocolT<T_Int>(Interfaces::NativeInterface *mInterface,
                               CCMI::ConnectionManager::CommSeqConnMgr *cmgr,
                               pami_geometry_t g,
                               pami_xfer_t *coll,
                               pami_callback_t cb_done):
      CCMI::Executor::Composite(),
      _native(mInterface),
      _geometry((PAMI_GEOMETRY_CLASS*)g),
      _app_cb_done(cb_done)
      {
        // The T_Int struct idea only works if they all match - so assert on size
        COMPILE_TIME_ASSERT((sizeof(xfer_union_t) == sizeof(pami_alltoallv_t)) && (sizeof(xfer_union_t) == sizeof(pami_alltoallv_int_t)));
        xfer_union_t *xfer_union;
        xfer_union = (xfer_union_t *)&coll->cmd.xfer_alltoallv;

        //pami_task_t self = __global.mapping.task();
        PAMI::Topology * all = (PAMI::Topology *)_geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX);
        size_t topo_size = all->size();
        TRACE_ADAPTOR((stderr, "<%p>All2AllvProtocol size %zu\n", this, topo_size));
        //_my_index = all->rank2Index(self);


        _my_cb_done.function = a2aDone;
        _my_cb_done.clientdata = this;
        _donecount = 0;

        /// \todo only supporting PAMI_TYPE_BYTE right now
        //PAMI_Type_sizeof(xfer_union->stype);
        PAMI_assert(xfer_union->stype == PAMI_TYPE_BYTE);

        /// \todo presumed size of PAMI_TYPE_BYTE?
        //size_t bytes = topo_size * xfer_union->stypecount * 1;

        pami_result_t rc;
        rc = __global.heap_mm->memalign(&_initbuf, 0, 2 * sizeof(T_Int) * topo_size);
        _sendinit =  (T_Int *) _initbuf;
        _recvinit =  (T_Int *) ((char *)_initbuf + sizeof(T_Int) * topo_size);
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc init bufs");

        for (size_t i = 0; i < topo_size; ++i)
        {
          _sendinit[i] = xfer_union->stypecounts[i];
          _recvinit[i] = 0;
        }

        _send.type   = VType<T_Int>::type;
        _send.buffer = &_sendpwq;
        _sendpwq.configure(
                          xfer_union->sndbuf,
                          topo_size,
                          &xfer_union->stype,
                          xfer_union->sdispls,
                          xfer_union->stypecounts,
                          _sendinit);
#if 0
        for(size_t i = 0; i<topo_size; ++i)
        {
          fprintf(stderr,"_sendinit[%zu] %zu\n",i,(size_t)_sendinit[i]);
          char*  tmp      = _sendpwq.bufferToConsume(i);         
          size_t tmpbytes = _sendpwq.bytesAvailableToConsume(i); 
          fprintf(stderr,"sendpwq(%zu) buffer %p, bytes %zu\n",i, tmp, tmpbytes);
        }
#endif
        _send.participants = all;

        _recv.type   = VType<T_Int>::type;
        _recv.buffer = &_recvpwq;
        _recvpwq.configure(
                          xfer_union->rcvbuf,
                          topo_size,
                          &xfer_union->rtype,
                          xfer_union->rdispls,
                          xfer_union->rtypecounts,
                          _recvinit);
#if 0
        for(size_t i = 0; i<topo_size; ++i)
        {
          char*  tmp      = _recvpwq.bufferToProduce(i);         
          size_t tmpbytes = _recvpwq.bytesAvailableToProduce(i); 
          fprintf(stderr,"recvpwq(%zu) buffer %p, bytes %zu\n",i, tmp, tmpbytes);
        }
#endif
        _recv.participants = all;

        _m2m_info.send = _send;

        // only comm is used in the header
        //_metadata._root  = -1U;
        //_metadata._comm  = _geometry->comm();
        //_metadata._count = -1U;
        //_metadata._phase = 0;
        //_metadata._iteration  = 0;
        //_metadata._op    = 0;
        //_metadata._dt    = 0;

        _m2m_info.msginfo  = NULL; //(pami_quad_t*) & _metadata;
        _m2m_info.msgcount = 0;   //1;

        _m2m_info.roles = -1U;

        _m2m_info.client = 0; /// \todo does NOT support multiclient
        _m2m_info.context = 0; /// \todo does NOT support multicontext

        unsigned comm = _geometry->comm();
        _m2m_info.connection_id = comm; //cmgr->getConnectionId_impl(comm, -1, 0, 0, -1);
        TRACE_ADAPTOR((stderr, "<%p>All2AllvProtocol::All2AllvProtocol() connection_id %u\n", this, _m2m_info.connection_id));
        //cmgr->updateConnectionId(comm);

        _m2m_info.cb_done.function   = a2aDone;
        _m2m_info.cb_done.clientdata = this;

      }

      virtual void start()
      {
        TRACE_ADAPTOR((stderr, "<%p>All2AllvProtocol::start()\n", this));

        // Start the barrier. When it completes, it will start the m2m
        CCMI::Executor::Composite *barrier = (CCMI::Executor::Composite *)
                                             _geometry->getKey((size_t)0, /// \todo does NOT support multicontext
                                                               PAMI::Geometry::CKEY_OPTIMIZEDBARRIERCOMPOSITE);
        CCMI_assert(barrier != NULL);
        barrier->setDoneCallback (cb_barrier_done, this);
        //barrier->setConsistency (consistency);
        barrier->start();

      }
      void startA2A()
      {
        TRACE_ADAPTOR((stderr, "<%p>All2AllvProtocol::startA2A()\n", this));
        _native->manytomany(&_m2m_info);
      }
      static void cb_barrier_done(pami_context_t context,
                                  void *arg,
                                  pami_result_t err)
      {
        TRACE_ADAPTOR((stderr, "<%p>All2AllvProtocol::cb_barrier_done\n", arg));
        All2AllvProtocolT<T_Int> *a2a = (All2AllvProtocolT<T_Int> *) arg;
        CCMI_assert(a2a != NULL);

        // Barrier is done, start the active message a2a
        a2a->startA2A();
      }


      void notifyRecv(pami_manytomanybuf_t **recv,
                      pami_callback_t *cb_done)
      {
        TRACE_ADAPTOR((stderr, "<%p>All2AllvProtocol::notifyRecv() recv %p\n", this, recv));
        *recv = & _recv;
        *cb_done = _my_cb_done;
      }

      static void a2aDone(pami_context_t context,
                          void *arg,
                          pami_result_t err)
      {
        TRACE_ADAPTOR((stderr, "<%p>All2AllvProtocol::a2aDone()\n", arg));
        All2AllvProtocolT<T_Int> *a2a = (All2AllvProtocolT<T_Int> *) arg;
        CCMI_assert(a2a != NULL);

        a2a->done(context, err);
      }

      void done(pami_context_t context,
                pami_result_t err)
      {
        TRACE_ADAPTOR((stderr, "<%p>All2AllvProtocol::done() count %u\n", this, _donecount));
        _donecount++;

        if ((_donecount == 2) && (_app_cb_done.function))
        {
          _app_cb_done.function (context,
                                 _app_cb_done.clientdata,
                                 err);
          /// \todo allocator?  reuse from factory?
          __global.heap_mm->free(_initbuf);
        }
      }

    };

    typedef All2AllvProtocolT<int>       All2AllvProtocolInt;
    typedef All2AllvProtocolT<size_t>    All2AllvProtocolLong;

    template <class T_Composite, MetaDataFn get_metadata, class T_Conn>
    class All2AllvFactoryT: public CollectiveProtocolFactory
    {
    protected:
      pami_mapidtogeometry_fn _cb_geometry;
      T_Conn *_cmgr;
      Interfaces::NativeInterface *_native;
      pami_dispatch_manytomany_function _fn;
      CCMI::Adaptor::CollOpPoolT<pami_xfer_t, T_Composite> _free_pool;
    public:
      All2AllvFactoryT(T_Conn *cmgr,
                       Interfaces::NativeInterface *native):
      CollectiveProtocolFactory(),
      _cmgr(cmgr),
      _native(native)
      {
        TRACE_ADAPTOR((stderr, "<%p>All2AllvFactoryT\n", this));
        _fn = cb_manytomany;
        _native->setManytomanyDispatch(_fn, this);
      }

      virtual ~All2AllvFactoryT()
      {
        TRACE_ADAPTOR((stderr, "<%p>~All2AllvFactoryT\n", this));
      }


      void operator delete(void * p)
      {
        CCMI_abort();
      }

      unsigned getKey(PAMI_GEOMETRY_CLASS *g, T_Conn **cmgr)
      {
        return g->comm();
      }

      virtual void metadata(pami_metadata_t *mdata)
      {
        get_metadata(mdata);
      }

      virtual Executor::Composite * generate(pami_geometry_t g,
                                             void *op)
      {
        TRACE_ADAPTOR((stderr, "<%p>All2AllvFactoryT::generate()\n", this));
        T_Composite *a2a = NULL;
        pami_callback_t cb_exec_done;
        PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) g;
        CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *coll_object = NULL;

        unsigned key = getKey((PAMI_GEOMETRY_CLASS*) g, &_cmgr);
        coll_object = _free_pool.allocate(key);

        cb_exec_done.function = exec_done;
        cb_exec_done.clientdata = (void *) coll_object;

        a2a = new (coll_object->getComposite())
              T_Composite(_native,
                          _cmgr,
                          (PAMI_GEOMETRY_CLASS *) g,
                          (pami_xfer_t *)op,
                          cb_exec_done);

        coll_object->setXfer((pami_xfer_t *)op);
        coll_object->setFlag(LocalPosted);
        coll_object->setFactory(this);
        geometry->asyncCollectivePostQ().pushTail(coll_object);
        TRACE_ADAPTOR((stderr, "<%p>All2AllvFactoryT::generate() key %u, coll_object %p, a2a %p\n", this, key, coll_object, a2a));
        return a2a;
      }

      static void cb_manytomany(pami_context_t ctxt,
                                void *arg,
                                unsigned conn_id,
                                pami_quad_t *msginfo,
                                unsigned msgcount,
                                pami_manytomanybuf_t **recv,
                                pami_callback_t *cb_done)
      {
        TRACE_ADAPTOR((stderr, "<%p>All2AllvFactoryT::cb_manytomany() conn_id %u, msginfo %p, msgcount %u, recv %p\n", arg, conn_id, msginfo, msgcount, recv));
        All2AllvFactoryT *factory = (All2AllvFactoryT *) arg;
        //CollHeaderData *md = (CollHeaderData *) msginfo;
        //PAMI_assert(msgcount >= sizeof(CollHeaderData) / (sizeof(pami_quad_t)));
        T_Composite *a2a = NULL;
        int comm = conn_id; //md->_comm;
        PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry(ctxt, comm);

        T_Conn *cmgr = factory->_cmgr;
        unsigned key = factory->getKey(geometry, &cmgr);

        CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *coll_object =
        (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)
        geometry->asyncCollectivePostQ().findAndDelete(key);
        a2a = (T_Composite *) coll_object->getComposite();
        TRACE_ADAPTOR((stderr, "<%p>All2AllvFactoryT::cb_manytomany() key %u, coll_object %p, a2a %p\n", arg, key, coll_object, a2a));
        a2a->notifyRecv(recv, cb_done);
      }


      static void exec_done(pami_context_t context,
                            void *coll_obj,
                            pami_result_t err)
      {
        TRACE_ADAPTOR((stderr, "<%p>All2AllvFactoryT::exec_done()\n", coll_obj));
        CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *coll_object =
        (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *) coll_obj;
        unsigned flag = coll_object->getFlags();

        if (flag & LocalPosted)
        {
          pami_xfer_t *xfer = coll_object->getXfer();

          if (xfer->cb_done)
            xfer->cb_done(NULL, xfer->cookie, PAMI_SUCCESS);

          All2AllvFactoryT *factory = (All2AllvFactoryT *)
                                      coll_object->getFactory();
          factory->_free_pool.free(coll_object);
        }
      }
    }; //- All2AllvFactoryT


  };
};

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
