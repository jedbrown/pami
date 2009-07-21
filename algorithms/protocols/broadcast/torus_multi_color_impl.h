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

#ifndef   __ccmi_adaptor_broadcast_torus_multi_color_impl__
#define   __ccmi_adaptor_broadcast_torus_multi_color_impl__

#include "algorithms/schedule/Rectangle.h"
#include "algorithms/schedule/OneColorTorusRect.h"
#include "./MultiColorCompositeT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {

      template <class R> void get_rcolors (Geometry                  * g,
                                           unsigned                    bytes,
                                           CCMI::Schedule::Color     * colors,
                                           unsigned                  & ncolors,
                                           unsigned                  & pwidth)
      {
        unsigned ideal = 0, max = 0;
        R::getColors (*g->rectangle(), ideal, max, colors);

        unsigned numEnvColors = g->getNumColors();       
        if( numEnvColors > 0  && ideal > numEnvColors )
          ideal = numEnvColors;

        CCMI_assert (ideal >= 1);

        pwidth = MIN_PIPELINE_WIDTH_TORUS;       

        if((bytes / ideal) > MIN_PIPELINE_WIDTH_TORUS * 256)
          pwidth = 32 * MIN_PIPELINE_WIDTH_TORUS;
        else if((bytes / ideal) > MIN_PIPELINE_WIDTH_TORUS * 128)
          pwidth = 16 * MIN_PIPELINE_WIDTH_TORUS;
        else if((bytes / ideal) > MIN_PIPELINE_WIDTH_TORUS * 32)
          pwidth = 4 * MIN_PIPELINE_WIDTH_TORUS;
        else if((bytes / ideal) > MIN_PIPELINE_WIDTH_TORUS * 8)
          pwidth = 2 * MIN_PIPELINE_WIDTH_TORUS;
        else if((bytes / ideal) > MIN_PIPELINE_WIDTH_TORUS * 2)
          pwidth = MIN_PIPELINE_WIDTH_TORUS;

        if(bytes < ideal * pwidth)
        {
          ideal = bytes / pwidth;       
          if(ideal == 0)
            ideal = 1;
        }

        CCMI_assert (ideal >= 1);

        ncolors = ideal;
      }

      typedef MultiColorCompositeT < MAX_BCAST_COLORS, CCMI::Schedule::OneColorTorusRect, 
      get_rcolors<CCMI::Schedule::OneColorTorusRect> ,CCMI::TorusCollectiveMapping> TorusRectComposite;            
      template<> void TorusRectComposite::create_schedule  ( void                      * buf,
                                                             unsigned                    size,
                                                             Geometry                  * g,
                                                             CCMI::Schedule::Color       color)
      {
        new (buf) CCMI::Schedule::OneColorTorusRect (_mapping, (unsigned)color, *g->rectangle());
      }

      typedef MultiColorBroadcastFactoryT <TorusRectComposite, rectangle_analyze,CCMI::TorusCollectiveMapping> TorusRectBcastFactory;
    };
  };
};



#endif
