/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/ShortAsyncCompositeT.h
 * \brief CCMI allreduce async composite template
 *
 * \todo AsyncCompositeT and CompositeT are very similar and could be combined.
 */

#ifndef __algorithms_protocols_allreduce_ShortAsyncCompositeT_h__
#define __algorithms_protocols_allreduce_ShortAsyncCompositeT_h__

#include "algorithms/protocols/allreduce/AsyncCompositeT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      // class ShortAsyncCompositeT
      ///
      /// \brief Allreduce protocol composite
      ///
      ///
      ///
      template <class SCHEDULE, class EXECUTOR, class MAP> class ShortAsyncCompositeT : public CCMI::Adaptor::Allreduce::AsyncCompositeT<SCHEDULE,EXECUTOR,MAP>
      {
      protected:
        unsigned _my_coord   [5]; //T,X,Y,Z,T
        unsigned _rect_start [5]; //T,X,Y,Z,T
      public:
        ShortAsyncCompositeT (PAMI_CollectiveRequest_t  * req,
                              MAP             * map,
                              CCMI::ConnectionManager::ConnectionManager *cmgr,
                              PAMI_Callback_t             cb_done,
                              CCMI_Consistency            consistency,
                              CCMI::MultiSend::OldMulticastInterface *mf,
                              Geometry                  * geometry,
                              char                      * srcbuf,
                              char                      * dstbuf,
                              unsigned                    offset,
                              unsigned                    count,
                              PAMI_Dt                     dtype,
                              PAMI_Op                     op,
                              ConfigFlags                 flags,
                              CollectiveProtocolFactory           * factory,
                              unsigned                    iteration,
                              int                         root = -1,
                              CCMI::Schedule::Color       color=CCMI::Schedule::XP_Y_Z) :
        CCMI::Adaptor::Allreduce::AsyncCompositeT<SCHEDULE,EXECUTOR,MAP>(req,
                                                                         map,
                                                                         cmgr,
                                                                         cb_done,
                                                                         consistency,
                                                                         mf,
                                                                         geometry,
                                                                         srcbuf,
                                                                         dstbuf,
                                                                         offset,
                                                                         count,
                                                                         dtype,
                                                                         op,
                                                                         flags,
                                                                         factory,
                                                                         iteration,
                                                                         root,
                                                                         color)
        {
            _my_coord[0] = map->GetCoord(CCMI_T_DIM);
            _my_coord[4] = map->GetCoord(CCMI_T_DIM);
            _my_coord[1] = map->GetCoord(CCMI_X_DIM);
            _my_coord[2] = map->GetCoord(CCMI_Y_DIM);
            _my_coord[3] = map->GetCoord(CCMI_Z_DIM);

            _rect_start [0] = geometry->rectangle()->t0;
            _rect_start [1] = geometry->rectangle()->x0;
            _rect_start [2] = geometry->rectangle()->y0;
            _rect_start [3] = geometry->rectangle()->z0;
            _rect_start [4] = geometry->rectangle()->t0;
        }
        unsigned getSrcPeIndex (unsigned phase, unsigned connid)
        {
          unsigned coord = _my_coord [phase];
          unsigned rstart = _rect_start[phase];
          TRACE_ADAPTOR((stderr,
                         "<%p>Allreduce::Short::%s::AsyncCompositeT::getSrcPeIndex phase%d, connid %d\n",
                         this, CCMI::Adaptor::Allreduce::AsyncCompositeT<SCHEDULE,EXECUTOR>::name, phase, connid));
          CCMI_assert (connid != coord); //A node cant receive a message from itself
          return(connid < coord) ? (connid - rstart) : (connid - rstart - 1);
        }
      };
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
