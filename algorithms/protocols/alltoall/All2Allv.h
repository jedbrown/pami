/**
 * \file algorithms/protocols/alltoall/All2Allv.h
 * \brief Simple composite based on multicast
 */
#ifndef __algorithms_protocols_alltoall_All2Allv_h__
#define __algorithms_protocols_alltoall_All2Allv_h__

#include "algorithms/composite/Composite.h"
#include "util/ccmi_util.h"
#include "components/devices/ManytomanyModel.h"

namespace CCMI
{
  namespace Adaptor
  {
    class All2AllvProtocol: public CCMI::Executor::Composite
    {
      protected:
        Interfaces::NativeInterface *_native;
        PAMI_GEOMETRY_CLASS   * _geometry;
        pami_manytomanybuf_t    _send;
        pami_manytomanybuf_t    _recv;
        pami_manytomany_t       _m2m_info;
        pami_callback_t         _my_cb_done;
        pami_callback_t         _app_cb_done;
        //size_t _my_index;
        size_t                * _sendinit;
        size_t                * _recvinit;
        unsigned                _donecount;
        PAMI::M2MPipeWorkQueue  _sendpwq;
        PAMI::M2MPipeWorkQueue  _recvpwq;
        CollHeaderData          _metadata;
      public:
        All2AllvProtocol() {};
        All2AllvProtocol(Interfaces::NativeInterface *mInterface,
                        CCMI::ConnectionManager::CommSeqConnMgr *cmgr,
                        pami_geometry_t g,
                        pami_xfer_t *coll,
                        pami_callback_t cb_done):
            CCMI::Executor::Composite(),
            _native(mInterface),
            _geometry((PAMI_GEOMETRY_CLASS*)g),
            _app_cb_done(cb_done)
        {
          //pami_task_t self = __global.mapping.task();
          PAMI::Topology * all = (PAMI::Topology *)_geometry->getTopology(0);
          size_t topo_size = all->size();
          TRACE_ADAPTOR((stderr, "<%p>All2AllvProtocol size %zu\n", this, topo_size));
          //_my_index = all->rank2Index(self);


          _my_cb_done.function = a2aDone;
          _my_cb_done.clientdata = this;
          _donecount = 0;

          /// \todo only supporting PAMI_BYTE right now
          //PAMI_Type_sizeof(coll->cmd.xfer_alltoallv.stype);
          PAMI_assert(coll->cmd.xfer_alltoallv.stype == PAMI_BYTE);

          /// \todo presumed size of PAMI_BYTE?
          //size_t bytes = topo_size * coll->cmd.xfer_alltoallv.stypecount * 1;

          _sendinit = (size_t*) malloc(sizeof(size_t) * topo_size);
          PAMI_assert(_sendinit);

          for (size_t i = 0; i < topo_size; ++i)
            {
              _sendinit[i] = coll->cmd.xfer_alltoallv.stypecounts[i];
            }

          _send.buffer = &_sendpwq;
          _send.buffer->configure(NULL,
                                  coll->cmd.xfer_alltoallv.sndbuf,
                                  topo_size,
                                  &coll->cmd.xfer_alltoallv.stype,
                                  coll->cmd.xfer_alltoallv.sdispls,
                                  coll->cmd.xfer_alltoallv.stypecounts,
                                  _sendinit);

          _send.participants = all;

          _recvinit = (size_t*) malloc(sizeof(size_t) * topo_size);
          PAMI_assert(_recvinit);
          memset(_recvinit, 0x00, sizeof(size_t)*topo_size);

          _recv.buffer = &_recvpwq;
          _recv.buffer->configure(NULL,
                                  coll->cmd.xfer_alltoallv.rcvbuf,
                                  topo_size,
                                  &coll->cmd.xfer_alltoallv.rtype,
                                  coll->cmd.xfer_alltoallv.rdispls,
                                  coll->cmd.xfer_alltoallv.rtypecounts,
                                  _recvinit);

          _recv.participants = all;

          _m2m_info.send = _send;

          // only comm is used in the header
          _metadata._root  = -1U;
          _metadata._comm  = _geometry->comm();
          _metadata._count = -1U;
          _metadata._phase = 0;
          _metadata._iteration  = 0;
          _metadata._op    = 0;
          _metadata._dt    = 0;

          _m2m_info.msginfo = (pami_quad_t*) & _metadata;
          _m2m_info.msgcount = 1;

          _m2m_info.roles = -1U;

          _m2m_info.client = 0; /// \todo does NOT support multiclient
          _m2m_info.context = 0; /// \todo does NOT support multicontext

          unsigned comm = _geometry->comm();
          _m2m_info.connection_id = cmgr->getConnectionId_impl(comm, -1, 0, 0, -1);
          TRACE_ADAPTOR((stderr, "<%p>All2AllvProtocol::All2AllvProtocol() connection_id %u\n", this, _m2m_info.connection_id));
          cmgr->updateConnectionId(comm);

          _m2m_info.cb_done.function   = a2aDone;
          _m2m_info.cb_done.clientdata = this;

        }

        virtual void start()
        {
          TRACE_ADAPTOR((stderr, "<%p>All2AllvProtocol::start()\n", this));

          // Start the barrier. When it completes, it will start the m2m
          CCMI::Executor::Composite *barrier = (CCMI::Executor::Composite *)
                                               _geometry->getKey((size_t)0, /// \todo does NOT support multicontext
                                                                 PAMI::Geometry::PAMI_CKEY_BARRIERCOMPOSITE1);
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
          All2AllvProtocol *a2a = (All2AllvProtocol*) arg;
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
          All2AllvProtocol *a2a = (All2AllvProtocol *) arg;
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
              free(_sendinit);
              free(_recvinit);
            }
        }

    };


    template <class T_Composite, MetaDataFn get_metadata, class C>
    class All2AllvFactoryT: public CollectiveProtocolFactory
    {
      protected:
        pami_mapidtogeometry_fn _cb_geometry;
        C *_cmgr;
        Interfaces::NativeInterface *_native;
        pami_dispatch_manytomany_fn _fn;
        CCMI::Adaptor::CollOpPoolT<pami_xfer_t, T_Composite> _free_pool;
      public:
        All2AllvFactoryT(C *cmgr,
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

        unsigned getKey(PAMI_GEOMETRY_CLASS *g, C **cmgr)
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

        static void cb_manytomany(void *arg,
                                  unsigned conn_id,
                                  pami_quad_t *msginfo,
                                  unsigned msgcount,
                                  pami_manytomanybuf_t **recv,
                                  pami_callback_t *cb_done)
        {
          TRACE_ADAPTOR((stderr, "<%p>All2AllvFactoryT::cb_manytomany() conn_id %u, msginfo %p, msgcount %u, recv %p\n", arg, conn_id, msginfo, msgcount, recv));
          All2AllvFactoryT *factory = (All2AllvFactoryT *) arg;
          CollHeaderData *md = (CollHeaderData *) msginfo;
          PAMI_assert(msgcount >= sizeof(CollHeaderData) / (sizeof(pami_quad_t)));
          All2AllvProtocol *a2a = NULL;
          int comm = md->_comm;
          PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)
                                          PAMI_GEOMETRY_CLASS::getCachedGeometry(comm);

          if (!geometry)
            {
              geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry(comm);
              PAMI_GEOMETRY_CLASS::updateCachedGeometry(geometry, comm);
            }

          C *cmgr = factory->_cmgr;
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
