/**
 * \file xmi_ext_percs.h
 * \brief platform-specific messaging interface
 * \todo  put this into percs subdir and rename to xmi_ext.h
 */
#ifndef __xmi_ext_percs_h__
#define __xmi_ext_percs_h__

#include "xmi.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*****************************************************************************/
  /**
   * \defgroup ext_percs PERCS specific extensions to core XMI
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
   * \param[in] context   XMI communication context
   * \param[in] cookie    Event callback application argument
   * \param[in] offset    Starting data offset (???)
   * \param[in] pipe_addr Address of the XMI pipe buffer
   * \param[in] pipe_size Size of the XMI pipe buffer
   *
   * \return Number of bytes processed (read or written) from the pipe address
   */
  typedef size_t (*xmi_data_function) ( xmi_context_t   context,
                                        void          * cookie,
                                        size_t          offset,
                                        void          * pipe_addr,
                                        size_t          pipe_size );

  /**
   * \brief Structure for send parameters unique to a direct active message send
   */
  typedef struct
  {
    xmi_send_t             send;     /**< Common send parameters */
    struct
    {
      xmi_data_function    data_fn;  /**< Data callback function */
      xmi_event_function   local_fn; /**< Local message completion event */
      xmi_event_function   remote_fn;/**< Remote message completion event ------ why is this needed ? */
    } direct;                        /**< Direct send parameters */
  } xmi_send_direct_t;

  /**
   * \brief Non-blocking active message send for direct data injection
   *
   * Source data to transfer is provided by the xmi client via a series of send
   * callbacks in which the xmi client will copy the source data directly into
   * the available network resources.
   *
   * The input parameters of the data callback will specify the output data
   * address and the maximum data size in bytes. As a convenience, the xmi
   * client may query the configuration attribute \c DIRECT_SEND_LIMIT to
   * obtain the maximum direct data size outside of the callback mechanism.
   *
   * Typically, the \c DIRECT_SEND_LIMIT is associated with a network
   * attribute, such as a packet size.
   *
   * \see xmi_data_function
   *
   * \param[in] context    XMI communication context
   * \param[in] parameters Send direct parameter structure
   */
  xmi_result_t XMI_Send_direct (xmi_context_t       context,
                                xmi_send_direct_t * parameters);




  /*****************************************************************************/
  /**
   * \defgroup mutex_cond XMI context mutext and condition
   *
   * Some brief documentation on active message stuff ...
   * \{
   */
  /*****************************************************************************/

  typedef void * xmi_cond_t;

  /**
   * \brief Get the owner of the context lock
   *
   * \param[in]  context XMI communication context
   * \param[out] owner   Owner of the context lock, compatible to pthread_t
   */
  xmi_result_t XMI_Mutex_getowner (xmi_context_t context, unsigned long *owner);

  /*
   * The following functions are modelled after pthread_cond_* functions.
   */
  /**
   * \brief Create a condition
   *
   * \param[in]  context XMI communication context
   * \param[out] cond    The condition created
   */
  xmi_result_t XMI_Cond_create (xmi_context_t context, xmi_cond_t *cond);

  /**
   * \brief Wait on a condition
   * \note  The caller must have the context lock. Upon return, the caller
   *        still has the lock.
   *
   * \param[in]  context XMI communication context
   * \param[in]  cond    The condition to wait
   */
  xmi_result_t XMI_Cond_wait (xmi_context_t context, xmi_cond_t cond);

  /**
   * \brief Wait on a condition with timeout
   * \note  The caller must have the context lock. Upon return, the caller
   *        still has the lock.
   *
   * \param[in]  context XMI communication context
   * \param[in]  cond    The condition to wait
   */
  xmi_result_t XMI_Cond_timedwait (xmi_context_t context, xmi_cond_t cond,
          struct timespec *time);

  /**
   * \brief Signal to wake up a waiter on a condition
   *
   * \param[in]  context XMI communication context
   * \param[in]  cond    The condition to signal
   */
  xmi_result_t XMI_Cond_signal (xmi_context_t context, xmi_cond_t cond);

  /**
   * \brief Signal to wake up all waiters on a condition
   *
   * \param[in]  context XMI communication context
   * \param[in]  cond    The condition to sigal
   */
  xmi_result_t XMI_Cond_broadcast (xmi_context_t context, xmi_cond_t cond);

  /**
   * \brief Destroy a condition
   *
   * \param[in]  context XMI communication context
   * \param[in]  cond    The condition to destroy
   */
  xmi_result_t XMI_Cond_destroy (xmi_context_t context, xmi_cond_t cond);

  /** \} */ /* end of "mutex_cond" group */
  /** \} */ /* end of "ext_percs" group */

#ifdef __cplusplus
}
#endif

#endif
