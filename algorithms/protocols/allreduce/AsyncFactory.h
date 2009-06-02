/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file ccmi/adaptor/protocols/allreduce/AsyncFactory.h
 * \brief CCMI factory for [all]reduce composite
 */


#ifndef __ccmi_adaptor_allreduce_async_factory_h__
#define __ccmi_adaptor_allreduce_async_factory_h__

#include "AsyncComposite.h"
#include "collectives/interface/Geometry.h"
#include "collectives/algorithms/protocols/ProtocolFactory.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      // A temporary callback for asynchronous generates. The local
      // restart will set the true callback and replace this one.
      void temp_done_callback(void* cd, CCMI_Error_t *err)
      {
        CCMI_abort();
      }

      //-- AsyncFactory
      /// \brief Base class for allreduce factory implementations.
      ///
      /// It provides the unexpected multisend callback and the basic
      /// function to retrieve an executor from a geometry (associated
      /// with a single comm id).
      /// 
      class AsyncFactory : public ProtocolFactory
      {
      protected:
        ///
        /// \brief Multisend interface
        ///
        CCMI::MultiSend::MulticastInterface   * _minterface;

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
        CCMI::Mapping                          * _mapping;

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
        inline AsyncFactory(CCMI::Mapping                                      * mapping,
                            CCMI::MultiSend::MulticastInterface        * minterface,
                            CCMI_mapIdToGeometry                           cb_geometry,
                            ConfigFlags                                   flags ) :
        _minterface (minterface),
        _connmgr    (NULL),
        _cb_geometry(cb_geometry),
        _mapping    (mapping),
        _flags      (flags)
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::AsyncFactory::ctor() ALERT:\n",(int)this));
          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::AsyncFactory::ctor(%#X) "
                         "flags %#X\n",
                         (int)this, sizeof(*this),*(unsigned*)&_flags));
        }

        inline void setConnectionManager 
        (CCMI::ConnectionManager::ConnectionManager  * connmgr)
        {
          _connmgr = connmgr;
        }

        ///
        /// \brief Generate a non-blocking allreduce message.
        ///
        virtual CCMI::Executor::Composite *generate
        (CCMI_CollectiveRequest_t * request,
         CCMI_Callback_t            cb_done,
         CCMI_Consistency           consistency,
         Geometry                 * geometry,
         char                     * srcbuf,
         char                     * dstbuf,
         unsigned                   count,
         CCMI_Dt                    dtype,
         CCMI_Op                    op,
         int                        root = -1 ) = 0;

        ///
        /// \brief Generate a non-blocking async allreduce message
        /// without starting it.
        ///
        virtual CCMI::Executor::Composite *generateAsync
        (Geometry                 * geometry,
         unsigned                   count,
         CCMI_Dt                    dtype,
         CCMI_Op                    op,
         unsigned                   iteration,
         int                        root = -1 ) = 0;

        ///
        /// \brief Generate a non-blocking allreduce message.
        ///
        static CCMI_Request_t *   cb_receiveHead(const CCMIQuad    * info,
                                                 unsigned          count,
                                                 unsigned          peer,
                                                 unsigned          sndlen,
                                                 unsigned          conn_id,
                                                 void            * arg,
                                                 unsigned        * rcvlen,
                                                 char           ** rcvbuf,
                                                 unsigned        * pipewidth,
                                                 CCMI_Callback_t * cb_done)
        {
          TRACE_ADAPTOR((stderr, 
                         "<%#.8X>Allreduce::AsyncFactory::cb_receiveHead peer %d, conn_id %d\n",
                         (int)arg, peer, conn_id));
          CCMI_assert (info && arg);
          CollHeaderData  *cdata = (CollHeaderData *) info;
          AsyncFactory *factory = (AsyncFactory *) arg;

          Geometry *geometry = (Geometry *)factory->_cb_geometry(cdata->_comm);
          CCMI::Adaptor::Allreduce::AsyncComposite *composite = 
          factory->getAllreduceComposite(geometry, cdata->_iteration);

          CCMI::Executor::AllreduceBase *allreduce = 
          factory->getAllreduce(geometry, cdata->_iteration);

          TRACE_ADAPTOR((stderr, 
                         "<%#.8X>Allreduce::AsyncFactory::cb_receiveHead "
                         "comm %#X, root %#X, count %#X, dt %#X, op %#X, iteration %#X,"
                         "composite %#.8X, executor %#.8X %s\n",
                         (int)factory, cdata->_comm, cdata->_root, cdata->_count,
                         cdata->_dt, cdata->_op, cdata->_iteration,
                         (int)composite, (int)allreduce, 
                         (composite == NULL?" ":
                          ((composite->isIdle())?"(Idle)":" "))));

          if((allreduce == NULL) || (composite == NULL))
          {
            composite = factory->buildComposite (geometry, cdata);
            allreduce = (CCMI::Executor::AllreduceBase *) composite->getExecutor (0);
          }
          else if(composite->isIdle())
          {
            composite->restartAsync(allreduce,
                                    cdata->_count,
                                    (CCMI_Dt)(cdata->_dt),
                                    (CCMI_Op)(cdata->_op),
                                    cdata->_root);
          }

          TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::AsyncFactory::"
                         "cb_receiveHead(%#X,%#.8X)\n",
                         (int)factory,cdata->_comm,(int)allreduce));

          return allreduce->notifyRecvHead (info,      count,
                                            peer,      sndlen,
                                            conn_id,   arg,
                                            rcvlen,    rcvbuf,
                                            pipewidth, cb_done);
        };


        AsyncComposite          * buildComposite (Geometry         * geometry,
                                                  CollHeaderData   * cdata)
        {
          AsyncComposite *composite = (CCMI::Adaptor::Allreduce::AsyncComposite *)
                                      generateAsync(geometry,
                                                    cdata->_count,
                                                    (CCMI_Dt)(cdata->_dt),
                                                    (CCMI_Op)(cdata->_op),
                                                    cdata->_iteration);
          composite->setQueueing();

          return composite;
        }


        ///
        /// \brief Get the executor associated with a comm id (and
        /// color/iteration id)
        ///
        CCMI::Executor::AllreduceBase * getAllreduce(Geometry *geometry, 
                                                     unsigned iter)
        {
          CCMI::Executor::Composite *composite = 
          geometry->getAllreduceComposite(iter);

          CCMI::Executor::AllreduceBase *executor = (composite)?
                                                    (CCMI::Executor::AllreduceBase *) composite->getExecutor (0):
                                                    (CCMI::Executor::AllreduceBase *)NULL;

          TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::AsyncFactory::"
                         "getAllreduce(comm id X, color %#X)"
                         " callback %#X, composite %#.8X  executor %#.8X\n",
                         (int)this,// comm, 
                         iter, 
                         (int) _cb_geometry,
                         (int) composite,
                         (int) executor));
          return executor;
        }
        ///
        /// \brief Get the composite associated with a comm id (and
        /// iteration id).  It is expected to be associated with this Factory,
        /// otherwise destroy it and return NULL.
        ///
        CCMI::Adaptor::Allreduce::AsyncComposite * getAllreduceComposite(Geometry *geometry, 
                                                                             unsigned iteration)
        {
          CCMI::Adaptor::Allreduce::AsyncComposite *composite = (CCMI::Adaptor::Allreduce::AsyncComposite *)
                                                                    geometry->getAllreduceComposite(iteration);

          TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::AsyncFactory::"
                         "getAllreduceComposite(comm id X, iteration %#X)"
                         " callback %#X, composite's factory %#.8X  composite %#.8X\n",
                         (int)this, //comm, 
                         iteration, 
                         (int) _cb_geometry,
                         (int) (composite? composite->getFactory(): NULL),
                         (int)composite));

          if(composite && (composite->getFactory() != this))
          {
            CCMI_assert(!composite->isQueueing() && !composite->isStarted());//unfinished operation?
            composite->~AsyncComposite();
            composite = NULL;
            geometry->setAllreduceComposite(composite, iteration);
          }

          return composite;
        }
      }; // class AsyncFactory
    };
  };
}; // namespace CCMI::Adaptor::Allreduce

#endif
