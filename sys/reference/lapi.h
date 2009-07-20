/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/* Licensed Materials - Property of IBM                                   */
/*                                                                        */
/* (C) COPYRIGHT International Business Machines Corp. 2002,2008          */
/* All Rights Reserved                                                    */
/*                                                                        */
/* US Government Users Restricted Rights - Use, duplication or            */
/* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.      */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
#ifndef _lapi_h_
#define _lapi_h_
/***************************************************************************
"@(#) 1.39.1.24 src/rsct/lapi/include/lapi.h, lapi, comm_rcf 08/12/19 13:03:24"
 Name: lapi.h

 Description:

 Licensed Materials - Property of IBM

 5765-296 5765-529 (C) Copyright IBM Corp. 1996, 1997, 1998, 1999
 5765-F07          (C) Copyright IBM Corp. 2002

 All rights reserved.
 US Government Users Restricted Rights - Use, Duplication or Disclosure
 restricted by GSA ADP Schedule Contract with IBM Corp.

***************************************************************************/


#include <pthread.h>
#include <css_shared.h>
#include <sys/types.h>
#include <netinet/in.h>


/*
** LAPI Return/Error Codes
*/
#define LAPI_SUCCESS              (0)
#define LAPI_ERR_UNKNOWN          (400)/* Internal communication error.       */
                                       /* This is asynchronus error.          */
#define LAPI_ERR_ALL_HNDL_IN_USE  (401)/* All available LAPI instances in use.*/
#define LAPI_ERR_BOTH_NETSTR_SET  (402)/* Both Network statements set for     */
                                       /* single LAPI instance.               */
#define LAPI_ERR_CKPT_LOAD_FAILED (403)/* Unable to load checkpoint support.  */
#define LAPI_ERR_CSS_LOAD_FAILED  (404)/* Unable to load user space support   */
                                       /* library.                            */
#define LAPI_ERR_INFO_NULL        (405)/* lapi_info pointer is NULL.          */
#define LAPI_ERR_MSG_API          (406)/* MP_MSG_API environment setting      */
                                       /* has an error in it.                 */
#define LAPI_ERR_NO_NETSTR_SET    (407)/* No Network statement is set.        */
#define LAPI_ERR_NO_UDP_HNDLR     (408)/* Told LAPI to use user udp_hndlr but */
                                       /* udp_hndlr is set to NULL.           */
#define LAPI_ERR_HDR_HNDLR_NULL   (409)/* Header handler is NULL.             */
                                       /* This is can be returned in async.   */
                                       /* asynchronus error handler.          */
#define LAPI_ERR_PSS_NON_ROOT     (410)/* Try to initialize PSS as non root.  */
#define LAPI_WARN_PARM_NULL       (411)/* Warning NULL address passed in.     */
#define LAPI_ERR_SHM_KE_NOT_LOADED  (412)/* Shared Memory kernel extension not*/
                                       /* loaded.                             */
#define LAPI_ERR_TIMEOUT          (413)/* Communication timeout has occurred. */
                                       /* This is returned in asynchronus     */
                                       /* error handler.                      */
#define LAPI_ERR_REG_TIMER        (414)/* Error during re-registering timer.  */
#define LAPI_ERR_UDP_PKT_SZ       (415)/* UDP pkt size is not in valid range. */
#define LAPI_ERR_USER_UDP_HNDLR_FAIL  (416)/* User udp handler return error.  */
#define LAPI_ERR_HNDL_INVALID     (417)/* Invalid handle passed into LAPI.    */
#define LAPI_ERR_RET_PTR_NULL     (418)/* Out data pointer is NULL.           */
#define LAPI_ERR_ADDR_HNDL_RANGE  (419)/* Address handle range is not valid.  */
#define LAPI_ERR_ADDR_TBL_NULL    (420)/* Out address table is NULL.          */
#define LAPI_ERR_TGT_PURGED       (421)/* Destination task is purged.         */
#define LAPI_ERR_MULTIPLE_WAITERS (422)/* Error, multiple threads are waiting */
                                       /* on the same counter.                */
#define LAPI_ERR_MEMORY_EXHAUSTED (423)/* Error, unable to allocate storage.  */
#define LAPI_ERR_INFO_NONZERO     (424)/* Error, unused fields in lapi_info_t */
                                       /* structure need to be zero'ed out.   */
#define LAPI_ERR_ORG_ADDR_NULL    (425)/* Source address pointer is NULL.     */
#define LAPI_ERR_TGT_ADDR_NULL    (426)/* Target address pointer is NULL.     */
#define LAPI_ERR_DATA_LEN         (427)/* Length passed in is too big.        */
#define LAPI_ERR_TGT              (428)/* Target is not valid.                */
#define LAPI_ERR_UHDR_NULL        (429)/* uhdr is NULL but uhdr_len > 0.      */
#define LAPI_ERR_UHDR_LEN         (430)/* uhdr_len is too big.                */
#define LAPI_ERR_HDR_LEN          (431)/* uhdr_len is not word aligned.       */
#define LAPI_ERR_ORG_EXTENT       (432)/* Source vector's extent is too big.  */
#define LAPI_ERR_ORG_STRIDE       (433)/* Source vector's stride is less than */
                                       /* its block.                          */
#define LAPI_ERR_NO_CONNECTIVITY  (434)/* Error, no connectivity to task.     */
#define LAPI_ERR_ADAPTERS_DOWN    (435)/* Error, all adapters are down.       */
#define LAPI_ERR_RECV_INCOMP      (436)/* Error, RECV of msg did not complete.*/
#define LAPI_ERR_SEND_INCOMP      (437)/* Error, SEND of msg did not complete.*/
#define LAPI_ERR_SEND_TIMEOUT     (438)/* Error, SEND of msg timed out.       */
#define LAPI_ERR_SHM_SETUP        (439)/* Error, setup of shared memory failed. */
#define LAPI_ERR_ORG_VEC_ADDR     (440)/* Source vector address is NULL but   */
                                       /* its len > 0.                        */
#define LAPI_ERR_ORG_VEC_LEN      (441)/* Source vector's length is too big.  */
#define LAPI_ERR_ORG_VEC_NULL     (442)/* Source vector pointer is NULL.      */
#define LAPI_ERR_ORG_VEC_TYPE     (443)/* Source vector type is not valid.    */
#define LAPI_ERR_STRIDE_ORG_VEC_ADDR_NULL  (444)/* Source stride vector       */
                                                /* address is NULL.           */
#define LAPI_ERR_STRIDE_TGT_VEC_ADDR_NULL  (445)/* Target stride vector       */
                                                /* address is NULL.           */
#define LAPI_ERR_TGT_EXTENT       (446)/* Target vector's extent is too big.  */
#define LAPI_ERR_TGT_STRIDE       (447)/* Target vector's stride is less than */
                                       /* its block.                          */
#define LAPI_ERR_TGT_VEC_ADDR     (448)/* Target vector address is NULL but   */
                                       /* its len > 0.                        */
#define LAPI_ERR_TGT_VEC_LEN      (449)/* Target vector's length is too big.  */
#define LAPI_ERR_CKPT             (450)/* Error in checkpoint/restart/resume. */
#define LAPI_ERR_TGT_VEC_NULL     (451)/* Target vector pointer is NULL.      */
#define LAPI_ERR_TGT_VEC_TYPE     (452)/* Target vector type is not valid.    */
#define LAPI_ERR_VEC_NUM_DIFF     (453)/* Source & Target vectors have        */
                                       /* different num_vecs.                 */
#define LAPI_ERR_VEC_TYPE_DIFF    (454)/* Source & Target vectors have        */
                                       /* different vec_type.                 */
#define LAPI_ERR_VEC_LEN_DIFF     (455)/* Source & Target vectors have        */
                                       /* different len[].                    */
#define LAPI_ERR_MSG_INFO_NULL    (456)/* info ptr in LAPI_Msgpoll is NULL.   */
#define LAPI_ERR_POLLING_NET      (457)/* Another thread is polling, try      */
                                       /* again later.                        */
#define LAPI_ERR_CNTR_NULL        (458)/* Counter pointer is NULL.            */
#define LAPI_ERR_CNTR_VAL         (459)/* Counter value passed in is less than*/
                                       /* zero for LAPI_Nopoll_wait call.     */
