/**
 * \file sys/pami_ext_percs.h
 * \brief platform-specific messaging interface
 * \todo  put this into percs subdir and rename to pami_ext.h
 */
#ifndef __pami_ext_percs_h__
#define __pami_ext_percs_h__

#include "pami.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*****************************************************************************/
  /**
   * \defgroup ext_percs PERCS specific extensions to core PAMI
   *
   * Some brief documentation on percs extensions ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Callback to provide data at send side or consume data at receive side
   *
   * Returns number of bytes copied into pipe buffer at send side
   * or number of bytes copied out from pipe buffer at receive side.
   *
   * TBD: <=0 byte being copied implies truncation
   *
   * \param[in] context   PAMI communication context
   * \param[in] cookie    Event callback application argument
   * \param[in] offset    Starting data offset (???)
   * \param[in] pipe_addr Address of the PAMI pipe buffer
   * \param[in] pipe_size Size of the PAMI pipe buffer
   *
   * \return Number of bytes processed (read or written) from the pipe address
   */
  typedef size_t (*pami_data_function) ( pami_context_t   context,
                                        void          * cookie,
                                        size_t          offset,
                                        void          * pipe_addr,
                                        size_t          pipe_size );

  /**
   * \brief Structure for send parameters unique to a direct active message send
   */
  typedef struct
  {
    pami_send_t             send;     /**< Common send parameters */
    struct
    {
      pami_data_function    data_fn;  /**< Data callback function */
      pami_event_function   local_fn; /**< Local message completion event */
      pami_event_function   remote_fn;/**< Remote message completion event ------ why is this needed ? */
    } direct;                        /**< Direct send parameters */
  } pami_send_direct_t;

  /**
   * \brief Non-blocking active message send for direct data injection
   *
   * Source data to transfer is provided by the pami client via a series of send
   * callbacks in which the pami client will copy the source data directly into
   * the available network resources.
   *
   * The input parameters of the data callback will specify the output data
   * address and the maximum data size in bytes. As a convenience, the pami
   * client may query the configuration attribute \c DIRECT_SEND_LIMIT to
   * obtain the maximum direct data size outside of the callback mechanism.
   *
   * Typically, the \c DIRECT_SEND_LIMIT is associated with a network
   * attribute, such as a packet size.
   *
   * \see pami_data_function
   *
   * \param[in] context    PAMI communication context
   * \param[in] parameters Send direct parameter structure
   */
  pami_result_t PAMI_Send_direct (pami_context_t       context,
                                pami_send_direct_t * parameters);




  /*****************************************************************************/
  /**
   * \defgroup mutex_cond PAMI context mutext and condition
   *
   * Some brief documentation on active message stuff ...
   * \{
   */
  /*****************************************************************************/

  typedef void * pami_cond_t;

  /**
   * \brief Get the owner of the context lock
   *
   * \param[in]  context PAMI communication context
   * \param[out] owner   Owner of the context lock, compatible to pthread_t
   */
  pami_result_t PAMI_Mutex_getowner (pami_context_t context, unsigned long *owner);

  /*
   * The following functions are modelled after pthread_cond_* functions.
   */
  /**
   * \brief Create a condition
   *
   * \param[in]  context PAMI communication context
   * \param[out] cond    The condition created
   */
  pami_result_t PAMI_Cond_create (pami_context_t context, pami_cond_t *cond);

  /**
   * \brief Wait on a condition
   * \note  The caller must have the context lock. Upon return, the caller
   *        still has the lock.
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to wait
   */
  pami_result_t PAMI_Cond_wait (pami_context_t context, pami_cond_t cond);

  /**
   * \brief Wait on a condition with timeout
   * \note  The caller must have the context lock. Upon return, the caller
   *        still has the lock.
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to wait
   */
  pami_result_t PAMI_Cond_timedwait (pami_context_t context, pami_cond_t cond,
          struct timespec *time);

  /**
   * \brief Signal to wake up a waiter on a condition
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to signal
   */
  pami_result_t PAMI_Cond_signal (pami_context_t context, pami_cond_t cond);

  /**
   * \brief Signal to wake up all waiters on a condition
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to sigal
   */
  pami_result_t PAMI_Cond_broadcast (pami_context_t context, pami_cond_t cond);

  /**
   * \brief Destroy a condition
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to destroy
   */
  pami_result_t PAMI_Cond_destroy (pami_context_t context, pami_cond_t cond);

  /** \} */ /* end of "mutex_cond" group */
  /** \} */ /* end of "ext_percs" group */

#ifdef __cplusplus
}
#endif

#endif
