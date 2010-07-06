///
/// \file test/api/p2p/send/long-header-hard-opp.c
/// \Matrixed "Long header" point-to-point PAMI_send() test
///
///   send hints       recv hints     header    exp
/// no long header | no long header    size    result   NOTES:
/// ============== | ============== | ====== | ====== | ================================
/// 0 (hard)       | 1 (hard)       | short  | pass   | Testsuite #1
/// 0 (hard)       | 1 (hard)       | long   | fail   | Test with mismatched hard hints
/// 1 (hard)       | 0 (hard)       | short  | pass   | Same dispatch ID with unique hints
/// 1 (hard)       | 0 (hard)       | long   | fail   |
/// 0 (soft)       | 0 (hard)       | short  | pass   | Testsuite #2
/// 0 (soft)       | 0 (hard)       | long   | pass   | Use soft hint to turn mismatched
/// 1 (soft)       | 1 (hard)       | short  | pass   | hard hints into matched hints
/// 1 (soft)       | 1 (hard)       | long   | fail   |

#include <pami.h>
#include <stdio.h>

//#define ENABLE_TRACE

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
    pami_endpoint_t origin,
pami_recv_t         * recv)        /**< OUT: receive message structure */
{
  TRACE((stderr, "Called dispatch function.  cookie = %p, active: %zu, header_addr = %p, header_size = %zu\n", cookie,  *((volatile size_t *) cookie), header_addr, header_size));

  recv->local_fn = recv_done;
  recv->cookie   = cookie;
  recv->kind = PAMI_AM_KIND_SIMPLE;
  recv->data.simple.addr  = NULL;
  recv->data.simple.bytes = 0;
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

  uint8_t short_header[128];
  uint8_t long_header[1024];

  // Create matrix controls and values
  size_t d = 0;                              // controls dispatch id/hard hint & soft hint
  size_t h = 0;                              // controls header pointer and header size arrays
  uint8_t * header_ary[2] = {short_header, long_header};
  size_t header_size_ary[2] = {128, 1024};
  size_t soft_hint_ary[2] = {1, 0};           // soft hint = opposite of hard hint
  char header_type_str[2][50] = {"short header", "long header"};
  pami_result_t mixd_expected_ary [2][2];  // expected results based on h & sending task values
  pami_result_t fixd_expected_ary [2][2];  // expected results based on h & sending task values
  mixd_expected_ary[0][0] = PAMI_SUCCESS;  // t0d0 no_long_header = 0, send short_header, t1d0 no_long_header = 1
  mixd_expected_ary[0][1] = PAMI_SUCCESS;  // t0d0 no_long_header = 0, send short_header, t1d0 no_long_header = 1
  mixd_expected_ary[1][0] = PAMI_ERROR;    // t0d0 no_long_header = 0, send long_header, t1d0 no_long_header = 1
  mixd_expected_ary[1][1] = PAMI_INVAL;    // t0d0 no_long_header = 0, send long_header, t1d0 no_long_header = 1
  fixd_expected_ary[0][0] = PAMI_SUCCESS;  // t0d0 no_long_header = 0 -> 1, send short_header, t1d0 no_long_header = 1
  fixd_expected_ary[0][1] = PAMI_SUCCESS;  // t0d0 no_long_header = 0, send short_header, t1d0 no_long_header = 1 -> 0
  fixd_expected_ary[1][0] = PAMI_INVAL;    // t0d0 no_long_header = 0 -> 1, send long_header, t1d0 no_long_header = 1
  fixd_expected_ary[1][1] = PAMI_SUCCESS;  // t0d0 no_long_header = 0, send long_header, t1d0 no_long_header = 1 -> 0

  pami_send_t parameters;
  parameters.send.data.iov_base   = NULL;
  parameters.send.data.iov_len    = 0;
  parameters.events.cookie        = (void *) &send_active;
  parameters.events.local_fn      = send_done_local;
  parameters.events.remote_fn     = send_done_remote;


  // ======== Testsuite #1 ========
  // Test results of sending short and long headers
  // when no_long_header is set to opposite values using hard hints

  if (task_id == 0)
  {

    // Create Dispatch ID 0 with no_long_header = 0 (opposite of ID 0 in task 1)
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

    for (d = 0; d < 1; d++) {
      TRACE((stderr, "before send ...\n"));
      parameters.send.dispatch        = d;
      for ( h = 0; h < 2; h++) {
	parameters.send.header.iov_base = (void *) header_ary[h];
	parameters.send.header.iov_len  = header_size_ary[h];
	PAMI_Endpoint_create (client, 1, 0, &parameters.send.dest);

	fprintf(stderr, "Sending %s (%zu bytes) from task %zu -> %zu:\n", &header_type_str[h][0], header_size_ary[h], task_id, soft_hint_ary[task_id]);
	fprintf(stderr, "task %zu no_long_header hard hint = %zu\n", task_id, task_id);
	fprintf(stderr, "task %zu no_long_header hard hint = %zu\n", soft_hint_ary[task_id], soft_hint_ary[task_id]);

	result = PAMI_Send (context, &parameters);
	if (result != mixd_expected_ary[h][task_id])
	{
	  fprintf(stderr, "Error. Expected rc = %d after sending a %s (%zu bytes) with task 0 hard no_long_header hint = %zu and task 1 hard no_long_header hint = %zu, but got rc = %d\n", mixd_expected_ary[h][task_id], &header_type_str[h][0], header_size_ary[h], task_id, soft_hint_ary[task_id], result);
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
    } // end dispatch id loop
  } // end task = 0
  else
  {

    // Create Dispatch ID 0 with no_long_header = 1 (opposite of ID 0 in task 0)
    options.no_long_header = 1;
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

    for (d = 0; d < 1; d++) {
      for ( h = 0; h < 2; h++) {
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
	parameters.send.dispatch = d;
	parameters.send.header.iov_base = (void *) header_ary[h];
	parameters.send.header.iov_len  = header_size_ary[h];
	PAMI_Endpoint_create (client, 0, 0, &parameters.send.dest);

	fprintf(stderr, "Sending %s (%zu bytes) from task %zu -> %zu:\n", &header_type_str[h][0], header_size_ary[h], task_id, soft_hint_ary[task_id]);
	fprintf(stderr, "task %zu no_long_header hard hint = %zu\n", task_id, task_id);
	fprintf(stderr, "task %zu no_long_header hard hint = %zu\n", soft_hint_ary[task_id], soft_hint_ary[task_id]);

	result = PAMI_Send (context, &parameters);
	if (result != mixd_expected_ary[h][task_id])
	{
	  fprintf(stderr, "Error. Expected rc = %d after sending a %s (%zu bytes) with task 1 hard no_long_header hint = %zu and task 0 hard no_long_header hint = %zu, but got rc = %d\n", mixd_expected_ary[h][task_id], &header_type_str[h][0], header_size_ary[h], task_id, soft_hint_ary[task_id], result);
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
    } // end dispatch loop
  } // end task = 1 loop

  // ======== Testsuite #2 ========
  // Test results of sending short and long headers
  // when no_long_headers is set to opposite values using hard hints
  // and soft hints are used to make the no_long_headers hints match

  if (task_id == 0)
  {
    for (d = 0; d < 1; d++) {
      TRACE((stderr, "before send ...\n"));
      parameters.send.dispatch        = d;
      for ( h = 0; h < 2; h++) { // 0 = short_header, 1 = long_header
	parameters.send.header.iov_base = (void *) header_ary[h];
	parameters.send.header.iov_len  = header_size_ary[h];
	PAMI_Endpoint_create (client, 1, 0, &parameters.send.dest);
	// Use soft hint to match Dispatch ID 0 in task 0
	parameters.send.hints.no_long_header = soft_hint_ary[task_id];

	fprintf(stderr, "Sending %s (%zu bytes) from task %zu -> %zu:\n", &header_type_str[h][0], header_size_ary[h], task_id, soft_hint_ary[task_id]);
	fprintf(stderr, "task %zu no_long_header hard hint = %zu\n", task_id, task_id);
	fprintf(stderr, "task %zu no_long_header soft hint = %zu\n", task_id, soft_hint_ary[task_id]);
	fprintf(stderr, "task %zu no_long_header hard hint = %zu\n", soft_hint_ary[task_id], soft_hint_ary[task_id]);

	result = PAMI_Send (context, &parameters);
	if (result != fixd_expected_ary[h][task_id])
	{
	  fprintf(stderr, "Error. Expected rc = %d after sending a %s (%zu bytes) with: task 0 hard no_long_header hint = %zu, task 1 hard no_long_header = %zu and task 0 soft no_long_header hint = %zu, but got rc = %d\n", fixd_expected_ary[h][task_id], &header_type_str[h][0], header_size_ary[h], task_id, soft_hint_ary[task_id], soft_hint_ary[task_id], result);
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
    } // end dispatch id loop
  } // end task = 0
  else
  {
    for (d = 0; d < 1; d++) {
      for ( h = 0; h < 2; h++) {
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
	parameters.send.dispatch = d;
	parameters.send.header.iov_base = (void *) header_ary[h];
	parameters.send.header.iov_len  = header_size_ary[h];
	PAMI_Endpoint_create (client, 0, 0, &parameters.send.dest);
	parameters.send.hints.no_long_header = soft_hint_ary[task_id];

	fprintf(stderr, "Sending %s (%zu bytes) from task %zu -> %zu:\n", &header_type_str[h][0], header_size_ary[h], task_id, soft_hint_ary[task_id]);
	fprintf(stderr, "task %zu no_long_header hard hint = %zu\n", task_id, task_id);
	fprintf(stderr, "task %zu no_long_header soft hint = %zu\n", task_id, soft_hint_ary[task_id]);
	fprintf(stderr, "task %zu no_long_header hard hint = %zu\n", soft_hint_ary[task_id], soft_hint_ary[task_id]);

	result = PAMI_Send (context, &parameters);
	if (result != fixd_expected_ary[h][task_id])
	{
	  fprintf(stderr, "Error. Expected rc = %d after sending a %s (%zu bytes) with: task 1 hard no_long_header hint = %zu, task 0 hard no_long_header = %zu and task 1 soft no_long_header hint = %zu, but got rc = %d\n", fixd_expected_ary[h][task_id], &header_type_str[h][0], header_size_ary[h], task_id, soft_hint_ary[task_id], soft_hint_ary[task_id], result);
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
    } // end dispatch loop
  } // end task = 1 loop

  result = PAMI_Context_destroyv(&context, 1);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to destroy pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy(&client);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to finalize pami client. result = %d\n", result);
    return 1;
  }

  return 0;
};
