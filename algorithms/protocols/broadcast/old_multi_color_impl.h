/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/old_multi_color_impl.h
 * \brief ???
 */

#ifndef __algorithms_protocols_broadcast_old_multi_color_impl_h__
#define __algorithms_protocols_broadcast_old_multi_color_impl_h__

#include "algorithms/schedule/Rectangle.h"
#include "algorithms/schedule/BinomialTree.h"
#include "algorithms/schedule/RingSchedule.h"
#include "algorithms/protocols/broadcast/OldMultiColorCompositeT.h"
#include "algorithms/connmgr/ColorGeometryConnMgr.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      void old_get_colors (PAMI_GEOMETRY_CLASS                  * g,
                           unsigned                    bytes,
                           CCMI::Schedule::Color     * colors,
                           unsigned                  & ncolors,
                           unsigned                  & pwidth)
      {
        ncolors = 1;
        colors[0] = CCMI::Schedule::NO_COLOR;
        pwidth = MIN_PIPELINE_WIDTH;
      }

      //This method is for schedules/executors that dont use pipelining
      inline void old_get_colors_nopw (PAMI_GEOMETRY_CLASS                  * g,
                                       unsigned                    bytes,
                                       CCMI::Schedule::Color     * colors,
                                       unsigned                  & ncolors,
                                       unsigned                  & pwidth)
      {
        ncolors = 1;
        colors[0] = CCMI::Schedule::NO_COLOR;
        pwidth = bytes;
      }

      void old_binom_bcast_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"OldCCMIBinomBcast");
      }

      void old_ring_bcast_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"OldCCMIRingBcast");
      }



      typedef OldMultiColorCompositeT <1,
                                    CCMI::Schedule::BinomialTreeSchedule,
                                    old_get_colors,
                                    PAMI_COLL_MCAST_CLASS,
                                    CCMI::ConnectionManager::ColorGeometryConnMgr>
        OldBinomialBcastComposite;
      template<> void OldBinomialBcastComposite::create_schedule ( void                      * buf,
                                                                unsigned                    size,
                                                                PAMI_GEOMETRY_CLASS                  * g,
                                                                CCMI::Schedule::Color       color)
      {
        new (buf) CCMI::Schedule::BinomialTreeSchedule (g->nranks(), g->ranks());
      }


      //typedef MultiColorCompositeT < MAX_BCAST_COLORS, CCMI::Schedule::OneColorRectBcastSched,
      //create_rschedule<CCMI::Schedule::OneColorRectBcastSched>,
      //get_rcolors<CCMI::Schedule::OneColorRectangle> > RectangleBcastComposite;

      typedef OldMultiColorCompositeT <1,
                                      CCMI::Schedule::OldRingSchedule,
                                      old_get_colors,
                                      PAMI_COLL_MCAST_CLASS,
                                      CCMI::ConnectionManager::ColorGeometryConnMgr>
                                      OldRingBcastComposite;
      template<> void OldRingBcastComposite::create_schedule ( void                      * buf,
                                                            unsigned                    size,
                                                            PAMI_GEOMETRY_CLASS        * g,
                                                            CCMI::Schedule::Color       color)
      {
        new (buf) CCMI::Schedule::OldRingSchedule (g->nranks(), g->ranks());
      }


      typedef OldMultiColorBroadcastFactoryT <OldBinomialBcastComposite,
                                           old_binom_bcast_md,
                                           PAMI_COLL_MCAST_CLASS,
                                           CCMI::ConnectionManager::ColorGeometryConnMgr> OldBinomialBcastFactory;
      //typedef MultiColorBroadcastFactoryT <RectangleBcastComposite, rectangle_analyze> RectBcastFactory;
      typedef OldMultiColorBroadcastFactoryT <OldRingBcastComposite,
                                           old_ring_bcast_md,
                                           PAMI_COLL_MCAST_CLASS,
                                           CCMI::ConnectionManager::ColorGeometryConnMgr> OldRingBcastFactory;
    };
  };
};



#endif
