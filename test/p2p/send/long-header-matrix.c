///
/// \file test/p2p/send/long-header-matrix.c
/// \Matrixed "Long header" point-to-point PAMI_send() test
///
///   send hints       recv hints     header    exp
/// no long header | no long header    size    result   NOTES:
/// ============== | ============== | ====== | ====== | ================================
/// 0 (hard)       | 0 (hard)       | short  | pass   |


#include <pami.h>
#include <stdio.h>

#define ENABLE_TRACE

#ifdef ENABLE_TRACE
#define TRACE(x) fprintf x
#else
#define TRACE(x)
#endif

static void recv_done (pami_context_t   context,
                       void          * cookie,
                       pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE((stderr, "Called recv_done function.  active: %zu -> %zu\n", *active, *active-1));
  (*active)--;
}

static void test_dispatch (
    pami_context_t        context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    const void         * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    const void         * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_recv_t         * recv)        /**< OUT: receive message structure */
{
  TRACE((stderr, "Called dispatch function.  cookie = %p, active: %zu, header_addr = %p, header_size = %zu\n", cookie,  *((volatile size_t *) cookie), header_addr, header_size));

  recv->local_fn = recv_done;
  recv->cookie   = cookie;
  recv->type     = PAMI_BYTE;
  recv->addr     = NULL;
  recv->offset   = 0;
  TRACE((stderr, "... dispatch function.  recv->local_fn = %p\n", recv->local_fn));

  return;
}

static void send_done_local (pami_context_t   context,
                             void          * cookie,
                             pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE((stderr, "Called send_done_local function.  active: %zu -> %zu\n", *active, *active-1));
  (*active)--;
}

static void send_done_remote (pami_context_t   context,
                              void          * cookie,
                              pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  TRACE((stderr, "Called send_done_remote function.  active: %zu -> %zu\n", *active, *active-1));
  (*active)--;
}

