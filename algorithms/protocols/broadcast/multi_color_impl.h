/**
 * \file algorithms/protocols/broadcast/multi_color_impl.h
 * \brief ???
 */

#ifndef __algorithms_protocols_broadcast_multi_color_impl_h__
#define __algorithms_protocols_broadcast_multi_color_impl_h__

#include "algorithms/schedule/MultinomialTree.h"
#include "algorithms/schedule/RingSchedule.h"
#include "algorithms/protocols/broadcast/BcastMultiColorCompositeT.h"
#include "algorithms/connmgr/ColorGeometryConnMgr.h"
//#include "algorithms/protocols/broadcast/old_multi_color_impl.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      void get_colors (PAMI::Topology             * t,
                       unsigned                    bytes,
                       unsigned                  * colors,
                       unsigned                  & ncolors)
      {
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        ncolors = 1;
        colors[0] = CCMI::Schedule::NO_COLOR;
      }

      void binom_bcast_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        strcpy(&m->name[0], "CCMIBinomBcast");
      }

      void ring_bcast_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        strcpy(&m->name[0], "CCMIRingBcast");
      }

      typedef BcastMultiColorCompositeT < 1,
      CCMI::Schedule::ListMultinomial,
      CCMI::ConnectionManager::ColorGeometryConnMgr,
      get_colors > BinomialBcastComposite;

      typedef CollectiveProtocolFactoryT < BinomialBcastComposite,
      binom_bcast_md,
      CCMI::ConnectionManager::ColorGeometryConnMgr> BinomialBcastFactory;

      typedef BcastMultiColorCompositeT < 1,
      CCMI::Schedule::RingSchedule,
      CCMI::ConnectionManager::ColorGeometryConnMgr,
      get_colors > RingBcastComposite;

      typedef CollectiveProtocolFactoryT < RingBcastComposite,
      ring_bcast_md,
      CCMI::ConnectionManager::ColorGeometryConnMgr> RingBcastFactory;

    };
  };
};

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
