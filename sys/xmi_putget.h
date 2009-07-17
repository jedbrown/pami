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

typedef struct {
    xmi_task_t           dest;         /* destination task */
    void               * local_buffer; /* data address */
    xmi_data_type_t      local_type;   /* data type of local buffer */
    void               * remote_buffer;/* remote buffer address */
    xmi_data_type_t      remote_type;  /* data type of remote buffer */
    xmi_size_t           data_size;    /* data size in bytes */
    xmi_event_callback_t send_done;    /* message is sent */
    xmi_event_callback_t msg_done;     /* message is received at remote side */
    void               * cookie;       /* argument to event callbacks */
    xmi_send_hint_t      hints;        /* hints for sending the message */
} xmi_put_t;

xmi_result_t XMI_Put_contig(xmi_context_t context, xmi_put_t * put);
xmi_result_t XMI_Put_typed(xmi_context_t context, xmi_put_t * put);

typedef struct {
    xmi_task_t           dest;         /* destination task */
    void               * local_buffer; /* data address */
    xmi_data_type_t      local_type;   /* data type of local buffer */
    void               * remote_buffer;/* remote buffer address */
    xmi_data_type_t      remote_type;  /* data type of remote buffer */
    xmi_size_t           data_size;    /* data size in bytes */
    xmi_event_callback_t msg_done;     /* message is received at remote side */
    void               * cookie;       /* argument to event callbacks */
    xmi_send_hint_t      hints;        /* hints for sending the message */
} xmi_get_t;

xmi_result_t XMI_Get_contig(xmi_context_t context, xmi_get_t * get);
xmi_result_t XMI_Get_typed(xmi_context_t context, xmi_get_t * get);

xmi_result_t XMI_Register_mem(xmi_context_t context, void *buffer, xmi_size_t len);

#endif
