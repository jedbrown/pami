#ifndef __xmi_am_h__
#define __xmi_am_h__

typedef int           xmi_result_t;
typedef void *        xmi_context_t;
typedef unsigned int  xmi_task_t;
typedef unsigned long xmi_size_t;
typedef void *        xmi_dispatch_t;
typedef void *        xmi_data_type_t;

/* 
 * Callback to provide data at send side or consume data at receive side
 * 
 * Returns number of bytes copied into pipe buffer at send side
 * or number of bytes copied out from pipe buffer at receive side.
 *
 * TBD: <=0 byte being copied implies truncation
 */
typedef xmi_size_t (*xmi_data_callback_t) (
    xmi_context_t        context,      /* IN: XMI context */
    void               * cookie,       /* IN: callback cookie */
    xmi_size_t           offset,       /* IN: starting data offset */
    void               * pipe_addr,    /* IN: address of XMI pipe buffer */
    xmi_size_t           pipe_size);   /* IN: size of XMI pipe buffer */

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
    unsigned  use_rdma:1;
    unsigned  no_rdma:1;
    unsigned  no_local_copy:1;
    unsigned  contig_buffers:1;
    unsigned  interrupt_on_recv:1;
    unsigned  high_priority:1;
    unsigned  reserved:25;
} xmi_send_hint_t;

/*
 * Hints for receiving a message
 *
 * TBD: better names for the hints
 */
typedef struct {
    unsigned  data_in_pipe:1;          /* sync_send */
    unsigned  inline_completion:1; 
    unsigned  reserved:30;
} xmi_recv_hint_t;

/*
 * Send message structure
 *
 * Valid specifications for data buffer
 *   (address,  XMI_BYTE,    size, 0)
 *   (callback, XMI_UNTYPED, size, 0)
 *   (address,  type,        size, offset)
 *
 * TBD: one message structure for each send function
 */
typedef struct {
    xmi_task_t           dest;         /* destination task */

    xmi_dispatch_t       dispatch;     /* dispatch function */
    void               * header_addr;  /* header address */
    xmi_size_t           header_size;  /* header size */

    union {
        void               * address;  /* data address */
        xmi_data_callback_t  callback; /* data callback, untyped send only */
    } data;
    xmi_data_type_t      data_type;    /* data type, typed send only */
    xmi_size_t           data_size;    /* data size to send */
    xmi_size_t           offset;       /* starting offset */

    xmi_event_callback_t local_done;   /* message completed locally */
    xmi_event_callback_t remote_done;  /* message completed remotely */
    void               * cookie;       /* argument to all message callbacks */
    xmi_send_hint_t      hints;        /* hints for sending the message */
} xmi_send_t;

/*
 * Contiguous send
 */
xmi_result_t XMI_Send_contig(xmi_context_t context, xmi_send_t * send);

/*
 * Short contiguous send
 */
xmi_result_t XMI_Send_contig_short(xmi_context_t context, xmi_send_t * send);

/*
 * Typed (DGSP) send
 */
xmi_result_t XMI_Send_typed(xmi_context_t context, xmi_send_t * send);

/*
 * Untyped send with callback requesting for data
 */
xmi_result_t XMI_Send_untyped(xmi_context_t context, xmi_send_t * send);

/*
 * Receive message structure
 */
typedef struct {
    union {
        void               * address;  /* data address */
        xmi_data_callback_t  callback; /* data callback, untyped recv only */
    } data;
    xmi_data_type_t      data_type;    /* data type, typed recv only */
    xmi_size_t           data_size;    /* data size to receive */
    xmi_size_t           offset;       /* starting offset */

    xmi_event_callback_t local_done;   /* message completed locally */
    void               * cookie;       /* argument to all message callbacks */ 
    xmi_recv_hint_t      hints;        /* hints for receiving the message */
} xmi_recv_t;

/*
 * Dispatch callback
 *
 * TBD: make input parameters a struct
 */
typedef void (*xmi_dispatch_callback_t) (
    xmi_context_t        context,      /* IN: XMI context */
    void               * cookie,       /* IN: dispatch cookie */
    xmi_task_t           source,       /* IN: source task */
    void               * header_addr,  /* IN: header address */
    xmi_size_t           header_size,  /* IN: header size */               
    void               * pipe_addr,    /* IN: address of XMI pipe buffer */
    xmi_size_t           pipe_size,    /* IN: size of XMI pipe buffer */
    xmi_recv_t         * recv);        /* OUT: receive message structure */

#endif
