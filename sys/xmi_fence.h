/**
 * \file xmi_fence.h
 * \brief XMI memory synchronization interface
 * \defgroup sync Memory synchronization and data fence interface
 * \{
 */
#ifndef __xmi_fence_h__
#define __xmi_fence_h__

#include "xmi.h"

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
 * \param[in] context    XMI communication context
 * \param[in] fence_done Event callback to invoke when the fence is complete
 * \param[in] cookie     Event callback argument
 */
xmi_result_t XMI_Fence_all (xmi_context_t          context,
                            xmi_event_callback_t   fence_done,
                            void                 * cookie);

/**
 * \brief Syncronize all transfers between two tasks.
 *
 * \param[in] context    XMI communication context
 * \param[in] fence_done Event callback to invoke when the fence is complete
 * \param[in] cookie     Event callback argument
 * \param[in] task       Remote task to synchronize
 */
xmi_result_t XMI_Fence_task (xmi_context_t          context,
                             xmi_event_callback_t   fence_done,
                             void                 * cookie,
                             size_t                 task);

/**
 * \brief Clean up local resources to a task in preparation for
 *        task shutdown or checkpoint
 *
 *        It is the user of this API's responsibility to ensure
 *        that all communication has been quiesced to and from
 *        the destination via a fence call and synchronization
 *
 *
 * \param[in] context    XMI communication context
 * \param[in] dest       Array of destinations to close connections to
 * \param[in] count      Number of destinations in the array dest
 *
 * \ingroup dynamictasks
 */

xmi_result_t XMI_Purge_totask(xmi_context_t context, size_t *dest, size_t count);

/**
 * \brief Setup local resources to a task in preparation for
 *        task restart or creation
 *
 *
 * \param[in] context    XMI communication context
 * \param[in] dest       Array of destinations to close connections to
 * \param[in] count      Number of destinations in the array dest
 *
 * \ingroup dynamictasks
 */
xmi_result_t XMI_Resume_totask(xmi_context_t context, size_t *dest, size_t count);

/**
 * \}
 * \addtogroup sync
 *
 * More documentation for memory synchronization stuff....
 */

#endif
