/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/gather/AsyncLongGatherT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_gather_AsyncLongGatherT_h__
#define __algorithms_protocols_gather_AsyncLongGatherT_h__

#include "algorithms/ccmi.h"
#include "algorithms/executor/Gather.h"
#include "algorithms/connmgr/CommSeqConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/CollOpT.h"
#include "algorithms/protocols/gather/AsyncGatherT.h"

#if defined DEBUG
#undef DEBUG
#define DEBUG(x) // fprintf x
#else
#define DEBUG(x) // fprintf x
#endif

namespace CCMI
{
  namespace Adaptor
  {
    namespace Gather
    {

      ///
      /// \brief Asyc Gather Composite. It is single color right now
      ///
      template <typename T_Gather_type, class T_Bcast_Schedule, class T_Gather_Schedule, class T_Conn,
                 ScheduleFn create_bcast_schedule, ScheduleFn create_gather_schedule>
	class AsyncLongGatherT : public CCMI::Executor::Composite
      {
      protected:
        CCMI::Executor::BroadcastExec<T_Conn> _bcast_executor __attribute__((__aligned__(16)));
        CCMI::Executor::GatherExec<T_Conn, T_Gather_Schedule, T_Gather_type>   _gather_executor __attribute__((__aligned__(16)));
        T_Bcast_Schedule                      _bcast_schedule;
        T_Gather_Schedule                     _gather_schedule;
        T_Conn                                *_cmgr;
        // dummy buffer to make sure broadcast are carried out
        char _bbuf[4];

      public:

        ///
        /// \brief For long message gather, the done call back to be called
        ///        when 0-byte broadcast finishes
        ///
        static void cb_bcast_done(pami_context_t context, void *me, pami_result_t err)
        {
          AsyncLongGatherT * composite = (AsyncLongGatherT *) me;

          composite->getGatherExecutor().start();
        }

        ///
        /// \brief Constructor
        ///
        AsyncLongGatherT () {};
        AsyncLongGatherT (Interfaces::NativeInterface   * native,
                         T_Conn                        * cmgr,
                         pami_callback_t                  cb_done,
                         PAMI_GEOMETRY_CLASS            * geometry,
                         void                           *cmd) :
        Executor::Composite(),
        _bcast_executor (native, cmgr, geometry->comm()),
        _gather_executor (native, cmgr, geometry->comm(), (PAMI::Topology*)geometry->getTopology(0)),
        _cmgr(cmgr)
        {
          TRACE_ADAPTOR ((stderr, "<%p>Gather::AsyncLongGatherT() \n",this));
          T_Gather_type *a_xfer;
          Gather::getGatherXfer<T_Gather_type>(&a_xfer, &((pami_xfer_t *)cmd)->cmd);

          COMPILE_TIME_ASSERT(sizeof(_bcast_schedule) >= sizeof(T_Bcast_Schedule));
          create_bcast_schedule(&_bcast_schedule, sizeof(_bcast_schedule), a_xfer->root, native, geometry);
          _bcast_executor.setRoot(a_xfer->root);
          _bcast_executor.setSchedule (&_bcast_schedule, 0);
          _bcast_executor.setBuffers (&_bbuf[0],&_bbuf[0],1); // Need to confirm that 0-byte message gets delivered
          _bcast_executor.setDoneCallback (cb_bcast_done, this);

          COMPILE_TIME_ASSERT(sizeof(_gather_schedule) >= sizeof(T_Gather_Schedule));
          create_gather_schedule(&_gather_schedule, sizeof(_gather_schedule), a_xfer->root, native, geometry);
          // how do we reuse the sequence number as connection id ???
          _gather_executor.setRoot(a_xfer->root);
          _gather_executor.setSchedule(&_gather_schedule);
          _gather_executor.setBuffers(a_xfer->sndbuf, a_xfer->rcvbuf, a_xfer->stypecount);
          _gather_executor.setDoneCallback (cb_done.function, cb_done.clientdata);

        }

        CCMI::Executor::BroadcastExec<T_Conn> &getBroadcastExecutor()
        {
          return _bcast_executor;
        }

        CCMI::Executor::GatherExec<T_Conn, T_Gather_Schedule, T_Gather_type> &getGatherExecutor()
        {
          return _gather_executor;
        }

        T_Conn *connmgr()
        {
          return _cmgr;
        }

        typedef T_Gather_type xfer_type;

      }; //- AsyncLongGatherT

      template <class T_Composite, MetaDataFn get_metadata, class C, ConnectionManager::GetKeyFn getKey>
        class AsyncLongGatherFactoryT: public CollectiveProtocolFactory
	{
	protected:
	  ///
	  /// \brief get geometry from comm id
	  ///
	  pami_mapidtogeometry_fn      _cb_geometry;

