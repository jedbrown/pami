///
/// \file common/bgq/WakeupManager.h
/// \brief PAMI wakeup manager implementation.
///
#ifndef __common_bgq_WakeupManager_h__
#define __common_bgq_WakeupManager_h__

#include "common/WakeupManagerInterface.h"

#undef USE_WAKEUP_VECTORS

namespace PAMI {

        class WakeupManager : public Interface::WakeupManager<PAMI::WakeupManager> {
        public:
		inline WakeupManager() :
		Interface::WakeupManager<PAMI::WakeupManager>()
		{
		}

		inline pami_result_t init_impl(int num, int key) {
			return PAMI_ERROR;
		}

		inline void *getWakeupVec_impl(int num) {
			return NULL;
		}

		inline pami_result_t wakeup_impl(void *v) {
			return PAMI_ERROR;
		}

		inline pami_result_t clear_impl(void *v) {
			return PAMI_ERROR;
		}

		inline pami_result_t sleep_impl(void *v) {
			return PAMI_ERROR;
		}

		inline pami_result_t trySleep_impl(void *v) {
			return PAMI_ERROR;
		}

		inline pami_result_t poll_impl(void *v) {
			return PAMI_ERROR;
		}

	private:
        }; // class WakeupManager
}; // namespace PAMI

#endif // __common_bgq_WakeupManager_h__
