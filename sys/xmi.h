/**
 * \file xmi.h
 * \brief messaging interface
 * \todo API style - all lowercase types? mixed-case? all uppercase?
 * \todo API style - abbreviated or explicit method/type/variable names ?
 * \todo API style - use MPI convention to name function pointer typedefs
 *       and function pointer parameters. see -> https://svn.mpi-forum.org/trac/mpi-forum-web/ticket/7
 * \todo API style - define parameter list convention. Most common parameters first?
 */
#ifndef __xmi_h__
#define __xmi_h__

#include <stdlib.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C"
{
#endif


  typedef enum
  {
    XMI_SUCCESS = 0,  /**< Successful execution        */
    XMI_NERROR  = -1, /**< Generic error (-1)          */
    XMI_ERROR   = 1,  /**< Generic error (+1)          */
    XMI_INVAL,        /**< Invalid argument            */
    XMI_UNIMPL,       /**< Function is not implemented */
    XMI_EAGAIN,       /**< Not currently availible     */
    XMI_SHUTDOWN,     /**< Rank has shutdown           */
    XMI_CHECK_ERRNO,  /**< Check the errno val         */
    XMI_OTHER,        /**< Other undefined error       */
  }
  xmi_result_t;

  typedef void* xmi_client_t;
  typedef void* xmi_context_t;
  typedef void* xmi_hint_t;
  typedef void* xmi_dispatch_t;
  typedef void* xmi_type_t;


  /**
   * \brief Callback to handle message events
   *
   * \todo Move this to a common header file
   *
   * \param[in] context   XMI communication context that invoked the callback
   * \param[in] cookie    Event callback application argument
   * \param[in] result    Asynchronous result information (was error information)
   */
  typedef void (*xmi_event_callback_t) ( xmi_context_t   context,
                                         void          * cookie,
                                         xmi_result_t    result );


  /*****************************************************************************/
  /**
   * \defgroup configuration xmi configuration interface
   *
   * Some brief documentation on configuration stuff ...
   * \{
   */
  /*****************************************************************************/

  typedef unsigned int  xmi_intr_mask_t;
  typedef unsigned int  xmi_bool_t;

  typedef char *        xmi_user_key_t;
  typedef char *        xmi_user_value_t;

  typedef struct
  {
    xmi_user_key_t    key;
    xmi_user_value_t  value;
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
#include "xmi_ext.h"      /**< platform-specific */

  typedef struct
  {
    xmi_attribute_t  attr;
    void *           value;
  } xmi_configuration_t;


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

  /** \} */ /* end of "configuration" group */



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
   * \param[out] owner   Owner of the context lock
   */
  xmi_result_t XMI_Mutex_getowner (xmi_context_t context, pthread_t *owner);
  
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

  /*****************************************************************************/
  /**
   * \defgroup datatype xmi non-contiguous datatype interface
   *
   * Some brief documentation on active message stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Create a new type for noncontiguous transfers
   *
   * \todo provide example code
   *
   * \param[out] type Type identifier to be created
   */
  xmi_result_t XMI_Type_create (xmi_type_t * type);

  /**
   * \brief Append a simple contiguous buffer to an existing type identifier
   *
   * \todo doxygen for offset parameter
   * \todo provide example code
   *
   * \param[in,out] type   Type identifier to be modified
   * \param[in]     bytes  Number of contiguous bytes to append
   * \param[in]     offset Offset from the end of the type to place the buffer
   * \param[in]     count  Number of buffers
   * \param[in]     stride Data stride
   */
  xmi_result_t XMI_Type_add_simple (xmi_type_t type,
                                    size_t     bytes,
                                    size_t     offset,
                                    size_t     count,
                                    size_t     stride);

  /**
   * \brief Append a typed buffer to an existing type identifier
   *
   * \todo doxygen for offset parameter
   * \todo provide example code
   *
   * \warning It is considered \b illegal to append an imcomplete type to
   *          another type.
   *
   * \param[in,out] type    Type identifier to be modified
   * \param[in]     subtype Subtype to append
   * \param[in]     offset  Offset from the end of the type to place the buffer
   * \param[in]     count   Number of buffers
   * \param[in]     stride  Data stride
   */
  xmi_result_t XMI_Type_add_typed (xmi_type_t type,
                                   xmi_type_t subtype,
                                   size_t     offset,
                                   size_t     count,
                                   size_t     stride);

  /**
   * \brief Complete the type identifier
   *
   * \warning It is considered \b illegal to modify a type identifier after it
   *          has been completed.
   *
   * \param[in] type Type identifier to be completed
   */
  xmi_result_t XMI_Type_complete (xmi_type_t type);

  /**
   * \brief Get the byte size of a completed type
   *
   * \param[in] type Type identifier to get size from
   */
  xmi_result_t XMI_Type_sizeof(xmi_type_t type);

  /**
   * \brief Destroy the type
   *
   * Q. What if some in-flight messages are still using it?  What if some
   * other types have references to it?
   *
   * A. Maintain an internal reference count and release internal type
   * resources when the count hits zero.
   *
   * \param[in] type Type identifier to be destroyed
   */
  xmi_result_t XMI_Type_destroy (xmi_type_t type);

  /** \} */ /* end of "datatype" group */



  /*****************************************************************************/
  /**
   * \defgroup activemessage xmi active messaging interface
   *
   * Some brief documentation on active message stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Hints for sending a message
   *
   * \todo better names for the hints
   * \todo better documentation for the hints
   */
  typedef struct
  {
    uint32_t consistency       : 1; /**< Force match ordering semantics                          */
    uint32_t sync_send         : 1; /**< Assert that all sends will be synchronously received    */
    uint32_t buffer_registered : 1; /**< ???                                                     */
    uint32_t use_rdma          : 1; /**< Assert/enable rdma operations                           */
    uint32_t no_rdma           : 1; /**< Disable rdma operations                                 */
    uint32_t no_local_copy     : 1; /**< ???                                                     */
    uint32_t interrupt_on_recv : 1; /**< Interrupt the remote task when the first packet arrives */
    uint32_t high_priority     : 1; /**< Message is delivered with high priority,
                                         which may result in out-of-order delivery               */
    uint32_t reserved          :24; /**< Unused at this time                                     */
  } xmi_send_hint_t;


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
  typedef size_t (*xmi_data_callback_t) ( xmi_context_t   context,
                                          void          * cookie,
                                          size_t          offset,
                                          void          * pipe_addr,
                                          size_t          pipe_size );

  typedef enum {
    XMI_AM_KIND_SIMPLE = 0,
    XMI_AM_KIND_DIRECT,
    XMI_AM_KIND_TYPED,
    XMI_AM_KIND_COUNT
  } xmi_am_kind_t;

  /**
   * \brief Structure for send parameters unique to a simple active message send
   */
  typedef struct
  {
    size_t                 bytes;    /**< Number of bytes of data */
    void                 * addr;     /**< Address of the buffer */
  } xmi_send_simple_t;

  /**
   * \brief Structure for send parameters unique to a direct active message send
   */
  typedef struct
  {
    xmi_data_callback_t    callback; /**< Data callback function */
  } xmi_send_direct_t;

  /**
   * \brief Structure for send parameters unique to a typed active message send
   */
  typedef struct
  {
    size_t                 bytes;    /**< Number of bytes of data */
    void                 * addr;     /**< Starting address of the buffer */
    size_t                 offset;   /**< Starting offset */
    xmi_type_t             type;     /**< Datatype */
  } xmi_send_typed_t;

  /**
   * \brief Active message send parameter structure
   */
  typedef struct
  {
    xmi_dispatch_t         dispatch; /**< Dispatch identifier */
    xmi_send_hint_t        hints;    /**< Hints for sending the message */
    size_t                 task;     /**< Destination task */
    void                 * cookie;   /**< Argument to \b all event callbacks */
    xmi_event_callback_t   local;    /**< Local message completion event */
    xmi_event_callback_t   remote;   /**< Remote message completion event ------ why is this needed ? */
    struct
    {
      size_t               bytes;    /**< Header buffer size in bytes */
      void               * addr;     /**< Header buffer address */
    } header;                        /**< Send message metadata header */
    union
    {
      xmi_send_simple_t    simple;   /**< Required, and only valid for, XMI_Send() and XMI_Send_immediate() */
      xmi_send_direct_t    direct;   /**< Required, and only valid for, XMI_Send_direct() */
      xmi_send_typed_t     typed;    /**< Required, and only valid for, XMI_Send_typed() */
    } data;                          /**< Send message source data */
  } xmi_send_t;

  /**
   * \brief Non-blocking active message send for contiguous data
   *
   * \param[in] context    XMI communication context
   * \param[in] parameters Send parameter structure
   */
  xmi_result_t XMI_Send (xmi_context_t context, xmi_send_t * parameters);

  /**
   * \brief Immediate active message send for small contiguous data
   *
   * The blocking send is only valid for small data buffers. The implementation
   * configuration attribute \code IMMEDIATE_SEND_LIMIT defines the upper
   * bounds for the size of data buffers, including header data, that can be
   * sent with this function. This function will return an error if a data
   * buffer larger than the \code IMMEDIATE_SEND_LIMIT is attempted.
   *
   * This function provides a low-latency send that can be optimized by the
   * specific xmi implementation. If network resources are immediately
   * available the send data will be injected directly into the network. If
   * resources are not available the specific xmi implementation may internally
   * buffer the send parameters and data until network resource are available
   * to complete the transfer. In either case the send will immediately return,
   * no doce callback is invoked, and is considered complete.
   *
   * The low-latency send operation may be further enhanced by using a
   * specially configured dispatch id which asserts that all dispatch receive
   * callbacks will not exceed a certain limit. The implementation
   * configuration attribute \code SYNC_SEND_LIMIT defines the upper bounds for
   * the size of data buffers that can be completely received with a single
   * dispatch callback. Typically this limit is associated with a network
   * resource attribute, such as a packet size.
   * 
   * \see xmi_send_hint_t::sync_send
   * \see XMI_Configuration_query()
   * 
   * \todo Better define send parameter structure so done callback is not required
   * \todo Define configuration attribute for the size limit
   *
   * \param[in] context    XMI communication context
   * \param[in] parameters Send parameter structure
   */
  xmi_result_t XMI_Send_immediate (xmi_context_t context, xmi_send_t * parameters);

  /**
   * \brief Non-blocking active message send for non-contiguous typed data
   *
   * Transfers data according to a predefined data memory layout, or type, to
   * the remote task.
   *
   * Conceptually, the data is transfered as a byte stream which may be
   * received by the remote task into a different format, such as a contiguous
   * buffer or the same or different predefined type.
   *
   * \param[in] context    XMI communication context
   * \param[in] parameters Send parameter structure
   */
  xmi_result_t XMI_Send_typed (xmi_context_t context, xmi_send_t * parameters);

  /**
   * \brief Non-blocking active message send for direct data injection
   *
   * Source data to transfer is provided by the xmi client via a series of send
   * callbacks in which the xmi client will copy the source data directly into
   * the available network resources. 
   *
   * The input parameters of the data callback will specify the output data
   * address and the maximum data size in bytes. As a convenience, the xmi
   * client may query the configuration attribute \code DIRECT_SEND_LIMIT to
   * obtain the maximum direct data size outside of the callback mechanism.
   *
   * Typically, the \code DIRECT_SEND_LIMIT is associated with a network
   * attribute, such as a packet size. 
   *
   * \see xmi_data_callback_t 
   *
   * \param[in] context    XMI communication context
   * \param[in] parameters Send parameter structure
   */
  xmi_result_t XMI_Send_direct (xmi_context_t context, xmi_send_t * parameters);

  /**
   * \brief Active message receive hints
   */
  typedef struct
  {
    uint32_t inline_completion :  1; /**< The receive completion callback
                                      *   \b must be invoked by the thread that
                                      *   receives the dispatch notification. */
    uint32_t reserved          : 30;
  } xmi_recv_hint_t;

  /**
   * \brief Receive message structure
   *
   * This structure is initialized and then returned as an output parameter from
   * the active message dispatch callback to direct the xmi runtime how to
   * receive the data stream.
   *
   * \see xmi_dispatch_callback_t
   */
  typedef struct
  {
    xmi_recv_hint_t        hints;    /**< Hints for receiving the message */
    void                 * cookie;   /**< Argument to \b all event callbacks */
    xmi_event_callback_t   local;    /**< Local message completion event */
    xmi_am_kind_t          kind;     /**< Which kind receive is to be done */
    union
    {
      xmi_send_simple_t    simple;   /**< Receive into a simple contiguous buffer */
      xmi_send_direct_t    direct;   /**< Receive via explicit data callbacks */
      xmi_send_typed_t     typed;    /**< Receive into a non-contiguous buffer */
    } data;                          /**< Receive message destination data */
  } xmi_recv_t;

  /**
   * \brief Dispatch callback
   */
  typedef void (*xmi_dispatch_callback_t) (
    xmi_context_t        context,      /**< IN: XMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    size_t               task,         /**< IN: source task */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of XMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of XMI pipe buffer */
    xmi_recv_t         * recv);        /**< OUT: receive message structure */

  /** \} */ /* end of "active message" group */



  /*****************************************************************************/
  /**
   * \defgroup dispatch xmi dispatch interface
   *
   * Some brief documentation on dispatch stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Initialize the dispatch functions for a dispatch id.
   *
   * This is a local, non-collective operation. There is no communication
   * between tasks.
   *
   * \param[in] context    XMI communication context
   * \param[in] dispatch   Dispatch identifier to initialize
   * \param[in] fn         Dispatch receive function
   * \param[in] clientdata Dispatch function clientdata
   * \param[in] options    Dispatch registration options
   *
   */
  xmi_result_t XMI_Dispatch_set (xmi_context_t           * context,
                                 xmi_dispatch_t            dispatch,
                                 xmi_dispatch_callback_t   fn,
                                 void                    * cookie,
                                 xmi_send_hint_t           options);

  /** \} */ /* end of "dispatch" group */



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

  /** \} */ /* end of "sync" group */



  /*****************************************************************************/
  /**
   * \defgroup rma xmi remote memory access data transfer interface
   *
   * Some brief documentation on rma stuff ...
   * \{
   */
  /*****************************************************************************/

  typedef void * xmi_memregion_t;

  /**
   * \brief Default "global" memory region
   *
   * The global memory region may be used in the one-sided operations to make use
   * of the system-managed memory region support.
   *
   * User-managed memory regions may result in higher performance for one-sided
   * operations due to system memory region caching, internal memory region
   * exchange operations, and other implementation-specific management features.
   */
  extern xmi_memregion_t XMI_MEMREGION_GLOBAL;

  /**
   * \brief Register a local memory region for one sided operations
   *
   * The local memregion may be transfered, via a send message, to a remote task
   * to allow the remote task to perform one-sided operations with this local
   * task
   *
   * \todo Define, exactly, what the sematics are if a NULL pointer is passed
   *       as the memregion to initialize.
    *
   * \param[in]  context   XMI application context
   * \param[in]  address   Virtual address of memory region
   * \param[in]  bytes     Number of bytes to register
   * \param[out] memregion Memory region object. Can be NULL.
   */
  xmi_result_t XMI_Memory_register (xmi_context_t     context,
                                    void            * address,
                                    size_t            bytes,
                                    xmi_memregion_t * memregion);

  /**
   * \brief Deregister a local memory region for one sided operations
   *
   * It is illegal to deregister the "global" memory region.
   *
   * \param[in] context   XMI application context
   * \param[in] memregion Memory region object
   */
  xmi_result_t XMI_Memory_deregister (xmi_context_t   context,
                                      xmi_memregion_t memregion);

  /**
   * \brief Provide one or more contiguous segments to transfer.
   *
   * \todo this desperatly needs example code
   *
   * \see XMI_Put_iterate
   * \see XMI_Get_iterate
   *
   * \param[in]     context       XMI application context
   * \param[in]     cookie        Event callback argument
   * \param[in,out] local_offset  Array of byte offsets from the local buffer
   * \param[in,out] remote_offset Array of byte offsets from the remote buffer
   * \param[in,out] bytes         Array of bytes to transfer
   * \param[in,out] segments      Number of segments available to be initialized
   *                              and the number of segments actually initialized
   *
   * \retval  0 Iterate complete, do not invoke the iterate callback again for
   *            this transfer
   * \retval !0 Iterate is not complete, the iterate callback must be invoked
   *            again for this transfer
   */
  typedef size_t (*xmi_iterate_fn) (xmi_context_t   context,
                                    void          * cookie,
                                    size_t        * local_offset,
                                    size_t        * remote_offset,
                                    size_t        * bytes,
                                    size_t        * segments);

  /**
   * \brief Input parameter structure for rma iterator transfers
   */
  typedef struct
  {
    xmi_iterate_fn  function; /**< Non-contiguous iterate function */
  } xmi_rma_iterate_t;

  /**
   * \brief Input parameter structure for rma simple transfers
   */
  typedef struct
  {
    size_t          bytes;    /**< Data transfer size in bytes */
  } xmi_rma_simple_t;

  /**
   * \brief Input parameter structure for rma typed transfers
   */
  typedef struct
  {
    size_t          bytes;    /**< Data transfer size in bytes */
    xmi_type_t      local;    /**< Data type of local buffer */
    xmi_type_t      remote;   /**< Data type of remote buffer */
  } xmi_rma_typed_t;





  /**
   * \defgroup put Put data transfer operations
   *
   * ???
   */

  /**
   * \brief Input parameters for the XMI put functions
   * \ingroup put
   *
   * \see XMI_Put
   * \see XMI_Put_typed
   **/
  typedef struct
  {
    size_t                 task;      /**< Destination task */
    void                 * local;     /**< Local buffer virtual address */
    void                 * remote;    /**< Remote buffer virtual address */
    xmi_event_callback_t   send_done; /**< All local data has been sent */
    xmi_event_callback_t   recv_done; /**< All local data has been received */
    void                 * cookie;    /**< Argument to \b all event callbacks */
    xmi_send_hint_t        hints;     /**< Hints for sending the message */
    union
    {
      xmi_rma_simple_t     simple;    /**< Required, and only valid for, XMI_Put() */
      xmi_rma_typed_t      typed;     /**< Required, and only valid for, XMI_Put_typed() */
    };
  } xmi_put_t;

  xmi_result_t XMI_Put (xmi_context_t context, xmi_put_t * parameters);
  xmi_result_t XMI_Put_typed (xmi_context_t context, xmi_put_t * parameters);

  /**
   * \brief Input parameters for the XMI get functions
   * \ingroup get
   *
   * \see XMI_Get
   * \see XMI_Get_typed
   **/
  typedef struct
  {
    size_t                 task;      /**< Destination task */
    void                 * local;     /**< Local buffer virtual address */
    void                 * remote;    /**< Remote buffer virtual address */
    xmi_event_callback_t   done;      /**< All local data has been sent */
    xmi_event_callback_t   recv_done; /**< All local data has been received */
    void                 * cookie;    /**< Argument to \b all event callbacks */
    xmi_send_hint_t        hints;     /**< Hints for sending the message */
    union
    {
      xmi_rma_simple_t     simple;    /**< Required, and only valid for, XMI_Get() */
      xmi_rma_typed_t      typed;     /**< Required, and only valid for, XMI_Get_typed() */
    };
  } xmi_get_t;

  xmi_result_t XMI_Get (xmi_context_t context, xmi_put_t * parameters);
  xmi_result_t XMI_Get_typed (xmi_context_t context, xmi_put_t * parameters);

  /****************************************************************************
   *
   *   RDMA interface starts
   *
   ****************************************************************************
   */

  /**
   * \brief Input parameters for the XMI put functions
   * \ingroup put
   *
   * \see XMI_Rput
   * \see XMI_Rput_iterate
   * \see XMI_Rput_typed
   **/
  typedef struct
  {
    size_t                 task;      /**< Destination task */
    void                 * local_va;  /**< Local buffer virtual address */
    xmi_memregion_t        local_mr;  /**< Local buffer memory region */
    void                 * remote_va; /**< Remote buffer virtual address */
    xmi_memregion_t        remote_mr; /**< Remote buffer memory region */
    xmi_event_callback_t   send_done; /**< All local data has been sent */
    xmi_event_callback_t   recv_done; /**< All local data has been received */
    void                 * cookie;    /**< Argument to \b all event callbacks */
    xmi_send_hint_t        hints;     /**< Hints for sending the message */
    union
    {
      xmi_rma_simple_t     simple;    /**< Required, and only valid for, XMI_Rput() */
      xmi_rma_iterate_t    iterate;   /**< Required, and only valid for, XMI_Rput_iterate() */
      xmi_rma_typed_t      typed;     /**< Required, and only valid for, XMI_Rput_typed() */
    };
  } xmi_rput_t;

  /**
   * \brief Simple put operation for one-sided contiguous data transfer.
   *
   * \param[in] context    XMI application context
   * \param[in] parameters Input parameters structure
   *
   * \ingroup put
   */
  xmi_result_t XMI_Rput (xmi_context_t context, xmi_rput_t * parameters);

  /**
   * \brief Put operation for callback-driven one-sided non-contiguous data transfer.
   *
   * \param[in] context    XMI application context
   * \param[in] parameters Input parameters structure
   *
   * \ingroup put
   */
  xmi_result_t XMI_Rput_iterate (xmi_context_t context, xmi_rput_t * parameters);

  /**
   * \brief Put operation for data type specific one-sided data transfer.
   *
   * \param[in] context    XMI application context
   * \param[in] parameters Input parameters structure
   *
   * \ingroup put
   */
  xmi_result_t XMI_Rput_typed (xmi_context_t context, xmi_rput_t * parameters);





  /**
   * \defgroup get Get data transfer operations
   *
   * ???
   */

  /**
   * \brief Input parameters for the XMI get functions
   * \ingroup get
   *
   * \see XMI_Rget
   * \see XMI_Rget_iterate
   * \see XMI_Rget_typed
   **/
  typedef struct
  {
    size_t                 task;      /**< Destination task */
    void                 * local_va;  /**< Local buffer virtual address */
    xmi_memregion_t        local_mr;  /**< Local buffer memory region */
    void                 * remote_va; /**< Remote buffer virtual address */
    xmi_memregion_t        remote_mr; /**< Remote buffer memory region */
    xmi_event_callback_t   done;      /**< All remote data has been received */
    void                 * cookie;    /**< Argument to \b all event callbacks */
    xmi_send_hint_t        hints;     /**< Hints for sending the message */
    union
    {
      xmi_rma_simple_t     simple;    /**< Required, and only valid for, XMI_Get() */
      xmi_rma_iterate_t    iterate;   /**< Required, and only valid for, XMI_Get_iterate() */
      xmi_rma_typed_t      typed;     /**< Required, and only valid for, XMI_Get_typed() */
    };
  } xmi_rget_t;


  /**
   * \brief Simple get operation for one-sided contiguous data transfer.
   *
   * \param[in] context    XMI application context
   * \param[in] parameters Input parameters structure
   *
   * \ingroup get
   */
  xmi_result_t XMI_Rget (xmi_context_t context, xmi_rget_t * parameters);

  /**
   * \brief Get operation for callback-driven one-sided non-contiguous data transfer.
   *
   * \param[in] context    XMI application context
   * \param[in] parameters Input parameters structure
   *
   * \ingroup get
   */
  xmi_result_t XMI_Rget_iterate (xmi_context_t context, xmi_rget_t * parameters);

  /**
   * \brief Get operation for data type specific one-sided data transfer.
   *
   * \param[in] context    XMI application context
   * \param[in] parameters Input parameters structure
   *
   * \ingroup get
   */
  xmi_result_t XMI_Rget_typed (xmi_context_t context, xmi_rget_t * parameters);

  /** \} */ /* end of "rma" group */



  /*****************************************************************************/
  /**
   * \defgroup dynamictasks xmi dynamic task interface
   *
   * Some brief documentation on dynamic task stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Clean up local resources to a task in preparation for
   *        task shutdown or checkpoint
   *
   *        It is the user of this API's responsibility to ensure
   *        that all communication has been quiesced to and from
   *        the destination via a fence call and synchronization
   *
   * \param[in] context    XMI communication context
   * \param[in] dest       Array of destination tasks to close connections to
   * \param[in] count      Number of tasks in the array dest
   */

  xmi_result_t XMI_Purge_totask (xmi_context_t   context,
                                 size_t        * dest,
                                 size_t          count);

  /**
   * \brief Setup local resources to a task in preparation for
   *        task restart or creation
   *
   * \param[in] context    XMI communication context
   * \param[in] dest       Array of destination tasks to resume connections to
   * \param[in] count      Number of tasks in the array dest
   */
  xmi_result_t XMI_Resume_totask (xmi_context_t   context,
                                  size_t        * dest,
                                  size_t          count);

  /** \} */ /* end of "dynamic tasks" group */


#ifdef __cplusplus
};
#endif

/*
 * astyle info    http://astyle.sourceforge.net
 *
 * astyle options --style=gnu --indent=spaces=2 --indent-classes
 * astyle options --indent-switches --indent-namespaces --break-blocks
 * astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
 */

#endif /* __xmi_h__ */
