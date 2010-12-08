/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/gather/AsyncGatherT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_gather_AsyncGatherT_h__
#define __algorithms_protocols_gather_AsyncGatherT_h__

#include "algorithms/ccmi.h"
#include "algorithms/executor/Gather.h"
#include "algorithms/connmgr/CommSeqConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/CollOpT.h"

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

      template <class T_Gather_type>
      inline void getGatherXfer(T_Gather_type **xfer, pami_collective_t *coll)
      {
        COMPILE_TIME_ASSERT(0 == 1);
      }

      template <>
      inline void getGatherXfer<pami_gather_t>(pami_gather_t **xfer, pami_collective_t *coll)
      {
        *xfer =  &(coll->xfer_gather);
      }

      template <>
      inline void getGatherXfer<pami_gatherv_t>(pami_gatherv_t **xfer, pami_collective_t *coll)
      {
        *xfer =  &(coll->xfer_gatherv);
      }

      template <>
      inline void getGatherXfer<pami_gatherv_int_t>(pami_gatherv_int_t **xfer, pami_collective_t *coll)
      {
        *xfer =  &(coll->xfer_gatherv_int);
      }

      template <class T_Gather_type>
      inline void setTempGatherXfer(pami_collective_t *xfer)
      {
        COMPILE_TIME_ASSERT(0 == 1);
      }

      template <>
      inline void setTempGatherXfer<pami_gather_t> (pami_collective_t *xfer)
      {
        xfer->xfer_gather.root   = -1;
        xfer->xfer_gather.sndbuf = NULL;
        xfer->xfer_gather.stype  = PAMI_BYTE;
        xfer->xfer_gather.stypecount = 0;
        xfer->xfer_gather.rcvbuf = NULL;
        xfer->xfer_gather.rtype  = PAMI_BYTE;
        xfer->xfer_gather.rtypecount = 0;
      }

      template <>
      inline void setTempGatherXfer<pami_gatherv_t> (pami_collective_t *xfer)
      {
        xfer->xfer_gatherv.root    = -1;
        xfer->xfer_gatherv.sndbuf  = NULL;
        xfer->xfer_gatherv.stype   = PAMI_BYTE;
        xfer->xfer_gatherv.stypecount = 0;
        xfer->xfer_gatherv.rcvbuf  = NULL;
        xfer->xfer_gatherv.rtype   = PAMI_BYTE;
        xfer->xfer_gatherv.rtypecounts = NULL;
        xfer->xfer_gatherv.rdispls = NULL;
      }

      template <>
      inline void setTempGatherXfer<pami_gatherv_int_t> (pami_collective_t *xfer)
      {
        xfer->xfer_gatherv_int.root    = -1;
        xfer->xfer_gatherv_int.sndbuf  = NULL;
        xfer->xfer_gatherv_int.stype   = PAMI_BYTE;
        xfer->xfer_gatherv_int.stypecount = 0;
        xfer->xfer_gatherv_int.rcvbuf  = NULL;
        xfer->xfer_gatherv_int.rtype   = PAMI_BYTE;
        xfer->xfer_gatherv_int.rtypecounts = NULL;
        xfer->xfer_gatherv_int.rdispls = NULL;
      }

      template <class T_Schedule, class T_Conn, SFunc<PAMI_GEOMETRY_CLASS>::ScheduleFn create_schedule, typename T_Gather_type>
      class AsyncGatherT : public CCMI::Executor::Composite
      {
        protected:
          CCMI::Executor::GatherExec<T_Conn, T_Schedule, T_Gather_type>  _executor __attribute__((__aligned__(16)));
          T_Schedule                                 _schedule;
          T_Conn                                     *_cmgr;
          unsigned                                   _bytes;

        public:

          ///
          /// \brief Constructor
          ///
          AsyncGatherT () {};
          AsyncGatherT (Interfaces::NativeInterface   * native,
                        T_Conn                        * cmgr,
                        pami_callback_t                  cb_done,
                        PAMI_GEOMETRY_CLASS            * geometry,
                        void                           *cmd) :
              Executor::Composite(),
              _executor (native, cmgr, geometry->comm(), (PAMI::Topology*)geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX)),
              _cmgr(cmgr)
          {
            TRACE_ADAPTOR ((stderr, "<%p>Gather::AsyncGatherT() \n", this));
            T_Gather_type *g_xfer;
            getGatherXfer<T_Gather_type>(&g_xfer, &((pami_xfer_t *)cmd)->cmd);

            unsigned bytes = g_xfer->stypecount;

            COMPILE_TIME_ASSERT(sizeof(_schedule) >= sizeof(T_Schedule));
            create_schedule(&_schedule, sizeof(_schedule), g_xfer->root, native, geometry);
            _executor.setRoot(g_xfer->root);
            _executor.setSchedule (&_schedule);

            DEBUG((stderr, "In GatherExec ctor, root = %d, sbuf = %x, rbuf = %x, bytes = %d\n", g_xfer->root, g_xfer->sndbuf, g_xfer->rcvbuf, bytes);)
            _executor.setVectors (g_xfer);
            _executor.setBuffers (g_xfer->sndbuf, g_xfer->rcvbuf, bytes);
            _executor.setDoneCallback (cb_done.function, cb_done.clientdata);

          }

          CCMI::Executor::GatherExec<T_Conn, T_Schedule, T_Gather_type> &executor()
          {
            return _executor;
          }

          T_Conn *connmgr()
          {
            return _cmgr;
          }

          typedef T_Gather_type xfer_type;

      }; //- AsyncGatherT

      template <class T_Composite, MetaDataFn get_metadata, class C, ConnectionManager::GetKeyFn getKey>
      class AsyncGatherFactoryT: public CollectiveProtocolFactory
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
          PAMI::MemoryAllocator < sizeof(EADescriptor), 16 > _ead_allocator;

          ///
          /// \brief memory allocator for early arrival buffers
          ///
          PAMI::MemoryAllocator<32768, 16>                 _eab_allocator;

          C                                             * _cmgr;
          Interfaces::NativeInterface                   * _native;

        public:
          AsyncGatherFactoryT (C                           *cmgr,
                               Interfaces::NativeInterface *native):
              CollectiveProtocolFactory(),
              _cmgr(cmgr),
              _native(native)
          {
            native->setMulticastDispatch(cb_async, this);
          }

          virtual ~AsyncGatherFactoryT ()
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

          char *allocateBuffer (unsigned size)
          {
            if (size <= 32768)
              return (char *)_eab_allocator.allocateObject();

	    char *buf;
	    pami_result_t prc;
	    prc = __global.heap_mm->memalign((void **)&buf, 0, size);
	    return prc == PAMI_SUCCESS ? buf : NULL;
	  }

          void freeBuffer (unsigned size, char *buf)
          {
            if (size <= 32768)
              return _eab_allocator.returnObject(buf);

	    __global.heap_mm->free(buf);
	  }

          typedef typename T_Composite::xfer_type gather_type;

          virtual Executor::Composite * generate(pami_geometry_t              g,
                                                 void                      * cmd)
          {
            T_Composite* a_composite = NULL;
            CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co = NULL;
            gather_type *g_xfer;
            getGatherXfer<gather_type>(&g_xfer, &((pami_xfer_t *)cmd)->cmd);

            PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)g;
            C *cmgr = _cmgr;
            unsigned key;
            key = getKey(g_xfer->root, (unsigned) - 1, (PAMI_GEOMETRY_CLASS*)g, (ConnectionManager::BaseConnectionManager **) & cmgr);

            DEBUG((stderr, "%d: Using Key %d\n", _native->myrank(), key);)
            co = (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)geometry->asyncCollectiveUnexpQ().findAndDelete(key);

            /// Try to match in unexpected queue
            if (co)
              {

                DEBUG((stderr, "key = %d, found early arrival in unexpected queue\n", key);)

                CCMI_assert(co->getFlags() & EarlyArrival);

                co->setXfer((pami_xfer_t*)cmd);
                co->setFlag(LocalPosted);

                a_composite = co->getComposite();
                // update send buffer pointer and, at root, receive buffer pointers
                a_composite->executor().updateBuffers(g_xfer->sndbuf, g_xfer->rcvbuf, g_xfer->rtypecount);
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
            AsyncGatherFactoryT *factory = (AsyncGatherFactoryT *) arg;

            CollHeaderData *cdata = (CollHeaderData *) info;
            T_Composite* a_composite = NULL;

            int comm = cdata->_comm;
            PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry (ctxt, comm);

            DEBUG((stderr, "AsyncGatherFactory::cb_async(), root = %d, connection id = %d\n",
                   cdata->_root, conn_id);)

            C *cmgr = factory->getConnMgr();
            unsigned key;
            key = getKey (cdata->_root, conn_id, geometry, (ConnectionManager::BaseConnectionManager **) & cmgr);
            CCMI_assert(cmgr == NULL);

            CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co =
              (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *) geometry->asyncCollectivePostQ().find(key);

            if (!co)
              {
                // it is still possible that there are other early arrivals
                DEBUG((stderr, "key = %d, no local post, try early arrival\n", key);)
                co = (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)geometry->asyncCollectiveUnexpQ().find(key);
              }

            if (!co)
              {
                DEBUG((stderr, "key = %d no local post or early arrival, create new co\n", key);)
                co = factory->_free_pool.allocate(key);
                pami_callback_t cb_exec_done;
                cb_exec_done.function = exec_done;
                cb_exec_done.clientdata = co;

                pami_xfer_t a_xfer;
                setTempGatherXfer<gather_type>(&(a_xfer.cmd));

                gather_type *g_xfer;
                getGatherXfer<gather_type>(&g_xfer, &(a_xfer.cmd));
                g_xfer->root       = cdata->_root;
                g_xfer->stypecount = cdata->_count;

                a_composite = new (co->getComposite())
                T_Composite ( factory->_native,
                              cmgr,
                              cb_exec_done,
                              geometry,
                              (void *)&a_xfer);

                co->setFlag(EarlyArrival);
                co->setFactory (factory);
                co->setGeometry(geometry);

                if (cmgr == NULL)
                  a_composite->executor().setConnectionID(key);

                geometry->asyncCollectiveUnexpQ().pushTail(co);
              }
            else
              {
                DEBUG((stderr, "key = %d, found existing co\n", key);)
                a_composite = (T_Composite *) co->getComposite();
                // CCMI_assert (a_composite->executor().getBytes() == sndlen);
              }

            DEBUG((stderr, "key = %d, calling notifyRecv in cb_async()\n", key);)

            // what parameters does executor require through notifyRecv ???
            // a_composite->executor().notifyRecv(info, count,
            //                     conn_id, peer, sndlen, arg, rcvlen,
            //                     rcvpwq, cb_done);
            *rcvlen = sndlen;
            a_composite->executor().notifyRecv(peer, *info, (PAMI::PipeWorkQueue **)rcvpwq, cb_done);
            return;
          }

          static void exec_done (pami_context_t context, void *cd, pami_result_t err)
          {
            CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> * co =
              (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)cd;

            //fprintf (stderr, "%d: exec_done for key %d\n", ((AsyncGatherFactoryT *)co->getFactory())->_native->myrank(), co->key());

            DEBUG((stderr, "key = %d, execution done, clean up\n", co->key());)

            PAMI_GEOMETRY_CLASS *geometry = co->getGeometry();
            unsigned                 flag;
            flag = co->getFlags();

            CCMI_assert(flag & LocalPosted);

            pami_xfer_t *xfer = co->getXfer();
            AsyncGatherFactoryT *factory = (AsyncGatherFactoryT *)co->getFactory();

            // activate user callback
            if (xfer->cb_done)
              xfer->cb_done(NULL, xfer->cookie, PAMI_SUCCESS);

            // must be on the posted queue, dequeue it
            geometry->asyncCollectivePostQ().deleteElem(co);

            // free the executor composite
            co->getComposite()->~T_Composite();

            // free the CollOp object
            factory->_free_pool.free(co);
          }

      }; //- Async Composite Factory
    }  //- end namespace Gather
  }  //- end namespace Adaptor
}  //- end CCMI


#endif
