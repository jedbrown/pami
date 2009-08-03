/**
 * \file xmi_send.c
 * \brief Example XMI_Send source code.
 *
 * ***** need more description of this example ***
 */

#include <stdio.h>
#include <string.h>
#include "xmi_send.h"

typedef struct example_recv_info
{
  uint8_t data[1024];
  uint8_t ready;
} example_recv_info_t;

/**
 * \brief Simple incrementer callback function.
 * \see xmi_callback_fn
 */
void increment_callback_fn (void * arg, xmi_error_t error)
{
  *((size_t *)arg)++;
}



/**
 * \see xmi_recv_fn
 */
void sync_recv_fn (void                 * clientdata,
                   size_t                 rank,
                   const xmi_metadata_t * metadata,
                   size_t                 bytes,
                   const void           * source,
                   xmi_recv_t           * recv)
{
  example_recv_info_t * info =
    (example_recv_info_t *) clientdata;

  memcpy (info[rank].data, source, bytes);
  info[rank].ready = 1;

  return;
};

/**
 * \see xmi_recv_fn
 */
void async_recv_fn (void                 * clientdata,
                    size_t                 rank,
                    const xmi_metadata_t * metadata,
                    size_t                 bytes,
                    const void           * source,
                    xmi_recv_t           * recv)
{
  example_recv_info_t * info =
    (example_recv_info_t *) clientdata;

  info[rank].ready = 0;

  recv->destination = info[rank].data;
  recv->bytes       = bytes;

  recv->callback.fn  = increment_callback_fn;
  recv->callback.arg = &info[rank].ready;

  return;
};

/**
 * \see xmi_recv_fn
 */
void generic_recv_fn (void                 * clientdata,
                      size_t                 rank,
                      const xmi_metadata_t * metadata,
                      size_t                 bytes,
                      const void           * source,
                      xmi_recv_t           * recv)
{
  example_recv_info_t * info =
    (example_recv_info_t *) clientdata;

  if (source != NULL)
  {
    memcpy (info[rank].data, source, bytes);
    info[rank].ready = 1;
  }
  else
  {
    info[rank].ready = 0;

    recv->destination = info[rank].data;
    recv->bytes       = bytes;

    recv->callback.fn  = increment_callback_fn;
    recv->callback.arg = &info[rank].ready;
  }

  return;
};





void initialize_send_parameters (void            * buffer,
                                 void            * metadata,
                                 volatile size_t   send_complete,
                                 volatile size_t   recv_complete,
                                 xmi_send_t      * parameters)
{
  /* Initialize the local send completion callback. When this callback is
     invoked it is now safe to reuse, access, or deallocate the source
     and metadata buffers. */
  parameters.local.fn  = increment_callback_fn;
  parameters.local.arg = (void *) &send_complete;

  /* Initialize the remote recv completion callback. The remote rank has
     received the entire source buffer when this callback is invoked on
     the send/origin rank. */
  parameters.send.remote.fn  = increment_callback_fn;
  parameters.send.remote.arg = (void *) &recv_complete;

  /* Set the local source buffer address. */
  parameters.send.source = buffer;

  /* Specify the application metadata to send with the source data bufer.
     Typically, this would be information such as the MPI msginfo struct. */
  parameters.send.metadata.source  = metadata;
  parameters.send.metadata.bytes   = 16;
  parameters.send.metadata.options = 0;

  /* Set the hints (none specified). */
  parameters.send.hints = 0;
}


void do_send (xmi_context_t * context, xmi_send_t * parameters)
{
  /* Begin the send operation.  This operation is not complete until all bytes
     from the source data buffer have been processed. After the last byte has
     been processed the local completion callback will be invoked.

     In this example, the local completion callback will increment a variable
     on the stack. The status of this variable is then polled to determine when
     the send operation completed. This emulates a blocking send operation. */
  XMI_Send (context, &parameters);

  while (!send_complete)
    XMI_Advance (context);

  fprintf (stderr, "XMI_Send() complete locally. The source data buffer may now be accessed.\n");

  while (!recv_complete)
    XMI_Advance (context);

  fprintf (stderr, "XMI_Send() complete remotely. The remote rank has processed the entire source data buffer.\n");

  return;
}




