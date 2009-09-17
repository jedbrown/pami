  /**
 * \file sys/xmi_fence.h
 * \brief fence interface
 */
#ifndef __xmi_fence_h__
#define __xmi_fence_h__

#include <stdlib.h>
#include <stdint.h>
#include "xmi_types.h"
#include "xmi_p2p.h"

#ifdef __cplusplus
extern "C"
{
#endif
  /*****************************************************************************/
  /**
   * \defgroup sync xmi memory synchronization and data fence interface
   *
   * Some brief documentation on sync stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Begin a memory synchronization region
   *
   * A fence region is defined as an area of program control on the local task
   * bounded by the XMI_Fence_begin() and XMI_Fence_end() functions.
   *
   * \warning It is considered \b illegal to invoke a fence operation outside of
   *          a fence region.
   *
   * \warning It is considered \b illegal to begin a fence region inside an
   *          existing fence region. Fence regions can not be nested.
   *
   * \param[in] context XMI communication context
   */
  xmi_result_t XMI_Fence_begin (xmi_context_t context);

  /**
   * \brief End a memory synchronization region
   *
   * A fence region is defined as an area of program control on the local task
   * bounded by the XMI_Fence_begin() and XMI_Fence_end() functions.
   *
   * \warning It is considered \b illegal to invoke a fence operation outside of
   *          a fence region.
   *
   * \warning It is considered \b illegal to end a fence region outside of an
   *          existing fence region.
   *
   * \param[in] context XMI communication context
   */
  xmi_result_t XMI_Fence_end (xmi_context_t context);


  /**
   * \brief Syncronize all transfers between all tasks.
   *
   * \param[in] context XMI communication context
   * \param[in] done_fn Event callback to invoke when the fence is complete
   * \param[in] cookie  Event callback argument
   */
  xmi_result_t XMI_Fence_all (xmi_context_t        context,
                              xmi_event_function   done_fn,
                              void               * cookie);

  /**
   * \brief Syncronize all transfers between two tasks.
   *
   * \param[in] context XMI communication context
   * \param[in] done_fn Event callback to invoke when the fence is complete
   * \param[in] cookie  Event callback argument
   * \param[in] task    Remote task to synchronize
   */
  xmi_result_t XMI_Fence_task (xmi_context_t        context,
                               xmi_event_function   done_fn,
                               void               * cookie,
                               size_t               task);

  /** \} */ /* end of "sync" group */
#ifdef __cplusplus
};
#endif

#endif /* __xmi_fence_h__ */