#define LAPI_ERR_QUERY_TYPE       (460)/* Query is not a valid query type.    */
#define LAPI_ERR_IN_VAL_NULL      (461)/* LAPI_RMW's in_val pointer is NULL.  */
#define LAPI_ERR_RMW_OP           (462)/* RMW operator is not valid.          */
#define LAPI_ERR_TGT_VAR_NULL     (463)/* LAPI_RMW's tar_var address is NULL. */
#define LAPI_ERR_SET_VAL          (464)/* LAPI_SENV's set_val is not valid.   */
#define LAPI_ERR_DGSP             (465)/* DGSP is NULL or not registered.     */
#define LAPI_ERR_DGSP_ATOM        (466)/* DGSP atom_size is not valid.        */
#define LAPI_ERR_DGSP_BRANCH      (467)/* DGSP processed incorrect branch.    */
#define LAPI_ERR_DGSP_CTL         (468)/* DGSP control block has errors.      */
#define LAPI_ERR_DGSP_COPY_SZ     (469)/* DGSP has bad copy length.           */
#define LAPI_ERR_DGSP_FREE        (470)/* Try to free invalid DGSP.           */
#define LAPI_ERR_DGSP_OPC         (471)/* DGSP op-code is not valid.          */
#define LAPI_ERR_DGSP_REPS        (472)/* DGSP has bad reps < 0.              */
#define LAPI_ERR_DGSP_STACK       (473)/* DGSP stack has depth <= 0.          */
#define LAPI_ERR_OP_SZ            (474)/* lapi_rmw_t size not set to 32 or 64.*/
#define LAPI_ERR_UDP_PORT_INFO    (475)/* udp_port info pointer is NULL.      */
#define LAPI_ERR_XFER_CMD         (476)/* Invalid LAPI_Xfer command type.     */
#define LAPI_ERR_UTIL_CMD         (477)/* Invalid LAPI_Util command type.     */
#define LAPI_ERR_CATALOG_FAIL     (478)/* Cannot open message catalog.        */
#define LAPI_ERR_PACK_SZ          (479)/* Pack buffer too small.              */
#define LAPI_ERR_DGSP_OTHER       (480)/* Various DGSP error, e.g. 0 code size*/
#define LAPI_ERR_UDP_SOCKET       (481)/* Error on udp socket operation.      */
#define LAPI_ERR_COLLECTIVE_PSS   (482)/* PSS system attempted collective call*/
#define LAPI_ERR_DMA_HNDL         (483)/* KLAPI DMA hndl error.               */
#define LAPI_ERR_DMA_TAG          (484)/* KLAPI DMA Posted tag error.         */
#define LAPI_ATT_FR_SETUP_INTERNAL (485) /* Failover setup failed (generic).  */
#define LAPI_ATT_FR_SETUP_NONAM   (486)/* Failover setup failed (no NAM).     */
#define LAPI_ATT_FR_SETUP_NOHAGS  (487)/* Failover setup failed (no HAGS).    */
#define LAPI_ATT_FR_SETUP_BADPOE  (488)/* Failover setup failed, old POE.     */
#define LAPI_ATT_FR_SETUP_NOPOE   (489)/* Failover setup failed, no POE obj.  */
#define LAPI_ATT_FR_SETUP_NONSNX  (490)/* Failover setup failed, non-snX dev. */
#define LAPI_ATT_FR_HALTED        (491)/* Failover function halted.           */
#define LAPI_ERR_TGT_CONTEXT      (492)/* Invalid target context.             */
#define LAPI_ERR_SRC_CONTEXT      (493)/* Invalid source context.             */
#define LAPI_ERR_TGT_BUFHNDL      (494)/* Invalid target buffer handle.       */
#define LAPI_ERR_SRC_BUFHNDL      (495)/* Invalid source buffer handle.       */
#define LAPI_ERR_TGT_OFFSET       (496)/* Invalid target offset.              */
#define LAPI_ERR_SRC_OFFSET       (497)/* Invalid source offset.              */
#define LAPI_ERR_NO_RDMA_RESOURCE (498)/* No rdma resources.                  */
#define LAPI_ATT_BULK_ENABLED     (499)/* Bulk transfer enabled.              */
#define LAPI_ATT_COMM_TIMEOUT     (500)/* Timeout between multiple tasks      */
#define LAPI_ATT_VERSION          (501)/* Lapi version string                 */
#define LAPI_ATT_SHMINIT_FAIL     (502)/* Shared memory initialization failed */
#define LAPI_ATT_SHMINIT_CKPT_FAIL (503)/* Shared memory init fails at chkpt  */
#define LAPI_ATT_SHM_SINGLE_TASK  (504)/* Single task on no - no shared mem   */
#define LAPI_ATT_SHM_CREATE_FAIL  (505)/* Cannot create shm segment           */
#define LAPI_ATT_SHM_GET_FAIL     (506)/* Cannot attach to shm segment        */
#define LAPI_ATT_SHM_ATT_FAIL     (507)/* xmem attache failed                 */
#define LAPI_ATT_SHM_NO_SEGATT    (508)/* Cannot obtain xmem copy segment     */
#define LAPI_ATT_PORT_START       (509)/* Hal port to start message passing   */
#define LAPI_ERR_NO_ENV_VAR       (510)/* Required env variable is not set    */
#define LAPI_ERR_NO_CALLBACK_REG  (511)/* No callback registered              */
#define LAPI_ERR_NO_CNTR_REG      (512)/* No counter registered               */
#define LAPI_ERR_XLATE_FAILED     (513)/* RDMA translation failed             */
#define LAPI_ERR_RDMA_RESOURCES   (514)/* No more RDMA resoureces             */
#define LAPI_ERR_LW_DATA_LEN      (515)/* Incorrect data/header length for 
                                          LAPI_AM_LW_XFER                     */
#define LAPI_ERR_NO_HNDLR_SET     (516)/* Not header handler has been set for
                                          using LAPI_AM_LW_XFER               */
#define LAPI_ERR_OFFSET_LEN       (517)/* Amx dgsp send offset outof range    */
#define LAPI_ERR_RCXT_CANCEL      (518)/* Failure to cancel rCxt              */
#define LAPI_ERR_CODE_UNKNOWN     (519)/* The error code is unknown to LAPI.  */
#define LAPI_ATT_PREEMPT_SETUP_INTERNAL (520)/* Preemption setup failed (generic).  */
#define LAPI_ERR_DUP_TRIGGER      (521)/* Trigger has registered already.     */  
#define LAPI_ERR_TRIGGER_UNKNOWN  (522)/* Trigger is not registered.          */
#define LAPI_ERR_TASK_NUM         (523) /* Task number is not correct */
#define LAPI_ERR_NULL_LIST        (524) /* Task list is null */
#define LAPI_ERR_GRP_MEMBER       (525) /* Incorrect group member id */
#define LAPI_ERR_NULL_GRP         (526) /* Group handle is null */
#define LAPI_ERR_GRP              (527) /* Invalid group handle passed */
#define LAPI_ERR_STAT_SAVE_FAIL   (528) /* Failed to upload user's stat to pnsd */
#define LAPI_ERR_FIRSTCODE        (LAPI_ERR_UNKNOWN)
#define LAPI_ERR_LASTCODE         (LAPI_ERR_STAT_SAVE_FAIL) 



/******************************************************************************/
/* These error codes will no longer be returned by LAPI.                      */
/* These error codes are left here for source code compatibility.             */
/* If these definitions are used, the source code should be updated           */
/* accordingly by the user.                                                   */
/******************************************************************************/
#define LAPI_ERR_UNKNOWN_DEVICE   (401)/* Obsolete error code                 */
#define LAPI_ERR_UNKNOWN_FUNCTION (402)/* Obsolete error code                 */
#define LAPI_ERR_NOMORE_PORTS     (403)/* Obsolete error code                 */
#define LAPI_ERR_INIT_FAILED      (404)/* Obsolete error code                 */
#define LAPI_ERR_OPEN_FAILED      (405)/* Obsolete error code                 */
#define LAPI_ERR_CLOSE_FAILED     (406)/* Obsolete error code                 */
#define LAPI_ERR_SWITCH_FAULT     (407)/* Obsolete error code                 */
#define LAPI_ERR_INTR_HNDLR       (408)/* Obsolete error code                 */
#define LAPI_ERR_NO_HD_HNDLR      (409)/* Obsolete error code                 */
#define LAPI_ERR_BAD_PARAMETER    (410)/* Obsolete error code                 */
#define LAPI_ERR_RETRANSMISSION   (412)/* Obsolete error code                 */
#define LAPI_ERR_TIMEOUT          (413)/* Obsolete error code                 */
#define LAPI_ERR_QUEUE_FULL       (414)/* Obsolete error code                 */
#define LAPI_ERR_INIT_LOCK_FAILED (415)/* Obsolete error code                 */
#define LAPI_ERR_GET_LOCK_FAILED  (416)/* Obsolete error code                 */
#define LAPI_ERR_REL_LOCK_FAILED  (417)/* Obsolete error code                 */
#define LAPI_ERR_TRY_LOCK_FAILED  (418)/* Obsolete error code                 */
#define LAPI_ERR_INVALID_OPTIONS  (419)/* Obsolete error code                 */
#define LAPI_WAIT_INTERRUPTED     (420)/* Obsolete error code                 */
#define LAPI_ERR_PURGED_TASK      (421)/* Obsolete error code                 */
#define LAPI_ERR_MULTIPLE_WAITERS (422)/* Obsolete error code                 */
#define LAPI_ERR_EXHAUSTED_MEMORY (423)/* Obsolete error code                 */
#define LAPI_ERR_NONZERO_INFO     (424)/* Obsolete error code                 */
#define LAPI_ERR_BAD_FIRST_PARM   (425)/* Obsolete error code                 */
#define LAPI_ERR_BAD_SECOND_PARM  (426)/* Obsolete error code                 */
#define LAPI_ERR_BAD_THIRD_PARM   (427)/* Obsolete error code                 */
#define LAPI_ERR_BAD_FOURTH_PARM  (428)/* Obsolete error code                 */
#define LAPI_ERR_BAD_FIFTH_PARM   (429)/* Obsolete error code                 */
#define LAPI_ERR_BAD_SIXTH_PARM   (430)/* Obsolete error code                 */
#define LAPI_ERR_BAD_SEVENTH_PARM (431)/* Obsolete error code                 */
#define LAPI_ERR_BAD_EIGHTH_PARM  (432)/* Obsolete error code                 */
#define LAPI_ERR_BAD_NINTH_PARM   (433)/* Obsolete error code                 */
#define LAPI_ERR_NO_CONNECTIVITY  (434)/* Obsolete error code                 */
#define LAPI_ERR_ADAPTERS_DOWN    (435)/* Obsolete error code                 */
#define LAPI_ERR_RECV_INCOMP      (436)/* Obsolete error code                 */
#define LAPI_ERR_SEND_INCOMP      (437)/* Obsolete error code                 */
#define LAPI_ERR_SEND_TIMEOUT     (438)/* Obsolete error code                 */
#define LAPI_ERR_SHM_SETUP        (439)/* Obsolete error code                 */
#define LAPI_ERR_SHM_CLEANUP      (440)/* Obsolete error code                 */
#define LAPI_ERR_SHM_REGISTER     (441)/* Obsolete error code                 */
#define LAPI_ERR_SHM_RELEASE      (442)/* Obsolete error code                 */
#define LAPI_ERR_SHM_ATTDET       (443)/* Obsolete error code                 */
#define LAPI_ERR_SHM_ATTACH       (444)/* Obsolete error code                 */
#define LAPI_ERR_SHM_DETACH       (445)/* Obsolete error code                 */
#define LAPI_ERR_SHM_ENV_SET      (446)/* Obsolete error code                 */
#define LAPI_ERR_SHM_NOCOMM_TASK  (447)/* Obsolete error code                 */
#define LAPI_ERR_SHM_SETSHM       (448)/* Obsolete error code                 */
#define LAPI_ERR_SHM_LOAD         (449)/* Obsolete error code                 */
#define LAPI_ERR_CHECKPOINT       (450)/* Obsolete error code                 */
/******************************************************************************/

