/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/alltoall/Alltoall.h
 * \brief ???
 */

#ifndef __ADAPTOR_ALL_TO_ALL_PROTOCOL__
#define __ADAPTOR_ALL_TO_ALL_PROTOCOL__

namespace CCMI
{
  namespace Adaptor
  {
    template <class T_Manytomany, class T_Sysdep, class T_Counter>
    class A2AProtocol
    {
    protected:
      XMI_Request_t    _sreq    __attribute__((__aligned__(16)));
      XMI_Request_t    _rreq    __attribute__((__aligned__(16)));
      XMI_Callback_t   _my_cb_done;
      XMI_Callback_t   _app_cb_done;

      unsigned             _donecount;

      const char         * _sndbuf;
      T_Counter          * _sndlens;
      T_Counter          * _sdispls;
      T_Counter          * _sndcounters;
      XMI_GEOMETRY_CLASS * _geometry;
      T_Manytomany       * _minterface;

    public:
      A2AProtocol (T_Sysdep           * mapping,
                   T_Manytomany       * minterface,
                   XMI_Callback_t       cb_done,
                   xmi_consistency_t     consistency,
                   XMI_GEOMETRY_CLASS * geometry,
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
                             (long unsigned int*)_geometry->ranks(),
                             (long unsigned int*)_geometry->permutation(),
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

      static void done (void *ctxt, void *arg, xmi_result_t err)
      {
        A2AProtocol *proto = (A2AProtocol *) arg;
        proto->_donecount ++;
        if((proto->_donecount == 2) && (proto->_app_cb_done.function))
          proto->_app_cb_done.function (NULL, proto->_app_cb_done.clientdata, XMI_SUCCESS);
      }
    };

    template <class T_Manytomany, class T_Sysdep, class T_Counter>
    class AlltoallFactory : public CCMI::Adaptor::CollectiveProtocolFactory
    {
    protected:
      T_Manytomany  * _minterface;
      T_Sysdep      * _mapping;

    public:

      AlltoallFactory (T_Manytomany *minterface,
                       T_Sysdep     *mapping)
      {
        _minterface = minterface;
        _mapping   = mapping;
      }

      //virtual ~AlltoallFactory () {}
      //void operator delete (void *p) {CCMI_abort();}
      virtual bool Analyze(XMI_GEOMETRY_CLASS *geometry)
      {
        return(geometry->isTorus());
      }

      virtual unsigned generate (XMI_CollectiveRequest_t   * request,
                                 XMI_Callback_t    cb_done,
                                 xmi_consistency_t   consistency,
                                 XMI_GEOMETRY_CLASS         * geometry,
                                 const char       * sndbuf,
                                 T_Counter         * sndlens,
                                 T_Counter         * sdispls,
                                 char             * rcvbuf,
                                 T_Counter         * rcvlens,
                                 T_Counter         * rdispls,
                                 T_Counter         * sndcounters,
                                 T_Counter         * rcvcounters)
      {
        new (request) A2AProtocol<T_Manytomany, T_Sysdep, T_Counter> (_mapping, _minterface, cb_done,
                                                                      consistency,
                                                                      geometry,
                                                                      sndbuf, sndlens, sdispls, rcvbuf,
                                                                      rcvlens, rdispls, sndcounters,
                                                                      rcvcounters);
        
        // Lets do the barrier to sync
        CCMI::Executor::Executor *barrier =(CCMI::Executor::Executor *)
          geometry->getKey(XMI::Geometry::XMI_GKEY_BARRIEREXECUTOR);
        CCMI_assert(barrier != NULL);
        barrier->setDoneCallback (cb_barrier_done, request);
        barrier->setConsistency (consistency);
        barrier->start();

        return 0;
      }

      static void cb_barrier_done (void *ctxt, void *arg, xmi_result_t err)
      {
        A2AProtocol<T_Manytomany, T_Sysdep, T_Counter> *proto = (A2AProtocol<T_Manytomany, T_Sysdep, T_Counter> *) arg;
        proto->start();
      }

    };
  };
};


#endif
