/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/util.h
 * \brief ???
 */

#ifndef __test_util_h__
#define __test_util_h__

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "sys/xmi.h"
//#include "util/common.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif


typedef struct
{
  size_t send;
  size_t recv;
} barrier_connection_t;

barrier_connection_t __barrier_active[2];
size_t __barrier_phase;
size_t __barrier_size;
size_t __barrier_task;
size_t __barrier_next_task;

size_t         __barrier_dispatch;
xmi_context_t  __barrier_context;


/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
static void barrier_dispatch_function (
    xmi_context_t        context,      /**< IN: XMI context */
    size_t               contextid,
    void               * cookie,       /**< IN: dispatch cookie */
    xmi_task_t           task,         /**< IN: source task */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of XMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of XMI pipe buffer */
    xmi_recv_t         * recv)        /**< OUT: receive message structure */
{
  size_t phase = *((size_t *) header_addr);
  size_t index = phase%2;

  TRACE_ERR((stderr, ">>> barrier_dispatch_function(), __barrier_active[%zd].send = %zd, __barrier_active[%zd].recv = %zd\n", index, __barrier_active[index].send, index, __barrier_active[index].recv));

  __barrier_active[index].recv++;
  recv->data.simple.bytes = 0;

  // Forward this barrier notification to the next task.
  TRACE_ERR((stderr, "    barrier_dispatch_function(), __barrier_active[%zd].send %% __barrier_size (%zd) = %zd\n", index, __barrier_size, __barrier_active[index].send % __barrier_size));
  if (__barrier_active[index].send % __barrier_size !=  0)
  {
    xmi_send_immediate_t parameters;
    parameters.send.dispatch     = __barrier_dispatch;
    parameters.send.header.addr  = &phase;
    parameters.send.header.bytes = sizeof (size_t);
    parameters.immediate.addr    = NULL;
    parameters.immediate.bytes   = 0;
    parameters.send.task         = __barrier_next_task;

    TRACE_ERR((stderr, "    forward barrier notification (phase = %zd => %zd) to task: %zd\n", phase, index, parameters.send.task));

    xmi_result_t result = XMI_Send_immediate (__barrier_context, &parameters);
    __barrier_active[index].send++;
  }
  TRACE_ERR((stderr, "<<< barrier_dispatch_function(), __barrier_active[%zd].send = %zd, __barrier_active[%zd].recv = %zd\n", index, __barrier_active[index].send, index, __barrier_active[index].recv));
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */


void barrier ()
{
  TRACE_ERR((stderr, "#### enter barrier(), \"poor man's barrier\" ...\n"));

  size_t phase = __barrier_phase++;
  size_t index = phase%2;
  __barrier_active[index].send++;
  __barrier_active[index].recv++;

  xmi_send_immediate_t parameters;
  parameters.send.dispatch = __barrier_dispatch;
  parameters.send.header.addr = &phase;
  parameters.send.header.bytes = sizeof (phase);
  parameters.immediate.addr  = NULL;
  parameters.immediate.bytes = 0;
  parameters.send.task = __barrier_next_task;

  TRACE_ERR((stderr, "     barrier(), before send, phase = %zd, __barrier_active[%zd].send = %zd, __barrier_active[%zd].recv = %zd\n", phase, index, __barrier_active[index].send, index, __barrier_active[index].recv));
  xmi_result_t result = XMI_Send_immediate (__barrier_context, &parameters);
  TRACE_ERR((stderr, "     barrier(),  after send, phase = %zd, __barrier_active[%zd].send = %zd, __barrier_active[%zd].recv = %zd\n", phase, index, __barrier_active[index].send, index, __barrier_active[index].recv));

  // Increment barrier notification count to account fo this task.
  __barrier_active[index].send++;

  TRACE_ERR((stderr, "     barrier() Before send advance\n"));
  while (__barrier_active[index].send % __barrier_size != 0)
    XMI_Context_advance (__barrier_context, 100);

  TRACE_ERR((stderr, "     barrier() Before recv advance\n"));
  while (__barrier_active[index].recv % __barrier_size != 0)
    XMI_Context_advance (__barrier_context, 100);

  TRACE_ERR((stderr, "####  exit barrier(), \"poor man's barrier\"\n"));
  return;
}

void barrier_init (xmi_context_t context, size_t dispatch)
{
  TRACE_ERR((stderr, "enter barrier_init() ...\n"));

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  xmi_result_t result = XMI_Configuration_query (context, &configuration);
  __barrier_task = configuration.value.intval;

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query (context, &configuration);
  __barrier_size = configuration.value.intval;

  __barrier_next_task = (__barrier_task + 1) % __barrier_size;

  __barrier_context  = context;
  __barrier_dispatch = dispatch;
  __barrier_phase = 0;
  __barrier_active[0].send = 0;
  __barrier_active[0].recv = 0;
  __barrier_active[1].send = 0;
  __barrier_active[1].recv = 0;

  xmi_dispatch_callback_fn fn;
  fn.p2p = barrier_dispatch_function;
  xmi_send_hint_t options;
  TRACE_ERR((stderr, "Before XMI_Dispatch_set() ...\n"));
  result = XMI_Dispatch_set (context,
                             dispatch,
                             fn,
                             (void *)__barrier_active,
                             options);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable register xmi dispatch. result = %d\n", result);
    abort();
  }

  barrier();
  TRACE_ERR((stderr, "... exit barrier_init()\n"));
}

#undef TRACE_ERR
#endif // __tests_util_h__