#define LAPI_ERR_INVALID_SEND_REPLY_TGT (501) /* Error, message target within inline completion handler is not original message source */




/*
**  KE can return these error codes.
*/
#define CSS_KE_INTERNAL_ERROR       (500)
#define CSS_KE_UCODE_ERROR          (502)

/*
** LAPI maximum positive lapi_msglen_t
*/

#if defined(__64BIT__)
#define LAPI_MAX_MSG_SZ      0x7fffffffffffffffLL
#else /* __64BIT__ */
#define LAPI_MAX_MSG_SZ      0x7fffffff 
#endif /* __64BIT__ */


/*
** LAPI Structures and typedefs.
*/
typedef css_dev_t lapi_dev_t;
typedef enum {FETCH_AND_ADD=0, FETCH_AND_OR, SWAP, COMPARE_AND_SWAP} RMW_ops_t;

typedef enum {
        TASK_ID=0,     /* Query task id of current task in job               */
        NUM_TASKS,     /* Query number of tasks in job                       */
        MAX_UHDR_SZ,   /* Query max. user header size for AM                 */
        MAX_DATA_SZ,   /* Query max. data length that can be sent            */
        ERROR_CHK,     /* Query & Set parameter checking on(1)/off(0)        */
        TIMEOUT,       /* Query & Set current comm. timeout setting in secs. */
        MIN_TIMEOUT,   /* Query minimum comm. timeout setting in secs.       */
        MAX_TIMEOUT,   /* Query maxmium comm. timeout setting in secs.       */
        INTERRUPT_SET, /* Query & Set interrupt on(1)/off(0)                 */
        MAX_PORTS,     /* Query max. available comm. ports                   */
        MAX_PKT_SZ,    /* This is the payload size of 1 packet               */
        NUM_REX_BUFS,  /* Number of retransmission buffers                   */
        REX_BUF_SZ,    /* Size of Each retransmission buffer in bytes        */
        LOC_ADDRTBL_SZ,/* Size of address store table used by LAPI_Addr_set  */
        EPOCH_NUM,     /* Epoch number set by the user                       */
        USE_THRESH,    /* Single comm thread on(1)/ Multi comm threads off(0)*/
        RCV_FIFO_SIZE, /* Query Size of the Receive FIFO                     */
        MAX_ATOM_SIZE, /* Query Maximum ATOM size for DGSP accmulate transfer*/
        BUF_CP_SIZE,   /* Query Size of message buffer to save - default 64b */
        MAX_PKTS_OUT,  /* Query Max # of messages outstanding / destination  */
        ACK_THRESHOLD, /* Query & Set threshold of ACKS going back to the src*/
        QUERY_SHM_ENABLED,/* Query see if shared memory is enabled or not    */
        QUERY_SHM_NUM_TASKS,/* Query get number of tasks that uses shared mem*/
        QUERY_SHM_TASKS,/* Query get list of task ids that make up shared mem*/
                        /* Must pass in array of size QUERY_SHM_NUM_TASKS    */
        QUERY_STATISTICS,/* Query to get back packet statistics from LAPI as */
                         /* defined by lapi_statistics_t structure.  For this*/
                         /* query, the user must pass in lapi_statistics_t * */
                         /* instead of 'int *ret_val', otherwise the data    */
                         /* will overflow the buffer.                        */
        PRINT_STATISTICS,/* Query debug print funtion to print out statistics*/
        QUERY_SHM_STATISTICS,/* Similar query as QUERY_STATISTICS for share  */
                             /* memory path.                                 */
        QUERY_LOCAL_SEND_STATISTICS ,/* Similar query as QUERY_STATISTICS    */
                                     /* for local copy path.                 */
        BULK_XFER,      /* Query if bulk xfer is in use                      */
        BULK_MIN_MSG_SIZE,  /* Size of smallest bulk_xfer message            */
        RDMA_REMOTE_RCXT_AVAIL, /* Number of available remote rcxts          */
        RDMA_REMOTE_RCXT_TOTAL, /* Total number of remote rcxts              */
        RC_MAX_QP,       /* Query maximum number of RC QPs                   */
        RC_QP_IN_USE,    /* Query number of RC QPs in use                    */
        RC_QP_USE_LRU,   /* Query whether LRU policy is being used           */
        RC_QP_USE_LMC,   /* Query whether we are using one RC QP per path    */
        RC_QP_INIT_SETUP,/* Query whether RC QP setup was initiated at Init  */
        LAST_QUERY
} lapi_query_t;

typedef enum {GET_ERR=0,PUT_ERR,RMW_ERR,AM_ERR,INT_ERR,TMR_ERR} lapi_err_t;
typedef enum {L1_LIB=0,L2_LIB,L3_LIB,L4_LIB,LAST_LIB} lapi_lib_t;

typedef uint lapi_handle_t;              /* Handle used in the LAPI calls */
typedef uint lapi_group_t;                /* Group handle used in the LAPI calls */
typedef unsigned long long lapi_long_t;  /* 64 bit type in both 32 and 64 bit 
                                            programs, store addresses and
                                            long arguments */

/*
** For LAPI_Nopoll_wait: dest_status codes
*/
#define LAPI_MSG_INITIAL          (0)
#define LAPI_MSG_RECVD            (1)
#define LAPI_MSG_PURGED           (2)
#define LAPI_MSG_PURGED_RCVD      (3)
#define LAPI_MSG_INVALID          (4)

typedef union {
    pthread_cond_t    Pt;
    long long         Lw;
} lapi_cond_t;

typedef struct lapi_counter_t{        /* LAPI Counter type */
     int          cntr;
     lapi_cond_t  signal;           /* Lock for Nopoll wait */
     volatile int cntr_q_flg; /* Predicate for the signal                    */
     uint         state;      /* Internal state of LAPI_Nopoll_wait          */
     uint         num_dest;   /* Number of destinations waiting for this cntr*/
     int          wait_val;   /* value wait to be reached                    */
     uint         *dest;      /* Passed in by user if checking per dest is   */
     int          *dest_status; /* required, state returned in this array    */
     struct lapi_counter_t *pre_cntr;
     struct lapi_counter_t *next_cntr;
     pthread_mutex_t cntr_lock;
     ulong        tmp1;   /* Reserved */
     ulong        tmp2;   /* Reserved */
     void         *ptr1;  /* Reserved */
     void         *ptr2;  /* Reserved */
} lapi_count_t;

typedef union {
   int cntr;
   lapi_count_t new_cntr;
} lapi_cntr_t;


typedef void (LAPI_err_hndlr)(lapi_handle_t *hndl,  /* LAPI handle            */
                              int *error_code,      /* Error code             */
                              lapi_err_t *err_type, /* GET/PUT/RMW/AM/INTERNAL*/
                              int *task_id,         /* Current node           */
                              int *src);            /* Source node            */

