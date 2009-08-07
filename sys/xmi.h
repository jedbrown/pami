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


typedef enum
  {
    XMI_SUCCESS = 0,  /**< Successful execution        */
    XMI_NERROR  =-1,  /**< Generic error (-1)          */
    XMI_ERROR   = 1,  /**< Generic error (+1)          */
    XMI_INVAL,        /**< Invalid argument            */
    XMI_UNIMPL,       /**< Function is not implemented */
    XMI_EAGAIN,       /**< Not currently availible     */
    XMI_SHUTDOWN,     /**< Rank has shutdown           */
    XMI_CHECK_ERRNO,  /**< Check the errno val         */
    XMI_OTHER,        /**< Other undefined error       */
  }
  xmi_result_t;
typedef void* xmi_context_t;
typedef void* xmi_application_t;
typedef void* xmi_error_t;
typedef void* xmi_hint_t;
typedef void* xmi_configuration_t;
typedef void* xmi_dispatch_t;
typedef void* xmi_data_type_t;


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

typedef struct {
    xmi_user_key_t    key;
    xmi_user_value_t  value;
} xmi_user_config_t;

/*
 * This enum contains ALL possible attributes for all hardware
 */
typedef enum {
    // Attribute        // Init / Query / Update
    XMI_PROTOCOL_NAME=1,// IQ  : char *            : name of the protocol
    XMI_RELIABILITY,    // IQ  : xmi_bool_t        : guaranteed reliability
    XMI_ATTRIBUTES,     //  Q  : xmi_attribute_t[] : attributes on the platform terminated with NULL
    XMI_TASK_ID,        //  Q  : size_t            : ID of this task
    XMI_NUM_TASKS,      //  Q  : size_t            : total number of tasks
    XMI_RECV_INTR_MASK, //  QU : xmi_intr_mask_t   : receive interrupt mask
    XMI_CHECK_PARAMS,   //  QU : xmi_bool_t        : check parameters
    XMI_USER_KEYS,      //  Q  : xmi_user_key_t[]  : user-defined keys terminated with NULL
    XMI_USER_CONFIG,    //  QU : xmi_user_config_t : user-defined configuration key and value are shallow-copied for update
} xmi_attribute_t;

#define XMI_EXT_ATTR 1000 // starting value for extended attributes
#include "xmi_ext.h"    // platform-specific

typedef struct {
    xmi_attribute_t  attr;
    void *           value;
} xmi_config_t;

#if 0
/* Replaced by XMI_Context_create */
xmi_result_t XMI_Init(xmi_context_t *context,
                      int num_configs, xmi_config_t config[]);
#endif
char * XMI_Last_error();

/*
 * \brief NULL_CONTEXT to allow queries outside a context
 */

#define NULL_CONTEXT  ((xmi_context_t)0)

/*
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
xmi_result_t XMI_Config_query(xmi_context_t context, xmi_attribute_t attribute,
        void* value_out);

/*
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
xmi_result_t XMI_Config_update(xmi_context_t context, xmi_attribute_t attribute,
        void* value_in);

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

/** \} */ /* end of "context" group */



/*****************************************************************************/
/**
 * \defgroup datatype xmi non-contiguous datatype interface
 *
 * Some brief documentation on active message stuff ...
 * \{
 */
/*****************************************************************************/

typedef void *        xmi_type_t;

/* Create a new type */
int XMI_Type_create(xmi_type_t *type);

/* Add simple buffers */
int XMI_Type_add_simple(xmi_type_t type, size_t bytes, 
        size_t offset, size_t reps, size_t stride);

/* Add typed buffers */
int XMI_Type_add_typed(xmi_type_t type, xmi_type_t sub_type, 
        size_t offset, size_t reps, size_t stride);

/* Commit the type, which can no longer be modified afterwards */
int XMI_Type_commit(xmi_type_t type);

/* Destroy the type */
/*
   What if some in-flight messages are still using it?
   What if some other types have references to it?
 */
int XMI_Type_destroy(xmi_type_t type);

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
typedef struct {
  uint32_t consistency       :1; /**< Force match ordering semantics */
  uint32_t sync_send         :1; /**< Assert that all sends will be synchronously received */
  uint32_t buffer_registered :1;
  uint32_t use_rdma          :1; /**< Assert/enable rdma operations */
  uint32_t no_rdma           :1; /**< Disable rdma operations */
  uint32_t no_local_copy     :1;
  uint32_t interrupt_on_recv :1; /**< Interrupt the remote task when the first packet arrives */
  uint32_t high_priority     :1; /**< Message is delivered with high priority,
                                      which may result in out-of-order delivery */
  uint32_t reserved:24;
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
  XMI_AM_KIND_ITERATE,
  XMI_AM_KIND_TYPED,
  XMI_AM_KIND_COUNT
} xmi_am_kind_t;

