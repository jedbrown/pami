#ifndef __test_async_progress_h__
#define __test_async_progress_h__

static pami_extension_t async_prog_ext;
typedef void (*pamix_async_function) (pami_context_t context, void * cookie);
typedef enum { PAMI_ASYNC_ALL = 0 } pamix_async_t;
static pami_result_t (*async_prog_register)(pami_context_t context,
					pamix_async_function progress_fn,
					pamix_async_function suspend_fn,
					pamix_async_function resume_fn,
					void *cookie);
static pami_result_t (*async_prog_enable)(pami_context_t context,
					pamix_async_t event_type);
static pami_result_t (*async_prog_disable)(pami_context_t context,
					pamix_async_t event_type);

static inline pami_result_t init_async_prog() {
	pami_result_t rc;
	rc = PAMI_Extension_open(NULL, "EXT_async_progress", &async_prog_ext);
	if (rc != PAMI_SUCCESS) return rc;
	async_prog_register = PAMI_Extension_symbol(async_prog_ext,
						"register");
	if (async_prog_register == NULL) return PAMI_ERROR;
	async_prog_enable = PAMI_Extension_symbol(async_prog_ext,
						"enable");
	if (async_prog_enable == NULL) return PAMI_ERROR;
	async_prog_disable = PAMI_Extension_symbol(async_prog_ext,
						"disable");
	if (async_prog_disable == NULL) return PAMI_ERROR;
	return PAMI_SUCCESS;
}

#endif // __test_async_progress_h__