typedef struct {
    uint        task_id;       /* Task id                       */
    uint        tot_num_tasks; /* Total number of tasks         */
    uint        num_tasks;     /* Number of tasks to initialize */
    uint        instance_no;   /* Instance [1-16]               */
    ulong       reserve[10];   /* Reserved                      */
} lapi_udpinfo_t;

typedef struct {
    in_addr_t   ip_addr;       /* Local IP address to use        */
    in_port_t   port_no;       /* Local port address            */
} lapi_udp_t;

typedef union {
    in_addr_t ipv4_addr;
    struct in6_addr ipv6_addr;
} inet_address;

typedef struct {
    inet_address addr;
    int af; 
} in_addr_storage;

typedef struct {
    inet_address ip_addr;
    in_port_t   port_no;       /* Local port address            */
    int af;
} lapi_udp_storage;

typedef int (udp_init_hndlr)(lapi_handle_t *hndl, lapi_udp_storage *local_addr,
                             lapi_udp_storage *addr_list, lapi_udpinfo_t *info);

typedef struct {
    udp_init_hndlr     *udp_hndlr;      /* Obselete for backword compatibility */
    lapi_udp_t         *add_udp_addrs;  /* Obselete for backword compatibility */
    uint           num_udp_addr;    /* Size of udp_addrs array              */
    uint           pad;             /* PAD                                  */
    udp_init_hndlr     *udp_ext_hndlr;      /* Callback to fill in global UDP array */
    lapi_udp_storage   *add_udp_ext_addrs;  /* If callback not used, must pass in   */
                                    /* ALL UDP address information          */
    ulong          reserve[8];     /* Reserved                             */
} lapi_extend_t;

typedef struct {                      
    lapi_dev_t     protocol;     /* In/Out Protocol device returned            */
    lapi_lib_t     lib_vers;     /* LAPI library version -- user supplied      */
    uint           epoch_num;    /* first 15 bit used to solve trickle traffic */
    int            num_compl_hndlr_thr; /* Deprecated and ignored              */
    uint           instance_no;  /* Instance of LAPI to initialize [1-16]      */
    int            info6;        /* Future support                             */
    LAPI_err_hndlr *err_hndlr;   /* User registered error handler              */
    com_thread_info_t *lapi_thread_attr;  /* Support thread att and init func  */
    void           *adapter_name;/* What adapter to initialize, i.e. css0, ml0 */
    lapi_extend_t  *add_info;    /* Additional structure extention             */
} lapi_info_t;


#define LAPI_ADDR_NULL ((void*) 0)    /* For Fortran NULL pointer support */
#define LAPI_MAX_ERR_STRING  (160)    /* Maximum error string length      */

/* Bit flags that can be given | into the protocol field as input         */
/* into LAPI_Init() call.                                                 */
#define LAPI_SHARE_PROTOCOL 0x80000000   /* Shared protocol field set */
#define LAPI_PSS_GPFS       0x40000000   /* GPFS PSS field set */
#define LAPI_INTERRUPT_OFF  0x20000000   /* Turn off interrupt as part of  */
                                         /* LAPI_Init()  - default is on.  */
                                         /* It can be changed later by     */
                                         /* LAPI_Senv() call.              */

typedef uint lapi_msg_state_t;
/* States set for lapi_msg_state_t type - return from LAPI_Msgpoll         */
/* through the lapi_msg_info_t structure.  USE BIT PROCESSING              */
#define LAPI_COMPLETE       0x00000000  /* Nothing completed               */
#define LAPI_SEND_COMPLETE  0x00000001  /* A send has completed            */
#define LAPI_RECV_COMPLETE  0x00000002  /* A recv has completed            */
#define LAPI_BOTH_COMPLETE  0x00000003  /* Both send/recv completed        */
#define LAPI_POLLING_NET    0x00000004  /* Another thread is already polling */
#define LAPI_DISP_CNTR      0x00000008  /* Count through dispatcher w/o msg processing */

typedef int lapi_ret_flags_t;
/* States to set for lapi_ret_flags_t type - return from Header Handler    */
/* through the lapi_return_info_t structure.  USE BIT PROCESSING           */
#define LAPI_NORMAL       0x00000000 /* Default - normal LAPI behavior     */
#define LAPI_LOCAL_STATE  0x00000001 /* No communication, run in-line      */
#define LAPI_SEND_REPLY   0x00000002 /* Try to run LAPI replies in-line    */
                                     /* Only non-vector functions apply    */
                                     /* No guarantee, do if possible       */

typedef struct {
    lapi_msg_state_t  status;      /* Message status return from LAPI_Msgpoll */
    ulong             reserve[10]; /* Reserved                                */
} lapi_msg_info_t;

typedef uint lapi_status_t; /* status type */

typedef struct {
    lapi_long_t Tot_dup_pkt_cnt;      /* Total duplicate packet count  */
    lapi_long_t Tot_retrans_pkt_cnt;  /* Total retransmit packet count */
    lapi_long_t Tot_gho_pkt_cnt;      /* Total Ghost packet count      */
    lapi_long_t Tot_pkt_sent_cnt;     /* Total packet sent count       */
    lapi_long_t Tot_pkt_recv_cnt;     /* Total packet recv count       */
    lapi_long_t Tot_data_sent;        /* Total data sent               */
    lapi_long_t Tot_data_recv;        /* Total data received           */
    lapi_long_t Tot_ack_send_cnt;     /* Total acks sent count         */
    lapi_long_t Tot_ack_recv_cnt;     /* Total acks recv count         */
} lapi_statistics_t;



typedef enum {LAPI_GEN_IOVECTOR=0, LAPI_GEN_STRIDED_XFER,
              LAPI_GEN_GENERIC} lapi_vectype_t;

typedef struct  {
    lapi_vectype_t  vec_type;
    uint            num_vecs;
    lapi_long_t     *info;
    ulong           *len;
} lapi_lvec_t;     

typedef struct  {
    lapi_vectype_t  vec_type;
    uint            num_vecs;
    void            **info;
    ulong           *len;
} lapi_vec_t;

typedef struct {
    uint            src;          /* Target task id */
    uint            reason;       /* LAPI return error codes */
    ulong           reserve[6];   /* Reserved */
} lapi_sh_info_t;

/* 
** DGSM support data structures and interfaces - start
*/

typedef enum {LAPI_DGSM_COPY=0, LAPI_DGSM_MCOPY, LAPI_DGSM_GOSUB, 
              LAPI_DGSM_ITERATE, LAPI_DGSM_CONTROL} lapi_dgsm_opcode_t;

typedef struct {
    lapi_dgsm_opcode_t opcode;   /* must be LAPI_DGSM_COPY                */
    long               bytes;    /* number of bytes to copy/combine       */
    long               offset;   /* byte offset                           */
} lapi_dgsm_copy_t;

typedef struct {
    long               block_disp; /* displacement of this block          */
    long               block_len;  /* length of block in bytes            */
} lapi_dgsm_block_t;

typedef struct {
    lapi_dgsm_opcode_t opcode;     /* must be LAPI_DGSM_MCOPY             */
    int                count;      /* number of blocks in the MCOPY       */
    lapi_dgsm_block_t  block[1];   /* MCOPY is variable length but will   
                                      have at least 1 block               */
} lapi_dgsm_mcopy_t;

typedef struct {
    lapi_dgsm_opcode_t opcode;   /* must be LAPI_DGSM_GOSUB               */
    int                to_loc;   /* relative loc (RL) of subroutine start */
    int                ret_loc;  /* RL of post-return instruction         */
    long               reps;     /* number of successive applications     */
    long               offset;   /* byte offset                           */
    long               stride;   /* byte stride between application       */
} lapi_dgsm_gosub_t;

typedef struct {
    lapi_dgsm_opcode_t opcode;   /* must be LAPI_DGSM_ITERATE             */
    int                iter_loc; /* relative loc (RL) of subroutine start */
} lapi_dgsm_iterate_t;

/* Any value of operation for the LAPI_DGSM_CONTROL instruction other than
   LAPI_DGSM_NO_CTL puts the DGSM into CONTROL mode.  An operation value
   of LAPI_DGSM_NO_CTL takes the machine out of CONTROL mode.  The 
   LAPI_DGSM_CONTROL instruction is only to be used for SCATTER via the 
   registered DDM function. LAPI_DGSM_CONTROL is seldom appropriate.      */

#define LAPI_DGSM_NO_CTL -1

typedef struct {
    lapi_dgsm_opcode_t opcode;   /* must be LAPI_DGSM_CONTROL             */
    int                operation;/* operation index                       */
    int                operand;  /* element type index                    */
} lapi_dgsm_control_t;

#define LAPI_DGSM_COPY_SIZE        (sizeof(lapi_dgsm_copy_t)   /sizeof(int))
#define LAPI_DGSM_GOSUB_SIZE       (sizeof(lapi_dgsm_gosub_t)  /sizeof(int))
#define LAPI_DGSM_ITERATE_SIZE     (sizeof(lapi_dgsm_iterate_t)/sizeof(int))
#define LAPI_DGSM_CONTROL_SIZE     (sizeof(lapi_dgsm_control_t)/sizeof(int))
#define LAPI_DGSM_MCOPY_SIZE       (sizeof(lapi_dgsm_mcopy_t)  /sizeof(int))
#define LAPI_DGSM_MCOPY_BLOCK_SIZE (sizeof(lapi_dgsm_block_t)  /sizeof(int))

