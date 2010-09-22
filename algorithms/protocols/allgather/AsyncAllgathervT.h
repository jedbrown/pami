/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allgather/AsyncAllgathervT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_allgather_AsyncAllgathervT_h__
#define __algorithms_protocols_allgather_AsyncAllgathervT_h__

#include "algorithms/ccmi.h"
#include "algorithms/executor/Allgatherv.h"
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
    namespace Allgatherv
    {

      template <class T_Allgather_type>
      inline void getAllgatherXfer(T_Allgather_type **xfer, pami_collective_t *coll)
      {
         COMPILE_TIME_ASSERT(0==1);
      }

      template <>
      inline void getAllgatherXfer<pami_allgather_t>(pami_allgather_t **xfer, pami_collective_t *coll)
      {
         *xfer =  &(coll->xfer_allgather);
      }

      template <>
      inline void getAllgatherXfer<pami_allgatherv_t>(pami_allgatherv_t **xfer, pami_collective_t *coll)
      {
         *xfer =  &(coll->xfer_allgatherv);
      }

      template <>
      inline void getAllgatherXfer<pami_allgatherv_int_t>(pami_allgatherv_int_t **xfer, pami_collective_t *coll)
      {
         *xfer =  &(coll->xfer_allgatherv_int);
      }

      template <class T_Allgather_type>
      inline void setTempAllgatherXfer(pami_collective_t *xfer)
      {
         COMPILE_TIME_ASSERT(0 == 1);
      }

      template <>
      inline void setTempAllgatherXfer<pami_allgather_t> (pami_collective_t *xfer)
      {
         xfer->xfer_allgather.sndbuf = NULL;
         xfer->xfer_allgather.stype  = PAMI_BYTE;
         xfer->xfer_allgather.stypecount = 0;
         xfer->xfer_allgather.rcvbuf = NULL;
         xfer->xfer_allgather.rtype  = PAMI_BYTE;
         xfer->xfer_allgather.rtypecount = 0;
      }

      template <>
      inline void setTempAllgatherXfer<pami_allgatherv_t> (pami_collective_t *xfer)
      {
         xfer->xfer_allgatherv.sndbuf  = NULL;
         xfer->xfer_allgatherv.stype   = PAMI_BYTE;
         xfer->xfer_allgatherv.stypecount = 0;
         xfer->xfer_allgatherv.rcvbuf  = NULL;
         xfer->xfer_allgatherv.rtype   = PAMI_BYTE;
         xfer->xfer_allgatherv.rtypecounts = NULL;
         xfer->xfer_allgatherv.rdispls = NULL;
      }

      template <>
      inline void setTempAllgatherXfer<pami_allgatherv_int_t> (pami_collective_t *xfer)
      {
         xfer->xfer_allgatherv_int.sndbuf  = NULL;
         xfer->xfer_allgatherv_int.stype   = PAMI_BYTE;
         xfer->xfer_allgatherv_int.stypecount = 0;
         xfer->xfer_allgatherv_int.rcvbuf  = NULL;
         xfer->xfer_allgatherv_int.rtype   = PAMI_BYTE;
         xfer->xfer_allgatherv_int.rtypecounts = NULL;
         xfer->xfer_allgatherv_int.rdispls = NULL;
      }

      ///
      /// \brief Asyc Allgatherv Composite.
      ///
      template <class T_Conn, typename T_Type>
	class AsyncAllgathervT : public CCMI::Executor::Composite
      {

      protected:
        CCMI::Executor::AllgathervExec<T_Conn, T_Type>  _executor __attribute__((__aligned__(16)));
        // CCMI::Interfaces::Schedule                      *_schedule;
        T_Conn                                          *_cmgr;

      public:

        ///
        /// \brief Constructor
        ///
        AsyncAllgathervT () {};
        AsyncAllgathervT (Interfaces::NativeInterface   * native,
                         T_Conn                        * cmgr,
                         pami_callback_t                  cb_done,
                         PAMI_GEOMETRY_CLASS            * geometry,
                         void                           *cmd) :
        Executor::Composite(),
        _executor (native, cmgr, geometry->comm(), (PAMI::Topology*)geometry->getTopology(0)),
        _cmgr(cmgr)
        {
          TRACE_ADAPTOR ((stderr, "<%p>Allgatherv::AsyncAllgathervT() \n",this));
          T_Type *a_xfer;

          getAllgatherXfer<T_Type>(&a_xfer, &((pami_xfer_t *)cmd)->cmd);
          _executor.setSchedule(NULL);
          _executor.setVectors (a_xfer);
          _executor.setDoneCallback (cb_done.function, cb_done.clientdata);
        }

        CCMI::Executor::AllgathervExec<T_Conn, T_Type> &executor()
        {
          return _executor;
        }

        T_Conn *connmgr()
        {
          return _cmgr;
        }

      }; //- AsyncAllgathervT

      template <typename T_Type, class T_Composite, MetaDataFn get_metadata, class C, ConnectionManager::GetKeyFn getKey>
        class AsyncAllgathervFactoryT: public CollectiveProtocolFactory
	{
	protected:
	  ///
	  /// \brief get geometry from comm id
	  ///
	  pami_mapidtogeometry_fn      _cb_geometry;

	  ///
	  /// \brief free memory pool for async allgatherv operation objects
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
	  AsyncAllgathervFactoryT (C                           *cmgr,
				  Interfaces::NativeInterface *native):
	  CollectiveProtocolFactory(),
	  _cmgr(cmgr),
	  _native(native)
	  {
	    native->setMulticastDispatch(cb_async, this);
	  }

	  virtual ~AsyncAllgathervFactoryT ()
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
	    T_Type *a_xfer;
            getAllgatherXfer<T_Type>(&a_xfer, &((pami_xfer_t *)cmd)->cmd);

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
              a_composite->executor().updateVectors(a_xfer);
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
                              (void *)cmd);

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
	    AsyncAllgathervFactoryT *factory = (AsyncAllgathervFactoryT *) arg;
	    //fprintf(stderr, "%d: <%#.8X>Allgatherv::AsyncAllgathervFactoryT::cb_async() connid %d\n",factory->_native->myrank(), (int)factory, conn_id);

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

              pami_xfer_t a_xfer;
              setTempAllgatherXfer<T_Type>(&(a_xfer.cmd));

	      a_composite = new (co->getComposite())
		T_Composite ( factory->_native,
			      cmgr,
			      cb_exec_done,
			      geometry,
                              (void *)&a_xfer);

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

	    //fprintf (stderr, "%d: exec_done for key %d\n", ((AsyncAllgathervFactoryT *)co->getFactory())->_native->myrank(), co->key());

            DEBUG((stderr, "key = %d, execution done, clean up\n", co->key());)

            PAMI_GEOMETRY_CLASS *geometry = co->getGeometry();
	    unsigned                 flag = co->getFlags();

            CCMI_assert(flag & LocalPosted);

	    pami_xfer_t *xfer = co->getXfer();
            AsyncAllgathervFactoryT *factory = (AsyncAllgathervFactoryT *)co->getFactory();

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
    }  //- end namespace Allgatherv
  }  //- end namespace Adaptor
}  //- end CCMI


#endif