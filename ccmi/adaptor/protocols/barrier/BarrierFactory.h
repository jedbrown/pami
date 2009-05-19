
#ifndef __ccmi_collectives_barrier_factory_h__
#define __ccmi_collectives_barrier_factory_h__

#include "../ProtocolFactory.h"
#include "../../../executor/Barrier.h"
#include "../../geometry/Geometry.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {
      ///
      /// \brief Barrier Factory Base class.
      ///
      class BarrierFactory : private ProtocolFactory
      {
      protected:
        CCMI::MultiSend::MulticastInterface          * _mcastInterface;
        CCMI::Mapping                                * _mapping;
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
        BarrierFactory (CCMI::MultiSend::MulticastInterface      * minterface,
                        CCMI::Mapping                            * map,
                        CCMI_mapIdToGeometry                       cb_geometry) :
        _mcastInterface (minterface),
        _mapping (map),
        _cb_geometry (cb_geometry)
        {
          TRACE_INIT((stderr,"<%#.8X>CCMI::Collectives::Barrier::BarrierFactory::ctor(%d)\n",
                     (int)this,(int)cb_geometry));
          minterface->setCallback (cb_head, this);
        }

        //   virtual bool Analyze(Geometry *geometry) = 0;

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

        static CCMI_Request_t *   cb_head   (const CCMIQuad    * info,
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
          CollHeaderData  *cdata = (CollHeaderData *) info;      
          BarrierFactory *factory = (BarrierFactory *) arg;

          Geometry *geometry = (Geometry *) Geometry::getCachedGeometry(cdata->_comm);
          if(geometry == NULL)
          {
            geometry = (Geometry *) factory->_cb_geometry (cdata->_comm);
            Geometry::updateCachedGeometry((CCMI_Geometry_t *)geometry, cdata->_comm);
          }

          CCMI::Executor::Barrier *executor = (CCMI::Executor::Barrier*)
                                              geometry->getBarrierExecutor();
          CCMI_assert (executor != NULL);
          TRACE_INIT((stderr,"<%#.8X>CCMI::Adaptor::Barrier::BarrierFactory::cb_head(%d,%x)\n",
                     (int)factory,cdata->_comm,(int)executor));

          //Override poly morphism
          executor->CCMI::Executor::Barrier::notifyRecv (peer, *info, NULL, 0);

          *rcvlen    = 0;
          //*rcvbuf    = NULL;
          *pipewidth = 0;
          cb_done->function    = NULL;
          //cb_done->clientdata = NULL;

          return NULL;
        }
      };  //- BarrierFactory
    };
  };
};  //namespace CCMI::Adaptor::Barrier

#endif
