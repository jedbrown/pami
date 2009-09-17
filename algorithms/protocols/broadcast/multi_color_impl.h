/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/multi_color_impl.h
 * \brief ???
 */

#ifndef   __ccmi_adaptor_broadcast_multi_color_impl__
#define   __ccmi_adaptor_broadcast_multi_color_impl__

#include "algorithms/schedule/Rectangle.h"
#include "algorithms/schedule/BinomialTree.h"
#include "algorithms/schedule/RingSchedule.h"
#include "./MultiColorCompositeT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {


      inline void get_colors (Geometry                  * g,
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
      inline void get_colors_nopw (Geometry                  * g,
                                   unsigned                    bytes,
                                   CCMI::Schedule::Color     * colors,
                                   unsigned                  & ncolors,
                                   unsigned                  & pwidth)
      {
        ncolors = 1;
        colors[0] = CCMI::Schedule::NO_COLOR;
        pwidth = bytes;
      }


      typedef MultiColorCompositeT <1, CCMI::Schedule::BinomialTreeSchedule,
      get_colors,CCMI::CollectiveMapping> BinomialBcastComposite;
      template<> void BinomialBcastComposite::create_schedule ( void                      * buf,
                                                                unsigned                    size,
                                                                Geometry                  * g,
                                                                CCMI::Schedule::Color       color)
      {
        new (buf) CCMI::Schedule::BinomialTreeSchedule (_mapping, g->nranks(), g->ranks());
      }


      //typedef MultiColorCompositeT < MAX_BCAST_COLORS, CCMI::Schedule::OneColorRectBcastSched,
      //create_rschedule<CCMI::Schedule::OneColorRectBcastSched>,
      //get_rcolors<CCMI::Schedule::OneColorRectangle> > RectangleBcastComposite;

      typedef MultiColorCompositeT <1, CCMI::Schedule::RingSchedule,
      get_colors,CCMI::CollectiveMapping> RingBcastComposite;
      template<> void RingBcastComposite::create_schedule ( void                      * buf,
                                                            unsigned                    size,
                                                            Geometry                  * g,
                                                            CCMI::Schedule::Color       color)
      {
        new (buf) CCMI::Schedule::RingSchedule (_mapping, g->nranks(), g->ranks());
      }


      typedef MultiColorBroadcastFactoryT <BinomialBcastComposite, true_analyze,CCMI::CollectiveMapping> BinomialBcastFactory;
      //typedef MultiColorBroadcastFactoryT <RectangleBcastComposite, rectangle_analyze> RectBcastFactory;
      typedef MultiColorBroadcastFactoryT <RingBcastComposite, true_analyze,CCMI::CollectiveMapping> RingBcastFactory;
    };
  };
};



#endif
