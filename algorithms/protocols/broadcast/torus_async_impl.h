/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/torus_async_impl.h
 * \brief ???
 */

#ifndef   __ccmi_adaptor_broadcast_torus_async_impl__
#define   __ccmi_adaptor_broadcast_torus_async_impl__

#include "algorithms/schedule/BinomialTree.h"
#include "algorithms/schedule/RingSchedule.h"
#include "./AsyncCompositeT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {

      typedef
      AsyncCompositeT < CCMI::Schedule::OneColorTorusRect, CCMI::TorusCollectiveMapping> AsyncTorusRectComposite;
      template<>
      void AsyncTorusRectComposite::create_schedule(void                      * buf,
                                                    unsigned                      size,
                                                    unsigned                      root,
                                                    CCMI::TorusCollectiveMapping* map,
                                                    Geometry                    * g)
      {
        CCMI::Schedule::Color colorS[6];
        int ideal = 0, max = 0;
        //Get the available color for this collective
        CCMI::Schedule::OneColorRectangle::getColors (*g->rectangle_mesh(), ideal, max, colorS);

        /// Async broadcast uses only one color, but each root
        /// may choose a different color so that if several nodes
        /// broadcast on a rectangle they will utilize all
        /// available colors
        CCMI::Schedule::Color color = colorS[root % ideal];

        new (buf) CCMI::Schedule::OneColorTorusRect(map, color, *g->rectangle_mesh());
      }

      //typedef AsyncCompositeT < CCMI::Schedule::OneColorRectBcastSched, CCMI::TorusCollectiveMapping>
      // AsyncTorusRectComposite;


      typedef AsyncCompositeFactoryT <AsyncTorusRectComposite, rectangle_analyze, CCMI::TorusCollectiveMapping>
      AsyncTorusRectFactory;
    };
  };
};

#endif