typedef enum {LAPI_DGSM_SPARSE=0, LAPI_DGSM_CONTIG,
              LAPI_DGSM_UNIT} lapi_dgsp_density_t;

typedef struct {
    int                  *code;     /* array containing the DGSP code */
    int                  code_size; /* size of the DGSP code array    */
    int                  depth;     /* required DGSM stack depth      */
    lapi_dgsp_density_t  density;   /* lapi_dgsp_density_t datatype   */
    long                 size;      /* datatype packed size           */
    long                 extent;    /* datatype extent (stride between reps) */
    long                 lext;      /* leftmost data byte position    */
    long                 rext;      /* rightmost data byte position   */
    int                  atom_size; /* 0 or packet filling unit       */
} lapi_dgsp_descr_t;

typedef lapi_dgsp_descr_t* lapi_dg_handle_t;
extern lapi_dg_handle_t LAPI_BYTE;

typedef long ddm_func_t (        /* return number of bytes processed */
        void      *in,           /* ptr to inbound data              */
        void      *inout,        /* ptr to destination space         */
        long      bytes,         /* number of bytes inbound          */
        int       operand,       /* CONTROL operand value            */
        int       operation      /* CONTROL operation value          */
);

/* RDMA information */
typedef unsigned int lapi_user_cxt_t;
typedef unsigned long long lapi_user_pvo_t;
typedef unsigned short     lapi_rdma_tag_t;
typedef unsigned short     lapi_rdma_op_t;
typedef unsigned short     lapi_rdma_flg_t;
typedef void (remote_hndlr_t)(lapi_handle_t *hndl, void *sinfo, int *src);
typedef enum {LAPI_RDMA_ACQUIRE=0, LAPI_RDMA_RELEASE} lapi_rdma_req_t;
#define LAPI_RDMA_GET      0x0001
#define LAPI_RDMA_PUT      0x0002
/* DO not change these bit fields */
#define LAPI_RCALLBACK     0x0004
#define LAPI_RCNTR_UPDATE  0x0008

typedef enum {LAPI_REGISTER_DGSP=0, LAPI_RESERVE_DGSP,
              LAPI_UNRESERVE_DGSP, LAPI_REG_DDM_FUNC,
              LAPI_DGSP_PACK, LAPI_DGSP_UNPACK,
              LAPI_ADD_UDP_DEST_PORT, LAPI_GET_THREAD_FUNC,
              LAPI_REMOTE_RCXT, LAPI_XLATE_ADDRESS, 
              LAPI_REGISTER_NOTIFICATION, 
              LAPI_TRIGGER_ADD, LAPI_TRIGGER_REMOVE,
              LAPI_ADD_UDP_DEST_EXT, LAPI_STAT_SAVE, LAPI_LAST_UTIL} lapi_util_type_t;

typedef enum {LAPI_NORMAL_CALL=0, LAPI_SEND_COMPL_HNDLR,
              LAPI_RECV_INLINE_HNDLR} lapi_usr_fcall_t;

typedef struct {
    char          *key;     /* unique key */
    lapi_long_t   value;    /* value associated with the key */
} lapi_stat_t;

typedef int (*lapi_trigger_function_t) (
        lapi_handle_t  hndl, 
        int            argc, 
        char*          argv[]);

typedef struct {
    lapi_util_type_t Util_type;    /* In - LAPI_TRIGGER_ADD or                 */
                                   /*      LAPI_TRIGGER_REMOVE                 */
    lapi_trigger_function_t trigger;/*IN - Trigger function pointer. It is     */
                                   /*      ignored for LAPI_TRIGGER_REMOVE     */
    char *trigger_name;            /* In - Name of the trigger function        */
} lapi_trigger_util_t;

typedef struct {
    lapi_util_type_t  Util_type;   /* In - LAPI_REGISTER_DGSP                  */
    lapi_dgsp_descr_t *idgsp;      /* In - Ptr to the descr for DGSP program   */
    lapi_dg_handle_t  dgsp_handle; /* Out - Handle to registered DGSP program  */
    lapi_usr_fcall_t  in_usr_func; /* In - Where this request is made          */
    lapi_status_t     status;      /* Out - Additional Status                  */
} lapi_reg_dgsp_t;

typedef struct {
    lapi_util_type_t  Util_type;   /* In - LAPI_RESERVE_DGSP                   */
    lapi_dg_handle_t  dgsp_handle; /* In - Handle of registered DGSP program   */
    lapi_usr_fcall_t  in_usr_func; /* In - Where this request is made          */
    lapi_status_t     status;      /* Out - Additional Status                  */
} lapi_resv_dgsp_t;

typedef struct {
    lapi_util_type_t  Util_type;   /* In - LAPI_UNRESERVE_DGSP                 */
    lapi_dg_handle_t  dgsp_handle; /* In - hndl by val. of registered DGSP prog*/
    lapi_usr_fcall_t  in_usr_func; /* In - Where this request is made          */
    lapi_status_t     status;      /* Out - Additional Status                  */
} lapi_dref_dgsp_t;

typedef struct {
    lapi_util_type_t  Util_type;   /* In - LAPI_REG_DDM_FUNC                   */
    ddm_func_t        *ddm_func;   /* In - DDM function pointer                */
    lapi_usr_fcall_t  in_usr_func; /* In - Where this request is made          */
    lapi_status_t     status;      /* Out - Additional Status                  */
} lapi_reg_ddm_t;

/* This structure is obselete only for backfard compatibility, should use the  */
/* structure lapi_add_udp_port_ext                                             */
typedef struct {
    lapi_util_type_t  Util_type;   /* In - LAPI_ADD_UDP_DEST_PORT              */
    uint              tgt;         /* In - Taskid - tgt                        */
    lapi_udp_t        *udp_port;   /* In - UDP port info for tgt               */
    uint              instance_no; /* In - Instance of UDP to initialize       */
    lapi_usr_fcall_t  in_usr_func; /* In - Where this request is made          */
    lapi_status_t     status;      /* Out - Additional Status                  */
} lapi_add_udp_port_t;

typedef struct {
    lapi_util_type_t  Util_type;   /* In - LAPI_ADD_UDP_DEST_EXT               */
    uint              tgt;         /* In - Taskid - tgt                        */
    lapi_udp_storage  *udp_port;    /* In - UDP port info for tgt              */
    uint              instance_no; /* In - Instance of UDP to initialize       */
    lapi_usr_fcall_t  in_usr_func; /* In - Where this request is made          */
    lapi_status_t     status;      /* Out - Additional Status                  */
} lapi_add_udp_port_ext;

typedef struct {
    lapi_util_type_t  Util_type;   /* In - LAPI_DGSP_PACK                      */
    lapi_dg_handle_t  dgsp;        /* In - DGSP handle to use                  */
    void              *in_buf;     /* In - Source buffer for PACK              */
    ulong             bytes;       /* In - number of bytes to pack             */
    void              *out_buf;    /* In/Out - Out buffer for PACK             */
    ulong             out_size;    /* In - Output buffer size in bytes         */
    ulong             position;    /* In/Out - Packed buffer offset            */
    lapi_usr_fcall_t  in_usr_func; /* In - Where this request is made          */
    lapi_status_t     status;      /* In - Status to return                    */
} lapi_pack_dgsp_t;

typedef struct {
    lapi_util_type_t  Util_type;   /* In - LAPI_DGSP_UNPACK                    */
    lapi_dg_handle_t  dgsp;        /* In - DGSP handle to use                  */
    void              *in_buf;     /* In - Source buffer for UNPACK            */
    ulong             in_size;     /* In - Source buffer size in bytes         */
    void              *out_buf;    /* In/Out - Out buffer for UNPACK           */
    ulong             bytes;       /* In - Number of bytes to unpack           */
    ulong             position;    /* In/Out - Packed buffer offset            */
    lapi_usr_fcall_t  in_usr_func; /* In - Where this request is made          */
    lapi_status_t     status;      /* In - Status to return                    */
} lapi_unpack_dgsp_t;


typedef struct {
    lapi_util_type_t  Util_type;   /* In - LAPI_REMOTE_RCXT                   */
    lapi_rdma_req_t   operation;   /* Obtain or return                        */
    int               dest;        /* In - Ptr to the descr for DGSP program  */
    lapi_user_cxt_t   usr_rcxt;    /* Out - Remote rCxt                       */
} lapi_remote_cxt_t;

typedef struct {
    lapi_util_type_t  Util_type;   /* In - LAPI_XLATE_ADDRESS                  */
    uint              length;      /* In address length < 32M                  */
    lapi_user_pvo_t   usr_pvo;     /* Out - PVO                                */
    void              *address;    /* In - Ptr to memory to translate          */
    lapi_rdma_req_t   operation;   /* Translate or free                        */
} lapi_get_pvo_t;

