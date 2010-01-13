/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __common_lapiunix_Global_h__
#define __common_lapiunix_Global_h__
///
/// \file common/lapiunix/Global.h
/// \brief Global Object
///
/// This global object is constructed before main() and is a container class
/// for all other classes that need to be constructed and initialized before
/// the application starts.
///

#include "Platform.h"
#include "util/common.h"
#include "common/GlobalInterface.h"
#include "Mapping.h"
#include "Topology.h"
#include "Wtime.h"

namespace XMI
{
    class Global : public Interface::Global<XMI::Global>
    {
      public:

        inline Global () :
	  Interface::Global<XMI::Global>(),
	  mapping()
        {
	  // Time gets its own clockMHz
	  Interface::Global<XMI::Global>::time.init(0);
	  {
		size_t min=0, max=0;
		mapping.init(min, max);
#if 0

		XMI::Topology::static_init(&mapping);
		if (mapping.size() == max - min + 1) {
			new (&topology_global) XMI::Topology(min, max);
		} else {
			XMI_abortf("failed to build global-world topology %zd:: %zd..%zd", mapping.size(), min, max);
		}
		topology_global.subTopologyLocalToMe(&topology_local);
#endif
	  }
        };



        inline ~Global () {};

      public:

	XMI::Mapping		mapping;

  };   // class Global
};     // namespace XMI

extern XMI::Global __global;

#endif // __xmi_common_lapiunix_global_h__
