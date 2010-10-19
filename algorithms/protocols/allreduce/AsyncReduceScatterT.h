/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/AsyncReduceScatterT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_allreduce_AsyncReduceScatterT_h__
#define __algorithms_protocols_allreduce_AsyncReduceScatterT_h__

#include "algorithms/ccmi.h"
#include "algorithms/executor/Allreduce.h"
#include "algorithms/executor/Scatter.h"
#include "algorithms/connmgr/CommSeqConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/CollOpT.h"

#ifdef DEBUG
#undef DEBUG
#define DEBUG(x)  // fprintf x
#else
#define DEBUG(x)  // fprintf x
#endif

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {

      ///
      /// \brief Asyc Allreduce Composite. It is single color right now
      ///
      template <class T_Reduce_Schedule, class T_Scatter_Schedule, class T_Conn, ScheduleFn create_schedule>
	class AsyncReduceScatterT : public CCMI::Executor::Composite
      {

      protected:
        CCMI::Executor::AllreduceBaseExec<T_Conn>  _reduce_executor;
        T_Reduce_Schedule                          _reduce_schedule;
        CCMI::Executor::ScatterExec<T_Conn, T_Scatter_Schedule, pami_scatterv_t>
                                                   _scatter_executor;
        T_Scatter_Schedule                         _scatter_schedule;
        T_Conn                                     *_cmgr;
        size_t                                     *_sdispls;
        size_t                                     *_scounts;
        char                                       *_tmpbuf;
        char                                       *_relbuf;
        int                                        _scatter_done;
        int                                        _reduce_done;

      public:

        ///
        /// \brief Constructor
        ///
        AsyncReduceScatterT () {};
        AsyncReduceScatterT (Interfaces::NativeInterface   * native,
                         T_Conn                        * cmgr,
                         pami_callback_t                  cb_done,
                         PAMI_GEOMETRY_CLASS            * geometry,
                         void                           *cmd) :
        Executor::Composite(),
        _reduce_executor (native, cmgr, geometry->comm()),
        _reduce_schedule (native->myrank(), (PAMI::Topology*)geometry->getTopology(0), 0),
        _scatter_executor (native, cmgr, geometry->comm(), (PAMI::Topology*)geometry->getTopology(0)),
        _cmgr(cmgr),
        _sdispls(NULL),
        _scounts(NULL),
        _tmpbuf(NULL),
        _scatter_done(0),
        _reduce_done(0)
        {
          TRACE_ADAPTOR ((stderr, "<%p>Allreduce::AsyncReduceScatterT() \n",this));

          PAMI::Topology *topo = (PAMI::Topology*)geometry->getTopology(0);
          unsigned root = topo->index2Rank(0);
          coremath func;
          unsigned sizeOfType;
          pami_reduce_scatter_t *a_xfer = (pami_reduce_scatter_t *)&(((pami_xfer_t *)cmd)->cmd.xfer_reduce_scatter);

          CCMI::Adaptor::Allreduce::getReduceFunction(a_xfer->dt,a_xfer->op,a_xfer->stypecount,sizeOfType,func);
          // unsigned bytes = sizeOfType * a_xfer->stypecount;
          _reduce_executor.setRoot(root);

          prepReduceBuffers(a_xfer->sndbuf, a_xfer->rcvbuf, a_xfer->stypecount, native->myrank() == root);

          _reduce_executor.setDoneCallback (cb_done.function, cb_done.clientdata);

          COMPILE_TIME_ASSERT(sizeof(_reduce_schedule) >= sizeof(T_Reduce_Schedule));
          _reduce_executor.setSchedule (&_reduce_schedule, 0);
          _reduce_executor.setReduceInfo(a_xfer->stypecount / sizeOfType, a_xfer->stypecount, sizeOfType, func, a_xfer->op, a_xfer->dt);

          _reduce_executor.reset();

          _scatter_executor.setRoot(root);

          COMPILE_TIME_ASSERT(sizeof(_scatter_schedule) >= sizeof(T_Scatter_Schedule));
          create_schedule(&_scatter_schedule, sizeof(_scatter_schedule), root, native, geometry);
          _scatter_executor.setSchedule (&_scatter_schedule);
        }


        AsyncReduceScatterT (Interfaces::NativeInterface   * native,
                         T_Conn                        * cmgr,
                         pami_callback_t                  cb_done,
                         PAMI_GEOMETRY_CLASS            * geometry,
                         char                           * sndbuf,
                         char                           * rcvbuf,
                         size_t                         dt_count,
                         pami_dt                        dt,
                         pami_op                        op ) :
        Executor::Composite(),
        _reduce_executor (native, cmgr, geometry->comm()),
        _reduce_schedule (native->myrank(), (PAMI::Topology*)geometry->getTopology(0), 0),
        _scatter_executor (native, cmgr, geometry->comm(), (PAMI::Topology*)geometry->getTopology(0)),
        _cmgr(cmgr),
        _sdispls(NULL),
        _scounts(NULL),
        _tmpbuf(NULL),
        _scatter_done(0),
        _reduce_done(0)
        {
          TRACE_ADAPTOR ((stderr, "<%p>Allreduce::AsyncReduceScatterT() \n",this));
          unsigned sizeOfType;
          coremath func;
          PAMI::Topology *topo = (PAMI::Topology*)geometry->getTopology(0);
          unsigned root = topo->index2Rank(0);

          _reduce_executor.setRoot(root);

          CCMI::Adaptor::Allreduce::getReduceFunction(dt,op,dt_count,sizeOfType,func);
          unsigned bytes = dt_count * sizeOfType;
          _reduce_executor.setBuffers (sndbuf,rcvbuf, bytes);
          _reduce_executor.setDoneCallback (cb_done.function, cb_done.clientdata);

          COMPILE_TIME_ASSERT(sizeof(_reduce_schedule) >= sizeof(T_Reduce_Schedule));
          _reduce_executor.setSchedule (&_reduce_schedule, 0);
          _reduce_executor.setReduceInfo(dt_count, bytes, sizeOfType, func, op, dt);

          _reduce_executor.reset();

          _scatter_executor.setRoot(root);

          COMPILE_TIME_ASSERT(sizeof(_scatter_schedule) >= sizeof(T_Scatter_Schedule));
          create_schedule(&_scatter_schedule, sizeof(_scatter_schedule), root, native, geometry);
          _scatter_executor.setSchedule (&_scatter_schedule);

        }

        ~AsyncReduceScatterT ()
        {
          free(_sdispls);
          free(_scounts);
          free(_tmpbuf);
        }


        CCMI::Executor::AllreduceBaseExec<T_Conn> &getReduceExecutor()
        {
          return _reduce_executor;
        }

        CCMI::Executor::ScatterExec<T_Conn, T_Scatter_Schedule,  pami_scatterv_t> &getScatterExecutor()
        {
          return _scatter_executor;
        }

        void prepReduceBuffers(char *sndbuf, char *rcvbuf, size_t bytes, bool root)
        {
           if (root)
           {
              _tmpbuf = (char *) malloc(bytes);
              _relbuf = rcvbuf;
              _reduce_executor.setBuffers (sndbuf, _tmpbuf, bytes);
           }
           else
           {
             _reduce_executor.setBuffers(sndbuf, rcvbuf, bytes);
           }
        }

        void setScatterExecutor (char *sbuf, char *rbuf, size_t rtypecount, size_t *stypecounts, 
                                 pami_dt dt, pami_op op, unsigned counts, bool root, 
                                 pami_callback_t  cb_done)
        {

          pami_scatterv_t s_xfer;
          coremath func;
          unsigned sizeOfType;

          CCMI::Adaptor::Allreduce::getReduceFunction(dt,op,rtypecount,sizeOfType,func);

          _sdispls = (size_t *) malloc(counts * sizeof(size_t));
          _scounts = (size_t *) malloc(counts * sizeof(size_t));
          _sdispls[0] = 0;
          _scounts[0] = stypecounts[0] * sizeOfType;
          for (unsigned i = 0; i < counts; ++i)
          {
            _scounts[i+1] = stypecounts[i+1] * sizeOfType;
            _sdispls[i+1] = _sdispls[i] + stypecounts[i] * sizeOfType;
            //fprintf(stderr, "setScatterExecutor - _sdispls[%d] = %d, _scounts[%d] = %d\n", i+1, _sdispls[i+1], i+1, _scounts[i+1]);
          }

          s_xfer.stypecounts = _scounts;
          s_xfer.sdispls     = _sdispls;

          _scatter_executor.setRoot (0);
          _scatter_executor.setSchedule (&_scatter_schedule);
          _scatter_executor.setBuffers (sbuf, rbuf, rtypecount);
          if (root)
           {
             _scatter_executor.setBuffers(_tmpbuf, _relbuf, rtypecount);
           }
           else
           {
             _scatter_executor.setBuffers(sbuf, rbuf, rtypecount);
           }
          _scatter_executor.setVectors (&s_xfer);

          _scatter_executor.setDoneCallback (cb_done.function, cb_done.clientdata);

        }

        int &scatter_done_flag()
        {
          return _scatter_done;
        }

        int &reduce_done_flag()
        {
          return _reduce_done;
        }

        T_Conn *connmgr()
        {
          return _cmgr;
        }

      }; //- AsyncReduceScatterT

      template <class T_Composite, MetaDataFn get_metadata, class C, ConnectionManager::GetKeyFn getKey>
        class AsyncReduceScatterFactoryT: public CollectiveProtocolFactory
	{
	protected:
	  ///
	  /// \brief get geometry from comm id
	  ///
	  pami_mapidtogeometry_fn      _cb_geometry;

	  ///
	  /// \brief free memory pool for async reduce scatter operation objects
	  ///
	  CCMI::Adaptor::CollOpPoolT<pami_xfer_t,  T_Composite>   _free_pool;

	  ///
	  /// \brief memory allocator for early arrival descriptors
	  ///
	  PAMI::MemoryAllocator<sizeof(EADescriptor), 16> _ead_allocator;

	  ///
	  /// \brief memory allocator for early arrival buffers
	  ///
	  PAMI::MemoryAllocator<32768,16>                 _eab_allocator;

	  C                                             * _cmgr;
	  Interfaces::NativeInterface                   * _native;

	public:
	  AsyncReduceScatterFactoryT (C                           *cmgr,
				  Interfaces::NativeInterface *native):
	  CollectiveProtocolFactory(),
	  _cmgr(cmgr),
	  _native(native)
	  {
	    native->setMulticastDispatch(cb_async, this);
	  }

	  virtual ~AsyncReduceScatterFactoryT ()
	  {
	  }

	  /// NOTE: This is required to make "C" programs link successfully with virtual destructors
	  void operator delete(void * p)
	  {
	    CCMI_abort();
	  }

	  virtual void metadata(pami_metadata_t *mdata)
	  {
	    // TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
	    DO_DEBUG((templateName<MetaDataFn>()));
	    get_metadata(mdata);
	  }

          C *getConnMgr()
          {
            return _cmgr;
          }

	  //Override the connection manager in this call
          /*
	  unsigned getKey   (unsigned                 root,
			     unsigned                 connid,
			     PAMI_GEOMETRY_CLASS    * geometry,
			     C                     ** connmgr)
	  {
	    CCMI_abort();
	    return root;
	  }
          */

	  char *allocateBuffer (unsigned size) {
	    if (size <= 32768)
	      return (char *)_eab_allocator.allocateObject();

	    char *buf = (char *)malloc(size);
	    return buf;
	  }

	  void freeBuffer (unsigned size, char *buf) {
	    if (size <= 32768)
	      return _eab_allocator.returnObject(buf);

	    free(buf);
	  }

	  virtual Executor::Composite * generate(pami_geometry_t              g,
						 void                      * cmd)
          {
	    PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)g;
	    T_Composite* a_composite = NULL;
	    CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co = NULL;
	    pami_reduce_scatter_t *a_xfer = (pami_reduce_scatter_t *)&(((pami_xfer_t *)cmd)->cmd.xfer_reduce_scatter);
            PAMI::Topology *topo = (PAMI::Topology*)geometry->getTopology(0);
            unsigned root = topo->index2Rank(0);

	    C *cmgr = _cmgr;
            unsigned key;
	    key = getKey(root, (unsigned)-1, (PAMI_GEOMETRY_CLASS*)g, (ConnectionManager::BaseConnectionManager **)&cmgr);

            key = key << 1;
            cmgr = new C(key);

	    //fprintf (stderr, "%d: Using Key %d\n", _native->myrank(), key);
	    co = (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)geometry->asyncCollectiveUnexpQ().findAndDelete(key);

            /// Try to match in unexpected queue
            if(co)
            {
              DEBUG((stderr, "key = %d, found early arrival in unexpected queue\n", key);)

              CCMI_assert(co->getFlags() & EarlyArrival);

	      co->setXfer((pami_xfer_t*)cmd);
	      co->setFlag(LocalPosted);

              a_composite = co->getComposite();

              a_composite->prepReduceBuffers(a_xfer->sndbuf, a_xfer->rcvbuf, a_xfer->stypecount, _native->myrank() == root);
              a_composite->getReduceExecutor().setReduceConnectionManager(cmgr);
              a_composite->getReduceExecutor().setBroadcastConnectionManager(cmgr);

              pami_callback_t  cb_exec_done;
              cb_exec_done.function   = scatter_exec_done;
              cb_exec_done.clientdata = co;
              a_composite->setScatterExecutor(a_xfer->sndbuf, a_xfer->rcvbuf, a_xfer->rtypecount,a_xfer->rcounts,a_xfer->dt, a_xfer->op,_native->numranks(), _native->myrank() == root, cb_exec_done);
              a_composite->getScatterExecutor().setConnectionID(key + 1);
	    }
	    /// not found posted CollOp object, create a new one and
	    /// queue it in active queue
	    else
	    {
              DEBUG((stderr, "key = %d, no early arrival in unexpected queue, create new co\n", key);)

              co = _free_pool.allocate(key);
	      pami_callback_t  cb_exec_done;
	      cb_exec_done.function   = reduce_exec_done;
              cb_exec_done.clientdata = co;

	      a_composite = new (co->getComposite())
		T_Composite ( _native,
		              cmgr,
		              cb_exec_done,
			      (PAMI_GEOMETRY_CLASS *)g,
                              cmd );

              cb_exec_done.function = scatter_exec_done;
              a_composite->setScatterExecutor(a_xfer->sndbuf, a_xfer->rcvbuf, a_xfer->rtypecount, a_xfer->rcounts, a_xfer->dt, a_xfer->op, _native->numranks(), _native->myrank() == root, cb_exec_done);

	      co->setXfer((pami_xfer_t*)cmd);
	      co->setFlag(LocalPosted);
	      co->setFactory(this);
              co->setGeometry((PAMI_GEOMETRY_CLASS *)g);

	      //Use the Key as the connection ID
	      // if (cmgr == NULL) {
              //  a_composite->getReduceExecutor().setConnectionID(key);
                a_composite->getScatterExecutor().setConnectionID(key + 1);
              //}

	    }

	    geometry->asyncCollectivePostQ().pushTail(co);
            if (root != _native->myrank()) {
              DEBUG((stderr, "key = %d, start scatter executor in generate()\n", key);)
              a_composite->getScatterExecutor().start();
            }
            DEBUG((stderr, "key = %d, start reduce executor in generate()\n", key);)
            a_composite->getReduceExecutor().start();

	    return NULL;
	  }

	  static void cb_async
	    (const pami_quad_t     * info,
	     unsigned                count,
	     unsigned                org_conn_id,
	     size_t                  peer,
	     size_t                  sndlen,
	     void                  * arg,
	     size_t                * rcvlen,
	     pami_pipeworkqueue_t ** rcvpwq,
	     pami_callback_t       * cb_done)
	  {

            unsigned conn_id = org_conn_id;
            conn_id = (conn_id >> 1) << 1;

	    AsyncReduceScatterFactoryT *factory = (AsyncReduceScatterFactoryT *) arg;
	    //fprintf(stderr, "%d: <%#.8X>Allreduce::AsyncReduceScatterFactoryT::cb_async() connid %d\n",factory->_native->myrank(), (int)factory, conn_id);

	    CollHeaderData *cdata = (CollHeaderData *) info;
	    T_Composite* a_composite = NULL;

	    int comm = cdata->_comm;
	    PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) PAMI_GEOMETRY_CLASS::getCachedGeometry(comm);
	    if(geometry == NULL)
	    {
	      geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry (comm);
	      PAMI_GEOMETRY_CLASS::updateCachedGeometry(geometry, comm);
	    }

            PAMI::Topology *topo = (PAMI::Topology*)geometry->getTopology(0);
            unsigned root = topo->index2Rank(0);
            CCMI_assert(cdata->_root == root);

            C *cmgr;
            unsigned key = getKey (root, conn_id, geometry, (ConnectionManager::BaseConnectionManager **)&cmgr);
            CCMI_assert(cmgr == NULL);
	    cmgr = new C(key);

	    CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co =
	      (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *) geometry->asyncCollectivePostQ().find(key);

            if (!co) {
              // it is still possible that there are other early arrivals
              DEBUG((stderr, "key = %d, no local post, try early arrival\n", key);)
              co = (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)geometry->asyncCollectiveUnexpQ().find(key);
            }

	    if(!co)
	    {
              DEBUG((stderr, "key = %d, from %d, no local post or early arrival, create new co\n", key, peer);)
	      co = factory->_free_pool.allocate(key);
	      pami_callback_t cb_exec_done;
	      cb_exec_done.function = reduce_exec_done;
	      cb_exec_done.clientdata = co;

	      a_composite = new (co->getComposite())
		T_Composite ( factory->_native,
			      cmgr,
			      cb_exec_done,
			      geometry,
                              NULL, NULL,
                              cdata->_count,
                              (pami_dt) cdata->_dt,
                              (pami_op) cdata->_op);

	      co->setFlag(EarlyArrival);
	      co->setFactory (factory);
              co->setGeometry(geometry);

	      geometry->asyncCollectiveUnexpQ().pushTail(co);
	    }
	    else
	    {
              DEBUG((stderr, "key = %d, peer = %d, found existing co\n", key, peer);)
	      a_composite = (T_Composite *) co->getComposite();
	      // CCMI_assert (a_composite->executor().getBytes() == sndlen);
	    }

            if (org_conn_id % 2) {
              CCMI_assert(peer == root);
              DEBUG((stderr, "key = %d, calling notifyRecv in cb_async()\n", key);)
              a_composite->getScatterExecutor().notifyRecv(peer, *info, (PAMI::PipeWorkQueue **)rcvpwq, cb_done);
              *rcvlen = sndlen;
            } else {
              DEBUG((stderr, "key = %d, calling notifyRecvHead in cb_async()\n", key);)
	      a_composite->getReduceExecutor().notifyRecvHead(info, count,
                                 conn_id, peer, sndlen, arg, rcvlen,
                                 rcvpwq, cb_done);
            }
            return;
	  }


          static void reduce_exec_done(pami_context_t context, void *cd, pami_result_t err)
          {
            CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> * co =
              (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)cd;

            //fprintf (stderr, "%d: exec_done for key %d\n", ((AsyncReduceScatterFactoryT *)co->getFactory())->_native->myrank(), co->key());

            DEBUG((stderr, "key = %d, reduce execution done, clean up\n", co->key());)

            T_Composite * a_composite = co->getComposite();

            CCMI_assert(a_composite->reduce_done_flag() == 0);
            a_composite->reduce_done_flag() = 1;

            AsyncReduceScatterFactoryT *factory = (AsyncReduceScatterFactoryT *)co->getFactory();
            if (a_composite->scatter_done_flag() == 1) {
              factory->exec_done(context, cd, err);
              return;
            }

            PAMI_GEOMETRY_CLASS *geometry = co->getGeometry();
            PAMI::Topology *topo = (PAMI::Topology*)geometry->getTopology(0);
            unsigned root = topo->index2Rank(0);
            if (factory->_native->myrank() == root)
              a_composite->getScatterExecutor().start();

            return;

          }


	  static void scatter_exec_done (pami_context_t context, void *cd, pami_result_t err)
          {
             CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> * co =
              (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)cd;

            //fprintf (stderr, "%d: exec_done for key %d\n", ((AsyncReduceScatterFactoryT *)co->getFactory())->_native->myrank(), co->key());

            DEBUG((stderr, "key = %d, scatter execution done, clean up\n", co->key());)

            T_Composite * a_composite = co->getComposite();

            CCMI_assert(a_composite->scatter_done_flag() == 0);
            a_composite->scatter_done_flag() = 1;

            AsyncReduceScatterFactoryT *factory = (AsyncReduceScatterFactoryT *)co->getFactory();
            if (a_composite->reduce_done_flag() == 1) factory->exec_done(context, cd, err);

            return;

          }

	  static void exec_done (pami_context_t context, void *cd, pami_result_t err)
	  {
	    CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> * co =
              (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)cd;

	    //fprintf (stderr, "%d: exec_done for key %d\n", ((AsyncReduceScatterFactoryT *)co->getFactory())->_native->myrank(), co->key());

            DEBUG((stderr, "key = %d, scatter execution done, clean up\n", co->key());)

            PAMI_GEOMETRY_CLASS *geometry = co->getGeometry();
	    unsigned flag;
            flag = co->getFlags();

            CCMI_assert(flag & LocalPosted);

	    pami_xfer_t *xfer = co->getXfer();
            AsyncReduceScatterFactoryT *factory = (AsyncReduceScatterFactoryT *)co->getFactory();

            // actviate user callback
            if(xfer->cb_done)
              xfer->cb_done(NULL, xfer->cookie, PAMI_SUCCESS);

            // must be on the posted queue, dequeue it
            geometry->asyncCollectivePostQ().deleteElem(co);

            // connection manager may need cleanup
            // if (flag & EarlyArrival) {
              CCMI_assert(co->getComposite()->connmgr() != factory->getConnMgr());
              delete co->getComposite()->connmgr();
            // }

            // free the executor composite
            co->getComposite()->~T_Composite();

            // free the CollOp object
            factory->_free_pool.free(co);
          }

      }; //- Async Composite Factory
    };  //- end namespace Allreduce
  };  //- end namespace Adaptor
};  //- end CCMI


#endif
