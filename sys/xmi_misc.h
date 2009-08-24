/**
 * \file xmi_misc.h
 * \brief messaging interface
 */
#ifndef __xmi_misc_h__
#define __xmi_misc_h__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "xmi_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define XMI_abort()		abort()
#define XMI_abortf(fmt...)	{ fprintf(stderr, __FILE__ ":%d: ", __LINE__); fprintf(stderr, fmt); abort(); }

#if ASSERT_LEVEL==0

    /* No asserts at all */
#define XMI_assert(expr)
#define XMI_assertf(expr, fmt...)
#define XMI_assert_debug(expr)
#define XMI_assert_debugf(expr, fmt...)

#elif ASSERT_LEVEL==1

    /* No debug asserts, only "normal" ones */
#define XMI_assert(expr)			assert(expr)
#define XMI_assertf(expr, fmt...)	{ if (!(expr)) XMI_abortf(fmt); }
#define XMI_assert_debug(expr)
#define XMI_assert_debugf(expr, fmt...)

#else /* ASSERT_LEVEL==2 */

    /* all asserts */
#define XMI_assert(expr)			assert(expr)
#define XMI_assertf(expr, fmt...)	{ if (!(expr)) XMI_abortf(fmt); }
#define XMI_assert_debug(expr)		XMI_assert(expr)
#define XMI_assert_debugf(expr, fmt...)	XMI_assertf(expr, fmt)

#endif /* ASSERT_LEVEL */


    typedef enum {
        DCMF_MATCH_CONSISTENCY,
        DCMF_RELAXED_CONSISTENCY,
        DCMF_SEQUENTIAL_CONSISTENCY,
        DCMF_WEAK_CONSISTENCY
    } DCMF_Consistency;
#warning defining deprecated DCMF_Consistency




    /*****************************************************************************/
    /**
     * \defgroup configuration xmi configuration interface
     *
     * Some brief documentation on configuration stuff ...
     * \{
     */
    /*****************************************************************************/

    typedef unsigned int  xmi_intr_mask_t;  /**< ??? */
    typedef unsigned int  xmi_bool_t;       /**< ??? */
    typedef char *        xmi_user_key_t;   /**< ??? */
    typedef char *        xmi_user_value_t; /**< ??? */

    typedef struct
    {
        xmi_user_key_t    key;   /**< The configuration key   */
        xmi_user_value_t  value; /**< The configuration value */
    } xmi_user_config_t;

    /**
     * This enum contains ALL possible attributes for all hardware
     */
    typedef enum {
        /* Attribute            Init / Query / Update                                                                              */
        XMI_PROTOCOL_NAME=1, /**< IQ  : char *            : name of the protocol                                                   */
        XMI_RELIABILITY,     /**< IQ  : xmi_bool_t        : guaranteed reliability                                                 */
        XMI_ATTRIBUTES,      /**<  Q  : xmi_attribute_t[] : attributes on the platform terminated with NULL                        */
        XMI_TASK_ID,         /**<  Q  : size_t            : ID of this task                                                        */
        XMI_NUM_TASKS,       /**<  Q  : size_t            : total number of tasks                                                  */
        XMI_RECV_INTR_MASK,  /**<  QU : xmi_intr_mask_t   : receive interrupt mask                                                 */
        XMI_CHECK_PARAMS,    /**<  QU : xmi_bool_t        : check parameters                                                       */
        XMI_USER_KEYS,       /**<  Q  : xmi_user_key_t[]  : user-defined keys terminated with NULL                                 */
        XMI_USER_CONFIG,     /**<  QU : xmi_user_config_t : user-defined configuration key and value are shallow-copied for update */
    } xmi_attribute_t;

