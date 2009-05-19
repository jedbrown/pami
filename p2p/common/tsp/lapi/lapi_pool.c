#include "pgasrt_lapi.h"
#if defined(__cplusplus)
extern "C" {
#endif


static struct
{
  __pgasrt_lapi_request_t * top;
  int                       depth;
}
  _g_freelist = { NULL, 0 };

#define DEPTH_MAX 200

/* *********************************************************************** */
/*                initialize pool of requests                              */
/* *********************************************************************** */

void __pgasrt_lapi_pool_init()
{
  _g_freelist.top = NULL;
  _g_freelist.depth = 0;
}

/* *********************************************************************** */
/*         allocate a new request. get from pool if available.             */
/* *********************************************************************** */

__pgasrt_lapi_request_t * __pgasrt_lapi_pool_allocate ()
{
  __pgasrt_lapi_request_t * req = NULL;

#if 0
  if (_g_freelist.top)
    {
      req = _g_freelist.top;
      _g_freelist.top = req->next;
      _g_freelist.depth--;
    }
  else
#endif

    {
	req = (__pgasrt_lapi_request_t*)malloc(sizeof(__pgasrt_lapi_request_t));
      if (req == NULL)
        __pgasrt_fatalerror(-1, "Out of memory in LAPI request pool");
    }
  req->cmplt = 0;
  req->next = 0;
  return req;
}

/* *********************************************************************** */
/*        return a UPC request to pool; free if pool is overflowing.       */
/* *********************************************************************** */

void __pgasrt_lapi_pool_free (__pgasrt_lapi_request_t *req)
{
  if (!req) return;
  free (req);
#if 0
  if (_g_freelist.depth > DEPTH_MAX) free(req);
  else
    {
      req->next = _g_freelist.top;
      _g_freelist.top = req;
      _g_freelist.depth++;
    }
#endif
}

#if defined(__cplusplus)
}
#endif
