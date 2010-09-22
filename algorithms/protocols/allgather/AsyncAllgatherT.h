/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allgather/AsyncAllgatherT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_allgather_AsyncAllgatherT_h__
#define __algorithms_protocols_allgather_AsyncAllgatherT_h__

#include "algorithms/ccmi.h"
#include "algorithms/executor/Allgather.h"
#include "algorithms/connmgr/CommSeqConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/CollOpT.h"

#ifdef DEBUG
#undef DEBUG
#define DEBUG(x) // fprintf x
#else
#define DEBUG(x) // fprintf x
#endif

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allgather
    {

      ///
      /// \brief Asyc Allgather Composite.
      ///
      template <class T_Schedule, class T_Conn, ScheduleFn create_schedule>
	class AsyncAllgatherT : public CCMI::Executor::Composite
      {

      protected:
        CCMI::Executor::AllgatherExec<T_Conn, T_Schedule>  _executor __attribute__((__aligned__(16)));
        T_Schedule                                         _schedule;
        T_Conn                                             *_cmgr;

      public:

        ///
        /// \brief Constructor
        ///
        AsyncAllgatherT () {};
        AsyncAllgatherT (Interfaces::NativeInterface   * native,
                         T_Conn                        * cmgr,
                         pami_callback_t                  cb_done,
                         PAMI_GEOMETRY_CLASS            * geometry,
                         void                           *cmd) :
        Executor::Composite(),
        _executor (native, cmgr, geometry->comm(), (PAMI::Topology*)geometry->getTopology(0)),
        _cmgr(cmgr)
        {
          TRACE_ADAPTOR ((stderr, "<%p>Allgather::AsyncAllgatherT() \n",this));
          pami_allgather_t *a_xfer = &((pami_xfer_t *)cmd)->cmd.xfer_allgather;

          COMPILE_TIME_ASSERT(sizeof(_schedule) >= sizeof(T_Schedule));
          create_schedule(&_schedule, sizeof(_schedule), (unsigned)-1, native, geometry);
          _executor.setSchedule (&_schedule);
          _executor.setBuffers (a_xfer->sndbuf,a_xfer->rcvbuf, a_xfer->stypecount);
          _executor.setDoneCallback (cb_done.function, cb_done.clientdata);
        }

        AsyncAllgatherT (Interfaces::NativeInterface   * native,
                         T_Conn                        * cmgr,
                         pami_callback_t                  cb_done,
                         PAMI_GEOMETRY_CLASS            * geometry,
                         char                           * sndbuf,
                         pami_type_t                    stype,
                         size_t                         stypecount,
                         char                           * rcvbuf,
                         pami_type_t                    rtype,
                         size_t                         rtypecount) :
        Executor::Composite(),
        _executor (native, cmgr, geometry->comm(), (PAMI::Topology*)geometry->getTopology(0)),
        //_schedule (native->myrank(), (PAMI::Topology*)geometry->getTopology(0), 0),
        _cmgr(cmgr)
        {
          TRACE_ADAPTOR ((stderr, "<%p>Allgather::AsyncAllgatherT() \n",this));

          COMPILE_TIME_ASSERT(sizeof(_schedule) >= sizeof(T_Schedule));
          create_schedule(&_schedule, sizeof(_schedule), (unsigned)-1, native, geometry);
          _executor.setSchedule (&_schedule);
          _executor.setBuffers (sndbuf,rcvbuf, rtypecount);
          _executor.setDoneCallback (cb_done.function, cb_done.clientdata);

        }

        CCMI::Executor::AllgatherExec<T_Conn, T_Schedule> &executor()
        {
          return _executor;
        }

        T_Conn *connmgr()
        {
          return _cmgr;
        }

      }; //- AsyncAllgatherT

      template <class T_Composite, MetaDataFn get_metadata, class C, ConnectionManager::GetKeyFn getKey>
        class AsyncAllgatherFactoryT: public CollectiveProtocolFactory
	{
	protected:
	  ///
	  /// \brief get geometry from comm id
	  ///
	  pami_mapidtogeometry_fn      _cb_geometry;

	  ///
	  /// \brief free memory pool for async allgather operation objects
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
	  AsyncAllgatherFactoryT (C                           *cmgr,
				  Interfaces::NativeInterface *native):
	  CollectiveProtocolFactory(),
	  _cmgr(cmgr),
	  _native(native)
	  {
	    native->setMulticastDispatch(cb_async, this);
	  }

	  virtual ~AsyncAllgatherFactoryT ()
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
	    pami_allgather_t *a_xfer =  &((pami_xfer_t *)cmd)->cmd.xfer_allgather;

	    PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)g;
	    C *cmgr = _cmgr;
            unsigned key;
	    key = getKey((unsigned)-1, (unsigned)-1, (PAMI_GEOMETRY_CLASS*)g, (ConnectionManager::BaseConnectionManager **)&cmgr);

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
              // update send buffer pointer and, at root, receive buffer pointers
              a_composite->executor().updateBuffers(a_xfer->sndbuf, a_xfer->rcvbuf, a_xfer->rtypecount);
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
	      if (cmgr == NULL)
                a_composite->executor().setConnectionID(key);

	    }

	    geometry->asyncCollectivePostQ().pushTail(co);
            DEBUG((stderr, "key = %d, start executor in generate()\n", key);)
            a_composite->executor().start();

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
	    AsyncAllgatherFactoryT *factory = (AsyncAllgatherFactoryT *) arg;
	    //fprintf(stderr, "%d: <%#.8X>Allgather::AsyncAllgatherFactoryT::cb_async() connid %d\n",factory->_native->myrank(), (int)factory, conn_id);

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
            key = getKey (cdata->_root, conn_id, geometry, (ConnectionManager::BaseConnectionManager **)&cmgr);
            CCMI_assert(cmgr == NULL);

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
                              NULL,
                              PAMI_BYTE,
                              cdata->_count,
                              NULL,
                              PAMI_BYTE,
                              cdata->_count);

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

            DEBUG((stderr, "key = %d, calling notifyRecv in cb_async()\n", key);)

            *rcvlen = sndlen;
            a_composite->executor().notifyRecv(peer, *info, (PAMI::PipeWorkQueue **)rcvpwq, cb_done);
            return;
	  }

	  static void exec_done (pami_context_t context, void *cd, pami_result_t err)
	  {
	    CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> * co =
              (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)cd;

	    //fprintf (stderr, "%d: exec_done for key %d\n", ((AsyncAllgatherFactoryT *)co->getFactory())->_native->myrank(), co->key());

            DEBUG((stderr, "key = %d, execution done, clean up\n", co->key());)

            PAMI_GEOMETRY_CLASS *geometry = co->getGeometry();
	    unsigned                 flag = co->getFlags();

            CCMI_assert(flag & LocalPosted);

	    pami_xfer_t *xfer = co->getXfer();
            AsyncAllgatherFactoryT *factory = (AsyncAllgatherFactoryT *)co->getFactory();

            // activate user callback
            if(xfer->cb_done)
              xfer->cb_done(NULL, xfer->cookie, PAMI_SUCCESS);

            // must be on the posted queue, dequeue it
            geometry->asyncCollectivePostQ().deleteElem(co);

            // free the executor composite
            co->getComposite()->~T_Composite();

            // free the CollOp object
            factory->_free_pool.free(co);
          }

      }; //- Async Composite Factory
    }  //- end namespace Allgather
  }  //- end namespace Adaptor
}  //- end CCMI


#endif