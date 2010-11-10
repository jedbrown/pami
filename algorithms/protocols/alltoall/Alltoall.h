/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/alltoall/Alltoall.h
 * \brief ???
 */

#ifndef __algorithms_protocols_alltoall_Alltoall_h__
#define __algorithms_protocols_alltoall_Alltoall_h__

#include "algorithms/composite/Composite.h"

namespace CCMI
{
  namespace Adaptor
  {
    template <class T_Manytomany, class T_Counter>
    class A2AProtocol
    {
    protected:
      PAMI_Request_t    _sreq    __attribute__((__aligned__(16)));
      PAMI_Request_t    _rreq    __attribute__((__aligned__(16)));
      pami_callback_t   _my_cb_done;
      pami_callback_t   _app_cb_done;

      unsigned             _donecount;

      const char         * _sndbuf;
      T_Counter          * _sndlens;
      T_Counter          * _sdispls;
      T_Counter          * _sndcounters;
      PAMI_GEOMETRY_CLASS * _geometry;
      T_Manytomany       * _minterface;

    public:
      A2AProtocol (T_Manytomany       * minterface,
                   pami_callback_t       cb_done,
                   pami_consistency_t     consistency,
                   PAMI_GEOMETRY_CLASS * geometry,
                   const char         * sndbuf,
                   T_Counter          * sndlens,
                   T_Counter          * sdispls,
                   char               * rcvbuf,
                   T_Counter          * rcvlens,
                   T_Counter          * rdispls,
                   T_Counter          * sndcounters,
                   T_Counter          * rcvcounters):
      _sndbuf (sndbuf),  _sndlens (sndlens),
      _sdispls (sdispls), _sndcounters (sndcounters), _geometry(geometry),
      _minterface(minterface)
      {
        _my_cb_done.function   = done;
        _my_cb_done.clientdata = this;
        _donecount = 0;

        _app_cb_done = cb_done;

        CCMI_assert (_geometry != NULL);
        CCMI_assert (_minterface != NULL);

        ///
        /// post receive on connection 0. Use a connection manager
        /// later
        ///
        _minterface->postRecv (&_rreq, &_my_cb_done, 0, rcvbuf, rcvlens,
                               rdispls, rcvcounters, geometry->nranks(),
                               // we want myIdx, not COMM_WORLD rank.
                               geometry->myIdx());
        int idx = geometry->myIdx();
        int size = (sndlens[idx] < rcvlens[idx])?
                   (sndlens[idx]):(rcvlens[idx]);
        if(size > 0)
          memcpy (rcvbuf + rdispls[idx], sndbuf + sdispls[idx], size);
      }

      virtual ~A2AProtocol ()
      {
      }

      void operator delete (void *p)
      {
        CCMI_abort();
      }

      void start ()
      {
        CCMI_assert (_geometry != NULL);
        CCMI_assert (_geometry->ranks() != NULL);
        // this is a bit hackish, but better than templatizing the geometry
        // we'll replace with topology and do it right
        if(sizeof(T_Counter) == 4)
          _minterface->send (&_sreq,
                             &_my_cb_done,
                             0,
                             _geometry->myIdx(),
                             _sndbuf,
                             _sndlens,
                             _sdispls,
                             _sndcounters,
                             (T_Counter*)_geometry->ranks(),
                             (T_Counter*)_geometry->permutation(),
                             _geometry->nranks());
        else
          _minterface->send (&_sreq,
                             &_my_cb_done,
                             0,
                             _geometry->myIdx(),
                             _sndbuf,
                             _sndlens,
                             _sdispls,
                             _sndcounters,
                             (size_t*)_geometry->ranks_sizet(),
                             (size_t*)_geometry->permutation_sizet(),
                             _geometry->nranks());
      }

      static void done (pami_context_t context, void *arg, pami_result_t err)
      {
        A2AProtocol *proto = (A2AProtocol *) arg;
        proto->_donecount ++;
        if((proto->_donecount == 2) && (proto->_app_cb_done.function))
          proto->_app_cb_done.function (NULL, proto->_app_cb_done.clientdata, PAMI_SUCCESS);
      }
    };



    class SimpleExecutor:public CCMI::Executor::Composite
    {
    public:
      virtual void start() {};
    };



    template <class T_Manytomany, class T_Counter>
    class AlltoallFactory : public CCMI::Adaptor::CollectiveProtocolFactory
    {
    protected:
      T_Manytomany  * _minterface;

