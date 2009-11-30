
///
/// \file common/bgp/WakeupManager.h
/// \brief XMI wakeup manager implementation.
///
#ifndef __common_bgp_WakeupManager_h__
#define __common_bgp_WakeupManager_h__

#include "common/WakeupManagerInterface.h"

#undef USE_WAKEUP_VECTORS

namespace XMI {

        class WakeupManager : public Interface::WakeupManager<XMI::WakeupManager> {
        public:
		inline WakeupManager() :
		Interface::WakeupManager<XMI::WakeupManager>()
		{
		}

		inline xmi_result_t init_impl(int num, int key) {
			return XMI_ERROR;
		}

		inline void *getWakeupVec_impl(int num) {
			return NULL;
		}

		inline xmi_result_t wakeup_impl(void *v) {
			return XMI_ERROR;
		}

		inline xmi_result_t clear_impl(void *v) {
			return XMI_ERROR;
		}

		inline xmi_result_t sleep_impl(void *v) {
			return XMI_ERROR;
		}

		inline xmi_result_t try_impl(void *v) {
			return XMI_ERROR;
		}

		inline xmi_result_t poll_impl(void *v) {
			return XMI_ERROR;
		}

	private:
        }; // class WakeupManager
}; // namespace XMI

#endif // __common_bgp_WakeupManager_h__
