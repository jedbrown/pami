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
      template <class T_Schedule, class T_Conn>
	class AsyncBroadcastT : public CCMI::Executor::Composite
      {
      protected:
        CCMI::Executor::BroadcastExec<T_Conn>  _executor __attribute__((__aligned__(16)));
        T_Schedule                             _schedule;

      public:
        ///
        /// \brief Constructor
        ///
        AsyncBroadcastT () {};
        AsyncBroadcastT (Interfaces::NativeInterface   * native,
                         T_Conn                        * cmgr,
                         pami_callback_t                  cb_done,
                         PAMI_GEOMETRY_CLASS            * geometry,
                         unsigned                        connid,
                         unsigned                        root,
                         char                          * src,
                         unsigned                        bytes) :
	Executor::Composite(),
	_executor (native, cmgr, geometry->comm())
        {
          TRACE_ADAPTOR ((stderr, "<%#.8X>Broadcast::AsyncBroadcastT() \n",(int)this));

          _executor.setRoot (root);
          _executor.setBuffers (src, src, bytes);
          _executor.setDoneCallback (cb_done.function, cb_done.clientdata);

          COMPILE_TIME_ASSERT(sizeof(_schedule) >= sizeof(T_Schedule));
          create_schedule(&_schedule, sizeof(_schedule), root, native, geometry);
          _executor.setSchedule (&_schedule, 0);
        }

        ///
        /// \brief initialize the schedule based on input geometry.
        /// Template implementation must specialize this function.
        ///
        void  create_schedule(void                      * buf,
                              unsigned                    size,
                              unsigned                    root,
			      Interfaces::NativeInterface * native,
			      PAMI_GEOMETRY_CLASS        * g) {CCMI_abort();};

        CCMI::Executor::BroadcastExec<T_Conn> &executor()
        {
          return _executor;
        }
      }; //- AsyncBroadcastT


      template <class T_Composite, MetaDataFn get_metadata, class C>
        class AsyncBroadcastFactoryT: public CollectiveProtocolFactory
	{
	protected:
	  ///
	  /// \brief get geometry from comm id
	  ///
	  pami_mapidtogeometry_fn      _cb_geometry;

	  ///
	  /// \brief active asynchronuous broadcast queue
	  ///
	  CCMI::Adaptor::CollOpQueueT<pami_xfer_t, T_Composite> _active_queue;

	  ///
	  /// \brief free memory pool for async broadcast operation objects
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
	  AsyncBroadcastFactoryT (C                           *cmgr,
				  Interfaces::NativeInterface *native):
	  CollectiveProtocolFactory(),
	  _cmgr(cmgr),
	  _native(native)
	  {
	    //fprintf (stderr, "AsyncBroadcastFactoryT::AsyncBroadcastFactoryT\n");

	    pami_dispatch_callback_fn fn;
	    fn.multicast = (pami_dispatch_multicast_fn) cb_async;
	    native->setDispatch(fn, this);
	  }

	  virtual ~AsyncBroadcastFactoryT ()
	  {
	  }

	  /// NOTE: This is required to make "C" programs link successfully with virtual destructors
	  void operator delete(void * p)
	  {
	    CCMI_abort();
	  }

	  virtual void metadata(pami_metadata_t *mdata)
	  {
	    TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
	    DO_DEBUG((templateName<MetaDataFn>()));
	    get_metadata(mdata);
	  }

	  virtual Executor::Composite * generate(pami_geometry_t              g,
						 void                      * cmd) {
	    T_Composite* a_bcast = NULL;
	    CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co = NULL;
	    pami_broadcast_t *bcast_xfer = &((pami_xfer_t*)cmd)->cmd.xfer_broadcast;

	    CCMI_assert(bcast_xfer->typecount <= 32768);
	    unsigned connid = _cmgr->updateConnectionId( ((PAMI_GEOMETRY_CLASS *)g)->comm() );
	    //          dev->lock();

	    if( _native->myrank() == bcast_xfer->root )
	    {
	      co = _free_pool.allocate(connid);

	      pami_callback_t  cb_exec_done;
	      cb_exec_done.function   = exec_done;
	      cb_exec_done.clientdata = co;

	      a_bcast = new (co->getComposite())
		T_Composite ( _native,
			      _cmgr,
			      cb_exec_done,
			      (PAMI_GEOMETRY_CLASS *)g,
			      connid,
			      bcast_xfer->root,
			      bcast_xfer->buf,
			      bcast_xfer->typecount);

	      co->setXfer((pami_xfer_t*)cmd);
	      co->setFlag(LocalPosted);
	      co->setFactory(this);
	      _active_queue.pushTail(co);

	      //dev->unlock();

	      a_bcast->executor().start();
	    }
	    else
	    {
	      co = _active_queue.find(connid);

	      /// Try to match in active queue
	      if(co)
	      {
		CCMI_assert(co->getFlags() & EarlyArrival);

		EADescriptor *ead = (EADescriptor *) co->getEAQ()->peekTail();
		CCMI_assert(ead != NULL);
		CCMI_assert(ead->bytes == bcast_xfer->typecount);
		CCMI_assert(ead->cdata._root == bcast_xfer->root);

		if(ead->flag == EACOMPLETED)
		{
		  if (bcast_xfer->typecount)
		  {
		    char *eab = ead->buf;
		    CCMI_assert(eab != NULL);
		    memcpy (bcast_xfer->buf, eab, bcast_xfer->typecount);
		    _eab_allocator.returnObject(eab);
		  }
		  ead->flag = EANODATA;
		  co->getEAQ()->popTail();
		  _ead_allocator.returnObject(ead);

		  if(((pami_xfer_t *)cmd)->cb_done) {
		    //		    dev->unlock();
		    ((pami_xfer_t *)cmd)->cb_done(NULL, ((pami_xfer_t *)cmd)->cookie, PAMI_SUCCESS);
		    // dev->lock();
		  }
		  _active_queue.deleteElem(co);
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
		co = _free_pool.allocate(connid);

		pami_callback_t  cb_exec_done;
		cb_exec_done.function   = exec_done;
		cb_exec_done.clientdata = co;

		a_bcast = new (co->getComposite())
		  T_Composite ( _native,
				_cmgr,
				cb_exec_done,
				(PAMI_GEOMETRY_CLASS *)g,
				connid,
				bcast_xfer->root,
				bcast_xfer->buf,
				bcast_xfer->typecount);

		co->setXfer((pami_xfer_t*)cmd);
		co->setFlag(LocalPosted);
		co->setFactory(this);
		_active_queue.pushTail(co);
	      }

	      //dev->unlock();
	    }

	    return a_bcast;
	  }

	  static PAMI_Request_t *   cb_async
	    (const pami_quad_t     * info,
	     unsigned               count,
	     unsigned               conn_id,
	     unsigned               peer,
	     unsigned               sndlen,
	     void                 * arg,
	     size_t               * rcvlen,
	     pami_pipeworkqueue_t ** rcvpwq,
	     pami_callback_t       * cb_done)
	  {
	    AsyncBroadcastFactoryT *factory = (AsyncBroadcastFactoryT *) arg;
	    //fprintf(stderr, "%d: <%#.8X>Broadcast::AsyncBroadcastFactoryT::cb_async()\n",factory->_native->myrank(), (int)factory);

	    CollHeaderData *cdata = (CollHeaderData *) info;
	    T_Composite* a_bcast = NULL;

	    //PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)
	    //   factory->_cb_geometry(cdata->_comm);
	    //  factory->_cb_geometry(conn_id >> CCMI::ConnectionManager::CommSeqConnMgr::SEQBITS);

	    int comm = conn_id >> CCMI::ConnectionManager::CommSeqConnMgr::SEQBITS;
	    PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) PAMI_GEOMETRY_CLASS::getCachedGeometry(comm);
	    if(geometry == NULL)
	    {
	      geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry (comm);
	      PAMI_GEOMETRY_CLASS::updateCachedGeometry(geometry, comm);
	    }

	    CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co = factory->_active_queue.find(conn_id);

	    if(!co)
	    {
	      //fprintf (stderr, "Posted collective not found\n");
	      co = factory->_free_pool.allocate(conn_id);

	      pami_callback_t cb_exec_done;
	      cb_exec_done.function = exec_done;
	      cb_exec_done.clientdata = co;

	      EADescriptor * ead = (EADescriptor *) factory->_ead_allocator.allocateObject();
	      memcpy(&(ead->cdata), cdata, sizeof(cdata));
	      ead->flag  = EASTARTED;
	      ead->bytes = sndlen;
	      if (sndlen)
	      {
		ead->buf   = (char *)factory->_eab_allocator.allocateObject();
	      }

	      a_bcast = new (co->getComposite())
		T_Composite ( factory->_native,
			      factory->_cmgr,
			      cb_exec_done,
			      geometry,
			      conn_id,
			      cdata->_root,
			      ead->buf,
			      sndlen);

	      fprintf (stderr, "%d: Creating Unexp Execuor at %p\n", factory->_native->myrank(), &(a_bcast->executor()) );

	      co->getEAQ()->pushTail(ead);
	      co->setFlag(EarlyArrival);

	      factory->_active_queue.pushTail(co);
	    }
	    else
	    {
	      //fprintf (stderr, "Posted collective found, calling notify recv\n");

	      // use type count for now, need datatype handling !!!
	      // CCMI_assert (co->getXfer()->type != PAMI_BYTE);
	      CCMI_assert (co->getXfer()->cmd.xfer_broadcast.typecount == sndlen);
	      a_bcast = (T_Composite *) co->getComposite();
	    }

	    a_bcast->executor().notifyRecv(peer, *info, (PAMI::PipeWorkQueue **)rcvpwq, cb_done);

	    //We only support sndlen == rcvlen
	    * rcvlen  = sndlen;

	    return NULL;
	  }

	  static void exec_done (pami_context_t context, void *cd, pami_result_t err)
	  {
	    CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> * co =
              (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)cd;

	    //fprintf (stderr, "%d: exec_done\n", ((AsyncBroadcastFactoryT *)co->getFactory())->_native->myrank());

	    pami_xfer_t *xfer = co->getXfer();
	    pami_broadcast_t *bcast_xfer = &co->getXfer()->cmd.xfer_broadcast;

	    unsigned     flag = co->getFlags();

	    if (flag & LocalPosted)
	    {
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
                  factory->_eab_allocator.returnObject(eab);
                }
                ead->flag = EANODATA;
                ead->buf  = NULL;
                factory->_ead_allocator.returnObject(ead);
              }
              else
              {
                CCMI_assert(ead == NULL);
              }

              if(xfer->cb_done)
		xfer->cb_done(NULL, xfer->cookie, PAMI_SUCCESS);

              factory->_active_queue.deleteElem(co);
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