typedef struct {
    lapi_util_type_t  Util_type;   /* In - LAPI_REGISTER_NOTIFICATION         */
    lapi_rdma_tag_t   rdma_tag;    /* In - user rdma tag                      */
    lapi_rdma_flg_t   flags;       /* In - type of notification               */
    lapi_cntr_t       *cntr;       /* In - registered counter                 */
    remote_hndlr_t    *callback;   /* In - registered callback                */
    void              *sinfo;      /* In - registered callback  data          */
} lapi_rdma_notification_t;

typedef struct {
    lapi_util_type_t  Util_type;   /* In - LAPI_STAT_SAVE                     */
    int               count;       /* In - num of (key,value) pairs to save   */
    lapi_stat_t       *stat;       /* In - array of (key,value) pairs         */
} lapi_stat_util_t;

#define LAPI_VOID_HANDLE       ((lapi_handle_t)-1)
#define LAPI_NULL_THREAD_ID    ((pthread_t)-1)

typedef int (*lapi_mutex_lock_t)(lapi_handle_t);
typedef int (*lapi_mutex_unlock_t)(lapi_handle_t);
typedef int (*lapi_mutex_trylock_t)(lapi_handle_t);
typedef int (*lapi_mutex_getowner_t)(lapi_handle_t, pthread_t *);
typedef int (*lapi_cond_init_t)(lapi_handle_t, lapi_cond_t *);
typedef int (*lapi_cond_destroy_t)(lapi_handle_t, lapi_cond_t *);
typedef int (*lapi_cond_wait_t)(lapi_handle_t, lapi_cond_t *);
typedef int (*lapi_cond_timedwait_t)(lapi_handle_t, lapi_cond_t *,
                                     struct timespec *);
typedef int (*lapi_cond_signal_t)(lapi_handle_t, lapi_cond_t *);

typedef struct {
    lapi_util_type_t         Util_type;      /* In - LAPI_GET_THREAD_FUNC      */
    lapi_mutex_lock_t        mutex_lock;     /* Out - mutex lock               */
    lapi_mutex_unlock_t      mutex_unlock;   /* Out - mutex unlock             */
    lapi_mutex_trylock_t     mutex_trylock;  /* Out - mutex trylock            */
    lapi_mutex_getowner_t    mutex_getowner; /* Out - mutex getowner           */
    lapi_cond_wait_t         cond_wait;      /* Out - conditional wait         */
    lapi_cond_timedwait_t    cond_timedwait; /* Out - conditional timed wait   */
    lapi_cond_signal_t       cond_signal;    /* Out - conditional signal       */
    lapi_cond_init_t         cond_init;      /* Out - initialize condition     */
    lapi_cond_destroy_t      cond_destroy;   /* Out - destroy condition        */
} lapi_thread_func_t;

typedef union {
    lapi_util_type_t    Util_type;  /* Command type                            */
    lapi_reg_dgsp_t     RegDgsp;    /* Register DGSP type                      */
    lapi_dref_dgsp_t    DrefDgsp;   /* Unregister or unreserve a DGSP          */
    lapi_resv_dgsp_t    ResvDgsp;   /* Reserve and access a DGSP               */
    lapi_reg_ddm_t      DdmFunc;    /* Add Accumulate function command type    */
    lapi_add_udp_port_t Udp;        /* Update UDP port info                    */
    lapi_add_udp_port_ext Udp_ext;  /* Update UDP port info for IPv6 support   */
    lapi_pack_dgsp_t    PackDgsp;   /* Pack data guided by DGSP                */
    lapi_unpack_dgsp_t  UnpackDgsp; /* Unpack data guided by DGSP              */
    lapi_thread_func_t  ThreadFunc; /* Get thread related functions            */
    lapi_get_pvo_t      Pvo;        /* Get PVO                                 */
    lapi_remote_cxt_t   Rcxt;       /* Get remote  Cxt                         */
    lapi_rdma_notification_t RdmaNotify; /* Notification for remote RDMA op.   */
    lapi_trigger_util_t TriggerUtil;/* Register/Deregister a trigger function  */
} lapi_util_t;

/*
** LAPI header handler return info - DGSP enabled
*/

typedef enum {LAPI_DELIVER_MSG=0, LAPI_BURY_MSG, LAPI_DROP_PKT} lapi_ctl_flags_t;
typedef struct {
    ulong             msg_len;     /* To Match the old format */
    int               MAGIC;       /* Field Not to be changed */
    lapi_ret_flags_t  ret_flags;   /* Set flags for LAPI to process */
    lapi_ctl_flags_t  ctl_flags;   /* Normal, truncation, premature ReadySend */
    lapi_dg_handle_t  dgsp_handle; /* DGSP handle */
    ulong             bytes;       /* Number of bytes to process with DGSP */
    int               src;         /* Source of the Message */
    void              *udata_one_pkt_ptr;/* Ptr to data if message <= 1pkt */
    ulong  recv_offset_dgsp_bytes; /* OUT - Initial offset bytes to */
                                   /* run DGSP prior to actual data transfer */
} lapi_return_info_t;

/* 
 ** DGSM support data structures and interfaces - end
*/

/*
** compl_hndlr_t specifies the function signature of the user's completion
** handlers that will be invoked after data corresponding to am_send gets 
** copied. hdr_hndlr_t is the user header handler specified with the
** am_send call that will be invoked on the target node.
*/
typedef void (scompl_hndlr_t)(lapi_handle_t *hndl, void *completion_param,
                             lapi_sh_info_t *info);
typedef void (compl_hndlr_t)(lapi_handle_t *hndl, void * completion_param);
typedef void * (hdr_hndlr_t)(lapi_handle_t *hndl, void *uhdr, uint *uhdr_len, 
                             ulong *msg_len, compl_hndlr_t **comp_h, 
                             void **uinfo);
typedef lapi_vec_t  *(vhdr_hndlr_t) (lapi_handle_t *hndl, void *uhdr, 
                       uint *uhdr_len,  ulong *len_vec, compl_hndlr_t **comp_h,
                       void **uinfo);


/* Information regarding 'flags' field for lapi_xfer_type_t type or'd (|) in */
#define  USE_TGT_VEC_TYPE       0x00000002  /* Use tgt vector description */
#define  LAPI_USE_LAZY_ACKS     0x00000004  /* Delay acks on receiver     */
#define  BUFFER_BOTH_CONTIGUOUS 0x00000010  /* To Enable Zero Copy (hint) for 
                                               this message type */
#define  LAPI_NOT_USE_BULK_XFER 0x00000020  /* Tell LAPI not to use bulk xfer */
#define  LAPI_USE_BULK_XFER     0x00000040  /* Tell LAPI to use bulk xfer */
#define  LAPI_NOT_STRIPE        0x00000080  /* Use first/current handle only */
#define  LAPI_NO_LOCAL_COPY     0x00000100  /* Use first/current handle only */
#define  LAPI_RC_NO_LRU         0x00000200  /* Don't use RC LRU replacement */

/* LAPI vector flags */

typedef enum { LAPI_GET_XFER, LAPI_AM_XFER, LAPI_PUT_XFER,
               LAPI_GETV_XFER, LAPI_PUTV_XFER, LAPI_AMV_XFER,
               LAPI_RMW_XFER, LAPI_DGSP_XFER, LAPI_RDMA_XFER,
               LAPI_AM_LW_XFER, LAPI_AMX_XFER, LAPI_MC_XFER,
               LAPI_LAST_XFER
             } lapi_xfer_type_t;


typedef struct {
        lapi_xfer_type_t  Xfer_type; /* must be  LAPI_GET_XFER    */
        int               flags;     /* use zero copy for example */
        uint              tgt;       /* get target                */
#ifdef __64BIT__
        uint              pad;       /* padding alignment         */
#endif /* __64BIT__ */
        lapi_long_t       tgt_addr;  /* target address of get     */
        void              *org_addr; /* origin address            */
        ulong             len;       /* tranfer length            */
        lapi_long_t       tgt_cntr;  /* target counter            */
        lapi_cntr_t       *org_cntr; /* origin counter            */
        compl_hndlr_t     *chndlr;   /* completion handler        */
        void              *cinfo;    /* completion handler data   */
} lapi_get_t;

typedef struct {
        lapi_xfer_type_t  Xfer_type; /* must be  LAPI_AM_XFER     */
        int               flags;     /* use zero copy for example */
        uint              tgt;       /* target task               */
#ifdef __64BIT__
        uint              pad;       /* padding alignment         */
#endif /* __64BIT__ */
        lapi_long_t       hdr_hdl;   /* Am  header handler        */
        uint              uhdr_len;  /* user header length        */
        void              *uhdr;     /* user header data          */
        void              *udata;    /* user data to be xfered    */
        ulong             udata_len; /* transfer length           */
        scompl_hndlr_t    *shdlr;    /* send completion handler   */ 
        void              *sinfo;    /* send completion data      */
        lapi_long_t       tgt_cntr;  /* target counter            */
        lapi_cntr_t       *org_cntr; /* origin counter            */
        lapi_cntr_t       *cmpl_cntr;/* completion counter        */
} lapi_am_t;


