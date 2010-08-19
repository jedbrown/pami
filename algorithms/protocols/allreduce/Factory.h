/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/Factory.h
 * \brief CCMI factory for [all]reduce composite
 */


#ifndef __algorithms_protocols_allreduce_Factory_h__
#define __algorithms_protocols_allreduce_Factory_h__

#include "algorithms/protocols/allreduce/Composite.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "math/math_coremath.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      //-- BaeFactory
      /// \brief Base virtual class for allreduce factory implementations.
      ///
      class BaseFactory : public CollectiveProtocolFactory
      {
      public:
        /// NOTE: This is required to make "C" programs link
        /// successfully with virtual destructors
        void operator delete(void * p)
        {
          CCMI_abort();
        }
        ///
        /// \brief Generate a non-blocking allreduce message.
        ///
        virtual CCMI::Executor::Composite *generate
        (PAMI_CollectiveRequest_t  * request,
         pami_callback_t             cb_done,
         pami_consistency_t          consistency,
         PAMI_GEOMETRY_CLASS       * geometry,
         char                     * srcbuf,
         char                     * dstbuf,
         unsigned                   count,
         pami_dt                     dtype,
         pami_op                     op,
         int                        root = -1 ) = 0;
      }; // class BaseFactory

      //-- Factory
      /// \brief Base class for allreduce factory implementations.
      ///
      /// It provides the unexpected multisend callback and the basic
      /// function to retrieve an executor from a geometry (associated
      /// with a single comm id).
      ///
      template<class T_Mcast, class T_ConnectionManager>
      class Factory : public BaseFactory
      {
      protected:
        ///
        /// \brief Multisend interface
        ///
        T_Mcast   * _moldinterface;
//        CCMI::MultiSend::MulticombineInterface   * _minterface;

        ///
        ///  \brief Connection Manager for the allreduce
        ///
        T_ConnectionManager   * _connmgr;

        ///
        /// \brief get geometry from comm id
        ///
        pami_mapidtogeometry_fn               _cb_geometry;

        ///
        /// \brief Configuration flags
        //
        ConfigFlags                       _flags;

      public:


        /// NOTE: This is required to make "C" programs link
        /// successfully with virtual destructors
        void operator delete(void * p)
        {
          CCMI_abort();
        }

        ///
        /// \brief Constructor for allreduce factory implementations.
        ///
        inline Factory(T_Mcast                                * moldinterface,
//                       CCMI::MultiSend::MulticombineInterface        * minterface,
                       pami_mapidtogeometry_fn                            cb_geometry,
                       ConfigFlags                                     flags ) :
        _moldinterface (moldinterface),
//        _minterface (minterface),
        _connmgr    (NULL),
        _cb_geometry(cb_geometry),
        _flags      (flags)
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::Factory::ctor() ALERT:\n",this));
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::Factory::ctor(%#X) "
                         "flags %#X\n",
                         this, sizeof(*this),*(unsigned*)&_flags));
          _moldinterface->setCallback (cb_receiveHead, this);
        }

        inline void setConnectionManager
        (T_ConnectionManager  * connmgr)
        {
          _connmgr = connmgr;
        }

        ///
        /// \brief Generate a non-blocking allreduce message.
        ///
        static pami_quad_t *   cb_receiveHead(const pami_quad_t  * info,
                                                 unsigned          count,
                                                 unsigned          peer,
                                                 size_t            sndlen,
                                                 size_t            conn_id,
                                                 void            * arg,
                                                 unsigned        * rcvlen,
                                                 char           ** rcvbuf,
                                                 unsigned        * pipewidth,
                                                 pami_callback_t * cb_done)
        {
          TRACE_ADAPTOR((stderr,
                         "<%p>Allreduce::Factory::cb_receiveHead peer %d, conn_id %d\n",
                         arg, peer, conn_id));
          CCMI_assert (info && arg);
          CollHeaderData  *cdata = (CollHeaderData *) info;
          Factory *factory = (Factory *) arg;
          CCMI::Executor::OldAllreduceBase<T_Mcast,T_ConnectionManager> *allreduce =
          factory->getAllreduce(cdata->_comm, cdata->_iteration);

          CCMI_assert (allreduce != NULL);
          TRACE_ADAPTOR((stderr, "<%p>Allreduce::Factory::"
                         "cb_receiveHead(%#X,%p)\n",
                         factory,cdata->_comm,allreduce));

          return (pami_quad_t*)allreduce->notifyRecvHead (info,      count,
                                                         peer,      sndlen,
                                                         conn_id,   arg,
                                                         rcvlen,    rcvbuf,
                                                         pipewidth, cb_done);
        };

        ///
        /// \brief Get the executor associated with a comm id (and
        /// color/iteration id)
        ///
        CCMI::Executor::OldAllreduceBase<T_Mcast,T_ConnectionManager> * getAllreduce(unsigned comm,
                                               unsigned color)
        {
          CCMI::Executor::OldComposite *composite =(CCMI::Executor::OldComposite *)
            ((PAMI_GEOMETRY_CLASS *)_cb_geometry(comm))->getAllreduceComposite(color);
          CCMI::Executor::OldAllreduceBase<T_Mcast,T_ConnectionManager> *executor = (composite)?
                                              (CCMI::Executor::OldAllreduceBase<T_Mcast,T_ConnectionManager> *) composite->getExecutor (0):
                                              (CCMI::Executor::OldAllreduceBase<T_Mcast,T_ConnectionManager> *)NULL;

          TRACE_ADAPTOR((stderr, "<%p>Allreduce::Factory::"
                         "getAllreduce(comm id %#X, color %#X)"
                         " callback %#X, composite %#.8X  executor %#.8X\n",
                         this, comm, color,
                         (int) _cb_geometry,
                         (int) composite,
                         (int) executor));
          return executor;
        }
      }; // class Factory
    };
  };
}; // namespace CCMI::Adaptor::Allreduce

#endif
