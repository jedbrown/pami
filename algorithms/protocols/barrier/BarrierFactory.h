/**
 * \file algorithms/protocols/barrier/BarrierFactory.h
 * \brief ???
 */
#ifndef __algorithms_protocols_barrier_BarrierFactory_h__
#define __algorithms_protocols_barrier_BarrierFactory_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/executor/Barrier.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {

      // Old, deprecated interfaces for transition from OldMulticast to Multisync
      ///
      /// \brief Binomial barrier
      ///
      typedef bool (*AnalyzeFn) (PAMI_GEOMETRY_CLASS *g);
      template <class T_Schedule, AnalyzeFn afn, class T_Mcast>
      class OldBarrierT : public CCMI::Executor::Composite
      {
      protected:
        ///
        /// \brief The schedule for binomial barrier protocol
        ///
        CCMI::Executor::OldBarrier<T_Mcast>    _myexecutor;
        T_Schedule                             _myschedule;

      public:
        ///
        /// \brief Constructor for non-blocking barrier protocols.
        ///
        /// \param[in] mInterface  The multicast Interface
        /// \param[in] geometry    Geometry object
        ///
        OldBarrierT  (T_Mcast             * mInterface,
                      PAMI_GEOMETRY_CLASS  * geometry) :
        Composite(),
          _myexecutor (geometry->nranks(),
                       geometry->ranks(),
                       geometry->comm(),
                       0U,
                       mInterface),
          _myschedule (geometry->nranks(), geometry->ranks())
        {
          TRACE_INIT((stderr,"<%p>CCMI::Adaptors::Barrier::BarrierT::ctor(%X)\n",
                      this, geometry->comm()));
          _myexecutor.setCommSchedule (&_myschedule);
        }

        static bool analyze (PAMI_GEOMETRY_CLASS *geometry)
        {
          return((AnalyzeFn) afn)(geometry);
        }

        virtual void start() {
          _myexecutor.setDoneCallback (_cb_done, _clientdata);
          _myexecutor.start();
        }

        Executor::OldBarrier<T_Mcast> * getExecutor() { return &_myexecutor; }
      }; //- OldBarrierT


      ///
      /// \brief Barrier Factory Base class.
      ///
      template <class T, class T_Mcast>
      class OldBarrierFactoryT : public CollectiveProtocolFactory
      {
      protected:
        T_Mcast                * _mcastInterface;

      public:
        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
          CCMI_abort();
        }

        ///
        /// \brief Constructor for barrier factory implementations.
        ///
        OldBarrierFactoryT (T_Mcast                *minterface,
                            pami_mapidtogeometry_fn  cb_geometry) :
        _mcastInterface (minterface)
        {
          TRACE_INIT((stderr,"<%p>CCMI::Collectives::Barrier::BarrierFactory::ctor(%d)\n",
                     this,(int)cb_geometry));
          minterface->setCallback (cb_head, this);
          setMapIdToGeometry (cb_geometry);
        }

        virtual void metadata(pami_metadata_t *mdata)
        {
            strcpy(mdata->name, "OldCCMIBarrier");
        }

        virtual Executor::Composite * generate(pami_geometry_t              geometry,
                                               void                      * cmd)

          {
            PAMI_GEOMETRY_CLASS  *g = ( PAMI_GEOMETRY_CLASS *)geometry;
            return (Executor::Composite *) g->getKey((size_t)0, /// \todo does NOT support multicontext
                                                     PAMI::Geometry::CKEY_BARRIERCOMPOSITE0);
          }



        ///
        /// \brief Generate a non-blocking barrier message.
        ///
        /// \param[in]  request      Opaque memory to maintain internal
        ///                          message state.
        /// \param[in]  geometry     Geometry for the barrier operation
        ///
        /// \retval     executor     Pointer to barrier executor
        ///
        CCMI::Executor::Composite *generate
        (void                                * request,
         unsigned                              rsize,
         pami_context_t                         context,
         pami_geometry_t                        g,
         void                                * cmd)
        {
          CCMI_assert(rsize >= sizeof(T));
          PAMI_GEOMETRY_CLASS  *geometry = (PAMI_GEOMETRY_CLASS *)g;
          return new (request) T (this->_mcastInterface, geometry);
        }

        static pami_quad_t *   cb_head   (pami_context_t   ctxt,
                                          const pami_quad_t    * info,
                                         unsigned          count,
                                         unsigned          peer,
                                         size_t            sndlen,
                                         size_t            conn_id,
                                         void            * arg,
                                         unsigned        * rcvlen,
                                         char           ** rcvbuf,
                                         unsigned        * pipewidth,
                                         PAMI_Callback_t * cb_done)
        {
          CollHeaderData  *cdata = (CollHeaderData *) info;
          OldBarrierFactoryT *factory = (OldBarrierFactoryT *) arg;

          PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry(ctxt, cdata->_comm);

          PAMI_assert(geometry != NULL);

          T *composite = (T*) geometry->getKey((size_t)0, /// \todo does NOT support multicontext
                                               PAMI::Geometry::CKEY_BARRIERCOMPOSITE0);
          CCMI_assert (composite != NULL);
          TRACE_INIT((stderr,"<%p>CCMI::Adaptor::Barrier::BarrierFactory::cb_head(%d,%p)\n",
                      factory,cdata->_comm,composite));

          //Override poly morphism
          composite->getExecutor()->notifyRecv (peer, *info, NULL, 0);

          *rcvlen    = 0;
          //*rcvbuf    = NULL;
          *pipewidth = 0;
          cb_done->function    = NULL;
          //cb_done->clientdata = NULL;

          return NULL;
        }
      };  //- OldBarrierFactoryT
////////////////////////////////////////////////////////////////////////////
    };
  };
};  //namespace CCMI::Adaptor::Barrier

#endif