typedef struct {
        lapi_xfer_type_t  Xfer_type; /* must be  LAPI_PUT_XFER    */
        int               flags;     /* use zero copy for example */
        uint              tgt;       /* target task               */
#ifdef __64BIT__
        uint              pad;       /* structure pad             */
#endif  /*__64BIT__*/
        lapi_long_t       tgt_addr;  /* target address of put     */
        void              *org_addr; /* Origin data address       */
        ulong             len;       /* transfer length           */
        scompl_hndlr_t    *shdlr;    /* send completion handler   */ 
        void              *sinfo;    /* send completion data      */
        lapi_long_t       tgt_cntr;  /* target counter            */
        lapi_cntr_t       *org_cntr; /* origin counter            */
        lapi_cntr_t       *cmpl_cntr;/* completion counter        */
} lapi_put_t;


typedef struct {
        lapi_xfer_type_t  Xfer_type; /* must be  LAPI_GETV_XFER   */
        int               flags;     /* use zero copy for example */
        uint              tgt;       /* get target                */
#ifdef __64BIT__
        uint              pad;       /* padding alignment         */
#endif /* __64BIT__ */
        lapi_vec_t        *org_vec;  /* origin vector for vector  */
                                     /* get                       */
        void              *tgt_vec;  /* target vector for vector  */
        lapi_long_t       tgt_cntr;  /* target counter            */
        lapi_cntr_t       *org_cntr; /* origin counter            */
        compl_hndlr_t     *chndlr;   /* completion handler        */
        void              *cinfo;    /* completion handler data   */
} lapi_getv_t;

typedef struct {
        lapi_xfer_type_t  Xfer_type; /* must be  LAPI_AMV_XFER    */
        int               flags;     /* use zero copy for example */
        uint              tgt;       /* target task               */
#ifdef __64BIT__
        uint              pad;       /* padding alignment         */
#endif /* __64BIT__ */
        lapi_long_t       hdr_hdl;   /* Am  header handler        */
        uint              uhdr_len;  /* user header length        */
        void              *uhdr;     /* user header data          */
        scompl_hndlr_t    *shdlr;    /* send completion handler   */ 
        void              *sinfo;    /* send completion data      */
        lapi_vec_t        *org_vec;  /* origin vector for vector  */
                                     /* transfers                 */
        lapi_long_t       tgt_cntr;  /* target counter            */
        lapi_cntr_t       *org_cntr; /* origin counter            */
        lapi_cntr_t       *cmpl_cntr;/* completion counter        */
} lapi_amv_t;


typedef struct {
        lapi_xfer_type_t  Xfer_type; /* must be  LAPI_PUTV_XFER   */
        int               flags;     /* use zero copy for example */
        uint              tgt;       /* target task               */
#ifdef __64BIT__
        uint              pad;       /* structure pad             */
#endif  /*__64BIT__*/
        scompl_hndlr_t    *shdlr;    /* send completion handler   */ 
        void              *sinfo;    /* send completion data      */
        lapi_vec_t        *org_vec;  /* origin vector for vector  */
                                     /* transfers                 */
        void              *tgt_vec;  /* target vector pointer for */
                                     /* vector transfers          */
        lapi_long_t       tgt_cntr;  /* target counter            */
        lapi_cntr_t       *org_cntr; /* origin counter            */
        lapi_cntr_t       *cmpl_cntr;/* completion counter        */
} lapi_putv_t;

typedef struct {
        lapi_xfer_type_t  Xfer_type; /* must be LAPI_DGSP_XFER    */
        int               flags;     /* i.e. BUFFER_BOTH_CONTIGUOUS */
        uint              tgt;       /* target task               */
#ifdef __64BIT__
        uint              pad;       /* padding alignment         */
#endif /* __64BIT__ */
        lapi_long_t       hdr_hdl;   /* Am  header handler        */
        uint              uhdr_len;  /* user header length        */
        void              *uhdr;     /* user header data          */
        void              *udata;    /* user data to be xfered    */
        ulong             udata_len; /* transfer length           */
        scompl_hndlr_t    *shdlr;    /* send completion handler   */ 
        void              *sinfo;    /* send completion data      */
        lapi_long_t       tgt_cntr;  /* target counter            */
        lapi_cntr_t       *org_cntr; /* origin counter            */
        lapi_cntr_t       *cmpl_cntr;/* origin counter            */
        lapi_dg_handle_t  dgsp;      /* DGSP handle to use        */
        lapi_status_t     status;    /* Status to return          */
} lapi_amdgsp_t;


typedef struct {
        lapi_xfer_type_t  Xfer_type; /* must be  LAPI_RMW_XFER    */
        RMW_ops_t         op;        /* Specify operation type    */
        uint              tgt;       /* target task               */
        uint              size;      /* must be 32 or 64          */
        lapi_long_t       tgt_var;   /* Target address of RMW     */
        void              *in_val;   /* The value input to the op */
        void              *prev_tgt_val; /* previous value at target */
        lapi_cntr_t       *org_cntr; /* origin counter            */
        scompl_hndlr_t    *shdlr;    /* send completion handler   */
        void              *sinfo;    /* send completion data      */
} lapi_rmw_t;

typedef struct {
        lapi_xfer_type_t  Xfer_type; /* must be  LAPI_RDMA_XFER   */
        uint              tgt;       /* Target task               */
        lapi_rdma_op_t    op;        /* Specify operation type    */
        lapi_rdma_tag_t   rdma_tag;  /* User rdma tag             */
        lapi_user_cxt_t   remote_cxt; /* Context on remote side   */
        lapi_user_pvo_t   src_pvo;   /* Master PVO                */
        lapi_user_pvo_t   tgt_pvo;   /* Slave PVO                 */
        uint              src_offset;/* Source offset             */
        uint              tgt_offset;/* Target offset             */
        uint              len;       /* Length of data to be sent */
#ifdef __64BIT__
        uint              pad;       /* padding alignment         */
#endif /* __64BIT__ */
        lapi_cntr_t       *org_cntr; /* origin counter            */
        scompl_hndlr_t    *shdlr;    /* send completion handler   */
        void              *sinfo;    /* send completion data      */
} lapi_hwxfer_t;

typedef struct {
        lapi_xfer_type_t   Xfer_type; /* must be LAPI_AMX_XFER     */
        int                flags;     /* i.e. BUFFER_BOTH_CONTIGUOUS */
        uint               tgt;       /* target task               */
#ifdef __64BIT__
        uint               pad;       /* padding alignment         */
#endif /* __64BIT__ */
        lapi_long_t        hdr_hdl;   /* Am  header handler        */
        uint               uhdr_len;  /* user header length        */
        void               *uhdr;     /* user header data          */
        void               *udata;    /* user data to be xfered    */
        ulong              udata_len; /* transfer length           */
        scompl_hndlr_t     *shdlr;    /* send completion handler   */
        void               *sinfo;    /* send completion data      */
        lapi_long_t        tgt_cntr;  /* target counter            */
        lapi_cntr_t        *org_cntr; /* origin counter            */
        lapi_cntr_t        *cmpl_cntr;/* origin counter            */
        lapi_dg_handle_t   dgsp;      /* DGSP handle to use        */
        lapi_status_t      status;    /* Status to return          */
        ulong send_offset_dgsp_bytes; /* Initial offset bytes to   */
                         /* run DGSP prior to actual data transfer */
} lapi_amx_t;

typedef struct {
        lapi_xfer_type_t  Xfer_type; /* must be LAPI_MC_XFER      */
        int               flags;     /* use zero copy for example */
        lapi_group_t  group;       /* target group              */
#ifdef __64BIT__
        uint              pad;       /* padding alignment         */
#endif /* __64BIT__ */
        lapi_long_t       hdr_hdl;   /* Am  header handler        */
        uint              uhdr_len;  /* user header length        */
        void              *uhdr;     /* user header data          */
        void              *udata;    /* user data to be xfered    */
        ulong             udata_len; /* transfer length           */
        scompl_hndlr_t    *shdlr;    /* send completion handler   */
        void              *sinfo;    /* send completion data      */
} lapi_mc_t;

typedef union {
        lapi_xfer_type_t   Xfer_type;
        lapi_get_t         Get;
        lapi_am_t          Am;
        lapi_rmw_t         Rmw;
        lapi_put_t         Put;
        lapi_getv_t        Getv;
        lapi_putv_t        Putv;
        lapi_amv_t         Amv;
        lapi_amdgsp_t      Dgsp;
        lapi_hwxfer_t      HwXfer;
        lapi_amx_t         Amx;
        lapi_mc_t           Mc;
} lapi_xfer_t;