/**
 * \brief Structure for send parameters unique to a simple active message send
 */
typedef struct {
  size_t                 bytes;    /**< Number of bytes of data */
  void                 * addr;     /**< Address of the buffer */
} xmi_send_simple_t;

/**
 * \brief Structure for send parameters unique to an iterator active message send
 */
typedef struct {
  xmi_data_callback_t    callback; /**< Data callback function */
} xmi_send_iterate_t;

/**
 * \brief Structure for send parameters unique to a typed active message send
 */
typedef struct {
  size_t                 bytes;    /**< Number of bytes data */
  size_t                 offset;   /**< Starting offset */
  xmi_type_t             datatype; /**< Datatype */
} xmi_send_typed_t;

/**
 * \brief Active message send parameter structure
 */
typedef struct {
  xmi_dispatch_t         dispatch; /**< Dispatch identifier */
  xmi_send_hint_t        hints;    /**< Hints for sending the message */
  size_t                 task;     /**< Destination task */
  void                 * cookie;   /**< Argument to \b all event callbacks */
  xmi_event_callback_t   local;    /**< Local message completion event */
  xmi_event_callback_t   remote;   /**< Remote message completion event ------ why is this needed ? */
  struct {
    size_t               bytes;    /**< Header buffer size in bytes */
    void               * addr;     /**< Header buffer address */
  } header;                        /**< Send message metadata header */
  union {
    xmi_send_simple_t    simple;   /**< Required, and only valid for, XMI_Send() */
    xmi_send_iterate_t   iterate;  /**< Required, and only valid for, XMI_Send_iterate() */
    xmi_send_typed_t     typed;    /**< Required, and only valid for, XMI_Send_typed() */
  } data;                          /**< Send message source data */
} xmi_send_t;

/**
 * \brief Contiguous send
 *
 * \param[in] context    XMI communication context
 * \param[in] parameters Send parameter structure
 */
xmi_result_t XMI_Send (xmi_context_t context, xmi_send_t * parameters);

/**
 * \brief Short contiguous send
 * \todo Is this needed? Can't we use hints with XMI_Send() instead?
 *
 * \param[in] context    XMI communication context
 * \param[in] parameters Send parameter structure
 */
xmi_result_t XMI_Send_contig_short (xmi_context_t context, xmi_send_t * parameters);

/**
 * \brief Typed (DGSP) send
 *
 * \param[in] context    XMI communication context
 * \param[in] parameters Send parameter structure
 */
xmi_result_t XMI_Send_typed (xmi_context_t context, xmi_send_t * parameters);

/**
 * \brief Untyped send with callback requesting for data
 *
 * \param[in] context    XMI communication context
 * \param[in] parameters Send parameter structure
 */
xmi_result_t XMI_Send_iterate (xmi_context_t context, xmi_send_t * parameters);

