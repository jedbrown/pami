/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file components/devices/bgq/commthread/CommThreadWakeupStatic.h
/// \brief

#ifndef __components_devices_bgq_commthread_CommThreadWakeupStatic_h__
#define __components_devices_bgq_commthread_CommThreadWakeupStatic_h__

#include <pami.h>
#include "components/memory/MemoryManager.h"

namespace PAMI {
namespace Device {
namespace CommThread {

class BgqCommThread;
class Factory {
public:

// Called from __global...
//
// This isn't really a device, only using this for convenience but
//
static BgqCommThread *generate(Memory::MemoryManager *genmm,
                                                Memory::MemoryManager *l2xmm);

static pami_result_t shutdown(BgqCommThread *devs);

}; // class Factory

}; // namespace CommThread
}; // namespace Device
}; // namespace PAMI

#endif // __components_devices_bgq_commthread_CommThreadWakeupStatic_h__
