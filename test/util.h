/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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
#define TRACE_ERR(x) // fprintf x
#endif



unsigned __barrier_active[2];
size_t __barrier_phase;
size_t __barrier_size;
size_t __barrier_task;
size_t __barrier_next_task;
xmi_endpoint_t __barrier_next_endpoint;

size_t         __barrier_dispatch;
xmi_context_t  __barrier_context;


/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
static void barrier_dispatch_function (
    xmi_context_t        context,      /**< IN: XMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of XMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of XMI pipe buffer */
    xmi_recv_t         * recv)        /**< OUT: receive message structure */
{
  size_t phase = *((size_t *) header_addr);

  TRACE_ERR((stderr, ">>> barrier_dispatch_function(), __barrier_active[%zu] = %zu\n", phase, __barrier_active[phase]));

  --__barrier_active[phase];

  // Forward this barrier notification to the next task.
  //TRACE_ERR((stderr, "    barrier_dispatch_function(), __barrier_active[%zd].send %% __barrier_size (%zd) = %zd\n", index, __barrier_size, __barrier_active[index].send % __barrier_size));
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

void barrier ()
{
  TRACE_ERR((stderr, "#### enter barrier(),  ...\n"));

  __barrier_active[__barrier_phase] = __barrier_size-1;
  __barrier_phase = __barrier_phase^1;

  xmi_send_immediate_t parameters;
  parameters.dispatch        = __barrier_dispatch;
  parameters.header.iov_base = &__barrier_phase;
  parameters.header.iov_len  = sizeof (__barrier_phase);
  parameters.data.iov_base   = NULL;
  parameters.data.iov_len    = 0;
  parameters.dest            = __barrier_next_endpoint;

  TRACE_ERR((stderr, "     barrier(), before send, phase = %zu, __barrier_active[%zu] = %zu, parameters.dest = 0x%08x\n", __barrier_phase, __barrier_phase, __barrier_active[__barrier_phase], parameters.dest));
  xmi_result_t result = XMI_Send_immediate (__barrier_context, &parameters);


  TRACE_ERR((stderr, " barrier() Before recv advance\n"));
  while (__barrier_active[__barrier_phase]  != 0)
    XMI_Context_advance (__barrier_context, 100);

  TRACE_ERR((stderr, "####  exit barrier(), \n"));
  return;
}

void barrier_init (xmi_client_t client, xmi_context_t context, size_t dispatch)
{
  TRACE_ERR((stderr, "enter barrier_init() ...\n"));

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  xmi_result_t result = XMI_Configuration_query(client, &configuration);
  __barrier_task = configuration.value.intval;

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query(client, &configuration);
  __barrier_size = configuration.value.intval;

  __barrier_next_task = (__barrier_task + 1) % __barrier_size;
  __barrier_next_endpoint = XMI_Client_endpoint (client, __barrier_next_task, 0);

   TRACE_ERR((stderr,"__barrier_size:%d __barrier_task:%d\n",__barrier_size, __barrier_task));

  __barrier_context  = context;
  __barrier_dispatch = dispatch;
  __barrier_phase = 0;
  //__barrier_active[0].send = __barrier_size-1;
  __barrier_active[0] = __barrier_size-1;
  //__barrier_active[1].send = 0;
  __barrier_active[1] = __barrier_size-1;

  xmi_dispatch_callback_fn fn;
  fn.p2p = barrier_dispatch_function;
  xmi_send_hint_t options={0};
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
