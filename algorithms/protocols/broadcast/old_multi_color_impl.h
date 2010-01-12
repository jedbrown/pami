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
      void old_get_colors (XMI_GEOMETRY_CLASS                  * g,
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
      inline void old_get_colors_nopw (XMI_GEOMETRY_CLASS                  * g,
				       unsigned                    bytes,
				       CCMI::Schedule::Color     * colors,
				       unsigned                  & ncolors,
				       unsigned                  & pwidth)
      {
        ncolors = 1;
        colors[0] = CCMI::Schedule::NO_COLOR;
        pwidth = bytes;
      }


      typedef OldMultiColorCompositeT <1,
                                    CCMI::Schedule::BinomialTreeSchedule<XMI_SYSDEP_CLASS>,
                                    old_get_colors,
                                    XMI_SYSDEP_CLASS,
                                    XMI_COLL_MCAST_CLASS,
                                    CCMI::ConnectionManager::ColorGeometryConnMgr<XMI_SYSDEP_CLASS> >
	OldBinomialBcastComposite;
      template<> void OldBinomialBcastComposite::create_schedule ( void                      * buf,
                                                                unsigned                    size,
                                                                XMI_GEOMETRY_CLASS                  * g,
                                                                CCMI::Schedule::Color       color)
      {
        new (buf) CCMI::Schedule::BinomialTreeSchedule<XMI_SYSDEP_CLASS> (_sd, g->nranks(), g->ranks());
      }


      //typedef MultiColorCompositeT < MAX_BCAST_COLORS, CCMI::Schedule::OneColorRectBcastSched,
      //create_rschedule<CCMI::Schedule::OneColorRectBcastSched>,
      //get_rcolors<CCMI::Schedule::OneColorRectangle> > RectangleBcastComposite;

      typedef OldMultiColorCompositeT <1,
	                              CCMI::Schedule::RingSchedule<XMI_SYSDEP_CLASS>,
	                              old_get_colors,
	                              XMI_SYSDEP_CLASS,
	                              XMI_COLL_MCAST_CLASS,
	                              CCMI::ConnectionManager::ColorGeometryConnMgr<XMI_SYSDEP_CLASS> >
                                      RingBcastComposite;
      template<> void RingBcastComposite::create_schedule ( void                      * buf,
                                                            unsigned                    size,
                                                            XMI_GEOMETRY_CLASS        * g,
                                                            CCMI::Schedule::Color       color)
      {
        new (buf) CCMI::Schedule::RingSchedule<XMI_SYSDEP_CLASS> (_sd, g->nranks(), g->ranks());
      }


      typedef OldMultiColorBroadcastFactoryT <OldBinomialBcastComposite,
                                           true_analyze,
                                           XMI_SYSDEP_CLASS,
                                           XMI_COLL_MCAST_CLASS,
                                           CCMI::ConnectionManager::ColorGeometryConnMgr<XMI_SYSDEP_CLASS> > OldBinomialBcastFactory;
      //typedef MultiColorBroadcastFactoryT <RectangleBcastComposite, rectangle_analyze> RectBcastFactory;
      typedef OldMultiColorBroadcastFactoryT <RingBcastComposite,
                                           true_analyze,
                                           XMI_SYSDEP_CLASS,
                                           XMI_COLL_MCAST_CLASS,
                                           CCMI::ConnectionManager::ColorGeometryConnMgr<XMI_SYSDEP_CLASS> > RingBcastFactory;
    };
  };
};



#endif
