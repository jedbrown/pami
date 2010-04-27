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

#include "sys/pami.h"
#include "util/common.h"

#ifdef __pami_target_bgq__
#ifdef ENABLE_MAMBO_WORKAROUNDS
  #include "Global.h"
#endif
#endif

#include <unistd.h>

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

#ifdef __pami_target_bgq__
#ifdef ENABLE_MAMBO_WORKAROUNDS
// sleep() doesn't appear to work in mambo right now.  A hackey simulation...
#define mamboSleep(x) _mamboSleep(x, __LINE__)
unsigned _mamboSleep(unsigned seconds, unsigned from)
{
  if (__global.personality._is_mambo)
  {
    double dseconds = ((double)seconds)/1000; //mambo seconds are loooong.
    double start = PAMI_Wtime (), d=0;
    while (PAMI_Wtime() < (start+dseconds))
    {
      for (int i=0; i<200000; ++i) ++d;
      TRACE_ERR((stderr, "%s:%d sleep - %.0f, start %f, %f < %f\n",__PRETTY_FUNCTION__,from,d,start,PAMI_Wtime(),start+dseconds));
    }
  }
  else
    sleep(seconds);
  return 0;
}
#endif
#endif

unsigned __barrier_active[2];
size_t __barrier_phase;
size_t __barrier_size;
size_t __barrier_task;
size_t __barrier_next_task;
pami_endpoint_t __barrier_next_endpoint;

size_t         __barrier_dispatch;
pami_context_t  __barrier_context;
pami_client_t   __barrier_client;

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
static void barrier_dispatch_function (
    pami_context_t        context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_recv_t         * recv)        /**< OUT: receive message structure */
{
  size_t phase = *((size_t *) header_addr);

  TRACE_ERR((stderr, ">>> barrier_dispatch_function(), __barrier_active[%zu] = %u\n", phase, __barrier_active[phase]));

  --__barrier_active[phase];

  // Forward this barrier notification to the next task.
  //TRACE_ERR((stderr, "    barrier_dispatch_function(), __barrier_active[%zu].send %% __barrier_size (%zu) = %zu\n", index, __barrier_size, __barrier_active[index].send % __barrier_size));
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

void barrier ()
{
  size_t i;
  TRACE_ERR((stderr, "#### enter barrier(),  ...\n"));

  __barrier_active[__barrier_phase] = __barrier_size-1;
  __barrier_phase = __barrier_phase^1;

  pami_send_immediate_t parameters;
  parameters.dispatch        = __barrier_dispatch;
  parameters.header.iov_base = &__barrier_phase;
  parameters.header.iov_len  = sizeof (__barrier_phase);
  parameters.data.iov_base   = NULL;
  parameters.data.iov_len    = 0;

  __barrier_next_task = __barrier_task; // start with me (will increment immediately below)

  for(i=1; i< __barrier_size; ++i)  /// \todo This doesn't scale but it's simple
  {
    __barrier_next_task = (__barrier_next_task + 1) % __barrier_size;
    __barrier_next_endpoint = PAMI_Endpoint_create (__barrier_client, __barrier_next_task, 0);
    parameters.dest            = __barrier_next_endpoint;

    TRACE_ERR((stderr, "     barrier(), before send, phase = %zu, __barrier_active[%zu] = %u, parameters.dest = 0x%08x\n", __barrier_phase, __barrier_phase, __barrier_active[__barrier_phase], parameters.dest));
    //pami_result_t result =
    PAMI_Send_immediate (__barrier_context, &parameters);
  }

  TRACE_ERR((stderr, " barrier() Before recv advance\n"));
  while (__barrier_active[__barrier_phase]  != 0)
    PAMI_Context_advance (__barrier_context, 100);

  TRACE_ERR((stderr, "####  exit barrier(), \n"));
  return;
}

void barrier_init (pami_client_t client, pami_context_t context, size_t dispatch)
{
  TRACE_ERR((stderr, "enter barrier_init() ...\n"));

  __barrier_client = client;

  pami_configuration_t configuration;

  configuration.name = PAMI_TASK_ID;
  pami_result_t result = PAMI_Configuration_query(client, &configuration);
  __barrier_task = configuration.value.intval;

  configuration.name = PAMI_NUM_TASKS;
  result = PAMI_Configuration_query(client, &configuration);
  __barrier_size = configuration.value.intval;

  TRACE_ERR((stderr,"__barrier_size:%zu __barrier_task:%zu\n",__barrier_size, __barrier_task));

  __barrier_context  = context;
  __barrier_dispatch = dispatch;
  __barrier_phase = 0;
  //__barrier_active[0].send = __barrier_size-1;
  __barrier_active[0] = __barrier_size-1;
  //__barrier_active[1].send = 0;
  __barrier_active[1] = __barrier_size-1;

  pami_dispatch_callback_fn fn;
  fn.p2p = barrier_dispatch_function;
  pami_send_hint_t options={0};
  TRACE_ERR((stderr, "Before PAMI_Dispatch_set() ...\n"));
  result = PAMI_Dispatch_set (context,
                             dispatch,
                             fn,
                             (void *)__barrier_active,
                             options);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
    PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
  }
#ifdef __pami_target_bgq__
#ifdef ENABLE_MAMBO_WORKAROUNDS
  // Give other tasks a chance to init the MU device by sleeping
  if (__global.personality._is_mambo) /// \todo mambo hack
  {
    fprintf(stderr, "%s:%s sleep(15) hack to allow mambo to init the MU\n",__FILE__,__PRETTY_FUNCTION__);
    mamboSleep(5);
  }
#endif
#endif
  barrier();
  TRACE_ERR((stderr, "... exit barrier_init()\n"));
}



#undef TRACE_ERR
#endif // __tests_util_h__
