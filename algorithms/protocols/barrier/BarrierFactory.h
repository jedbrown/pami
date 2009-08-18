/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/barrier/BarrierFactory.h
 * \brief ???
 */

#ifndef __ccmi_collectives_barrier_factory_h__
#define __ccmi_collectives_barrier_factory_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/executor/Barrier.h"
#include "interface/Geometry.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {
      ///
      /// \brief Virtual Barrier Factory Base class.
      ///
      class BaseBarrierFactory : private CollectiveProtocolFactory
      {
      public:
        ///
        /// \brief Generate a non-blocking barrier message.
        ///
        /// \param[in]  request      Opaque memory to maintain internal
        ///                          message state.
        /// \param[in]  geometry     Geometry for the barrier operation
        ///
        /// \retval     executor     Pointer to barrier executor
        ///
        virtual CCMI::Executor::Executor *generate
        (CCMI_Executor_t           * request,
         Geometry                  * geometry) = 0;

      };  //- BarrierFactory

      ///
      /// \brief Barrier Factory Base class.
      ///
      template<class MAP>
        class BarrierFactory : private BaseBarrierFactory
        {
        protected:
          CCMI::MultiSend::MultisyncInterface  * _msyncInterface;
          MAP                                * _mapping;
          CCMI_mapIdToGeometry                           _cb_geometry;

        public:
          /// NOTE: This is required to make "C" programs link successfully with virtual destructors
          void operator delete(void * p)
          {
            CCMI_abort();
          }

          ///
          /// \brief Constructor for barrier factory implementations.
          ///
          BarrierFactory (CCMI::MultiSend::MultisyncInterface      * minterface,
                          MAP                           * map,
                          CCMI_mapIdToGeometry                       cb_geometry) :
          _msyncInterface (minterface),
          _mapping (map),
          _cb_geometry (cb_geometry)
          {
            TRACE_INIT((stderr,"<%#.8X>CCMI::Collectives::Barrier::BarrierFactory::ctor(%d)\n",
                       (int)this,(int)cb_geometry));
            //minterface->setCallback (cb_head, this);
          }

        };  //- BarrierFactory

// Old, deprecated, interfaces for transition from OldMulticast to Multisync

      ///
      /// \brief Barrier Factory Base class.
      ///
      template<class MAP>
      class OldBarrierFactory : private BaseBarrierFactory
      {
      protected:
        CCMI::MultiSend::OldMulticastInterface  * _mcastInterface;
        MAP                                     * _mapping;
        CCMI_mapIdToGeometry                           _cb_geometry;

      public:
        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
          CCMI_abort();
        }

        ///
        /// \brief Constructor for barrier factory implementations.
        ///
        OldBarrierFactory (CCMI::MultiSend::OldMulticastInterface      * minterface,
                        MAP                            * map,
                        CCMI_mapIdToGeometry                       cb_geometry) :
        _mcastInterface (minterface),
        _mapping (map),
        _cb_geometry (cb_geometry)
        {
          TRACE_INIT((stderr,"<%#.8X>CCMI::Collectives::Barrier::BarrierFactory::ctor(%d)\n",
                     (int)this,(int)cb_geometry));
          minterface->setCallback (cb_head, this);
        }

        static XMI_Request_t *   cb_head   (const XMIQuad    * info,
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
          CollHeaderData  *cdata = (CollHeaderData *) info;      
          OldBarrierFactory *factory = (OldBarrierFactory *) arg;

          Geometry *geometry = (Geometry *) Geometry::getCachedGeometry(cdata->_comm);
          if(geometry == NULL)
          {
            geometry = (Geometry *) factory->_cb_geometry (cdata->_comm);
            Geometry::updateCachedGeometry((CCMI_Geometry_t *)geometry, cdata->_comm);
          }

          CCMI::Executor::OldBarrier *executor = (CCMI::Executor::OldBarrier*)
                                              geometry->getBarrierExecutor();
          CCMI_assert (executor != NULL);
          TRACE_INIT((stderr,"<%#.8X>CCMI::Adaptor::Barrier::BarrierFactory::cb_head(%d,%x)\n",
                     (int)factory,cdata->_comm,(int)executor));

          //Override poly morphism
          executor->CCMI::Executor::OldBarrier::notifyRecv (peer, *info, NULL, 0);

          *rcvlen    = 0;
          //*rcvbuf    = NULL;
          *pipewidth = 0;
          cb_done->function    = NULL;
          //cb_done->clientdata = NULL;

          return NULL;
        }
      };  //- OldBarrierFactory
////////////////////////////////////////////////////////////////////////////
    };
  };
};  //namespace CCMI::Adaptor::Barrier

#endif
