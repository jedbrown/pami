/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/ShortAsyncFactoryT.h
 * \brief  CCMI factory for async [all]reduce compositeT
 *
 * \todo AsyncFactoryT and FactoryT are very similar and could be combined.
 * \todo should the mapping be a template parameter?  CollectiveMapping vs CollectiveMapping
 */

#ifndef __algorithms_protocols_allreduce_ShortAsyncFactoryT_h__
#define __algorithms_protocols_allreduce_ShortAsyncFactoryT_h__

#include "algorithms/protocols/allreduce/AsyncFactoryT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      /// class ShortAsyncFactoryT
      ///
      /// \brief Factory template class for ShortCompositeT
      ///
      /// This factory will generate a ShortCompositeT [all]reduce.
      ///
      template <class CONNMGR, class COMPOSITE, class MAP> class ShortAsyncFactoryT : public CCMI::Adaptor::Allreduce::AsyncFactoryT<CONNMGR, COMPOSITE, MAP>
      {
      protected:
        static PAMI_Request_t * cb_asyncShortReceiveHead(const PAMIQuad    * info,
                                                         unsigned          count,
                                                         unsigned          peer,
                                                         unsigned          sndlen,
                                                         unsigned          conn_id,
                                                         void            * arg,
                                                         unsigned        * rcvlen,
                                                         char           ** rcvbuf,
                                                         unsigned        * pipewidth,
                                                         PAMI_Callback_t * cb_done)
        {
          TRACE_ADAPTOR((stderr,
                         "<%p>Allreduce::Short::%s::AsyncFactoryT::cb_asyncShortReceiveHead peer %d, conn_id %d\n",
                         (int)arg, COMPOSITE::name, peer, conn_id));
          CCMI_assert (info && arg);
          CollHeaderData  *cdata = (CollHeaderData *) info;
          ShortAsyncFactoryT *factory = (ShortAsyncFactoryT *) arg;

          Geometry *geometry = (Geometry *)factory->_cb_geometry(cdata->_comm);
          COMPOSITE *composite =
          (COMPOSITE *) factory->getAllreduceComposite(geometry, cdata->_iteration);

          TRACE_ADAPTOR((stderr,
                         "<%p>Allreduce::Short::%s::AsyncFactoryT::cb_asyncShortReceiveHead "
                         "comm %#X, root %#X, count %#X, dt %#X, op %#X, iteration %#X,"
                         "composite %p, %s\n",
                         factory, COMPOSITE::name, cdata->_comm, cdata->_root, cdata->_count,
                         cdata->_dt, cdata->_op, cdata->_iteration,
                         composite,
                         (composite == NULL?" ":
                          ((composite->isIdle())?"(Idle)":" "))));

          CCMI::Executor::OldAllreduceBase *allreduce = NULL;
          if(composite == NULL)
          {
            composite = (COMPOSITE *) factory->buildComposite (geometry, cdata);
            allreduce = (CCMI::Executor::OldAllreduceBase *) composite->getExecutor (0);
          }
          else
          {
            ///Get the allreduce executor
            allreduce = (CCMI::Executor::OldAllreduceBase *) composite->getExecutor (0);

            if(composite->isIdle())
              composite->restartAsync(allreduce,
                                      cdata->_count,
                                      (PAMI_Dt)(cdata->_dt),
                                      (PAMI_Op)(cdata->_op),
                                      cdata->_root);
          }

          *pipewidth = (unsigned) -1;
          allreduce->notifyRecvShort (cdata->_phase,
                                      sndlen,
                                      composite->getSrcPeIndex(cdata->_phase, conn_id),
                                      rcvlen,
                                      rcvbuf,
                                      cb_done);
          return NULL;  //This must be a short one packet message
        }
      public:
        inline ShortAsyncFactoryT(MAP *mapping,
                                  CCMI::MultiSend::OldMulticastInterface *mf,
                                  CCMI_mapIdToGeometry cb_geometry,
                                  ConfigFlags flags) :
        CCMI::Adaptor::Allreduce::AsyncFactoryT<CONNMGR, COMPOSITE, MAP>(mapping, mf, cb_geometry, flags)
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::Short::%s::FactoryT() ALERT\n",this, COMPOSITE::name));
          TRACE_ADAPTOR((stderr, "<%p>Allreduce::Short::%s::FactoryT()\n",this, COMPOSITE::name));
          mf->setCallback (cb_asyncShortReceiveHead, this);
        }
      };
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
