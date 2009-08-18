/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/Factory.h
 * \brief CCMI factory for [all]reduce composite
 */


#ifndef __ccmi_adaptor_allreduce_factory_h__
#define __ccmi_adaptor_allreduce_factory_h__

#include "./Composite.h"
#include "interface/Geometry.h"
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
        (XMI_CollectiveRequest_t * request,
         XMI_Callback_t            cb_done,
         CCMI_Consistency           consistency,
         Geometry                 * geometry,
         char                     * srcbuf,
         char                     * dstbuf,
         unsigned                   count,
         XMI_Dt                    dtype,
         XMI_Op                    op,
         int                        root = -1 ) = 0;
      }; // class BaseFactory

      //-- Factory
      /// \brief Base class for allreduce factory implementations.
      ///
      /// It provides the unexpected multisend callback and the basic
      /// function to retrieve an executor from a geometry (associated
      /// with a single comm id).
      /// 
      template<class MAP>
      class Factory : public BaseFactory
      {
      protected:
        ///
        /// \brief Multisend interface
        ///
        CCMI::MultiSend::OldMulticastInterface   * _moldinterface;
        CCMI::MultiSend::MulticombineInterface   * _minterface;

        ///
        ///  \brief Connection Manager for the allreduce
        ///
        CCMI::ConnectionManager::ConnectionManager   * _connmgr;

        ///
        /// \brief get geometry from comm id
        ///
        CCMI_mapIdToGeometry               _cb_geometry;

        ///
        /// \brief mapping module
        ///
        MAP                          * _mapping;

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
        inline Factory(MAP                                           * mapping,
                       CCMI::MultiSend::OldMulticastInterface        * moldinterface,
                       CCMI::MultiSend::MulticombineInterface        * minterface,
                       CCMI_mapIdToGeometry                           cb_geometry,
                       ConfigFlags                                   flags ) :
        _moldinterface (moldinterface),
        _minterface (minterface),
        _connmgr    (NULL),
        _cb_geometry(cb_geometry),
        _mapping    (mapping),
        _flags      (flags)
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::Factory::ctor() ALERT:\n",(int)this));
          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Factory::ctor(%#X) "
                         "flags %#X\n",
                         (int)this, sizeof(*this),*(unsigned*)&_flags));
          _moldinterface->setCallback (cb_receiveHead, this);
        }

        inline void setConnectionManager 
        (CCMI::ConnectionManager::ConnectionManager  * connmgr)
        {
          _connmgr = connmgr;
        }

        ///
        /// \brief Generate a non-blocking allreduce message.
        ///
        static XMI_Request_t *   cb_receiveHead(const XMIQuad  * info,
                                                 unsigned          count,
                                                 unsigned          peer,
                                                 unsigned          sndlen,
                                                 unsigned          conn_id,
                                                 void            * arg,
                                                 unsigned        * rcvlen,
                                                 char           ** rcvbuf,
                                                 unsigned        * pipewidth,
                                                 XMI_Callback_t * cb_done)
        {
          TRACE_ADAPTOR((stderr, 
                         "<%#.8X>Allreduce::Factory::cb_receiveHead peer %d, conn_id %d\n",
                         (int)arg, peer, conn_id));
          CCMI_assert (info && arg);
          CollHeaderData  *cdata = (CollHeaderData *) info;
          Factory *factory = (Factory *) arg;
          CCMI::Executor::AllreduceBase *allreduce = 
          factory->getAllreduce(cdata->_comm, cdata->_iteration);

          CCMI_assert (allreduce != NULL);
          TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::Factory::"
                         "cb_receiveHead(%#X,%#.8X)\n",
                         (int)factory,cdata->_comm,(int)allreduce));

          return allreduce->notifyRecvHead (info,      count,
                                            peer,      sndlen,
                                            conn_id,   arg,
                                            rcvlen,    rcvbuf,
                                            pipewidth, cb_done);
        };

        ///
        /// \brief Get the executor associated with a comm id (and
        /// color/iteration id)
        ///
        CCMI::Executor::AllreduceBase * getAllreduce(unsigned comm, 
                                               unsigned color)
        {
          CCMI::Executor::Composite *composite = 
          ((Geometry *)_cb_geometry(comm))->getAllreduceComposite(color);
          CCMI::Executor::AllreduceBase *executor = (composite)?
                                              (CCMI::Executor::AllreduceBase *) composite->getExecutor (0):
                                              (CCMI::Executor::AllreduceBase *)NULL;

          TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::Factory::"
                         "getAllreduce(comm id %#X, color %#X)"
                         " callback %#X, composite %#.8X  executor %#.8X\n",
                         (int)this, comm, color, 
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