#define XMI_EXT_ATTR 1000 /**< starting value for extended attributes */

    /**
     * \brief General purpose configuration structure.
     */
    typedef struct
    {
        xmi_attribute_t  attr;  /**< Attribute type */
        void *           value; /**< Opaque pointer to attribute value. */
    } xmi_configuration_t;


    /**
     * \brief Query the value of an attribute
     *
     * \param [in]  context    The XMI context
     * \param [in]  attribute  The attribute of interest
     * \param [out] value      Pointer to the retrieved value
     *
     * \note
     * \returns
     *   XMI_SUCCESS
     *   XMI_ERR_CONTEXT
     *   XMI_ERR_ATTRIBUTE
     *   XMI_ERR_VALUE
     */
    xmi_result_t XMI_Configuration_query (xmi_context_t     context,
                                          xmi_attribute_t   attribute,
                                          void            * value);

    /**
     * \brief Update the value of an attribute
     *
     * \param [in]  context    The XMI context
     * \param [in]  attribute  The attribute of interest
     * \param [in]  value      Pointer to the new value
     *
     * \note
     * \returns
     *   XMI_SUCCESS
     *   XMI_ERR_CONTEXT
     *   XMI_ERR_ATTRIBUTE
     *   XMI_ERR_VALUE
     */
    xmi_result_t XMI_Configuration_update (xmi_context_t     context,
                                           xmi_attribute_t   attribute,
                                           void            * value);

    /**
     * \brief Provides the detailed description of the most recent xmi result.
     *
     * The "most recent xmi result" is specific to each thread. 
     *
     * \note  XMI implementations may provide translated (i18n) text.
     *
     * \param[in] string Character array to write the descriptive text
     * \param[in] length Length of the character array
     *
     * \return Number of characters written into the array
     */
    size_t XMI_Error_text (char * string, size_t length);


    /** \} */ /* end of "configuration" group */


    /*****************************************************************************/
    /**
     * \defgroup Time Timer functions required by MPI
     *
     * \{
     */
    /*****************************************************************************/

    /**
     * \brief  Returns an elapsed time on the calling processor.
     * \note   This has the same definition as MPI_Wtime
     * \return Time in seconds since an arbitrary time in the past.
     */
    double XMI_Wtime();

    /**
     * \brief  Computes the smallest clock resolution theoretically possible
     * \note   This has the same definition as MPI_Wtick
     * \return The duration of a single timebase clock increment in seconds
     */
    double XMI_Wtick();

    /**
     * \brief  Returns the number of "cycles" elapsed on the calling processor.
     * \return Number of "cycles" since an arbitrary time in the past.
     *
     * "Cycles" could be any quickly and continuously increasing counter
     * if true cycles are unavailable.
     */
    unsigned long long XMI_Wtimebase();

    /** \} */ /* end of "Time" group */


    /*****************************************************************************/
    /**
     * \defgroup context xmi multi-context messaging interface
     *
     * Some brief documentation on context stuff ...
     * \{
     */
    /*****************************************************************************/

    /**
     * \brief Initialize the XMI runtime for a client program
     *
     * A client program is any program that invokes any XMI function. This
     * includes applications, libraries, and other middleware. Some example
     * clienti names may include: "MPI", "UPC", "OpenSHMEM", and "ARMCI"
     *
     * A communication context must be created before any data transfer functions
     * may be invoked.
     *
     * \param[in]  name   XMI client unique name
     * \param[out] client XMI client handle
     */
    xmi_result_t XMI_Client_initialize (char         * name,
                                        xmi_client_t * client);

    /**
     * \brief Finalize the XMI runtime for a client program
     *
     * \warning It is \b illegal to invoke any XMI functions using the client
     *          handle from any thread after the finalize function.
     *
     * \param[in] client XMI client handle
     */
    xmi_result_t XMI_Client_finalize (xmi_client_t client);


    /**
     * \brief Create a new independent communication context for a client
     *
     * The context configuration attributes may include:
     * - Unique context name for this application, which is platform specific
     * - Context optimizations, such as shared memory, collective acceleration, etc
     *
     * Context creation is a local operation and does not involve communication or
     * syncronization with other tasks.
     *
     * \param[in]  client        Client handle
     * \param[in]  configuration List of configurable attributes and values
     * \param[in]  count         Number of configurations, may be zero
     * \param[out] context       XMI communication context
     */
    xmi_result_t XMI_Context_create (xmi_client_t           client,
                                     xmi_configuration_t    configuration[],
                                     size_t                 count,
                                     xmi_context_t        * context);

    /**
     * \brief Destroy an independent communication context
     *
     * \warning It is \b illegal to invoke any XMI functions using the
     *          communication context from any thread after the context is
     *          destroyed.
     *
     * \param[in] context XMI communication context
     */
    xmi_result_t XMI_Context_destroy (xmi_context_t context);

    /**
     * \brief Atomically post work to a context
     *
     * It is \b not required that the target context is locked, or otherwise
     * reserved, by an external atomic operation to ensure thread safety. The XMI
     * runtime will internally perform an atomic operation in order to post the
     * work to the context.
     *
     * The callback function will be invoked in the thread that advances the
     * communication context. There is no implicit completion notification provided
     * to the \em posting thread when the xmi client returns from the callback
     * function.  If the xmi client desires a completion notification in the
     * posting thread it must explicitly program such notifications, via the
     * XMI_Context_post() interface, from the target thread back to the origin
     * thread
     *
     * \note This is similar to the various DCMF_Handoff() interface(s)
     *
     * \param[in] context XMI communication context
     * \param[in] work_fn Event callback function to post to the context
     * \param[in] cookie  Opaque data pointer to pass to the event function
     */
    xmi_result_t XMI_Context_post (xmi_context_t        context,
                                   xmi_event_function   work_fn,
                                   void               * cookie);


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
     * \see XMI_Context_lock()
     * \see XMI_Context_trylock()
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
     * \note It is possible to define a set of communication contexts that are
     *       always advanced together by any xmi client thread.  It is the
     *       responsibility of the xmi client to atomically lock the context set,
     *       perhaps by using the XMI_Context_lock() function on a designated
     *       \em leader context, and to manage the xmi client threads to ensure
     *       that only one thread ever advances the set of contexts.
     *
     * \todo Define return code, event bitmask ?
     * \todo Rename function to something better
     *
     * \see XMI_Context_lock()
     * \see XMI_Context_trylock()
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

    /** \} */ /* end of "context" group */

#ifdef __cplusplus
};
#endif

#endif // __xmi_misc_h__
