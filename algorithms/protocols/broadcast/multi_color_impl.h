/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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
#include "algorithms/protocols/broadcast/MultiColorCompositeT.h"
#include "algorithms/connmgr/ColorGeometryConnMgr.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {


      void get_colors (XMI_GEOMETRY_CLASS                  * g,
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
      inline void get_colors_nopw (XMI_GEOMETRY_CLASS                  * g,
                                   unsigned                    bytes,
                                   CCMI::Schedule::Color     * colors,
                                   unsigned                  & ncolors,
                                   unsigned                  & pwidth)
      {
        ncolors = 1;
        colors[0] = CCMI::Schedule::NO_COLOR;
        pwidth = bytes;
      }


      typedef MultiColorCompositeT <1,
                                    CCMI::Schedule::BinomialTreeSchedule<XMI_COLL_SYSDEP_CLASS>,
                                    get_colors,
                                    XMI_COLL_SYSDEP_CLASS,
                                    XMI_COLL_MCAST_CLASS,
                                    CCMI::ConnectionManager::ColorGeometryConnMgr<XMI_COLL_SYSDEP_CLASS> >
                                    BinomialBcastComposite;
      template<> void BinomialBcastComposite::create_schedule ( void                      * buf,
                                                                unsigned                    size,
                                                                XMI_GEOMETRY_CLASS                  * g,
                                                                CCMI::Schedule::Color       color)
      {
        new (buf) CCMI::Schedule::BinomialTreeSchedule<XMI_COLL_SYSDEP_CLASS> (_sd, g->nranks(), g->ranks());
      }


      //typedef MultiColorCompositeT < MAX_BCAST_COLORS, CCMI::Schedule::OneColorRectBcastSched,
      //create_rschedule<CCMI::Schedule::OneColorRectBcastSched>,
      //get_rcolors<CCMI::Schedule::OneColorRectangle> > RectangleBcastComposite;

      typedef MultiColorCompositeT <1,
                                    CCMI::Schedule::RingSchedule<XMI_COLL_SYSDEP_CLASS>,
                                    get_colors,
                                    XMI_COLL_SYSDEP_CLASS,
                                    XMI_COLL_MCAST_CLASS,
                                    CCMI::ConnectionManager::ColorGeometryConnMgr<XMI_COLL_SYSDEP_CLASS> >
                                    RingBcastComposite;
      template<> void RingBcastComposite::create_schedule ( void                      * buf,
                                                            unsigned                    size,
                                                            XMI_GEOMETRY_CLASS        * g,
                                                            CCMI::Schedule::Color       color)
      {
        new (buf) CCMI::Schedule::RingSchedule<XMI_COLL_SYSDEP_CLASS> (_sd, g->nranks(), g->ranks());
      }


      typedef MultiColorBroadcastFactoryT <BinomialBcastComposite,
                                           true_analyze,
                                           XMI_COLL_SYSDEP_CLASS,
                                           XMI_COLL_MCAST_CLASS,
                                           CCMI::ConnectionManager::ColorGeometryConnMgr<XMI_COLL_SYSDEP_CLASS> > BinomialBcastFactory;
      //typedef MultiColorBroadcastFactoryT <RectangleBcastComposite, rectangle_analyze> RectBcastFactory;
      typedef MultiColorBroadcastFactoryT <RingBcastComposite,
                                           true_analyze,
                                           XMI_COLL_SYSDEP_CLASS,
                                           XMI_COLL_MCAST_CLASS,
                                           CCMI::ConnectionManager::ColorGeometryConnMgr<XMI_COLL_SYSDEP_CLASS> > RingBcastFactory;
    };
  };
};



#endif
