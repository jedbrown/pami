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
     The following functions are modelled after pthread_cond_* functions.
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

#ifdef __cplusplus
}
#endif

#endif
