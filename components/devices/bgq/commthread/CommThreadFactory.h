/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file components/devices/bgq/commthread/CommThreadWakeup.h
/// \brief

#ifndef __components_devices_bgq_commthread_CommFactory_h__
#define __components_devices_bgq_commthread_CommFactory_h__

namespace PAMI {
namespace Device {
namespace CommThread {

class BgqCommThread;
class Factory {
public:
        Factory(PAMI::Memory::MemoryManager *genmm, PAMI::Memory::MemoryManager *l2xmm);
        ~Factory();
        BgqCommThread *getCommThreads() { return _commThreads; }
private:
        BgqCommThread *_commThreads;
}; // class Factory 

}; // namespace CommThread
}; // namespace Device
}; // namespace PAMI

#endif // __components_devices_bgq_commthread_CommFactory_h__
