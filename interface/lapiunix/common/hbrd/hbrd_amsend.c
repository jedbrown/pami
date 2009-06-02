/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2005, 2007.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../include/pgasrt.h"

#if defined(__cplusplus)
extern "C" {
#endif

void *
__pgasrt_hbrd_amsend (__pgasrt_thread_t            destThread,
		      __pgasrt_AMHeader_t        * header,
		      const __pgasrt_local_addr_t  data,
		      unsigned                     datalen,
		      void                      (* comp_h) (void *),
		      void                       * info)
{
  if (!PGASRT_IS_LOCAL(destThread))
    return __pgasrt_tsp_amsend (PGASRT_NODE_ID(destThread),
				header,
				data,
				datalen,
				comp_h,
				info);

  if (!header->handler) return NULL;
  __pgasrt_AMHeaderHandler_t hndlr=__pgasrt_tsp_amsend_lookup(header->handler);
  void (* recv_comp_h)(void *, void *);
  void * recv_info;
  __pgasrt_local_addr_t recv_addr;
  assert (hndlr != NULL);
  recv_addr = hndlr (header, &recv_comp_h, &recv_info);
  if (datalen>0) memcpy (recv_addr, data, datalen);
  if (recv_comp_h) recv_comp_h (NULL, recv_info);
  if (comp_h) comp_h (info);
  return NULL;
}

#if defined(__cplusplus)
}
#endif
