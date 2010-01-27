/**
 * \file algorithms/protocols/broadcast/multi_color_impl.h
 * \brief ???
 */

#ifndef __algorithms_protocols_broadcast_multi_color_impl_h__
#define __algorithms_protocols_broadcast_multi_color_impl_h__

#include "algorithms/schedule/MultinomialTree.h"
#include "algorithms/protocols/broadcast/BcastMultiColorCompositeT.h"
#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/protocols/broadcast/old_multi_color_impl.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      void get_colors (XMI::Topology             * t,
		       unsigned                    bytes,
		       unsigned                  * colors,
		       unsigned                  & ncolors)
      {
        ncolors = 1;
        colors[0] = CCMI::Schedule::NO_COLOR;
      }

      typedef BcastMultiColorCompositeT <1,
	CCMI::Schedule::ListMultinomial,
	CCMI::ConnectionManager::ColorGeometryConnMgr<XMI_SYSDEP_CLASS>,
	get_colors> BinomialBcastComposite;

      typedef CollectiveProtocolFactoryT<BinomialBcastComposite, true_analyze, CCMI::ConnectionManager::ColorGeometryConnMgr<XMI_SYSDEP_CLASS> > BinomialBcastFactory;

      typedef BcastMultiColorCompositeT <1,
	CCMI::Schedule::RingSchedule,
	CCMI::ConnectionManager::ColorGeometryConnMgr<XMI_SYSDEP_CLASS>,
	get_colors> RingBcastComposite;

      typedef CollectiveProtocolFactoryT <RingBcastComposite, true_analyze, CCMI::ConnectionManager::ColorGeometryConnMgr<XMI_SYSDEP_CLASS> > RingBcastFactory;

    };
  };
};

#endif