	  ///
	  /// \brief free memory pool for async gather operation objects
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
	  AsyncLongGatherFactoryT (C                           *cmgr,
				  Interfaces::NativeInterface *native):
	  CollectiveProtocolFactory(),
	  _cmgr(cmgr),
	  _native(native)
	  {
	    native->setMulticastDispatch(cb_async, this);
	  }

	  virtual ~AsyncLongGatherFactoryT ()
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

          typedef typename T_Composite::xfer_type gather_type;

	  virtual Executor::Composite * generate(pami_geometry_t              g,
						 void                      * cmd)
          {
	    T_Composite* a_composite = NULL;
	    CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co = NULL;
            gather_type *a_xfer;
            Gather::getGatherXfer<gather_type>(&a_xfer, &((pami_xfer_t *)cmd)->cmd);

	    PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)g;
	    C *cmgr = _cmgr;
            unsigned key;
	    key = getKey(a_xfer->root, (unsigned)-1, (PAMI_GEOMETRY_CLASS*)g, (ConnectionManager::BaseConnectionManager **)&cmgr);

	    //fprintf (stderr, "%d: Using Key %d\n", _native->myrank(), key);
	    co = (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)geometry->asyncCollectiveUnexpQ().findAndDelete(key);

            /// Try to match in unexpected queue
            if(co)
            {

              DEBUG((stderr, "key = %d, found early arrival in unexpected queue\n", key);)
              CCMI_assert(_native->myrank() == a_xfer->root);
              CCMI_assert(co->getFlags() & EarlyArrival);

              EADescriptor *ead = (EADescriptor *) co->getEAQ()->peekTail();
              CCMI_assert(ead != NULL);
              CCMI_assert(ead->bytes == 0);
              CCMI_assert(ead->cdata._root == a_xfer->root);

              co->setXfer((pami_xfer_t*)cmd);
              co->setFlag(LocalPosted);

              a_composite = co->getComposite();
              // update send buffer pointer and, at root, receive buffer pointers
              a_composite->getGatherExecutor().updateBuffers(a_xfer->sndbuf, a_xfer->rcvbuf, a_xfer->stypecount);

              geometry->asyncCollectivePostQ().pushTail(co);

              if (ead->flag == EACOMPLETED)
              {
                co->getEAQ()->popTail();
                ead->flag = EANODATA;
                _ead_allocator.returnObject(ead);

                a_composite->getGatherExecutor().start();
              }
	    }
	    /// not found posted CollOp object, create a new one and
	    /// queue it in active queue
	    else
	    {
              DEBUG((stderr, "key = %d, no early arrival in unexpected queue, create new co\n", key);)

              co = _free_pool.allocate(key);
	      pami_callback_t  cb_exec_done;
	      cb_exec_done.function   = gather_exec_done;
              cb_exec_done.clientdata = co;

	      a_composite = new (co->getComposite())
              T_Composite ( _native,
		              cmgr,
		              cb_exec_done,
			      (PAMI_GEOMETRY_CLASS *)g,
                              (void *)cmd);

	      co->setXfer((pami_xfer_t*)cmd);
	      co->setFlag(LocalPosted);
	      co->setFactory(this);
              co->setGeometry((PAMI_GEOMETRY_CLASS *)g);
	      //Use the Key as the connection ID
	      if (cmgr == NULL) {
                a_composite->getBroadcastExecutor().setConnectionID(key);
                a_composite->getGatherExecutor().setConnectionID(key);
              }
	      geometry->asyncCollectivePostQ().pushTail(co);

              if (_native->myrank() == a_xfer->root)
              {
                DEBUG((stderr, "key = %d, start broadcast executor in generate()\n", key);)
                a_composite->getBroadcastExecutor().start();
              }
	    }

	    return NULL;
	  }

          static void cb_async
          (const pami_quad_t     * info,
           unsigned                count,
           unsigned                conn_id,
           size_t                  peer,
           size_t                  sndlen,
           void                  * arg,
           size_t                * rcvlen,
           pami_pipeworkqueue_t ** rcvpwq,
           pami_callback_t       * cb_done)
          {
            // CCMI_assert (sndlen == 0);

            AsyncLongGatherFactoryT *factory = (AsyncLongGatherFactoryT *) arg;
            CollHeaderData *cdata = (CollHeaderData *) info;
            T_Composite* a_composite = NULL;

            int comm = cdata->_comm;
            PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) PAMI_GEOMETRY_CLASS::getCachedGeometry(comm);

            if (geometry == NULL)
              {
                geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry (comm);
                PAMI_GEOMETRY_CLASS::updateCachedGeometry(geometry, comm);
              }

