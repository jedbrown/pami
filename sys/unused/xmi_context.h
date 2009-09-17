/**
 * \file sys/unused/xmi_context.h
 * \brief xmi multi-context messaging interface
 * \example config/xmi_config_test.c
 * \defgroup context Application and communication context and progress
 * \{
 */
#ifndef __xmi_context_h__
#define __xmi_context_h__

#include "xmi.h"
#include "xmi_config.h"

/**
 * \brief Initialize the XMI runtime for an application
 *
 * A communication context must be created before any data transfer functions
 * may be invoked.
 *
 * \param[in]  name        Application, or middleware, unique name
 * \param[out] application Application realm
 */
xmi_result_t XMI_Application_initialize (char              * name,
                                         xmi_application_t * application);

/**
 * \brief Finalize the XMI runtime for an application
 *
 * \warning It is \b illegal to invoke any XMI functions using the application
 *          realm after the finalize function.
 *
 * \param[in] application Application realm
 */
xmi_result_t XMI_Application_finalize (xmi_application_t application);


/**
 * \brief Create a new independent communication context for an application
 *
 * The context configuration attributes may include:
 * - Unique context name for this application, which is platform specific
 * - Context optimizations, such as shared memory, collective acceleration, etc
 *
 * Context creation is a local operation and does not involve communication or
 * syncronization with other tasks.
 *
 * \param[in]  application   Application realm
 * \param[in]  configuration List of configurable attributes and values
 * \param[in]  count         Number of configurations, may be zero
 * \param[out] context       XMI communication context
 */
xmi_result_t XMI_Context_create (xmi_application_t    application,
                                 xmi_config_t         configuration[],
                                 size_t               count,
                                 xmi_context_t      * context);

/**
 * \brief Destroy an independent communication context
 *
 * \warning It is \b illegal to invoke any XMI functions using the
 *          communication context after the context is destroyed.
 *
 * \param[in] context XMI communication context
 */
xmi_result_t XMI_Context_destroy (xmi_context_t context);

/**
 * \brief Atomically post work to a context
 *
 * It is \b not required that the target context is locked, or otherwise
 * reserved, by an external atomic operation to ensure thread safety. The XMI
 * runtime will internally perform and atomic operation in order to post the
 * work to the context.
 *
 * The callback function will be invoked in the thread that advances the
 * communication context. There is no implicit completion notification provided
 * to the \em posting thread when the application returns from the callback
 * function.  If the application desires a completion notification in the
 * posting thread it must explicitly program such notifications, via the
 * XMI_Context_post() interface, from the target thread back to the origin
 * thread
 *
 * \note This is similar to the various DCMF_Handoff() interface(s)
 *
 * \param[in] context XMI communication context
 * \param[in] work    Event callback to post to the context
 */
xmi_result_t XMI_Context_post (xmi_context_t        context,
                               xmi_event_callback_t work);


/**
 * \brief Advance the progress engine for a single communication context
 *
 * May complete zero, one, or more outbound transfers. May invoke dispatch
 * handlers for incoming transfers. May invoke work event callbacks previously
 * posted to the communication context.
 *
 * This polling advance function will return after the first poll iteration
 * that results in a processed event or if, no events are processed, after
 * polling for the maximum number of iterations.
 *
 * \warning This function is \b not \b threadsafe and the application must
 *          ensure that only one thread advances a context at any time.
 *
 * \todo Define return code, event bitmask ?
 *
 * \param[in] context XMI communication context
 * \param[in] maximum Maximum number of internal poll iterations
 */
xmi_result_t XMI_Context_advance (xmi_context_t context, size_t maximum);

/**
 * \brief Advance the progress engine for multiple communication contexts
 *
 * May complete zero, one, or more outbound transfers. May invoke dispatch
 * handlers for incoming transfers. May invoke work event callbacks previously
 * posted to a communication context.
 *
 * This polling advance function will return after the first poll iteration
 * that results in a processed event on any context, or if, no events are
 * processed, after polling for the maximum number of iterations.
 *
 * \warning This function is \b not \b threadsafe and the application must
 *          ensure that only one thread advances the contexts at any time.
 *
 * \todo Define return code, event bitmask ?
 *
 * \todo Rename function to something better
 *
 * \param[in] context Array of XMI communication contexts
 * \param[in] count   Number of communication contexts
 * \param[in] maximum Maximum number of internal poll iterations on each context
 */
xmi_result_t XMI_Context_multiadvance (xmi_context_t context[],
                                       size_t        count,
                                       size_t        maximum);

/**
 * \brief Acquire an atomic lock on a communication context
 *
 * \warning This function will block until the lock is aquired.
 *
 * \param[in] context XMI communication context
 */
xmi_result_t XMI_Context_lock (xmi_context_t context);

/**
 * \brief Attempt to acquire an atomic lock on a communication context
 *
 * May return \c EAGAIN if the lock was not acquired.
 *
 * \param[in] context XMI communication context
 */
xmi_result_t XMI_Context_trylock (xmi_context_t context);

/**
 * \brief Release an atomic lock on a communication context
 *
 * \param[in] context XMI communication context
 */
xmi_result_t XMI_Context_unlock (xmi_context_t context);

/**
 * \}
 * \addtogroup context
 *
 * More documentation for context stuff....
 */

#endif /* __xmi_context_h__ */
