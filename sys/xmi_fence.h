/**
 * \file xmi_fence.h
 * \brief XMI memory synchronization interface
 */
#ifndef __xmi_fence_h__
#define __xmi_fence_h__

#include "xmi.h"

/**
 * \defgroup sync Memory synchronization interfaces
 *
 * Documentation for fence stuff ...
 */

/**
 * \brief Begin a memory synchronization region
 *
 * \param[in] context XMI application context
 *
 * \ingroup sync
 */
xmi_result_t XMI_Fence_begin (xmi_context_t context);

/**
 * \brief End a memory synchronization region
 *
 * \param[in] context XMI application context
 *
 * \ingroup sync
 */
xmi_result_t XMI_Fence_end (xmi_context_t context);


/**
 * \brief Syncronize all transfers between all tasks.
 *
 * from the local task to the remote taskBegin a memory synchronization region
 *
 * \param[in] context    XMI application context
 * \param[in] fence_done Event callback to invoke when the fence is complete
 * \param[in] cookie     Event callback argument
 *
 * \ingroup sync
 */
xmi_result_t XMI_Fence_all (xmi_context_t          context,
                            xmi_event_callback_t   fence_done,
                            void                 * cookie);

/**
 * \brief Syncronize all transfers between two tasks.
 *
 * from the local task to the remote taskBegin a memory synchronization region
 *
 * \param[in] context    XMI application context
 * \param[in] fence_done Event callback to invoke when the fence is complete
 * \param[in] cookie     Event callback argument
 * \param[in] task       Remote task to synchronize
 *
 * \ingroup sync
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
 * \param[in] context    XMI application context
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
 * \param[in] context    XMI application context
 * \param[in] dest       Array of destinations to close connections to
 * \param[in] count      Number of destinations in the array dest
 *
 * \ingroup dynamictasks
 */
xmi_result_t XMI_Resume_totask(xmi_context_t context, size_t *dest, size_t count);

#endif
