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
         XMI_GEOMETRY_CLASS                  * geometry) = 0;

      };  //- BarrierFactory

      ///
      /// \brief Barrier Factory Base class.
      ///
      template<class T_Sysdep, class T_Msync>
        class BarrierFactory : private BaseBarrierFactory
        {
        protected:
          T_Msync               * _msyncInterface;
          T_Sysdep              * _mapping;
          xmi_mapidtogeometry_fn    _cb_geometry;

        public:
          /// NOTE: This is required to make "C" programs link successfully with virtual destructors
          void operator delete(void * p)
          {
            CCMI_abort();
          }

          ///
          /// \brief Constructor for barrier factory implementations.
          ///
          BarrierFactory (T_Msync               * minterface,
                          T_Sysdep              * map,
                          xmi_mapidtogeometry_fn    cb_geometry) :
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
      template<class T_Sysdep, class T_Mcast>
      class OldBarrierFactory : private BaseBarrierFactory
      {
      protected:
        T_Mcast                * _mcastInterface;
        T_Sysdep               * _mapping;
        xmi_mapidtogeometry_fn     _cb_geometry;

      public:
        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
          CCMI_abort();
        }

        ///
        /// \brief Constructor for barrier factory implementations.
        ///
        OldBarrierFactory (T_Mcast      * minterface,
                           T_Sysdep     * map,
                           xmi_mapidtogeometry_fn                       cb_geometry) :
        _mcastInterface (minterface),
        _mapping (map),
        _cb_geometry (cb_geometry)
        {
          TRACE_INIT((stderr,"<%#.8X>CCMI::Collectives::Barrier::BarrierFactory::ctor(%d)\n",
                     (int)this,(int)cb_geometry));
          minterface->setCallback (cb_head, this);
        }

        static xmi_quad_t *   cb_head   (const xmi_quad_t    * info,
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
          
          XMI_GEOMETRY_CLASS *geometry = (XMI_GEOMETRY_CLASS *) XMI_GEOMETRY_CLASS::getCachedGeometry(cdata->_comm);
          if(geometry == NULL)
          {
            geometry = (XMI_GEOMETRY_CLASS *) factory->_cb_geometry (cdata->_comm);
            XMI_GEOMETRY_CLASS::updateCachedGeometry(geometry, cdata->_comm);
          }
          assert(geometry != NULL);
          CCMI::Executor::OldBarrier<T_Mcast> *executor = (CCMI::Executor::OldBarrier<T_Mcast>*)
	    geometry->getKey(XMI::Geometry::XMI_GKEY_BARRIEREXECUTOR);
          CCMI_assert (executor != NULL);
          TRACE_INIT((stderr,"<%#.8X>CCMI::Adaptor::Barrier::BarrierFactory::cb_head(%d,%x)\n",
                     (int)factory,cdata->_comm,(int)executor));

          //Override poly morphism
          executor->CCMI::Executor::OldBarrier<T_Mcast>::notifyRecv (peer, *info, NULL, 0);

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