typedef struct {
  uint32_t data_in_pipe:1;          /* sync_send */
  uint32_t inline_completion:1;
  uint32_t reserved:30;
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
typedef struct {
  xmi_recv_hint_t        hints;    /**< Hints for receiving the message */
  void                 * cookie;   /**< Argument to \b all event callbacks */
  xmi_event_callback_t   local;    /**< Local message completion event */
  xmi_am_kind_t          kind;     /**< Which kind receive is to be done */
  union {
    xmi_send_simple_t    simple;   /**< Receive into a simple contiguous buffer */
    xmi_send_iterate_t   iterate;  /**< Receive via explicit data callbacks */
    xmi_send_typed_t     typed;    /**< Receive into a non-contiguous buffer */
  } data;                          /**< Receive message destination data */
} xmi_recv_t;

/**
 * \brief Dispatch callback
 *
 * TBD: make input parameters a struct
 */
typedef void (*xmi_dispatch_callback_t) (
    xmi_context_t        context,      /* IN: XMI context */
    void               * cookie,       /* IN: dispatch cookie */
    size_t               task,         /* IN: source task */
    void               * header_addr,  /* IN: header address */
    size_t               header_size,  /* IN: header size */
    void               * pipe_addr,    /* IN: address of XMI pipe buffer */
    size_t               pipe_size,    /* IN: size of XMI pipe buffer */
    xmi_recv_t         * recv);        /* OUT: receive message structure */

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
extern xmi_memregion_t xmi_global_memregion;

/**
 * \brief Register a local memory region for one sided operations
 *
 * The local memregion may be transfered, via a send message, to a remote task
 * to allow the remote task to perform one-sided operations with this local
 * task
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
 * \brief ???
 */
typedef struct {
  xmi_iterate_fn  function; /**< Non-contiguous iterate function */
} xmi_rma_iterate_t;

/**
 * \brief ???
 */
typedef struct {
  size_t          bytes;    /**< Data transfer size in bytes */
} xmi_rma_simple_t;

/**
 * \brief ???
 */
typedef struct {
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
typedef struct {
  size_t                 task;      /**< Destination task */
  void                 * local;     /**< Local buffer virtual address */
  void                 * remote;    /**< Remote buffer virtual address */
  xmi_event_callback_t   send_done; /**< All local data has been sent */
  xmi_event_callback_t   recv_done; /**< All local data has been received */
  void                 * cookie;    /**< Argument to \b all event callbacks */
  xmi_send_hint_t        hints;     /**< Hints for sending the message */
  union {
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
typedef struct {
  size_t                 task;      /**< Destination task */
  void                 * local;     /**< Local buffer virtual address */
  void                 * remote;    /**< Remote buffer virtual address */
  xmi_event_callback_t   done;      /**< All local data has been sent */
  xmi_event_callback_t   recv_done; /**< All local data has been received */
  void                 * cookie;    /**< Argument to \b all event callbacks */
  xmi_send_hint_t        hints;     /**< Hints for sending the message */
  union {
    xmi_rma_simple_t     simple;    /**< Required, and only valid for, XMI_Get() */
    xmi_rma_typed_t      typed;     /**< Required, and only valid for, XMI_Get_typed() */
  };
} xmi_get_t;

xmi_result_t XMI_Get (xmi_context_t context, xmi_put_t * parameters);
xmi_result_t XMI_Get_typed (xmi_context_t context, xmi_put_t * parameters);

/*************************************************************************/
/*
 *   RDMA interface starts
 */
/*************************************************************************/

/**
 * \brief Input parameters for the XMI put functions
 * \ingroup put
 *
 * \see XMI_RPut
 * \see XMI_RPut_iterate
 * \see XMI_RPut_typed
 **/
typedef struct {
  size_t                 task;      /**< Destination task */
  void                 * local_va;  /**< Local buffer virtual address */
  xmi_memregion_t        local_mr;  /**< Local buffer memory region */
  void                 * remote_va; /**< Remote buffer virtual address */
  xmi_memregion_t        remote_mr; /**< Remote buffer memory region */
  xmi_event_callback_t   send_done; /**< All local data has been sent */
  xmi_event_callback_t   recv_done; /**< All local data has been received */
  void                 * cookie;    /**< Argument to \b all event callbacks */
  xmi_send_hint_t        hints;     /**< Hints for sending the message */
  union {
    xmi_rma_simple_t     simple;    /**< Required, and only valid for, XMI_RPut() */
    xmi_rma_iterate_t    iterate;   /**< Required, and only valid for, XMI_RPut_iterate() */
    xmi_rma_typed_t      typed;     /**< Required, and only valid for, XMI_RPut_typed() */
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
xmi_result_t XMI_RPut (xmi_context_t context, xmi_rput_t * parameters);

/**
 * \brief Put operation for callback-driven one-sided non-contiguous data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup put
 */
xmi_result_t XMI_RPut_iterate (xmi_context_t context, xmi_rput_t * parameters);

/**
 * \brief Put operation for data type specific one-sided data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup put
 */
xmi_result_t XMI_RPut_typed (xmi_context_t context, xmi_rput_t * parameters);





/**
 * \defgroup get Get data transfer operations
 *
 * ???
 */

/**
 * \brief Input parameters for the XMI get functions
 * \ingroup get
 *
 * \see XMI_RGet
 * \see XMI_RGet_iterate
 * \see XMI_RGet_typed
 **/
typedef struct {
  size_t                 task;      /**< Destination task */
  void                 * local_va;  /**< Local buffer virtual address */
  xmi_memregion_t        local_mr;  /**< Local buffer memory region */
  void                 * remote_va; /**< Remote buffer virtual address */
  xmi_memregion_t        remote_mr; /**< Remote buffer memory region */
  xmi_event_callback_t   done;      /**< All remote data has been received */
  void                 * cookie;    /**< Argument to \b all event callbacks */
  xmi_send_hint_t        hints;     /**< Hints for sending the message */
  union {
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
xmi_result_t XMI_RGet (xmi_context_t context, xmi_rget_t * parameters);

/**
 * \brief Get operation for callback-driven one-sided non-contiguous data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup get
 */
xmi_result_t XMI_RGet_iterate (xmi_context_t context, xmi_rget_t * parameters);

/**
 * \brief Get operation for data type specific one-sided data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup get
 */
xmi_result_t XMI_RGet_typed (xmi_context_t context, xmi_rget_t * parameters);

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

#endif /* __xmi_h__ */