            C *cmgr = factory->_cmgr;
            unsigned key = getKey (cdata->_root, conn_id, geometry, (ConnectionManager::BaseConnectionManager **)&cmgr);
            CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co =
              (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *) geometry->asyncCollectivePostQ().findAndDelete(key);

            if (factory->_native->myrank() == cdata->_root) {
               DEBUG((stderr, "key = %d,  existing root co\n", key);)
               a_composite = (T_Composite *) co->getComposite();

               a_composite->getGatherExecutor().notifyRecv(peer, *info, (PAMI::PipeWorkQueue **)rcvpwq, cb_done);

               //We only support sndlen == rcvlen and payload of the message should be 0
               * rcvlen  = sndlen;

               return ;
            }

            if (!co)
            {

              DEBUG((stderr, "key = %d no local post or early arrival, create new co\n", key);)

              pami_xfer_t a_xfer;
              setTempGatherXfer<gather_type>(&(a_xfer.cmd));

              gather_type *g_xfer;
              getGatherXfer<gather_type>(&g_xfer, &(a_xfer.cmd));
              g_xfer->root       = cdata->_root;
              g_xfer->stypecount = cdata->_count;

              co = factory->_free_pool.allocate(key);
              pami_callback_t cb_exec_done;
              cb_exec_done.function = gather_exec_done;
              cb_exec_done.clientdata = co;

              a_composite = new (co->getComposite())
                T_Composite ( factory->_native,
                              cmgr,
                              cb_exec_done,
                              geometry,
                              (void *)&a_xfer);

              EADescriptor * ead = (EADescriptor *) factory->_ead_allocator.allocateObject();
              memcpy(&(ead->cdata), cdata, sizeof(cdata));
              ead->flag  = EASTARTED;
              ead->bytes = sndlen;

              co->getEAQ()->pushTail(ead);
              co->setFlag(EarlyArrival);
              co->setFactory (factory);

              //Use the Key as the connection ID
              if (cmgr == NULL) {
                a_composite->getBroadcastExecutor().setConnectionID(key);
                a_composite->getGatherExecutor().setConnectionID(key);
              }

              geometry->asyncCollectiveUnexpQ().pushTail(co);
            }
            else
            {
              DEBUG((stderr, "key = %d existing co\n", key);)
              a_composite = (T_Composite *) co->getComposite();
            }

            a_composite->getBroadcastExecutor().notifyRecv(peer, *info, (PAMI::PipeWorkQueue **)rcvpwq, cb_done);

            //We only support sndlen == rcvlen and payload of the message should be 0
            * rcvlen  = 1;

            return;
          }

          static void bcast_exec_done (pami_context_t context, void *cd, pami_result_t err)
          {
            CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> * co =
              (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)cd;
            DEBUG((stderr, "%d: bcast_exec_done for key %d\n", ((AsyncLongGatherFactoryT *)co->getFactory())->_native->myrank(), co->key());)

            unsigned     flag = co->getFlags();
            if (flag & LocalPosted)
            {
              pami_xfer_t *xfer = co->getXfer();

              EADescriptor *ead = (EADescriptor *) co->getEAQ()->popTail();
              AsyncLongGatherFactoryT *factory = (AsyncLongGatherFactoryT *)co->getFactory();

              if (flag & EarlyArrival)
              {
                CCMI_assert(ead != NULL);
                ead->flag = EANODATA;
                ead->buf  = NULL;
                factory->_ead_allocator.returnObject(ead);
              }
              else
              {
                CCMI_assert(ead == NULL);
              }

              co->getComposite()->getGatherExecutor().start();
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

	  static void gather_exec_done (pami_context_t context, void *cd, pami_result_t err)
	  {
	    CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> * co =
              (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)cd;

	    DEBUG((stderr, "%d: gather_exec_done for key %d\n", ((AsyncLongGatherFactoryT *)co->getFactory())->_native->myrank(), co->key());)

            DEBUG((stderr, "key = %d, execution done, clean up\n", co->key());)

            PAMI_GEOMETRY_CLASS *geometry = co->getGeometry();
	    unsigned                 flag;
	    flag = co->getFlags();

            CCMI_assert(flag & LocalPosted);

	    pami_xfer_t *xfer = co->getXfer();
            AsyncLongGatherFactoryT *factory = (AsyncLongGatherFactoryT *)co->getFactory();

            // activate user callback
            if(xfer->cb_done)
              xfer->cb_done(NULL, xfer->cookie, PAMI_SUCCESS);

            // must be on the posted queue, dequeue it
            geometry->asyncCollectivePostQ().deleteElem(co);

            // free the composite
            co->getComposite()->~T_Composite();

            // free the CollOp object
            factory->_free_pool.free(co);
          }

      }; //- Async Composite Factory
    }  //- end namespace Gather
  }  //- end namespace Adaptor
}  //- end CCMI


#endif
