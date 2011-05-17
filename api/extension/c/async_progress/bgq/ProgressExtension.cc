/**
 * \file api/extension/c/async_progress/bgq/ProgressExtension.cc
 * \brief PAMI extension "async progress" BGQ implementation
 */

#include "api/extension/c/async_progress/ProgressExtension.h"
#include "Global.h"
#include "components/devices/bgq/commthread/CommThreadFactory.h"

PAMI::ProgressExtension::ProgressExtension() {
}

pami_result_t PAMI::ProgressExtension::context_async_progress_register(
				pami_context_t context,
				pamix_async_function progress_fn,
				pamix_async_function suspend_fn,
				pamix_async_function resume_fn,
				void *cookie) {
	if (progress_fn) {
		return PAMI_ERROR;
	}
	pami_result_t rc;
	rc = __commThreads.registerAsync(context, NULL, suspend_fn, resume_fn, cookie);
	return rc;
}

pami_result_t PAMI::ProgressExtension::context_async_progress_enable(
				pami_context_t context,
				pamix_async_t options) {
	if (options != PAMI_ASYNC_ALL) {
		return PAMI_ERROR;
	}
	pami_result_t rc;
	rc = __commThreads.addContext(context);
	return rc;
}

pami_result_t PAMI::ProgressExtension::context_async_progress_disable(
				pami_context_t context,
				pamix_async_t options) {
	if (options != PAMI_ASYNC_ALL) {
		return PAMI_ERROR;
	}
	pami_result_t rc;
	__commThreads.registerAsync(context, NULL, NULL, NULL, NULL);
	rc = __commThreads.rmContexts(context, 1);
	return rc;
}