void do_test ()
{
  xmi_result_t result;

  /* Initialize the XMI context. This is done once at the begining of the
     application code. */
  xmi_context_t context;
  result = XMI_Initialize (&context);

  /* Maximum of 16 ranks supported by this example. */
  example_recv_info_t * info =
    (example_recv_info_t *) malloc (sizeof(example_recv_info_t) * 16);


  uint8_t buffer[1024];
  uint8_t metadata[16];
  volatile size_t send_complete = 0;
  volatile size_t recv_complete = 0;
  xmi_send_t parameters;

  initialize_send_parameters ((void *) buffer,
                              (void *) metadata,
                              send_complete,
                              recv_complete,
                              &parameters)

  /* This test will always send to rank 1. */
  parameters.send.rank = 1;


  /* Initialize a generic dispatch function for send operations.

     This dispatch function will process both synchronous and asynchronous
     receives. The benefit to this technique is that only a single dispatch
     function needs to be initialized, however, this single dispatch function
     must handle both cases - synchronous and asynchronous.
   */
  xmi_dispatch_t generic_id = 1;
  result = XMI_Dispatch_set (&context,
                             generic_id,
                             generic_recv_fn,
                             info,
                             0);

  if (rank == 0)
  {
    /* Set the local source buffer length and dispatch id. */
    parameters.send.bytes    = 128;
    parameters.send.dispatch = generic_id;

    /* Reset the completion variables. */
    send_complete = 0;
    recv_complete = 0;

    do_send (context, &parameters);

    /* Set the local source buffer length and dispatch id. */
    parameters.send.bytes    = 1024;
    parameters.send.dispatch = generic_id;

    /* Reset the completion variables. */
    send_complete = 0;
    recv_complete = 0;

    do_send (context, &parameters);
  }
  else
  {
    while (!info[0].ready)
      XMI_Advance (context);

    info[0].ready = 0;

    while (!info[0].ready)
      XMI_Advance (context);
  }



  /* Initialize an asynchronous-only dispatch function for send operations.

     This dispatch function will only process asynchronous receives. The
     benefit to this technique is that the dispatch function expects to process
     only asynchronous receives and can shorten the code path in the
     dispatch function implementation and improve latency performance. However,
     if a synchronous receive is dispatched to this asynchronous function the
     code will assert or otherwise fail. This case is considered a program error.
   */
  xmi_dispatch_t async_id = 2;
  result = XMI_Dispatch_set (&context,
                             async_id,
                             async_recv_fn,
                             info,
                             0);

  if (rank == 0)
  {
    /* Set the local source buffer length and dispatch id. */
    parameters.send.bytes    = 1024;
    parameters.send.dispatch = async_id;

    /* Reset the completion variables. */
    send_complete = 0;
    recv_complete = 0;

    do_send (context, &parameters);
  }
  else
  {
    while (!info[0].ready)
      XMI_Advance (context);
  }



  /* Initialize a synchronous-only dispatch function for send operations.

     This dispatch function will only process synchronous receives. The
     benefit to this technique is that the dispatch function expects to process
     only synchronous receives and can shorten the code path in the
     dispatch function implementation and improve latency performance. However,
     if an asynchronous receive is dispatched to this synchronous function the
     code will assert or otherwise fail. This case is considered a program error.
   */
  xmi_dispatch_t sync_id = 3;
  result = XMI_Dispatch_set (&context,
                             sync_id,
                             sync_recv_fn,
                             info,
                             0);

  if (rank == 0)
  {
    /* Set the local source buffer length and dispatch id. */
    parameters.send.bytes    = 128;
    parameters.send.dispatch = sync_id;

    /* Reset the completion variables. */
    send_complete = 0;
    recv_complete = 0;

    do_send (context, &parameters);
  }
  else
  {
    while (!info[0].ready)
      XMI_Advance (context);
  }


  return;
}
