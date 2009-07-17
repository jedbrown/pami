/**
 * \file sys/xmi_fence.h
 * \brief XMI memory synchronization interface
 */
#ifndef _XMI_FENCE_H_
#define _XMI_FENCE_H_

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
 * \defgroup sync Memory synchronization interfaces
 *
 * Documentation for fence stuff ...
 */

/**
 * \brief Begin a memory synchronization region
 *
 * \param[in] context XMI application context
 *
 * \ingroup sync
 */
xmi_result_t XMI_Fence_begin (xmi_context_t context);

/**
 * \brief End a memory synchronization region
 *
 * \param[in] context XMI application context
 *
 * \ingroup sync
 */
xmi_result_t XMI_Fence_end (xmi_context_t context);


/**
 * \brief Syncronize all transfers between all tasks.
 *
 * from the local task to the remote taskBegin a memory synchronization region
 *
 * \param[in] context    XMI application context
 * \param[in] fence_done Event callback to invoke when the fence is complete
 * \param[in] cookie     Event callback argument
 *
 * \ingroup sync
 */
xmi_result_t XMI_Fence_all (xmi_context_t          context,
                            xmi_event_callback_t   fence_done,
                            void                 * cookie);

/**
 * \brief Syncronize all transfers between two tasks.
 *
 * from the local task to the remote taskBegin a memory synchronization region
 *
 * \param[in] context    XMI application context
 * \param[in] fence_done Event callback to invoke when the fence is complete
 * \param[in] cookie     Event callback argument
 * \param[in] task       Remote task to synchronize
 *
 * \ingroup sync
 */
xmi_result_t XMI_Fence_task (xmi_context_t          context,
                             xmi_event_callback_t   fence_done,
                             void                 * cookie,
                             size_t                 task);


#endif
