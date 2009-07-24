/**
 * \file xmi_context.h
 * \brief xmi multi-context messaging interface
 */
#ifndef __xmi_context_h__
#define __xmi_context_h__

/**
 * \brief Initialize the XMI runtime
 *
 * A communication context must be created before any data transfer functions
 * may be invoked.
 */
xmi_result_t XMI_Initialize ();

/**
 * \brief Finalize the XMI runtime
 *
 * It is \b illegal to invoke any XMI functions after the finalize function.
 */
xmi_result_t XMI_Finalize ();


/** \brief XMI context "magic cookie" data type. */
typedef uint64_t xmi_cookie_t[2];

/**
 * \brief Create a new independent communication context
 *
 * The magic cookie should uniquely identify the application context so that a
 * context created on two different tasks with the same magic cookie will be
 * able to communicate.
 *
 * Example magic cookie values may include:
 *
 *   4d504943 48325f31 2e315f5f 00000001 => "MPICH2_1.1__" context 1
 *   4d504943 48325f31 2e315f5f 00000002 => "MPICH2_1.1__" context 2
 *   41524d43 495f342e 325f5f5f 00000001 => "ARMCI_4.2___" context 1
 *
 * \todo Is this a collective or some kind of synchronous operation? or is it
 *       strictly local?
 *
 * \param[in]  magic_cookie  Application specified unique context identifier
 * \param[in]  hints         Hints to create the context
 * \param[out] context       XMI communication context
 */
xmi_result_t XMI_Context_create (xmi_cookie_t         magic_cookie,
                                 xmi_context_hint_t   hints,
                                 xmi_context_t      * context);

/**
 * \brief Destroy an independent communication context
 *
 * \param[in] context XMI communication context
 */
xmi_result_t XMI_Context_destroy (xmi_context_t context);

/**
 * \brief Atomically post work to a context
 *
 * It is \b not required that the target context is locked, or otherwise
 * reserved, by an atomic operation.
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
 * handlers for incoming transfers.
 *
 * This polling advance function will return after the first poll iteration
 * that results in a processed event or if, no events are processed, after
 * polling for the maximum number of iterations.
 *
 * If the maximum poll count is zero the advance function will block and return
 * only after an event is processed. This allows platform-specific
 * optimizations to sleep, or unschedule, the thread until an event occurs.
 * Example events include packet arrival and a context post from another
 * thread.
 *
 * \warning This function is \b not \b threadsafe and the application must
 *          ensure that only one thread advances a context at any time.
 *
 * \todo Define return code, event bitmask ?
 *
 * \param[in] context XMI communication context
 * \param[in] maximum Maximum number of internal poll iterations
 */
xmi_result_t XMI_Advance (xmi_context_t context, size_t maximum);

/**
 * \brief Advance the progress engine for multiple communication contexts
 *
 * May complete zero, one, or more outbound transfers. May invoke dispatch
 * handlers for incoming transfers.
 *
 * This polling advance function will return after the first poll iteration
 * that results in a processed event or if, no events are processed, after
 * polling for the maximum number of iterations.
 *
 * If the maximum poll count is zero the advance function will block and return
 * only after an event is processed. This allows platform-specific
 * optimizations to sleep, or unschedule, the thread until an event occurs.
 * Example events include packet arrival and a context post from another
 * thread.
 *
 * \warning This function is \b not \b threadsafe and the application must
 *          ensure that only one thread advances the contexts at any time.
 *
 * \todo Define return code, event bitmask ?
 *
 * \param[in] context Array of XMI communication contexts
 * \param[in] count   Number of communication contexts
 * \param[in] maximum Maximum number of internal poll iterations
 */
xmi_result_t XMI_Advance_multiple (xmi_context_t * context,
                                   size_t          count,
                                   size_t          maximum);



/** \brief XMI lock data type */
typedef xmi_lock_t size_t;

/**
 * \brief Create an atomic lock.
 *
 * \note Does this need to be unique to a context? Can we drop the context
 *       parameter?
 *
 * \note Does the lock need to be an opaque object, or can it be a handle
 *       to some internal structure? Which is faster?
 *
 * \param[in]  context XMI communication context
 * \param[out] lock    XMI lock
 */
xmi_result_t XMI_Lock_create (xmi_context_t   context,
                              xmi_lock_t    * lock);

/**
 * \brief Attempt to acquire the XMI lock
 *
 * May return \c EAGAIN if the lock was not acquired.
 *
 * \param[in] lock XMI lock
 */
xmi_result_t XMI_Lock_try (xmi_lock_t lock);

/**
 * \brief Acquire the XMI lock
 *
 * Block until the lock is aquired.
 *
 * \param[in] lock XMI lock
 */
xmi_result_t XMI_Lock_acquire (xmi_lock_t lock);

/**
 * \brief Release the XMI lock
 *
 * \warning It is illegal to release a lock that has not been previously
 *          acquired.
 *
 * \param[in] lock XMI lock
 */
xmi_result_t XMI_Lock_release (xmi_lock_t lock);

/**
 * \brief Destroy the XMI lock
 *
 * \warning It is illegal to destroy a lock that has not been created or is
 *          currently acquired..
 *
 * \param[in] lock XMI lock
 */
xmi_result_t XMI_Lock_destroy (xmi_lock_t lock);


#endif /* __xmi_context_h__ */

