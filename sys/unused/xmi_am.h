/**
 * \file xmi_am.h
 * \brief xmi active messaging interface
 * \defgroup activemessage Active message
 * \{
 */
#ifndef __xmi_am_h__
#define __xmi_am_h__

#include "xmi.h"
//#include "../interface/ll_pipeworkqueue.h"

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

typedef struct {
  uint32_t data_in_pipe:1;          /* sync_send */
  uint32_t inline_completion:1;
  uint32_t reserved:30;
} xmi_recv_hint_t;


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
#if 1
  size_t                 bytes;    /**< Number of bytes data */
  size_t                 offset;   /**< Starting offset */
  xmi_data_type_t        datatype; /**< Datatype TODO */
#else
  LL_PipeWorkQueue_t    * wq;      /**< \see interface/ll_pipeworkqueue.h */
#endif
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

typedef enum {
  XMI_AM_KIND_SIMPLE        = 0,
  XMI_AM_KIND_ITERATE,
  XMI_AM_KIND_NONCONTIGUOUS,
  XMI_AM_KIND_COUNT
} xmi_am_kind_t;

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

/**
 * \}
 * \addtogroup activemessage
 *
 * More documentation for active message stuff....
 */

#endif
