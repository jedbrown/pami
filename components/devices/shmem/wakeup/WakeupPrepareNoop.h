///
/// \file components/wakeup/WakeupPrepareNoop.h
/// \brief Empty "noop" implementation of the wakeup prepare interface
///
#ifndef __components_wakeup_WakeupPrepareNoop_h__
#define __components_wakeup_WakeupPrepareNoop_h__

#include "components/wakeup/WakeupInterface.h"

namespace PAMI
{
  namespace Wakeup
  {
    namespace Prepare
    {
      class Noop : public Interface::WakeupPrepare <Noop>
      {
        public:
          friend class Interface::WakeupPrepare <Noop>;
          inline bool prepareToSleepFunctor_impl() { return true; };
      }; // class PAMI::Wakeup::Prepare::Noop
    }; // namespace PAMI::Wakeup::Prepare
  }; // namespace PAMI::Wakeup
}; // namespace PAMI

#endif // __components_wakeup_WakeupPrepareNoop_h__
