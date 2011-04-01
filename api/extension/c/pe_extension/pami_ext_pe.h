/**
 * \file sys/pami_ext_pe.h
 * \brief platform-specific messaging interface
 * \todo  put this into percs subdir and rename to pami_ext.h
 */
#ifndef __pami_ext_pe_h__
#define __pami_ext_pe_h__

#include "pami.h"
#ifndef PAMI_EXT_ATTR
#define PAMI_EXT_ATTR 1000 /**< starting value for extended attributes */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

  /*****************************************************************************/
  /**
   * \defgroup ext_percs PERCS specific extensions to core PAMI
   *
   * This extension defines features that are available only on PERCS and
   * InfiniBand platforms.
   * \{
   */
  /*****************************************************************************/

  /**
   * \defgroup ext_attr Extended attributes for configuration
   *
   * Explicit cast from \ref pami_attribute_name_ext_t into
   * \ref pami_attribute_name_t may be needed.
   *
   * \{
   */

  typedef enum {
    /* Attribute                     usage : type : default : description   */

    PAMI_ACTIVE_CLIENT             /**<  Q : pami_active_client_t : N/A : retrieve all active clients */
        = PAMI_EXT_ATTR,

    PAMI_CLIENT_RELIABLE           /**< CQ : bool : true : run in reliable mode */
        = PAMI_EXT_ATTR + 100,
    PAMI_CLIENT_THREAD_SAFE,       /**< CQ : bool : true : run in thread-safe mode */
    PAMI_CLIENT_PROTOCOL_NAME,     /**<  Q : string : N/A : retrieve protocol name of the client*/
    PAMI_CLIENT_ACTIVE_CONTEXT,    /**<  Q : pami_active_context_t : N/A : retrieve all active contexts in the client */

    PAMI_CONTEXT_CHECK_PARAM       /**< CQU: bool : true : check function parameters */
        = PAMI_EXT_ATTR + 200,
    PAMI_CONTEXT_ERROR_HANDLER,    /**< CQ : pami_error_handler_t : NULL : asynchronous error handler */
    PAMI_CONTEXT_STATISTICS,       /**<  Q : pami_statistics_t : N/A : retrieve communication statistics */
    PAMI_CONTEXT_TRIGGER,          /**<   U: pami_trigger_t : N/A : add or remove a trigger */
    PAMI_CONTEXT_NETWORK,          /**<  Q : string : N/A : retrieve network string */
    PAMI_CONTEXT_MAX_PKT_SZ,       /**<  Q : size_t : N/A : payload size of 1 packet */
    PAMI_CONTEXT_RFIFO_SZ,         /**<  Q : size_t : N/A : receiving fifo size */
    PAMI_CONTEXT_BULK_MIN_MSG_SZ,  /**<  Q : size_t : N/A : size of smallest RDMA message*/
    PAMI_CONTEXT_BULK_XFER,        /**<  Q : bool   : N/A : run in RDMA mode */
    PAMI_CONTEXT_SHM_NUM_TASKS,    /**<  Q : size_t : N/A : number of tasks that uses SHM */
    PAMI_CONTEXT_SHM_TASKS,        /**<  Q : pami_tasks_t* : N/A : list of task ids that make up SHM*/
    PAMI_CONTEXT_SHM_ENABLED,      /**<  Q : bool   : N/A : run in SHM mode*/
    PAMI_CONTEXT_ACK_THRESH,       /**<  Q : size_t : N/A : threshold of ACKs sent back to src */
    PAMI_CONTEXT_REXMIT_BUF_CNT,   /**<  Q : size_t : N/A : number of retransmission buffers */
    PAMI_CONTEXT_REXMIT_BUF_SZ,    /**<  Q : size_t : N/A : retransmission buffer  size */
    PAMI_CONTEXT_RC_MAX_QP,        /**<  Q : size_t : N/A : max number of RC QPs */
    PAMI_CONTEXT_RC_USE_LMC,       /**<  Q : bool   : N/A : run with one RC QP per path */
  } pami_attribute_name_ext_t;

  /** \} */ /* end of "ext_attr" group */

  /**
   * \defgroup error_handler Asynchronous error handler
   * \{
   */

  typedef void (*pami_async_function) (pami_context_t  context, 
                                       void *          cookie);

  typedef enum {
      PAMI_ASYNC_ALL                = 0,
      PAMI_ASYNC_RECV_INTERRUPT     = 1,
      PAMI_ASYNC_TIMER              = 2,
      PAMI_ASYNC_EXT                = 1000
  } pami_async_t;

  typedef pami_result_t (*async_progress_register_fn) (
          pami_context_t          context,
          pami_async_function     progress_fn,
          pami_async_function     suspend_fn,
          pami_async_function     resume_fn,
          void*                   cookie);

  typedef pami_result_t (*async_progress_enable_fn) (
          pami_context_t          context,
          pami_async_t            event_type);

  typedef pami_result_t (*async_progress_disable_fn) (
          pami_context_t          context,
          pami_async_t            event_type);

  /**
   * \brief PAMI asynchronous error handler
   *
   * In general, an asynchronous error handler should terminate the process when
   * invoked. If the handler returns, the process will not function normally.
   *
   * \param [in]  context        The PAMI context
   * \param [in]  result         The result (error code)
   *
   */
  typedef void (pami_error_handler_t)( pami_context_t  context,
                                       pami_result_t   result);

  /** \} */ /* end of "error_handler" group */

  /**
   * \defgroup comm_stat Communication statistics
   * \{
   *
   * \ref PAMI_Context_query with \c PAMI_CONTEXT_STATISTICS returns in
   * \ref pami_attribute_value_t.chararray a pointer to \ref pami_statistics_t
   * whose memory is managed by PAMI internally.
   *
   * \c counters field in \ref pami_statistics_t is a variable-length array.
   */

  #define PAMI_COUNTER_NAME_LEN  40   /**< Maximum counter name length */

  /**
   * \brief Counter for statistics
   */
  typedef struct
  {
    char               name[PAMI_COUNTER_NAME_LEN]; /**< Counter name  */
    unsigned long long value;                       /**< Counter value */
  } pami_counter_t;

  /**
   * \brief Array of counters for statistics
   */
  typedef struct
  {
    int                count;       /**< Number of counters */
    pami_counter_t     counters[1]; /**< Array of counters  */
  } pami_statistics_t;

  /**
   * \brief List of SHM tasks
   */
  typedef struct
  {
    int                count;       /**< Number of counters */
    pami_task_t        shm_tasks[1];/**< List of SHM tasks  */
  } pami_shm_tasks_t;

  /** \} */ /* end of "comm_stat" group */

  /**
   * \defgroup trigger User-defined triggers
   * \{
   *
   * \ref PAMI_Context_update with \c PAMI_CLIENT_TRIGGER passes in
   * \ref pami_attribute_value_t.chararray as a pointer to \ref pami_trigger_t
   * to add or remove a user-defined trigger.
   *
   * \c NULL as trigger_func in \ref pami_statistics_t indicates removal of the
   * trigger with name specified in trigger_name.
   */

  #define PAMI_VOID_CONTEXT  NULL   /**< PAMI context for indirect PAMI user */

  /**
   * \brief Signature of trigger functions
   */
  typedef int (*pami_trigger_func_t) (
    pami_context_t     context,
    int                argc,
    char               *argv[]);

  /**
   * \brief Triggers
   */
  typedef struct
  {
    pami_trigger_func_t trigger_func;
    char                *trigger_name;
  } pami_trigger_t;

  /** \} */ /* end of "trigger" group */

  /**
   * \defgroup act_clients Active PAMI clients
   * \{
   *
   * \ref PAMI_Context_query with \c PAMI_ACTIVE_CLIENT returns in
   * \ref pami_attribute_value_t.chararray a pointer to 
   * \ref pami_active_client_t whose memory is managed by PAMI internally.
   *
   */
  

  #define PAMI_VOID_CLIENT      NULL   /**< PAMI client used to open PE Extension before any client is created */

  /**
   * \brief Struct user get after query with PAMI_ACTIVE_CLIENT
   */
  typedef struct
  {
    const size_t        client_num;  
    const pami_client_t clients[1];
  } pami_active_client_t; 
  
  /** \} */ /* end of "act_clients" group */

  /**
   * \defgroup act_context Active PAMI Context in a Client
   * \{
   *
   * \ref PAMI_Context_query with \c PAMI_ACTIVE_CONTEXT returns in
   * \ref pami_attribute_value_t.chararray a pointer to 
   * \ref pami_active_context_t whose memory is managed by PAMI internally.
   *
   */

  /**
   * \brief Struct user get after query with PAMI_ACTIVE_CONTEXT
   */
  typedef struct
  {
    const size_t         context_num;  
    const pami_context_t contexts[1];
  } pami_active_context_t; 
  
  /** \} */ /* end of "act_clients" group */
  /*****************************************************************************/
  /**
   * \defgroup mutex_cond Context mutex and condition
   *
   * This extension defines more functions that are similar to
   * pthread_mutex_* and pthread_cond_* functions.
   * \{
   */
  /*****************************************************************************/

  typedef void * pami_cond_t;  /**< Condition */

  /**
   * \brief Get the owner of the context lock
   *
   * \param[in]  context PAMI communication context
   * \param[out] owner   Owner of the context lock, compatible to pthread_t
   *
   * \retval PAMI_SUCCESS  The mutex owner has been retrieved.
   * \retval PAMI_INVAL    The \c context or the \c owner pointer is invalid.
   */
  pami_result_t PAMI_Context_mutex_getowner (pami_context_t context, unsigned long *owner);

  /*
   * The following functions are modelled after pthread_cond_* functions.
   */
  /**
   * \brief Create a condition
   *
   * \param[in]  context PAMI communication context
   * \param[out] cond    The condition created
   *
   * \retval PAMI_SUCCESS  The condition has been created.
   * \retval PAMI_INVAL    The \c context or the \c cond pointer is invalid.
   */
  pami_result_t PAMI_Context_cond_create (pami_context_t context, pami_cond_t *cond);

  /**
   * \brief Wait on a condition
   * \note  The caller must have the context lock. Upon return, the caller
   *        still has the lock.
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to wait
   *
   * \retval PAMI_SUCCESS  The condition has been met.
   * \retval PAMI_INVAL    The \c context or the \c cond is invalid.
   */
  pami_result_t PAMI_Context_cond_wait (pami_context_t context, pami_cond_t cond);

  /**
   * \brief Wait on a condition with timeout
   * \note  The caller must have the context lock. Upon return, the caller
   *        still has the lock.
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to wait
   * \param[in]  time    The time to wait
   *
   * \retval PAMI_SUCCESS  The condition has been met.
   * \retval PAMI_EAGAIN   The wait time has expired.
   * \retval PAMI_INVAL    The \c context, the \c cond or the \c time pointer
   *                       is invalid.
   */
  pami_result_t PAMI_Context_cond_timedwait (pami_context_t context, pami_cond_t cond,
          struct timespec *time);

  /**
   * \brief Signal to wake up a waiter on a condition
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to signal
   *
   * \retval PAMI_SUCCESS  The signal has been delivered.
   * \retval PAMI_INVAL    The \c context or the \c cond is invalid.
   */
  pami_result_t PAMI_Context_cond_signal (pami_context_t context, pami_cond_t cond);

  /**
   * \brief Signal to wake up all waiters on a condition
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to sigal
   *
   * \retval PAMI_SUCCESS  The signal has been delivered.
   * \retval PAMI_INVAL    The \c context or the \c cond is invalid.
   */
  pami_result_t PAMI_Context_cond_broadcast (pami_context_t context, pami_cond_t cond);

  /**
   * \brief Destroy a condition
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to destroy
   *
   * \retval PAMI_SUCCESS  The condition has been destroyed.
   * \retval PAMI_INVAL    The \c context or the \c cond is invalid.
   */
  pami_result_t PAMI_Context_cond_destroy (pami_context_t context, pami_cond_t cond);

  /**
   * \brief Function pointers fot the above member functions
   */
  typedef pami_result_t (*global_query_fn) (pami_configuration_t configuration[], size_t num_configs);
  typedef pami_result_t (*mutex_getowner_fn) (pami_context_t context, unsigned long *owner);
  typedef pami_result_t (*cond_create_fn) (pami_context_t context, pami_cond_t *cond);
  typedef pami_result_t (*cond_wait_fn) (pami_context_t context, pami_cond_t *cond);
  typedef pami_result_t (*cond_timedwait_fn) (pami_context_t context, pami_cond_t *cond);
  typedef pami_result_t (*cond_signal_fn) (pami_context_t context, pami_cond_t *cond);
  typedef pami_result_t (*cond_broadcast_fn) (pami_context_t context, pami_cond_t *cond);
  typedef pami_result_t (*cond_destroy_fn) (pami_context_t context, pami_cond_t *cond);

  /** \} */ /* end of "mutex_cond" group */
  /** \} */ /* end of "ext_percs" group */

#ifdef __cplusplus
}
#endif

#endif
