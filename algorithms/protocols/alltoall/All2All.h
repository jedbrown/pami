/**
 * \file algorithms/protocols/broadcast/MultiCastComposite.h
 * \brief Simple composite based on multicast
 */
#ifndef __algorithms_protocols_all2all_h__
#define __algorithms_protocols_all2all_h__

#include "algorithms/composite/Composite.h"
#include "util/ccmi_util.h"


namespace CCMI
{
  namespace Adaptor
  {
    class All2AllProtocol: public CCMI::Executor::Composite
    {
    protected:
      Interfaces::NativeInterface *_native;
      PAMI_GEOMETRY_CLASS *_geometry;
      pami_manytomanybuf_t _send;
      pami_manytomanybuf_t _recv;
      pami_multisync_t _msync;
      pami_manytomany_t _m2m_info;
      pami_callback_t _my_cb_done;
      pami_callback_t _app_cb_done;
      size_t _my_index;    
      unsigned _donecount;

    public:
      All2AllProtocol() {};
    All2AllProtocol(Interfaces::NativeInterface *mInterface,
                    CCMI::ConnectionManager::CommSeqConnMgr *cmgr,
                    pami_geometry_t g,
                    pami_xfer_t *coll,
                    pami_callback_t cb_done):
      CCMI::Executor::Composite(),
        _native(mInterface),
        _app_cb_done(cb_done),
        _geometry((PAMI_GEOMETRY_CLASS*)g)
        {
          pami_task_t self = __global.mapping.task();
          pami_topology_t * all = _geometry->getTopology(0);
          size_t topo_size = PAMI_Topology_size(all);
          _my_index = PAMI_Topology_taskID2Index(all, self);

        
          _my_cb_done.function = a2aDone;
          _my_cb_done.clientdata = this;
          _donecount = 0;

          /// \todo only supporting PAMI_BYTE right now
          PAMI_Type_sizeof(coll->cmd.xfer_alltoall.stype);

          /// \todo presumed size of PAMI_BYTE?
          size_t bytes = topo_size * coll->cmd.xfer_alltoall.stypecount * 1;

          PAMI_PipeWorkQueue_config_flat(_send.buffer,
                                         coll->cmd.xfer_alltoall.sndbuf,
                                         bytes,
                                         bytes);
          PAMI_PipeWorkQueue_reset(_send.buffer);
          _send.participants = all;
          _send.num_vecs = 1;
          _send.lengths = &(coll->cmd.xfer_alltoall.stypecount);
          _send.offsets = (size_t *) NULL;
        
          PAMI_PipeWorkQueue_config_flat(_recv.buffer,
                                         coll->cmd.xfer_alltoall.rcvbuf,
                                         bytes,
                                         0);
          PAMI_PipeWorkQueue_reset(_recv.buffer);
          _recv.participants = all;
          _recv.num_vecs = 1;
          _recv.lengths = &(coll->cmd.xfer_alltoall.rtypecount);
          _recv.offsets = (size_t *) NULL;

          _m2m_info.send = _send;
          _m2m_info.metadata = (pami_quad_t*) NULL;
          _m2m_info.metacount = 0;
          _m2m_info.num_index = 1;
          _m2m_info.taskIndex = &_my_index;
          _m2m_info.client = 0;
          _m2m_info.context = 0; /// \todo ?
          _m2m_info.connection_id = 0; /// \todo ?
          _m2m_info.roles = -1U;

          // Initialize the msync
          _msync.client = 0;
          _msync.context = 0; /// \todo ?
          _msync.cb_done.function = cb_msync_done;
          _msync.cb_done.clientdata = this;
          _msync.connection_id = 0; /// \todo ?
          _msync.roles = -1U;
          _msync.participants = all;
        }

      virtual void start()
      {
        _m2m_info.cb_done.function   = a2aDone;
        _m2m_info.cb_done.clientdata = this;

        // Start the msync. When it completes, it will start the mcast.
        _native->multisync(&_msync);
      }
      void startA2A()
      {
        _native->manytomany(&_m2m_info);
      }
      static void cb_msync_done(pami_context_t context,
                                void *arg,
                                pami_result_t err)
      {
        All2AllProtocol *a2a = (All2AllProtocol*) arg;
        CCMI_assert(a2a != NULL);
        
        // Msync is done, start the active message a2a
        a2a->startA2A();
      }


      void notifyRecv(pami_manytomanybuf_t **recv,
                      size_t *myIndex,
                      pami_callback_t *cb_done)
      {
        *myIndex = _my_index;
        *recv = (pami_manytomanybuf_t*) &_recv;
        cb_done->function   = _my_cb_done.function;
        cb_done->clientdata = _my_cb_done.clientdata;
      }
      
      static void a2aDone(pami_context_t context,
                          void *arg,
                          pami_result_t err)
      {
        All2AllProtocol *a2a = (All2AllProtocol *) arg;
        CCMI_assert(a2a != NULL);
        a2a->_donecount++;
        if((a2a->_donecount == 2) && (a2a->_app_cb_done.function))
          a2a->_app_cb_done.function (NULL,
                                      a2a->_app_cb_done.clientdata,
                                      PAMI_SUCCESS);
      }
      
    };


    template <class T_Composite, MetaDataFn get_metadata, class C>
      class All2AllFactoryT: public CollectiveProtocolFactory
    {
    protected:
      pami_mapidtogeometry_fn _cb_geometry;
      C *_cmgr;
      Interfaces::NativeInterface *_native;
      pami_dispatch_callback_fn _fn;
      CCMI::Adaptor::CollOpPoolT<pami_xfer_t, T_Composite> _free_pool;
    public:
    All2AllFactoryT(C *cmgr,
                     Interfaces::NativeInterface *native):
      CollectiveProtocolFactory(),
        _cmgr(cmgr),
        _native(native)  
        {
          _fn.manytomany = cb_manytomany;
          _native->setDispatch(_fn, this);
        }
      
      virtual ~All2AllFactoryT()
      {
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
        return a2a;
      }

      static void cb_manytomany(void *arg,
                                unsigned conn_id,
                                pami_quad_t *metadata,
                                unsigned metacount,
                                pami_manytomanybuf_t **recv,
                                size_t *myIndex,
                                pami_callback_t *cb_done)
      {
        All2AllFactoryT *factory = (All2AllFactoryT *) arg;
        CollHeaderData *md = (CollHeaderData *) metadata;
        All2AllProtocol *a2a = NULL;
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
        a2a->notifyRecv(recv, myIndex, cb_done);
      }


      static void exec_done(pami_context_t context,
                            void *coll_obj,
                            pami_result_t err)
      {
        CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *coll_object =
          (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *) coll_obj;
        unsigned flag = coll_object->getFlags();
        if (flag & LocalPosted)
        {
          pami_xfer_t *xfer = coll_object->getXfer();
          if (xfer->cb_done)
            xfer->cb_done(NULL, xfer->cookie, PAMI_SUCCESS);
          All2AllFactoryT *factory = (All2AllFactoryT *)
            coll_object->getFactory();
          factory->_free_pool.free(coll_object);
        }
      }
    }; //- All2AllFactoryT

    
  };
};

#endif
