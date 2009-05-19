
#ifndef   __ccmi_adaptor_mc_broadcast_impl__
#define   __ccmi_adaptor_mc_broadcast_impl__

#include "schedule/BinomialTree.h"
#include "schedule/RingSchedule.h"
#include "MultiColorCompositeT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {

      template <class T> void create_schedule (void                      * buf,
                                               unsigned                    size,
                                               CCMI::Mapping             * map,
                                               Geometry                  * g,
                                               CCMI::Schedule::Color       color)
      {
        new (buf, size) T (map, g->nranks(), g->ranks());
      }

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
      create_schedule<CCMI::Schedule::BinomialTreeSchedule>, get_colors> BinomialBcastComposite;      

      typedef MultiColorCompositeT <1, CCMI::Schedule::RingSchedule, 
      create_schedule<CCMI::Schedule::RingSchedule>, get_colors> RingBcastComposite;      

      typedef MultiColorBroadcastFactoryT <BinomialBcastComposite, true_analyze> BinomialBcastFactory;
      typedef MultiColorBroadcastFactoryT <RingBcastComposite, true_analyze> RingBcastFactory;
    };
  };
};



#endif