#ifdef __cplusplus
extern "C" {
#endif
/*
***************************************************************
****************** LAPI C Externalized API's ******************
***************************************************************
*/
/*
** LAPI Setup, Query, Set and Terminate functions.
*/
int LAPI_Init(lapi_handle_t *hndl, lapi_info_t *lapi_info);
int LAPI_Term(lapi_handle_t hndl);
int LAPI_Qenv(lapi_handle_t hndl, lapi_query_t query, int *ret_val);
int LAPI_Senv(lapi_handle_t hndl, lapi_query_t query, int set_val);
int LAPI_Msg_string(int error_code, void *buf);
int LAPI_Xfer(lapi_handle_t hndl, lapi_xfer_t *xfer_cmd);
int LAPI_Msgpoll(lapi_handle_t hndl, uint cnt, lapi_msg_info_t *info);
int LAPI_Util(lapi_handle_t hndl, lapi_util_t *util_p);


/*
** LAPI Put & Get functions.
*/
int LAPI_Put(lapi_handle_t hndl, uint tgt, ulong len, void *tgt_addr,
               void *org_addr, lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr,
               lapi_cntr_t *cmpl_cntr);

int LAPI_Putv(lapi_handle_t hndl, uint tgt, lapi_vec_t *tgt_vec,
            lapi_vec_t *org_addr, lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr,
               lapi_cntr_t *cmpl_cntr);
int LAPI_Getv(lapi_handle_t hndl, uint tgt, lapi_vec_t *tgt_vec,
           lapi_vec_t *org_vec, lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr);
int LAPI_Amsendv(lapi_handle_t hndl, uint tgt, void *hdr_hdl, 
           void *uhdr,uint uhdr_len, lapi_vec_t *org_vec,
           lapi_cntr_t  *tgt_cntr, lapi_cntr_t *org_cntr,
           lapi_cntr_t *cmpl_cntr);



/*
** LAPI Counter & Probe functions.
*/
int LAPI_Probe(lapi_handle_t  hndl);
int LAPI_Getcntr(lapi_handle_t hndl, lapi_cntr_t *cntr, int *val);
int LAPI_Setcntr(lapi_handle_t hndl, lapi_cntr_t *cntr, int val);
int LAPI_Setcntr_wstatus(lapi_handle_t hndl, lapi_cntr_t *cntr, int val,
                         uint *dest, int *dest_status);
int LAPI_Waitcntr(lapi_handle_t hndl, lapi_cntr_t *cntr, int val,
                  int *cur_cntr_val);
int LAPI_Nopoll_wait(lapi_handle_t hndl, lapi_cntr_t *cntr_ptr, int val,
                    int *cur_cntr_val);
int LAPI_Purge_totask(lapi_handle_t hndl, css_task_t dest);
int LAPI_Resume_totask(lapi_handle_t hndl, css_task_t dest);

/*
** LAPI RMW function.
*/
int LAPI_Rmw(lapi_handle_t hndl, RMW_ops_t op, uint tgt, int *tgt_var, 
             int *in_val, int *prev_tgt_val, lapi_cntr_t *org_cntr);

int LAPI_Rmw64(lapi_handle_t hndl, RMW_ops_t op, uint tgt, long long *tgt_var,
             long long *in_val, long long *prev_tgt_val, lapi_cntr_t *org_cntr);

/*
** LAPI Fence functions.
*/
int LAPI_Fence(lapi_handle_t hndl);
int LAPI_Gfence(lapi_handle_t hndl);

/*
** LAPI Address exchange functions.
*/
int LAPI_Address(void *my_addr, ulong *ret_addr);
int LAPI_Address_init(lapi_handle_t hndl, void *my_tab, void *add_tab[]);
int LAPI_Address_init64(lapi_handle_t hndl, lapi_long_t my_tab,
                        lapi_long_t *add_tab);
int LAPI_Addr_set(lapi_handle_t hndl, void *addr, int addr_hndl);
int LAPI_Addr_get(lapi_handle_t hndl, void **addr, int addr_hndl);

/*
** LAPI Active Message functions.
*/
int LAPI_Get(lapi_handle_t hndl, uint tgt, ulong len, void *tgt_addr, 
               void *org_addr, lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr);
int LAPI_Amsend(lapi_handle_t hndl, uint tgt, void *hdr_hdl, void *uhdr,
                 uint uhdr_len, void *udata, ulong udata_len,
                 lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr,
                 lapi_cntr_t *cmpl_cntr);

/*
** LAPI Multicast functions.
*/
int LAPI_Group_create(lapi_handle_t hndl, uint num_tasks, 
           uint *task_list, lapi_group_t *group);
int LAPI_Group_free(lapi_handle_t hndl, lapi_group_t group);

/*
***************************************************************
****************** LAPI C Profiling API's ******************
***************************************************************
*/
/*
** LAPI Setup, Query, Set and Terminate functions.
*/
int PLAPI_Init(lapi_handle_t *hndl, lapi_info_t *lapi_info);
int PLAPI_Term(lapi_handle_t hndl);
int PLAPI_Qenv(lapi_handle_t hndl, lapi_query_t query, int *ret_val);
int PLAPI_Senv(lapi_handle_t hndl, lapi_query_t query, int set_val);
int PLAPI_Msg_string(int error_code, void *buf);
int PLAPI_Xfer(lapi_handle_t hndl, lapi_xfer_t *xfer_cmd);
int PLAPI_Msgpoll(lapi_handle_t hndl, uint cnt, lapi_msg_info_t *info);
int PLAPI_Util(lapi_handle_t hndl, lapi_util_t *util_p);


/*
** LAPI Put & Get functions.
*/
int PLAPI_Put(lapi_handle_t hndl, uint tgt, ulong len, void *tgt_addr,
               void *org_addr, lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr,
               lapi_cntr_t *cmpl_cntr);

int PLAPI_Putv(lapi_handle_t hndl, uint tgt, lapi_vec_t *tgt_vec,
            lapi_vec_t *org_addr, lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr,
               lapi_cntr_t *cmpl_cntr);
int PLAPI_Getv(lapi_handle_t hndl, uint tgt, lapi_vec_t *tgt_vec,
           lapi_vec_t *org_vec, lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr);
int PLAPI_Amsendv(lapi_handle_t hndl, uint tgt, void *hdr_hdl, 
           void *uhdr,uint uhdr_len, lapi_vec_t *org_vec,
           lapi_cntr_t  *tgt_cntr, lapi_cntr_t *org_cntr,
           lapi_cntr_t *cmpl_cntr);



/*
** LAPI Counter & Probe functions.
*/
int PLAPI_Probe(lapi_handle_t  hndl);
int PLAPI_Getcntr(lapi_handle_t hndl, lapi_cntr_t *cntr, int *val);
int PLAPI_Setcntr(lapi_handle_t hndl, lapi_cntr_t *cntr, int val);
int PLAPI_Setcntr_wstatus(lapi_handle_t hndl, lapi_cntr_t *cntr, int val,
                         uint *dest, int *dest_status);
int PLAPI_Waitcntr(lapi_handle_t hndl, lapi_cntr_t *cntr, int val,
                  int *cur_cntr_val);
int PLAPI_Nopoll_wait(lapi_handle_t hndl, lapi_cntr_t *cntr_ptr, int val,
                    int *cur_cntr_val);
int PLAPI_Purge_totask(lapi_handle_t hndl, css_task_t dest);
int PLAPI_Resume_totask(lapi_handle_t hndl, css_task_t dest);

/*
** LAPI RMW function.
*/
int PLAPI_Rmw(lapi_handle_t hndl, RMW_ops_t op, uint tgt, int *tgt_var, 
             int *in_val, int *prev_tgt_val, lapi_cntr_t *org_cntr);

int PLAPI_Rmw64(lapi_handle_t hndl, RMW_ops_t op, uint tgt, long long *tgt_var,
             long long *in_val, long long *prev_tgt_val, lapi_cntr_t *org_cntr);

/*
** LAPI Fence functions.
*/
int PLAPI_Fence(lapi_handle_t hndl);
int PLAPI_Gfence(lapi_handle_t hndl);

/*
** LAPI Address exchange functions.
*/
int PLAPI_Address(void *my_addr, ulong *ret_addr);
int PLAPI_Address_init(lapi_handle_t hndl, void *my_tab, void *add_tab[]);
int PLAPI_Address_init64(lapi_handle_t hndl, lapi_long_t my_tab,
                         lapi_long_t *add_tab);
int PLAPI_Addr_set(lapi_handle_t hndl, void *addr, int addr_hndl);
int PLAPI_Addr_get(lapi_handle_t hndl, void **addr, int addr_hndl);

/*
** LAPI Active Message functions.
*/
int PLAPI_Get(lapi_handle_t hndl, uint tgt, ulong len, void *tgt_addr, 
               void *org_addr, lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr);
int PLAPI_Amsend(lapi_handle_t hndl, uint tgt, void *hdr_hdl, void *uhdr,
                 uint uhdr_len, void *udata, ulong udata_len,
                 lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr,
                 lapi_cntr_t *cmpl_cntr);

/*
** LAPI Multicast functions.
*/
int PLAPI_Group_create(lapi_handle_t hndl, uint num_tasks, 
           uint *task_list, lapi_group_t *group);
int PLAPI_Group_free(lapi_handle_t hndl, lapi_group_t group);

#ifdef __cplusplus
}
#endif

#endif /* _lapi_h_ */
