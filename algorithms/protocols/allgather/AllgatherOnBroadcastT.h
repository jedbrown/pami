///
/// \file algorithms/protocols/allgather/AllgatherOnBroadcastT.h
/// \brief ???
///
#ifndef __algorithms_protocols_allgather_AllgatherOnBroadcastT_h__
#define __algorithms_protocols_allgather_AllgatherOnBroadcastT_h__

#include "algorithms/ccmi.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"
#include "math/Memcpy.x.h"

#include "util/trace.h"

#ifdef CCMI_TRACE_ALL
 #define DO_TRACE_ENTEREXIT 1
 #define DO_TRACE_DEBUG     1
#else
 #define DO_TRACE_ENTEREXIT 0
 #define DO_TRACE_DEBUG     0
#endif


namespace CCMI
{
  namespace Adaptor
  {
    namespace Allgather
    {
      static const void* ALLGV_IN_PLACE = (void *) (-1);

      ///
      /// \brief Asyc Allgather Composite.
      ///
      template <int NBCAST, int NCOLORS, class T_Bcast, class T_Conn,  PAMI::Geometry::topologyIndex_t T_Geometry_Index>
      class AllgatherOnBroadcastT: public Executor::Composite
      {
      protected:
        uint32_t                      _ncomplete;
        uint32_t                      _nranks;
        uint32_t                      _cur_nbcast;
        pami_event_function           _fn;
        void                        * _cookie;
        PAMI_GEOMETRY_CLASS         * _geometry;
        Interfaces::NativeInterface * _native;
        T_Conn                        _cmgr[NBCAST];
        pami_allgatherv_t             _cmd;
        T_Bcast                       _bcast[NBCAST];

      public:
        ///
        /// \brief Constructor
        ///
        AllgatherOnBroadcastT ()
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p>", this);
          TRACE_FN_EXIT();
        };

        AllgatherOnBroadcastT (Interfaces::NativeInterface              * native,
                               T_Conn                                   * cmgr,
                               pami_geometry_t                             g,
                               void                                     * cmd,
                               pami_event_function                         fn,
                               void                                     * cookie):
        Executor::Composite(),
        _ncomplete(0), 
        _nranks(((PAMI::Topology*)((PAMI_GEOMETRY_CLASS*)g)->getTopology(T_Geometry_Index))->size()), 
        _cur_nbcast(0),
        _fn (fn),
        _cookie (cookie),
        _geometry((PAMI_GEOMETRY_CLASS *)g),
        _native(native),
        _cmd (((pami_xfer_t *)cmd)->cmd.xfer_allgatherv)
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p> _nranks %u, geometry %p, native %p", this, _nranks, g, native);
          // allgatherv requires a ranklist, so make sure it's been created.
          _geometry->createTopology(PAMI::Geometry::LIST_TOPOLOGY_INDEX);
          for (unsigned i = 0; i < _nranks; i++)
          {
            if (_geometry->ranks()[i] == _native->myrank())
            {
              char *dst = _cmd.rcvbuf + _cmd.rdispls[i];
              char *src = _cmd.sndbuf; 
              unsigned bytes = _cmd.rtypecounts[i];

              if (src && (src != (char *)ALLGV_IN_PLACE))
                Core_memcpy(dst, src, bytes);
            }
          }

          nextStep();
          TRACE_FN_EXIT();
        }

        void nextStep ()
        {
          TRACE_FN_ENTER();
          unsigned i = 0, nc = 0;
          unsigned ncomplete = _ncomplete;

          //Allow each bcast to have atleast 3 colors
          for (i = 0; (i < NBCAST) && (ncomplete < _nranks) && nc < NCOLORS; i++, ncomplete++)
          {
            size_t root, bytes;
            char *src, *dst;

            bytes = _cmd.rtypecounts[ncomplete];
            root  = _geometry->ranks()[ncomplete];
            dst   = _cmd.rcvbuf + _cmd.rdispls[ncomplete];
            src   = dst;        //For non-inplace allgvs the src has been copied to dst on root

            TRACE_FORMAT("<%p> _cmgr[%u]=%p, _bcast[%u]=%p", this,i,&_cmgr[i],i,&_bcast[i]);
            new (&_cmgr[i]) T_Conn (nc);
            new (&_bcast[i]) T_Bcast(_native, &_cmgr[i], _geometry, done, this);
            _bcast[i].initialize (root, bytes, src, dst);       
            nc += _bcast[i].getNumColors();
            if (nc > NCOLORS)
              break;
          }
          _cur_nbcast = i;

          //if (T_BARRIER) {
          CCMI::Executor::Composite  *barrier =  (CCMI::Executor::Composite *)
                                                 _geometry->getKey((size_t)0, /// \todo does NOT support multicontext
                                                                   PAMI::Geometry::CKEY_BARRIERCOMPOSITE1);
          barrier->setDoneCallback(cb_barrier_done, this);
          barrier->start();
          //}
          //else {
          //for (i = 0; i < _cur_nbcast; i++) 
          //_bcast[i].start();
          //}
          TRACE_FN_EXIT();
        }

        void reset ()
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p>", this);
          _ncomplete = 0;
          TRACE_FN_EXIT();
        }

        void restart ()
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p>", this);
          reset ();
          nextStep();
          TRACE_FN_EXIT();
        }

        static void cb_barrier_done (pami_context_t   context,
                                     void           * cookie,
                                     pami_result_t    result) 
        {
          TRACE_FN_ENTER();
          AllgatherOnBroadcastT<NBCAST, NCOLORS, T_Bcast, T_Conn, T_Geometry_Index> *allg = (AllgatherOnBroadcastT<NBCAST, NCOLORS, T_Bcast, T_Conn, T_Geometry_Index> *) cookie;
          TRACE_FORMAT("<%p>", allg);
          for (unsigned i = 0; i < allg->_cur_nbcast; i++)
            allg->_bcast[i].start();
          TRACE_FN_EXIT();
        }

        static void done (pami_context_t   context,
                          void           * cookie,
                          pami_result_t    result) 
        {
          TRACE_FN_ENTER();
          AllgatherOnBroadcastT<NBCAST, NCOLORS, T_Bcast, T_Conn, T_Geometry_Index> *allg = (AllgatherOnBroadcastT<NBCAST, NCOLORS, T_Bcast, T_Conn, T_Geometry_Index> *) cookie;
          TRACE_FORMAT("<%p> ncomplete %d, nranks %d _fn %p", allg, allg->_ncomplete, allg->_nranks, allg->_fn);
          //	    printf ("<%p> In AllgatherOnBroadast::done 

          ++allg->_ncomplete;
          --allg->_cur_nbcast;

          if (allg->_cur_nbcast > 0)
            return;

          if (allg->_ncomplete < allg->_nranks)
            allg->nextStep();
          else
          {
            allg->_fn (context, allg->_cookie, PAMI_SUCCESS);
          }
          TRACE_FN_EXIT();
        }
      };
    };
  };
};

#endif
