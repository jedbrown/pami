/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/AsyncAllreduceT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_allreduce_AsyncAllreduceT_h__
#define __algorithms_protocols_allreduce_AsyncAllreduceT_h__

#include "algorithms/ccmi.h"
#include "algorithms/executor/Allreduce.h"
#include "algorithms/connmgr/CommSeqConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/CollOpT.h"

//  #define DEBUG(x) fprintf x
#define DEBUG(x) 

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {

      template <class T_reduce_type>
      inline void _get_xfer_and_root(T_reduce_type **xfer, pami_collective_t *coll, unsigned &root)
      {
         COMPILE_TIME_ASSERT(0==1);
      }

      template <>
      inline void _get_xfer_and_root<pami_reduce_t>(pami_reduce_t **xfer, pami_collective_t *coll, unsigned &root)
      {
         pami_reduce_t *lxfer   = &(coll->xfer_reduce);
         if (root == (unsigned)-1) root = lxfer->root; // in case coll is not initialized
         else lxfer->root = root;
         *xfer = lxfer;
      }

      template <>
      inline void _get_xfer_and_root<pami_allreduce_t>(pami_allreduce_t **xfer, pami_collective_t *coll, unsigned &root)
      {
         *xfer   =  &(coll->xfer_allreduce);
         root   = (unsigned)-1;
      }

      ///
      /// \brief Asyc Allreduce Composite. It is single color right now
      ///
      template <class T_Schedule, class T_Conn, class T_reduce_type>
	class AsyncAllreduceT : public CCMI::Executor::Composite
      {
      protected:
        CCMI::Executor::AllreduceBaseExec<T_Conn>  _executor __attribute__((__aligned__(16)));
        T_Schedule                                 _schedule;
        T_Conn                                     *_cmgr;
        unsigned                                   _bytes;

        public:
         
        ///
        /// \brief Constructor
        ///
        AsyncAllreduceT () {};
        AsyncAllreduceT (Interfaces::NativeInterface   * native,
                         T_Conn                        * cmgr,
                         pami_callback_t                  cb_done,
                         PAMI_GEOMETRY_CLASS            * geometry,
                         void                           *cmd) :
        Executor::Composite(),
        _executor (native, cmgr, geometry->comm()),
        _schedule (native->myrank(), (PAMI::Topology*)geometry->getTopology(0), 0),
        _cmgr(cmgr)
        {
          TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::AsyncAllreduceT() \n",(int)this));

          unsigned root = (unsigned)-1;
          coremath func;
          unsigned sizeOfType;
          T_reduce_type *a_xfer;
          _get_xfer_and_root(&a_xfer,(pami_collective_t *)cmd, root);
          _executor.setRoot(root);

          // ??? Why would getReduceFunction need data size ? use stypecount for now
          CCMI::Adaptor::Allreduce::getReduceFunction(a_xfer->dt,a_xfer->op,a_xfer->stypecount,sizeOfType,func);
          // unsigned bytes = sizeOfType * a_xfer->stypecount;
          _executor.setBuffers (a_xfer->sndbuf,a_xfer->rcvbuf, a_xfer->stypecount);
          _executor.setDoneCallback (cb_done.function, cb_done.clientdata);

          COMPILE_TIME_ASSERT(sizeof(_schedule) >= sizeof(T_Schedule));
          _executor.setSchedule (&_schedule, 0);
          _executor.setReduceInfo(a_xfer->stypecount / sizeOfType, a_xfer->stypecount, sizeOfType, func, a_xfer->op, a_xfer->dt);

          _executor.reset();
        }

        AsyncAllreduceT (Interfaces::NativeInterface   * native,
                         T_Conn                        * cmgr,
                         pami_callback_t                  cb_done,
                         PAMI_GEOMETRY_CLASS            * geometry,
                         char                           * sndbuf,
                         char                           * rcvbuf,
                         unsigned                       root,
                         size_t                         dt_count,
                         pami_dt                        dt,
                         pami_op                        op ) :
        Executor::Composite(),
        _executor (native, cmgr, geometry->comm()),
        _schedule (native->myrank(), (PAMI::Topology*)geometry->getTopology(0), 0),
        _cmgr(cmgr)
        {
          TRACE_ADAPTOR ((stderr, "<%#.8X>Allreduce::AsyncAllreduceT() \n",(int)this));
          unsigned sizeOfType;
          coremath func;

          _executor.setRoot(root);

          // ??? Why would getReduceFunction need data size ? use dt_count for now
          CCMI::Adaptor::Allreduce::getReduceFunction(dt,op,dt_count,sizeOfType,func);
          unsigned bytes = dt_count * sizeOfType;
          _executor.setBuffers (sndbuf,rcvbuf, bytes);
          _executor.setDoneCallback (cb_done.function, cb_done.clientdata);

          COMPILE_TIME_ASSERT(sizeof(_schedule) >= sizeof(T_Schedule));
          _executor.setSchedule (&_schedule, 0);
          _executor.setReduceInfo(dt_count, bytes, sizeOfType, func, op, dt);

          _executor.reset();
        }


        CCMI::Executor::AllreduceBaseExec<T_Conn> &executor()
        {
          return _executor;
        }

        T_Conn *connmgr() 
        {
          return _cmgr;
        }

      }; //- AsyncAllreduceT

      template <class T_Composite, MetaDataFn get_metadata, class C, class T_reduce_type, ConnectionManager::GetKeyFn getKey>
        class AsyncAllreduceFactoryT: public CollectiveProtocolFactory
	{
	protected:
	  ///
	  /// \brief get geometry from comm id
	  ///
	  pami_mapidtogeometry_fn      _cb_geometry;

	  ///
	  /// \brief free memory pool for async allreduce operation objects
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
	  AsyncAllreduceFactoryT (C                           *cmgr,
				  Interfaces::NativeInterface *native):
	  CollectiveProtocolFactory(),
	  _cmgr(cmgr),
	  _native(native)
	  {
	    pami_dispatch_callback_fn fn;
	    fn.multicast = (pami_dispatch_multicast_fn) cb_async;
	    native->setDispatch(fn, this);
	  }

	  virtual ~AsyncAllreduceFactoryT ()
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
	    T_Composite* a_composite = NULL;
	    CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co = NULL;
	    T_reduce_type *a_xfer;
            unsigned root = (unsigned)-1;
            _get_xfer_and_root(&a_xfer, &(((pami_xfer_t*)cmd)->cmd), root);

	    PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)g;
	    C *cmgr = _cmgr;
            unsigned key;
	    key = getKey(root, (unsigned)-1, (PAMI_GEOMETRY_CLASS*)g, (ConnectionManager::BaseConnectionManager **)&cmgr);

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
              a_composite->executor().setBuffers(a_xfer->sndbuf, a_xfer->rcvbuf, 0); // need number of bytes ???
              a_composite->executor().setReduceConnectionManager(_cmgr);
              a_composite->executor().setBroadcastConnectionManager(_cmgr);
	    }
	    /// not found posted CollOp object, create a new one and
	    /// queue it in active queue
	    else
	    {
              DEBUG((stderr, "key = %d, no early arrival in unexpected queue, create new co\n", key);)

              co = _free_pool.allocate(key);
	      pami_callback_t  cb_exec_done;
	      cb_exec_done.function   = exec_done;
              cb_exec_done.clientdata = co;

	      a_composite = new (co->getComposite())
		T_Composite ( _native,
		              cmgr,
		              cb_exec_done,
			      (PAMI_GEOMETRY_CLASS *)g,
                              &(((pami_xfer_t *)cmd)->cmd));

	      co->setXfer((pami_xfer_t*)cmd);
	      co->setFlag(LocalPosted);
	      co->setFactory(this);
              co->setGeometry((PAMI_GEOMETRY_CLASS *)g);
	      //Use the Key as the connection ID
	      // if (cmgr == NULL)
              //   a_composite->executor().setConnectionID(key);

	    }

	    geometry->asyncCollectivePostQ().pushTail(co);
            DEBUG((stderr, "key = %d, start executor in generate()\n", key);)
            a_composite->executor().start();

	    return NULL; 
	  }

	  static PAMI_Request_t    * cb_async
	    (const pami_quad_t     * info,
	     unsigned                count,
	     unsigned                conn_id,
	     unsigned                peer,
	     unsigned                sndlen,
	     void                  * arg,
	     size_t                * rcvlen,
	     pami_pipeworkqueue_t ** rcvpwq,
	     pami_callback_t       * cb_done)
	  {
	    AsyncAllreduceFactoryT *factory = (AsyncAllreduceFactoryT *) arg;
	    //fprintf(stderr, "%d: <%#.8X>Allreduce::AsyncAllreduceFactoryT::cb_async() connid %d\n",factory->_native->myrank(), (int)factory, conn_id);

	    CollHeaderData *cdata = (CollHeaderData *) info;
	    T_Composite* a_composite = NULL;

	    int comm = cdata->_comm;
	    PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) PAMI_GEOMETRY_CLASS::getCachedGeometry(comm);
	    if(geometry == NULL)
	    {
	      geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry (comm);
	      PAMI_GEOMETRY_CLASS::updateCachedGeometry(geometry, comm);
	    }

	    C *cmgr = factory->getConnMgr();
            unsigned key;
            unsigned root = cdata->_root;
            pami_collective_t cmd;
            T_reduce_type *a_xfer;
            // a_xfer->root gets updated by the call if the operation is reduce
            _get_xfer_and_root(&a_xfer, &cmd, root);

            key = getKey (root, conn_id, geometry, (ConnectionManager::BaseConnectionManager **)&cmgr);
            CCMI_assert(cmgr == NULL);

	    //Use the Key as the connection ID if necessary
            if (cmgr == NULL) cmgr = new C(key);

	    CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co =
	      (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *) geometry->asyncCollectivePostQ().find(key);

            if (!co) {
              // it is still possible that there are other early arrivals
              DEBUG((stderr, "key = %d, no local post, try early arrival\n", key);)
              co = (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)geometry->asyncCollectiveUnexpQ().find(key);
            }

	    if(!co)
	    {
              DEBUG((stderr, "key = %d no local post or early arrival, create new co\n", key);)
	      co = factory->_free_pool.allocate(key);
	      pami_callback_t cb_exec_done;
	      cb_exec_done.function = exec_done;
	      cb_exec_done.clientdata = co;

	      a_composite = new (co->getComposite())
		T_Composite ( factory->_native,
			      cmgr,
			      cb_exec_done,
			      geometry,
                              NULL, NULL,
                              (unsigned)cdata->_root,
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
              DEBUG((stderr, "key = %d, found existing co\n", key);)
	      a_composite = (T_Composite *) co->getComposite();
	      // CCMI_assert (a_composite->executor().getBytes() == sndlen);
	    }

            DEBUG((stderr, "key = %d, calling notifyRecvHead in cb_async()\n", key);)
	    return a_composite->executor().notifyRecvHead(info, count, 
                                 conn_id, peer, sndlen, arg, rcvlen,
                                 rcvpwq, cb_done);
	  }

	  static void exec_done (pami_context_t context, void *cd, pami_result_t err)
	  {
	    CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> * co =
              (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)cd;

	    //fprintf (stderr, "%d: exec_done for key %d\n", ((AsyncAllreduceFactoryT *)co->getFactory())->_native->myrank(), co->key());

            DEBUG((stderr, "key = %d, execution done, clean up\n", co->key());)

            PAMI_GEOMETRY_CLASS *geometry = co->getGeometry();
	    unsigned                 flag = co->getFlags();

            CCMI_assert(flag & LocalPosted);

	    pami_xfer_t *xfer = co->getXfer();
            AsyncAllreduceFactoryT *factory = (AsyncAllreduceFactoryT *)co->getFactory();

            // acviate user callback
            if(xfer->cb_done)
              xfer->cb_done(NULL, xfer->cookie, PAMI_SUCCESS);

            // must be on the posted queue, dequeue it
            geometry->asyncCollectivePostQ().deleteElem(co);

            // connection manager may need cleanup
            if (flag & EarlyArrival) {
              CCMI_assert(co->getComposite()->connmgr() != factory->getConnMgr());
              delete co->getComposite()->connmgr();
            }
  
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