int main (int argc, char ** argv)
{
  volatile size_t send_active = 2;
  volatile size_t recv_active = 1;


  pami_client_t client;
  pami_context_t context;
  char                  cl_string[] = "TEST";
  pami_result_t result = PAMI_ERROR;

  result = PAMI_Client_create (cl_string, &client);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }

        {  result = PAMI_Context_createv(client, NULL, 0, &context, 1); }
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to create pami context. result = %d\n", result);
    return 1;
  }

  pami_configuration_t configuration;

  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  TRACE((stderr, "My task id = %zu\n", task_id));

  configuration.name = PAMI_NUM_TASKS;
  result = PAMI_Configuration_query(client, &configuration);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  TRACE((stderr, "Number of tasks = %zu\n", num_tasks));
  if (num_tasks != 2)
  {
    fprintf(stderr, "Error. This test requires 2 tasks. Number of tasks in this job: %zu\n", num_tasks);
    return 1;
  }

  pami_send_hint_t options;
  pami_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;

  options.no_long_header = 0;
  TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
  result = PAMI_Dispatch_set (context,
                             0,
                             fn,
                             (void *)&recv_active,
                             options);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
    return 1;
  }

  options.no_long_header = 1;
  TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
  result = PAMI_Dispatch_set (context,
                             3,
                             fn,
                             (void *)&recv_active,
                             options);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
    return 1;
  }
  size_t n = 0;                              // controls task loop
  size_t send_hard_hint = 0;                 // hard hint value of sending task
  size_t send_soft_hint = 0;                 // soft hint value of sending task
  size_t recv_hard_hint = 0;                 // hard hint value of receiving task

  // Create header parms
  uint8_t short_header[128];
  uint8_t long_header[1024];
  size_t h = 0;                              // controls header pointer and header size arrays
  uint8_t * header_ary[2] = {short_header, long_header};
  size_t header_size_ary[2] = {128, 1024};
  char header_type_str[2][50] = {"short header", "long header"};

  size_t testcase = 0;
  size_t dispatch_ary_0[6] = {0, 1, 0, 1, 2, 3};
  size_t dispatch_ary_n[6] = {0, 2, 0, 2, 1, 3};

  pami_send_t parameters;
  parameters.send.data.iov_base   = NULL;
  parameters.send.data.iov_len    = 0;
  parameters.events.cookie        = (void *) &send_active;
  parameters.events.local_fn      = send_done_local;
  parameters.events.remote_fn     = send_done_remote;

  // ======== Combinations of header sizes and no_long_header hints that should pass  ========

  if (task_id == 0)
  {

    fprintf(stderr, "====== Combinations of header sizes and no_long_header hints that should pass ======\n");

    // Create task unique dispatch sets 1 & 2
    options.no_long_header = 0;
    TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
    result = PAMI_Dispatch_set (context,
				1,
				fn,
				(void *)&recv_active,
				options);
    if (result != PAMI_SUCCESS)
    {
      fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }

    options.no_long_header = 1;
    TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
    result = PAMI_Dispatch_set (context,
				2,
				fn,
				(void *)&recv_active,
				options);
    if (result != PAMI_SUCCESS)
    {
      fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }

    for (testcase = 0; testcase < 6; testcase++) {

      // Determine hint values
      send_hard_hint = (testcase >> 2) & 1;
      send_soft_hint = (testcase >> 1) & 1;
      recv_hard_hint = testcase & 1;

      parameters.send.dispatch        = dispatch_ary_0[testcase];

      // Communicate with each task
      for (n = 1; n < num_tasks; n++) {

	TRACE((stderr, "before send ...\n"));

	result = PAMI_Endpoint_create (client, n, 0, &parameters.send.dest);
	if (result != PAMI_SUCCESS) {
	  fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed with %d.\n", result);
	  return 1;
	}

	for ( h = 0; h < 2; h++) {

	  // Long headers only work for testcases 0 & 2
	  if ( h && ((testcase != 0) && (testcase != 2)) ) {
	    continue;
	  }

	  parameters.send.header.iov_base = (void *) header_ary[h];
	  parameters.send.header.iov_len  = header_size_ary[h];

	  fprintf(stderr, "Sending %s (%zu bytes) from task %zu -> %zu:\n\t\ttask %zu no_long_header hard hint = %zu\n\t\ttask %zu no_long_header soft hint = %zu\n\t\ttask %zu no_long_header hard hint = %zu\n", &header_type_str[h][0], header_size_ary[h], task_id, n, task_id, send_hard_hint, task_id, send_soft_hint, n, recv_hard_hint);

	  result = PAMI_Send (context, &parameters);
	  if (result != PAMI_SUCCESS)
	  {
	    fprintf(stderr, "Error. Sent a %s (%zu bytes) from task %zu (no_long_header hard hint = %zu, soft hint = %zu) to task %zu (no_long_header hard hint = %zu) and FAILED wth rc = %d\n", &header_type_str[h][0], header_size_ary[h], task_id, send_hard_hint, send_soft_hint, n, recv_hard_hint, result);
	    return 1;
	  }
	  TRACE((stderr, "... after send.\n"));


	  TRACE((stderr, "before send-recv advance loop ...\n"));
	  while (send_active || recv_active)
	  {
	    result = PAMI_Context_advance (context, 100);
	    if (result != PAMI_SUCCESS)
	      {
		fprintf(stderr, "Error. Unable to advance pami context. result = %d\n", result);
		return 1;
	      }
	  }
	  TRACE((stderr, "... after send-recv advance loop\n"));
	  send_active = 2;
	  recv_active = 1;
	} // end header loop
      } // end task id loop
    } // end testcase loop
  } // end task = 0
  else
  {

    // Create task unique dispatch sets 1 & 2
    options.no_long_header = 1;
    TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
    result = PAMI_Dispatch_set (context,
				1,
				fn,
				(void *)&recv_active,
				options);
    if (result != PAMI_SUCCESS)
    {
      fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }

    options.no_long_header = 0;
    TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
    result = PAMI_Dispatch_set (context,
				2,
				fn,
				(void *)&recv_active,
				options);
    if (result != PAMI_SUCCESS)
    {
      fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }

    result = PAMI_Endpoint_create (client, 0, 0, &parameters.send.dest);
    if (result != PAMI_SUCCESS) {
      fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed with %d.\n", result);
      return 1;
    }

    for (testcase = 0; testcase < 6; testcase++) {

      TRACE((stderr, "before send ...\n"));

      // Determine hint values
      send_hard_hint = (testcase >> 2) & 1;
      send_soft_hint = (testcase >> 1) & 1;
      recv_hard_hint = testcase & 1;

      parameters.send.dispatch = dispatch_ary_n[testcase];

      for ( h = 0; h < 2; h++) {

	// Long headers only work for testcases 0 & 2
	if ( h && ((testcase != 0) && (testcase != 2)) ) {
	  continue;
	}

	TRACE((stderr, "before recv advance loop ...\n"));
	while (recv_active != 0)
	{
	  result = PAMI_Context_advance (context, 100);
	  if (result != PAMI_SUCCESS)
	  {
	    fprintf(stderr, "Error. Unable to advance pami context. result = %d\n", result);
	    return 1;
	  }
	}
	TRACE((stderr, "... after recv advance loop\n"));
	recv_active = 1;

	TRACE((stderr, "before send ...\n"));

	parameters.send.header.iov_base = (void *) header_ary[h];
	parameters.send.header.iov_len  = header_size_ary[h];

	fprintf(stderr, "Sending %s (%zu bytes) from task %zu -> 0:\n\t\ttask %zu no_long_header hard hint = %zu\n\t\ttask %zu no_long_header soft hint = %zu\n\t\ttask 0 no_long_header hard hint = %zu\n", &header_type_str[h][0], header_size_ary[h], task_id, task_id, send_hard_hint, task_id, send_soft_hint, recv_hard_hint);

	result = PAMI_Send (context, &parameters);
	if (result != PAMI_SUCCESS)
	{
	  fprintf(stderr, "Error. Sent a %s (%zu bytes) from task %zu (no_long_header hard hint = %zu, soft hint = %zu) to task 0 (no_long_header hard hint = %zu) and FAILED wth rc = %d\n", &header_type_str[h][0], header_size_ary[h], task_id, send_hard_hint, send_soft_hint, recv_hard_hint, result);
	  return 1;
	}
	TRACE((stderr, "... after send.\n"));

	TRACE((stderr, "before send advance loop ...\n"));
	while (send_active)
	{
	  result = PAMI_Context_advance (context, 100);
	  if (result != PAMI_SUCCESS)
	  {
	    fprintf(stderr, "Error. Unable to advance pami context. result = %d\n", result);
	    return 1;
	  }
	}
	TRACE((stderr, "... after send advance loop\n"));
	send_active = 2;
      } // end header loop
    } // end testcase loop
  } // end task != 0

  // ======== Combinations of header sizes and no_long_header hints that should result in send FAILS  ========

  if (task_id == 0)
  {

    fprintf(stderr, "====== Combinations of header sizes and no_long_header hints that should result in send FAILS ======\n");

    // Only sending long headers
    h = 1;
    parameters.send.header.iov_base = (void *) header_ary[h];
    parameters.send.header.iov_len  = header_size_ary[h];

    for (testcase = 4; testcase < 6; testcase++) {

      // Determine hint values
      send_hard_hint = (testcase >> 2) & 1;
      send_soft_hint = (testcase >> 1) & 1;
      recv_hard_hint = testcase & 1;

      parameters.send.dispatch        = dispatch_ary_0[testcase];

      // Communicate with each task
      for (n = 1; n < num_tasks; n++) {

	TRACE((stderr, "before send ...\n"));

	result = PAMI_Endpoint_create (client, n, 0, &parameters.send.dest);
	if (result != PAMI_SUCCESS) {
	  fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed with %d.\n", result);
	  return 1;
	}

	fprintf(stderr, "Sending %s (%zu bytes) from task %zu -> %zu:\n\t\ttask %zu no_long_header hard hint = %zu\n\t\ttask %zu no_long_header soft hint = %zu\n\t\ttask %zu no_long_header hard hint = %zu\n", &header_type_str[h][0], header_size_ary[h], task_id, n, task_id, send_hard_hint, task_id, send_soft_hint, n, recv_hard_hint);

	result = PAMI_Send (context, &parameters);
	if (result != PAMI_INVAL)
	{
	  fprintf(stderr, "Error. Sent a %s (%zu bytes) from task %zu (no_long_header hard hint = %zu, soft hint = %zu) to task %zu (no_long_header hard hint = %zu).  Expected rc = %d, but got rc = %d\n", &header_type_str[h][0], header_size_ary[h], task_id, send_hard_hint, send_soft_hint, n, recv_hard_hint, PAMI_INVAL, result);
	  return 1;
	}
	TRACE((stderr, "... after send.\n"));
      } // end task id loop
    } // end testcase loop
  } // end task = 0
  else
  {

    // Only sending long headers
    h = 1;
    parameters.send.header.iov_base = (void *) header_ary[h];
    parameters.send.header.iov_len  = header_size_ary[h];

    result = PAMI_Endpoint_create (client, 0, 0, &parameters.send.dest);
    if (result != PAMI_SUCCESS) {
      fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed with %d.\n", result);
      return 1;
    }

    for (testcase = 4; testcase < 6; testcase++) {

      TRACE((stderr, "before send ...\n"));

      // Determine hint values
      send_hard_hint = (testcase >> 2) & 1;
      send_soft_hint = (testcase >> 1) & 1;
      recv_hard_hint = testcase & 1;

      parameters.send.dispatch        = dispatch_ary_n[testcase];

      fprintf(stderr, "Sending %s (%zu bytes) from task %zu -> 0:\n\t\ttask %zu no_long_header hard hint = %zu\n\t\ttask %zu no_long_header soft hint = %zu\n\t\ttask 0 no_long_header hard hint = %zu\n", &header_type_str[h][0], header_size_ary[h], task_id, task_id, send_hard_hint, task_id, send_soft_hint, recv_hard_hint);

      result = PAMI_Send (context, &parameters);
      if (result != PAMI_INVAL)
      {
	fprintf(stderr, "Error. Sent a %s (%zu bytes) from task %zu (no_long_header hard hint = %zu, soft hint = %zu) to task 0 (no_long_header hard hint = %zu). Expected rc = %d, but got rc = %d\n", &header_type_str[h][0], header_size_ary[h], task_id, send_hard_hint, send_soft_hint, recv_hard_hint, PAMI_INVAL, result);
	return 1;
      }
      TRACE((stderr, "... after send.\n"));
    } // end testcase loop
  } // end task = 1 loop

  // ======== Combinations of header sizes and no_long_header hints that should result in recv FAILS  ========

  if (task_id == 0)
  {

    fprintf(stderr, "====== Combinations of header sizes and no_long_header hints that should result in recv FAILS ======\n");

    // Only sending long headers
    h = 1;
    parameters.send.header.iov_base = (void *) header_ary[h];
    parameters.send.header.iov_len  = header_size_ary[h];

    for (testcase = 1; testcase < 4; testcase++) {

      // Skip testcase #2, (sending FAIL)
      if (testcase == 2) {
	continue;
      }

      // Determine hint values
      send_hard_hint = (testcase >> 2) & 1;
      send_soft_hint = (testcase >> 1) & 1;
      recv_hard_hint = testcase & 1;

      fprintf (stderr, "testcase = %zu, dispatch = %zu.\n", testcase, dispatch_ary_0[testcase]);

      parameters.send.dispatch        = dispatch_ary_0[testcase];

      // Communicate with each task
      for (n = 1; n < num_tasks; n++) {

	TRACE((stderr, "before send ...\n"));


	result = PAMI_Endpoint_create (client, n, 0, &parameters.send.dest);
	if (result != PAMI_SUCCESS) {
	  fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed with %d.\n", result);
	  return 1;
	}

	fprintf(stderr, "Sending %s (%zu bytes) from task %zu -> %zu:\n\t\ttask %zu no_long_header hard hint = %zu\n\t\ttask %zu no_long_header soft hint = %zu\n\t\ttask %zu no_long_header hard hint = %zu\n", &header_type_str[h][0], header_size_ary[h], task_id, n, task_id, send_hard_hint, task_id, send_soft_hint, n, recv_hard_hint);

	result = PAMI_Send (context, &parameters);
	if (result != PAMI_SUCCESS)
	{
	  fprintf(stderr, "Error. Sent a %s (%zu bytes) from task %zu (no_long_header hard hint = %zu, soft hint = %zu) to task %zu (no_long_header hard hint = %zu) and FAILED with rc = %d\n", &header_type_str[h][0], header_size_ary[h], task_id, send_hard_hint, send_soft_hint, n, recv_hard_hint, result);
	  return 1;
	}
	TRACE((stderr, "... after send.\n"));

	TRACE((stderr, "before send-recv advance loop ...\n"));
	while (send_active || recv_active)
	{
	  result = PAMI_Context_advance (context, 100);
	  if (result != PAMI_SUCCESS)
	  {
	    fprintf(stderr, "Error. Unable to advance pami context. result = %d\n", result);
	    return 1;
	  }
	}
	TRACE((stderr, "... after send-recv advance loop\n"));
	send_active = 2;
	recv_active = 1;
      } // end task id loop
    } // end testcase loop
  } // end task = 0
  else
  {

    // Only sending long headers
    h = 1;
    parameters.send.header.iov_base = (void *) header_ary[h];
    parameters.send.header.iov_len  = header_size_ary[h];

    result = PAMI_Endpoint_create (client, 0, 0, &parameters.send.dest);
    if (result != PAMI_SUCCESS) {
      fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed with %d.\n", result);
      return 1;
    }

    for (testcase = 1; testcase < 4; testcase++) {

      // Skip testcase #2, (sending FAIL)
      if (testcase == 2) {
	continue;
      }

      TRACE((stderr, "before recv advance loop ...\n"));
      while (recv_active != 0)
      {
	result = PAMI_Context_advance (context, 100);
	if (result != PAMI_SUCCESS)
	{
	  fprintf(stderr, "Error. Unable to advance pami context. result = %d\n", result);
	  return 1;
	}
      }
      TRACE((stderr, "... after recv advance loop\n"));
      recv_active = 1;

      TRACE((stderr, "before send ...\n"));

      // Determine hint values
      send_hard_hint = (testcase >> 2) & 1;
      send_soft_hint = (testcase >> 1) & 1;
      recv_hard_hint = testcase & 1;

      fprintf (stderr, "testcase = %zu, dispatch = %zu.\n", testcase, dispatch_ary_n[testcase]);

      parameters.send.dispatch        = dispatch_ary_n[testcase];

      fprintf(stderr, "Sending %s (%zu bytes) from task %zu -> 0:\n\t\ttask %zu no_long_header hard hint = %zu\n\t\ttask %zu no_long_header soft hint = %zu\n\t\ttask 0 no_long_header hard hint = %zu\n", &header_type_str[h][0], header_size_ary[h], task_id, task_id, send_hard_hint, task_id, send_soft_hint, recv_hard_hint);

      result = PAMI_Send (context, &parameters);
      if (result != PAMI_SUCCESS)
      {
	fprintf(stderr, "Error. Sent a %s (%zu bytes) from task %zu (no_long_header hard hint = %zu, soft hint = %zu) to task 0 (no_long_header hard hint = %zu) and FAILED with rc = %d\n", &header_type_str[h][0], header_size_ary[h], task_id, send_hard_hint, send_soft_hint, recv_hard_hint, result);
	return 1;
      }
      TRACE((stderr, "... after send.\n"));

      TRACE((stderr, "before send advance loop ...\n"));
      while (send_active)
      {
	result = PAMI_Context_advance (context, 100);
	if (result != PAMI_SUCCESS)
	{
	  fprintf(stderr, "Error. Unable to advance pami context. result = %d\n", result);
	  return 1;
	}
      }
      TRACE((stderr, "... after send advance loop\n"));
      send_active = 2;
    } // end testcase loop
  } // end task = 1 loop

  // ======== Cleanup ========

  result = PAMI_Context_destroyv(&context, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to destroy pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy (client);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to finalize pami client. result = %d\n", result);
    return 1;
  }

  return 0;
};
