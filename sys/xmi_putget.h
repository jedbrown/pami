/**
 * \file sys/xmi_putget.h
 * \brief XMI remote memory access interface
 */
#ifndef _XMI_PUTGET_H_
#define _XMI_PUTGET_H_

#ifndef _TO_REMOVE_
typedef int           xmi_result_t;
typedef void *        xmi_context_t;
typedef unsigned int  xmi_task_t;
typedef unsigned long xmi_size_t;
typedef void *        xmi_dispatch_t;
typedef void *        xmi_data_type_t;

/*
 * Callback to handle message events
 */
typedef void (*xmi_event_callback_t) (
    xmi_context_t        context,      /* IN: XMI context */
    void               * cookie,       /* IN: callback cookie */
    xmi_result_t         result);      /* IN: asynchronous result */

/*
 * Hints for sending a message
 *
 * TBD: better names for the hints
 */
typedef struct {
    unsigned  consistency:1;
    unsigned  buffer_registered:1;
    unsigned  use_rdma:1;
    unsigned  no_rdma:1;
    unsigned  no_local_copy:1;
    unsigned  contig_buffers:1;
    unsigned  interrupt_on_recv:1;
    unsigned  high_priority:1;
    unsigned  reserved:25;
} xmi_send_hint_t;
#endif


/**
 * \defgroup rma Remote Memory Access data transfer operations
 *
 * Documentation for RMA stuff ...
 */

/**
 * \brief Register a memory region for one sided operations
 *
 * \param[in] context    XMI application context
 * \param[in] address    Virtual address of memory region
 * \param[in] bytes      Number of bytes to register
 *
 * \ingroup rma
 */
xmi_result_t XMI_Memory_register (xmi_context_t   context,
                                  void          * address,
                                  size_t          bytes);


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
 *
 * \ingroup rma
 */
typedef size_t (*xmi_iterate_fn) (xmi_context_t   context,
                                  void          * cookie,
                                  size_t        * local_offset,
                                  size_t        * remote_offset,
                                  size_t        * bytes,
                                  size_t        * segments);

/**
 * \brief ???
 * \ingroup rma
 */
typedef struct xmi_rma_iterate {
  xmi_iterate_fn  function; /**< Non-contiguous iterate function */
} xmi_rma_iterate_t;

/**
 * \brief ???
 * \ingroup rma
 */
typedef struct xmi_rma_simple {
  size_t          bytes;    /**< Data transfer size in bytes */
} xmi_rma_simple_t;

/**
 * \brief ???
 * \ingroup rma
 */
typedef struct xmi_rma_typed {
  size_t          bytes;    /**< Data transfer size in bytes */
  xmi_data_type_t local;    /**< Data type of local buffer */
  xmi_data_type_t remote;   /**< Data type of remote buffer */
} xmi_rma_typed_t;





/**
 * \defgroup put Put data transfer operations
 * \ingroup rma
 *
 * Foo
 */

/**
 * \brief Input parameters for the XMI put functions
 * \ingroup put
 *
 * \see XMI_Put
 * \see XMI_Put_iterate
 * \see XMI_Put_typed
 **/
typedef struct xmi_put {
  size_t                 task;      /**< Destination task */
  void                 * local;     /**< Local buffer virtual address */
  void                 * remote;    /**< Remote buffer virtual addreess */
  xmi_event_callback_t   send_done; /**< All local data has been sent */
  xmi_event_callback_t   recv_done; /**< All local data has been received */
  void                 * cookie;    /**< Argument to \b all event callbacks */
  xmi_send_hint_t        hints;     /**< Hints for sending the message */
  union {
    xmi_rma_simple_t     simple;    /**< Required, and only valid for, XMI_Put() */
    xmi_rma_iterate_t    iterate;   /**< Required, and only valid for, XMI_Put_iterate() */
    xmi_rma_typed_t      typed;     /**< Required, and only valid for, XMI_Put_typed() */
  };
} xmi_put_t;

/**
 * \brief Simple put operation for one-sided contiguous data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup put
 */
xmi_result_t XMI_Put (xmi_context_t context, xmi_put_t * parameters);

/**
 * \brief Put operation for callback-driven one-sided non-contiguous data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup put
 */
xmi_result_t XMI_Put_iterate (xmi_context_t context, xmi_put_t * parameters);

/**
 * \brief Put operation for data type specific one-sided data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup put
 */
xmi_result_t XMI_Put_typed (xmi_context_t context, xmi_put_t * parameters);





/**
 * \defgroup get Get data transfer operations
 * \ingroup rma
 *
 * Foo
 */

/**
 * \brief Input parameters for the XMI get functions
 * \ingroup get
 *
 * \see XMI_Get
 * \see XMI_Get_iterate
 * \see XMI_Get_typed
 **/
typedef struct xmi_get {
  size_t                 task;      /**< Destination task */
  void                 * local;     /**< Local buffer virtual address */
  void                 * remote;    /**< Remote buffer virtual addreess */
  xmi_event_callback_t   done;      /**< All remote data has been received */
  void                 * cookie;    /**< Argument to \b all event callbacks */
  xmi_send_hint_t        hints;     /**< Hints for sending the message */
  union {
    xmi_rma_simple_t     simple;    /**< Required, and only valid for, XMI_Get() */
    xmi_rma_iterate_t    iterate;   /**< Required, and only valid for, XMI_Get_iterate() */
    xmi_rma_typed_t      typed;     /**< Required, and only valid for, XMI_Get_typed() */
  };
} xmi_get_t;


/**
 * \brief Simple get operation for one-sided contiguous data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup get
 */
xmi_result_t XMI_Get (xmi_context_t context, xmi_get_t * parameters);

/**
 * \brief Get operation for callback-driven one-sided non-contiguous data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup get
 */
xmi_result_t XMI_Get_iterate (xmi_context_t context, xmi_get_t * parameters);

/**
 * \brief Get operation for data type specific one-sided data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup get
 */
xmi_result_t XMI_Get_typed (xmi_context_t context, xmi_get_t * parameters);


#endif
