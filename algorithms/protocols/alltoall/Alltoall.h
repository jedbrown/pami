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

#include "multisend/multisend_impl.h"

namespace CCMI
{
  namespace Adaptor
  {
    class A2AProtocol
    {
    protected:
      XMI_Request_t    _sreq    __attribute__((__aligned__(16)));
      XMI_Request_t    _rreq    __attribute__((__aligned__(16)));
      XMI_Callback_t   _my_cb_done;
      XMI_Callback_t   _app_cb_done;

      unsigned           _donecount;

      const char       * _sndbuf;
      unsigned         * _sndlens;
      unsigned         * _sdispls;
      unsigned         * _sndcounters;
      Geometry         * _geometry;
      CCMI::MultiSend::ManytomanyInterface *_minterface;

    public:
      A2AProtocol (CCMI::CollectiveMapping                        *mapping,
                   CCMI::MultiSend::ManytomanyInterface *minterface,
                   XMI_Callback_t    cb_done,
                   CCMI_Consistency   consistency,
                   Geometry         * geometry,
                   const char       * sndbuf,
                   unsigned         * sndlens,
                   unsigned         * sdispls,
                   char             * rcvbuf,
                   unsigned         * rcvlens,
                   unsigned         * rdispls,
                   unsigned         * sndcounters,
                   unsigned         * rcvcounters):
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


        _minterface->send (&_sreq, &_my_cb_done, 0, _geometry->myIdx(),
                           _sndbuf, _sndlens, _sdispls, _sndcounters,
                           _geometry->ranks(), _geometry->permutation(),
                           _geometry->nranks());
      }

      static void done (void *arg, XMI_Error_t *err)
      {
        A2AProtocol *proto = (A2AProtocol *) arg;
        proto->_donecount ++;

        if((proto->_donecount == 2) && (proto->_app_cb_done.function))
          proto->_app_cb_done.function (proto->_app_cb_done.clientdata, NULL);
      }
    };


    class AlltoallFactory : public CCMI::Adaptor::CollectiveProtocolFactory
    {
    protected:
      CCMI::MultiSend::ManytomanyInterface  * _minterface;
      CCMI::CollectiveMapping                        * _mapping;

    public:

      AlltoallFactory (CCMI::MultiSend::ManytomanyInterface *minterface,
                       CCMI::CollectiveMapping   *mapping)
      {
        _minterface = minterface;
        _mapping   = mapping;
      }

      //virtual ~AlltoallFactory () {}
      //void operator delete (void *p) {CCMI_abort();}
      virtual bool Analyze(Geometry *geometry) 
      {
        return(geometry->isTorus());      
      }

      virtual unsigned generate (XMI_CollectiveRequest_t   * request,
                                 XMI_Callback_t    cb_done,
                                 CCMI_Consistency   consistency,
                                 Geometry         * geometry,
                                 const char       * sndbuf,
                                 unsigned         * sndlens,
                                 unsigned         * sdispls,
                                 char             * rcvbuf,
                                 unsigned         * rcvlens,
                                 unsigned         * rdispls,
                                 unsigned         * sndcounters,
                                 unsigned         * rcvcounters)
      {
        new (request) A2AProtocol (_mapping, _minterface, cb_done,
                                   consistency,
                                   geometry,
                                   sndbuf, sndlens, sdispls, rcvbuf,
                                   rcvlens, rdispls, sndcounters,
                                   rcvcounters);

        // Lets do the barrier to sync
        CCMI::Executor::Executor *barrier =
        geometry->getBarrierExecutor();
        CCMI_assert(barrier != NULL);
        barrier->setDoneCallback (cb_barrier_done, request);
        barrier->setConsistency (consistency);
        barrier->start();

        return 0;
      }

      static void cb_barrier_done (void *arg, XMI_Error_t *err)
      {
        A2AProtocol *proto = (A2AProtocol *) arg;
        proto->start();
      }

    };
  };
};


#endif
