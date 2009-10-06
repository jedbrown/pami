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

/* ********************************************************************** */
/*                LAPI Request objects                                    */
/* ********************************************************************** */
typedef struct __xmi_lapi_request_t
{
    unsigned              cmplt;           /* completion counter */
    struct __xmi_lapi_request_t * next; /* pointer to next request */
    unsigned              type;            /* type of request (not in use) */
    void                 (*comp_h)(void *);
    void                  *arg;
    lapi_vec_t            vec;             /* strided puts: vector */
    unsigned long long    info[3];         /* strided puts: vector info */
}
    __xmi_lapi_request_t;

/* *********************************************************************** */
/*                 handler declarations                                    */
/* *********************************************************************** */

#define XMI_LAPI_AMSENDREQUESTHANDLER 1
    void * __xmi_lapi_amSendRequestHandler   (lapi_handle_t        * hndl,
						 void                 * uhdr,
						 uint                 * uhdr_len,
						 ulong                * retinfo,
						 compl_hndlr_t       ** comp_h,
						 void                ** uinfo);

#define XMI_LAPI_GETREQUESTHANDLER 2
    void * __xmi_lapi_getRequestHandler      (lapi_handle_t        * hndl,
						 void                 * uhdr,
						 uint                 * uhdr_len,
						 ulong                * retinfo,
						 compl_hndlr_t       ** comp_h,
						 void                ** uinfo);

#define XMI_LAPI_GETREPLYHANDLER 3
    void * __xmi_lapi_getReplyHandler        (lapi_handle_t        * hndl,
						 void                 * uhdr,
						 uint                 * uhdr_len,
						 ulong                * retinfo,
						 compl_hndlr_t       ** comp_h,
						 void                ** uinfo);

#define XMI_LAPI_GETVREQUESTHANDLER 4
    void * __xmi_lapi_getvRequestHandler     (lapi_handle_t        * hndl,
						 void                 * uhdr,
						 uint                 * uhdr_len,
						 ulong                * retinfo,
						 compl_hndlr_t       ** comp_h,
						 void                ** uinfo);

#define XMI_LAPI_GETVREPLYHANDLER 5
    lapi_vec_t * __xmi_lapi_getvReplyHandler (lapi_handle_t       * hndl,
						 void                * uhdr,
						 uint                  uhdr_len,
						 uint                * vec_len[],
						 compl_hndlr_t      ** comp_h,
						 void               ** uinfo);

#define XMI_LAPI_PUTREQUESTHANDLER 6
    void * __xmi_lapi_putRequestHandler      (lapi_handle_t       * hndl,
						 void                * uhdr,
						 uint                * uhdr_len,
						 ulong               * retinfo,
						 compl_hndlr_t      ** comp_h,
						 void               ** uinfo);

#define XMI_LAPI_PUTREPLYHANDLER 7
    void * __xmi_lapi_putReplyHandler        (lapi_handle_t       * hndl,
						 void                * uhdr,
						 uint                * uhdr_len,
						 ulong               * retinfo,
						 compl_hndlr_t      ** comp_h,
						 void               ** uinfo);

#define XMI_LAPI_PUTVREQUESTHANDLER 8
    lapi_vec_t * __xmi_lapi_putvRequestHandler (lapi_handle_t     * hndl,
						   void              * uhdr,
						   uint                uhdr_len,
						   uint              * vec_len[],
						   compl_hndlr_t    ** comp_h,
						   void             ** uinfo);

#define XMI_LAPI_PUTVREPLYHANDLER 9
    void * __xmi_lapi_putvReplyHandler       (lapi_handle_t       * hndl,
						 void                * uhdr,
						 uint                * uhdr_len,
						 ulong               * retinfo,
						 compl_hndlr_t      ** comp_h,
						 void               ** uinfo);

#define XMI_LAPI_ACCREQUESTHANDLER 10
    void * __xmi_lapi_accRequestHandler      (lapi_handle_t       * hndl,
						 void                * uhdr,
						 uint                * uhdr_len,
						 ulong               * retinfo,
						 compl_hndlr_t      ** comp_h,
						 void               ** uinfo);

/* ********************************************************************** */
/*                        request allocation pool                         */
/* ********************************************************************** */

    void                      __xmi_lapi_pool_init();
    __xmi_lapi_request_t *    __xmi_lapi_pool_allocate ();
    void                      __xmi_lapi_pool_free (__xmi_lapi_request_t *);

#if defined(__cplusplus)
}
#endif


#endif
