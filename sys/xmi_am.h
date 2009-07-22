#ifndef __xmi_am_h__
#define __xmi_am_h__

typedef int           xmi_result_t;
typedef void *        xmi_context_t;
typedef unsigned int  xmi_task_t;
typedef unsigned long xmi_size_t;
typedef void *        xmi_dispatch_t;
typedef void *        xmi_data_type_t;

/**
 * \brief Callback to handle message events
 *
 * \todo Move this to a common header file
 *
 * \param[in] context   XMI application context
 * \param[in] cookie    Event callback application argument
 * \param[in] result    Asynchronous result information (was error information)
 */
typedef void (*xmi_event_callback_t) ( xmi_context_t   context,
                                       void          * cookie,
                                       xmi_result_t    result );


/**
 * \brief Callback to provide data at send side or consume data at receive side
 *
 * Returns number of bytes copied into pipe buffer at send side
 * or number of bytes copied out from pipe buffer at receive side.
 *
 * TBD: <=0 byte being copied implies truncation
 *
 * \param[in] context   XMI application context
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
                                        xmi_size_t      pipe_size );


/*
 * Hints for sending a message
 *
 * TBD: better names for the hints
 */
typedef struct {
  uint32_t consistency:1;
  uint32_t buffer_registered:1;
  uint32_t use_rdma:1;
  uint32_t no_rdma:1;
  uint32_t no_local_copy:1;
  uint32_t contig_buffers:1;
  uint32_t interrupt_on_recv:1;
  uint32_t high_priority:1;
  uint32_t reserved:25;
} xmi_send_hint_t;


typedef struct {
  uint32_t data_in_pipe:1;          /* sync_send */
  uint32_t inline_completion:1;
  uint32_t reserved:30;
} xmi_recv_hint_t;


typedef struct {
  size_t                 bytes;    /**< Number of bytes of data */
  void                 * addr;     /**< Address of the buffer */
} xmi_send_simple_t;

typedef struct {
  xmi_data_callback_t    callback; /**< Data callback function */
} xmi_send_iterate_t;

typedef struct {
  size_t                 bytes;    /**< Number of bytes data */
  size_t                 offset;   /**< Starting offset */
  xmi_datatype_t         datatype; /**< Datatype TODO */
} xmi_send_typed_t;

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
 */
xmi_result_t XMI_Send (xmi_context_t context, xmi_send_t * send);

/**
 * \brief Short contiguous send
 * \todo Is this needed? Can't we use hints with XMI_Send() instead?
 */
xmi_result_t XMI_Send_contig_short (xmi_context_t context, xmi_send_t * send);

/**
 * \brief Typed (DGSP) send
 */
xmi_result_t XMI_Send_typed (xmi_context_t context, xmi_send_t * send);

/**
 * \brief Untyped send with callback requesting for data
 */
xmi_result_t XMI_Send_iterate (xmi_context_t context, xmi_send_t * send);


/**
 * \brief Receive message structure
 */
typedef struct {
  xmi_recv_hint_t        hints;    /**< Hints for receiving the message */
  void                 * cookie;   /**< Argument to \b all event callbacks */
  xmi_event_callback_t   local;    /**< Local message completion event */
  union {
    xmi_send_simple_t    simple;   /**< Required, and only valid for, XMI_Send() */
    xmi_send_iterate_t   iterate;  /**< Required, and only valid for, XMI_Send_iterate() */
    xmi_send_typed_t     typed;    /**< Required, and only valid for, XMI_Send_typed() */
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

#endif