    public:

      AlltoallFactory (T_Manytomany *minterface)
      {
        _minterface = minterface;
      }

      //virtual ~AlltoallFactory () {}
      //void operator delete (void *p) {CCMI_abort();}
      virtual bool Analyze(PAMI_GEOMETRY_CLASS *geometry)
      {
        return(geometry->isTorus());
      }


      class collObj
      {
      public:
        collObj(pami_xfer_t *xfer):
          _rsize(sizeof(_req)),
          _xfer(*xfer),
          _user_done_fn(xfer->cb_done),
          _user_cookie(xfer->cookie)
          {
            _xfer.cb_done = alloc_done_fn;
            _xfer.cookie  = this;
          }
        PAMI_CollectiveRequest_t      _req[1];
        int                          _rsize;
        pami_xfer_t                   _xfer;
        pami_event_function           _user_done_fn;
        void                       * _user_cookie;
        SimpleExecutor               _simpleExec;
      };

      static void alloc_done_fn( pami_context_t   context,
                                 void          * cookie,
                                 pami_result_t    result )
        {
          collObj *cObj = (collObj*)cookie;
          cObj->_user_done_fn(context,cObj->_user_cookie,result);
          free(cObj);
        }



      virtual Executor::Composite * generate(pami_geometry_t              geometry,
                                             void                      * cmd)

        {
          collObj *obj = (collObj*)malloc(sizeof(*obj));
          new(obj) collObj((pami_xfer_t*)cmd);
          PAMI_Callback_t cb_done;
          cb_done.function   = obj->_xfer.cb_done;
          cb_done.clientdata = obj->_xfer.cookie;
          this->generate(&obj->_req[0],
                         cb_done,
                         PAMI_MATCH_CONSISTENCY,
                         (PAMI_GEOMETRY_CLASS *)geometry,
                         obj->_xfer.cmd.xfer_alltoallv.sndbuf,
                         obj->_xfer.cmd.xfer_alltoallv.stypecounts,
                         obj->_xfer.cmd.xfer_alltoallv.sdispls,
                         obj->_xfer.cmd.xfer_alltoallv.rcvbuf,
                         obj->_xfer.cmd.xfer_alltoallv.rtypecounts,
                         obj->_xfer.cmd.xfer_alltoallv.rdispls,
                         NULL,NULL);
          return &obj->_simpleExec;
        }

      virtual void metadata(pami_metadata_t *mdata)
          {
            strcpy(mdata->name, "CCMIAlltoall");
          }

      virtual unsigned generate (PAMI_CollectiveRequest_t   * request,
                                 pami_callback_t    cb_done,
                                 pami_consistency_t   consistency,
                                 PAMI_GEOMETRY_CLASS         * geometry,
                                 const char       * sndbuf,
                                 T_Counter         * sndlens,
                                 T_Counter         * sdispls,
                                 char             * rcvbuf,
                                 T_Counter         * rcvlens,
                                 T_Counter         * rdispls,
                                 T_Counter         * sndcounters,
                                 T_Counter         * rcvcounters)
      {
        COMPILE_TIME_ASSERT(sizeof(*request) >= sizeof(A2AProtocol<T_Manytomany,T_Counter>));
        new (request) A2AProtocol<T_Manytomany,T_Counter> (_minterface, cb_done,
                                                                      consistency,
                                                                      geometry,
                                                                      sndbuf, sndlens, sdispls, rcvbuf,
                                                                      rcvlens, rdispls, sndcounters,
                                                                      rcvcounters);

        // Lets do the barrier to sync
        CCMI::Executor::Composite *barrier =(CCMI::Executor::Composite *)
          geometry->getKey((size_t)0, /// \todo does NOT support multicontext
                           PAMI::Geometry::CKEY_BARRIERCOMPOSITE0);
        CCMI_assert(barrier != NULL);
        barrier->setDoneCallback (cb_barrier_done, request);
        //barrier->setConsistency (consistency);
        barrier->start();

        return 0;
      }

      static void cb_barrier_done (pami_context_t context, void *arg, pami_result_t err)
      {
        A2AProtocol<T_Manytomany,T_Counter> *proto = (A2AProtocol<T_Manytomany,T_Counter> *) arg;
        proto->start();
      }

    };
  };
};


#endif
