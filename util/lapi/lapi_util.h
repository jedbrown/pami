#ifndef ___LAPI_UTIL_H__
#define ___LAPI_UTIL_H__

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <lapi.h>

#undef TRACE
#define TRACE(x)

#if defined(__cplusplus)
extern "C" {
#endif


//#define TRACE(x) printf x

/* ********************************************************************** */
/*            macros to simplify the code                                 */
/* ********************************************************************** */

#define CALL_AND_CHECK_RC(func_and_args)                                  \
    {                                                                     \
	int lapi_rc;                                                      \
	char err_msg_buf[LAPI_MAX_ERR_STRING];                            \
                                                                          \
	if ((lapi_rc = (func_and_args)) != LAPI_SUCCESS) {                \
	    LAPI_Msg_string(lapi_rc, err_msg_buf);                        \
	    fprintf(stderr,"Aborting: %s line %d: %s returns error: %d\n", \
		    __FILE__,__LINE__, #func_and_args , lapi_rc);         \
	    abort();                                                      \
	}                                                                 \
    }

#define CHECK_NULL(val,func_and_args)                                     \
    {                                                                     \
	if ((val = (func_and_args)) == NULL) {                            \
	    fprintf(stderr,"Aborting in %s : %s returns NULL\n",          \
		    __FUNCTION__, #func_and_args );                       \
	    abort();                                                      \
	}                                                                 \
    }

#define ALLOC_REQUEST(r)                                                  \
    {                                                                     \
	r = (__xmi_lapi_request_t *)malloc(sizeof(__xmi_lapi_request_t)); \
	if (r == NULL) {                                                  \
	    fprintf(stderr,"Aborting in %s: cannot allocate "             \
		    "LAPI request\n", __FUNCTION__);                      \
	    abort();                                                      \
	}                                                                 \
    }


#if defined(__cplusplus)
}
#endif


#endif
