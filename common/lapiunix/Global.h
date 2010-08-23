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

namespace PAMI
{
    class Global : public Interface::Global<PAMI::Global>
    {
      public:

        inline Global () :
          Interface::Global<PAMI::Global>(),
          mapping()
        {
          // Time gets its own clockMHz
          time.init(0);
          {
                size_t min=0, max=0;
                mapping.init(min, max);
#if 0

                PAMI::Topology::static_init(&mapping);
                if (mapping.size() == max - min + 1) {
                        new (&topology_global) PAMI::Topology(min, max);
                } else {
                        PAMI_abortf("failed to build global-world topology %zu:: %zu..%zu", mapping.size(), min, max);
                }
                topology_global.subTopologyLocalToMe(&topology_local);
#endif
          }
        };



        inline ~Global () {};

      public:

        PAMI::Mapping		mapping;

  };   // class Global
};     // namespace PAMI

extern PAMI::Global __global;

#endif // __pami_common_lapiunix_global_h__
