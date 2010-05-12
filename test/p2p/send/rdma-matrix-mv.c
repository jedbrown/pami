///
/// \file test/p2p/send/rdma-matrix-mv.c
/// \Matrixed "rdma" point-to-point PAMI_send() test
///

#include "sys/pami.h"
#include <stdio.h>

//      PAMI_HINT_DEFAULT = 0, This hint leaves the option up to the PAMI implementation to choose.
//      PAMI_HINT_FORCE   = 1, This allows the user to force an option to be used.
//      PAMI_HINT_DENY    = 2, The user can force the implementation to not use this option.
//      PAMI_HINT_MAX     = 3, The largest hint value; it cannot actually be used.

//#define ENABLE_TRACE

#ifdef ENABLE_TRACE
#define TRACE(x) fprintf x
#else
#define TRACE(x)
#endif

uint8_t __recv_buffer[2048];
char recv_str[2048];               // used to print __recv_buffer as string
size_t __recv_size;
size_t __data_errors;
uint8_t reset_value[2] ={0, 255};  // reset value for each byte of __recv_buffer ...all 0's or all 1's (255)
size_t r = 0;                      // used to loop over reset values

unsigned validate (void * addr, size_t bytes, size_t test_n_plus_1)
{
  unsigned status = 1;
  uint8_t * byte = (uint8_t *) addr;
  size_t i, j = 0;

  for (i=0; i<bytes; i++)
  {
    if (byte[i] != (uint8_t)i)
    {
      fprintf (stderr, "validate(%p,%zu) .. ERROR .. byte[%zu] != %d (&byte[%zu] = %p, value is %d)\n", addr, bytes, i, (uint8_t)i, i, &byte[i], byte[i]);
      status = 0;
    }

  // Print element to string to print later if desired
    sprintf(&recv_str[j], "%d", byte[i]);
    if (byte[i] < 10) {
      j++;
    } else if (byte[i] < 100) {
      j+=2;
    } else {
      j+=3;
    }
  }

  // Add n-plus-1 byte to end of string so we can see what the default was
  sprintf(&recv_str[j], "%d", byte[i]);

  // Validate n-plus-1 byte (ensure it equals default value)
  if (test_n_plus_1) {
    if (byte[bytes] != reset_value[r]) {
      fprintf (stderr, "validate(%p,%zu) .. ERROR .. byte[%zu] != %d (&byte[%zu] = %p, value is %d)\n", addr, bytes, bytes, reset_value[r], bytes, &byte[bytes], byte[bytes]);
      status = 0;
    }
  }

  // Print __recv_buffer
  fprintf(stdout, "recv buffer[0:%zu] after send: %s\n", bytes, recv_str);

  return status;
}


static void recv_done (pami_context_t   context,
                       void          * cookie,
                       pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called recv_done function.  active(%p): %zu -> %zu, __recv_size = %zu\n", active, *active, *active-1, __recv_size);

  if (!validate(__recv_buffer, __recv_size, 1))
  {
    __data_errors++;
    fprintf (stderr, "validate data ERROR!\n");
  }

  (*active)--;
}

static void test_dispatch (
    pami_context_t        context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_recv_t         * recv)        /**< OUT: receive message structure */
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called dispatch function.  cookie = %p, active: %zu, header_size = %zu, pipe_size = %zu\n", cookie, *active, header_size, pipe_size);
  //(*active)--;
  //fprintf (stderr, "... dispatch function.  active = %zu\n", *active);

  if (!validate(header_addr, header_size, 0))
    fprintf (stderr, "validate header ERROR!\n");

  if (pipe_size == 0)
  {
    (*active)--;
  }
  else
  {
    __recv_size = pipe_size;

    recv->local_fn = recv_done;
    recv->cookie   = cookie;
    recv->kind = PAMI_AM_KIND_SIMPLE;
    recv->data.simple.addr  = __recv_buffer;
    recv->data.simple.bytes = pipe_size;
    //fprintf (stderr, "... dispatch function.  recv->local_fn = %p\n", recv->local_fn);
  }

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

        { size_t _n = 1; result = PAMI_Context_createv(client, NULL, 0, &context, _n); }
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
  /*  if (num_tasks != 2)
  {
    fprintf(stderr, "Error. This test requires 2 tasks. Number of tasks in this job: %zu\n", num_tasks);
    return 1;
    }*/

  pami_send_hint_t options;
  pami_dispatch_callback_fn fn;
  fn.p2p = test_dispatch;

  options.use_rdma = 0;
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

  options.use_rdma = 1;
  TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
  result = PAMI_Dispatch_set (context,
                             5,
                             fn,
                             (void *)&recv_active,
                             options);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
    return 1;
  }

  options.use_rdma = 2;
  TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
  result = PAMI_Dispatch_set (context,
                             10,
                             fn,
                             (void *)&recv_active,
                             options);
  if (result != PAMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
    return 1;
  }

  options.use_rdma = 3;
  TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
  result = PAMI_Dispatch_set (context,
                             3,
                             fn,
                             (void *)&recv_active,
                             options);
  if (result != PAMI_INVAL)
  {
    fprintf(stderr, "Error. Should not be able to set use_rdma >= 3. result = %d\n", result);
    return 1;
  }

  // Setup header and data vars
  size_t i = 0;
  uint8_t header[1024];
  uint8_t data[1024];
  for (i=0; i<1024; i++)
  {
    header[i] = (uint8_t)i;
    data[i]   = (uint8_t)i;
  }

  size_t hsize = 0;
  size_t header_bytes[16];
  header_bytes[hsize++] = 0;
  header_bytes[hsize++] = 16;
  header_bytes[hsize++] = 32;

  size_t psize = 0;
  size_t data_bytes[16];
  //data_bytes[psize++] = 0;
  //data_bytes[psize++] = 16;
  //data_bytes[psize++] = 32;
  //data_bytes[psize++] = 64;
  data_bytes[psize++] = 128;
  data_bytes[psize++] = 256;
  data_bytes[psize++] = 512;
  data_bytes[psize++] = 1024;

  // Setup hint vars
  size_t n = 0;                              // controls task loop
  size_t s = 0;                              // controls send hints loop
  size_t r = 0;                              // controls recv hints loop
  size_t send_hard_use_rdma_hint = 0;       // hard hint value of sending task
  size_t send_soft_use_rdma_hint = 0;       // soft hint value of sending task
  size_t send_hard_no_rdma_hint = 0;        // hard hint value of sending task
  size_t send_soft_no_rdma_hint = 0;        // soft hint value of sending task
  size_t recv_use_rdma_hint = 0;            // hard hint value of receiving task
  size_t recv_no_rdma_hint = 0;             // hard hint value of receiving task

  // Create dispatch arrays

  size_t dispatch_ary_0[3][12] = {{0, 0, 0, 0, 4, 0, 4, 4, 8, 8, 0, 8},
                                  {1, 1, 1, 1, 5, 0, 5, 5, 9, 9, 0, 9},
                                  {2, 2, 2, 2, 6, 0, 6, 6, 10, 10, 0, 10}};
  size_t dispatch_ary_n[3][12] = {{0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 0, 2},
                                  {4, 4, 4, 4, 5, 0, 5, 5, 6, 6, 0, 6},
                                  {8, 8, 8, 8, 9, 0, 9, 9, 10, 10, 0, 10}};

  // Init to skip failing scenarios ...enable failing scenarios later
  size_t skip_ary[3][12] =       {{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0},
                                  {0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1},
                                  {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0}};

  pami_send_t parameters;
  parameters.send.header.iov_base = header;
  parameters.send.header.iov_len  = header_bytes[1]; // 16 bytes
  parameters.send.data.iov_base   = data;
  parameters.send.data.iov_len    = data_bytes[2]; // 512 bytes
  parameters.events.cookie        = (void *) &send_active;
  parameters.events.local_fn      = send_done_local;
  parameters.events.remote_fn     = send_done_remote;

  // ======== Combinations of use rdma and no rdma hints that should pass  ========

  if (task_id == 0)
  {

    fprintf(stderr, "======== Combinations of use rdma and no rdma hints that should pass  ========\n");

    // Create task unique dispatch sets
    options.use_rdma = 0;
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

    options.use_rdma = 0;
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

    options.use_rdma = 1;
    TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
    result = PAMI_Dispatch_set (context,
				4,
				fn,
				(void *)&recv_active,
				options);
    if (result != PAMI_SUCCESS)
    {
      fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }

    options.use_rdma = 1;
    TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
    result = PAMI_Dispatch_set (context,
				6,
				fn,
				(void *)&recv_active,
				options);
    if (result != PAMI_SUCCESS)
    {
      fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }

    options.use_rdma = 2;
    TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
    result = PAMI_Dispatch_set (context,
				8,
				fn,
				(void *)&recv_active,
				options);
    if (result != PAMI_SUCCESS)
    {
      fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }

    options.use_rdma = 2;
    TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
    result = PAMI_Dispatch_set (context,
				9,
				fn,
				(void *)&recv_active,
				options);
    if (result != PAMI_SUCCESS)
    {
      fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }

    for (r = 0; r < 3; r++) {
      for (s = 0; s < 12; s++) {

	// Skip scenarios that are expected to fail
	if (skip_ary[r][s]) {
	  continue;
	}

	// Determine hint values
	send_hard_use_rdma_hint = (s >> 2) & 3;
	send_soft_use_rdma_hint = s & 3;
	recv_use_rdma_hint = r;

	parameters.send.dispatch        = dispatch_ary_0[r][s];

	// Communicate with each task
	for (n = 1; n < num_tasks; n++) {

	  TRACE((stderr, "before send ...\n"));

 	  result = PAMI_Endpoint_create (client, n, 0, &parameters.send.dest);
 	  if (result != PAMI_SUCCESS) {
 	    fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed with %d.\n", result);
 	    return 1;
 	  }

	  fprintf(stderr, "Sending %zu byte header and %zu byte data from task %zu -> %zu:\n\t\ttask %zu use_rdma hard hint = %zu\n\t\ttask %zu use_rdma soft hint = %zu\n\ttask %zu use rdma hard hint = %zu\n", header_bytes[1], data_bytes[2], task_id, n, task_id, send_hard_use_rdma_hint, task_id, send_soft_use_rdma_hint, n, recv_use_rdma_hint);

	  result = PAMI_Send (context, &parameters);
	  if (result != PAMI_SUCCESS)
	  {
	    fprintf(stderr, "Error. Sent %zu byte header and %zu byte data from task %zu (use_rdma hard hint = %zu, use_rdma soft hint = %zu) to task %zu (use_rdma hard hint = %zu) and FAILED wth rc = %d\n",header_bytes[1], data_bytes[2], task_id, send_hard_use_rdma_hint, send_soft_use_rdma_hint, n, recv_use_rdma_hint, result);
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
	} // end task loop
      } // end send hint loop
    } // end recv hint loop
  } // end task = 0
  else
  {

    // Create task unique dispatch sets
    options.use_rdma = 1;
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

    options.use_rdma = 2;
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

    options.use_rdma = 0;
    TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
    result = PAMI_Dispatch_set (context,
				4,
				fn,
				(void *)&recv_active,
				options);
    if (result != PAMI_SUCCESS)
    {
      fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }

    options.use_rdma = 2;
    TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
    result = PAMI_Dispatch_set (context,
				6,
				fn,
				(void *)&recv_active,
				options);
    if (result != PAMI_SUCCESS)
    {
      fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }

    options.use_rdma = 0;
    TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
    result = PAMI_Dispatch_set (context,
				8,
				fn,
				(void *)&recv_active,
				options);
    if (result != PAMI_SUCCESS)
    {
      fprintf(stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }

    options.use_rdma = 1;
    TRACE((stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active));
    result = PAMI_Dispatch_set (context,
				9,
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

    for (r = 0; r < 3; r++) {
      for (s = 0; s < 12; s++) {

	// Skip scenarios that are expected to fail
	if (skip_ary[r][s]) {
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
	send_hard_use_rdma_hint = (s >> 2) & 3;
	send_soft_use_rdma_hint = (s & 3;
	recv_use_rdma_hint = r;

	parameters.send.dispatch        = dispatch_ary_n[r][s];

	fprintf(stderr, "Sending %zu byte header and %zu byte data from task %zu -> 0:\n\t\ttask %zu use_rdma hard hint = %zu\n\t\ttask %zu use_rdma soft hint = %zu\n\t\ttask 0 use rdma hard hint = %zu\n", header_bytes[1], data_bytes[2], task_id, task_id, send_hard_use_rdma_hint, task_id, send_soft_use_rdma_hint, recv_use_rdma_hint);

	result = PAMI_Send (context, &parameters);
	if (result != PAMI_SUCCESS)
	{
	  fprintf(stderr, "Error. Sent %zu byte header and %zu byte data from task %zu (use_rdma hard hint = %zu, use_rdma soft hint = %zu) to task 0 (use_rdma hard hint = %zu) and FAILED wth rc = %d\n",header_bytes[1], data_bytes[2], task_id, send_hard_use_rdma_hint, send_soft_use_rdma_hint, recv_use_rdma_hint, recv_no_rdma_hint, result);
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


  // ======== Combinations of rdma hints that should result in recv FAILS  ========

  // Enable recv fail scenarios only ...skip passing scenarios
  for (i = 0; i < 4; i++) {
    for(j = 0; j < 11; j++) {
	skip_ary[i][j] = 1;
    }
  }

  // Execute send = 2,recv = 1
  skip_ary[1][8] = 0; skip_ary[1][9] = 0; skip_ary[1][11] = 0;
  // Execute send = 1,recv = 2
  skip_ary[2][4] = 0; skip_ary[2][6] = 0; skip_ary[2][7] = 0;

  if (task_id == 0)
  {

    fprintf(stderr, "======== Combinations of rdma hints that should result in recv FAILS  ========\n");

    for (r = 0; r < 3; r++) {
      for (s = 0; s < 12; s++) {

	// Skip scenarios we don't expect to result in a recv fail
	if (skip_ary[r][s]) {
	  continue;
	}

	// Determine hint values
	send_hard_use_rdma_hint = (s >> 2) & 3;
	send_soft_use_rdma_hint = s & 3;
	recv_use_rdma_hint = r;

	parameters.send.dispatch        = dispatch_ary_0[r][s];

	// Communicate with each task
	for (n = 1; n < num_tasks; n++) {

	  TRACE((stderr, "before send ...\n"));

	  result = PAMI_Endpoint_create (client, n, 0, &parameters.send.dest);
	  if (result != PAMI_SUCCESS) {
	    fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed with %d.\n", result);
	    return 1;
	  }

	  fprintf(stderr, "Sending %zu byte header and %zu byte data from task %zu -> %zu:\n\t\ttask %zu use_rdma hard hint = %zu\n\t\ttask %zu use_rdma soft hint = %zu\n\t\ttask %zu use rdma hard hint = %zu\n", header_bytes[1], data_bytes[2], task_id, n, task_id, send_hard_use_rdma_hint, task_id, send_soft_use_rdma_hint, n, recv_use_rdma_hint);

	  result = PAMI_Send (context, &parameters);
	  if (result != PAMI_SUCCESS)
	  {
	    fprintf(stderr, "Error. Sent %zu byte header and %zu byte data from task %zu (use_rdma hard hint = %zu, use_rdma soft hint = %zu) to task %zu (use_rdma hard hint = %zu) and FAILED wth rc = %d\n",header_bytes[1], data_bytes[2], task_id, send_hard_use_rdma_hint, send_soft_use_rdma_hint, n, recv_use_rdma_hint, result);
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
	} // end task loop
      } // end send hint loop
    } // end recv hint loop
  } // end task = 0
  else
  {

    result = PAMI_Endpoint_create (client, 0, 0, &parameters.send.dest);
    if (result != PAMI_SUCCESS) {
      fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed with %d.\n", result);
      return 1;
    }

    for (r = 0; r < 3; r++) {
      for (s = 0; s < 12; s++) {

	// Skip scenarios that are expected to fail
	if (skip_ary[r][s]) {
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
	send_hard_use_rdma_hint = (s >> 2) & 3;
	send_soft_use_rdma_hint = s & 3;
	recv_use_rdma_hint = r;

	parameters.send.dispatch        = dispatch_ary_n[r][s];

	fprintf(stderr, "Sending %zu byte header and %zu byte data from task %zu -> 0:\n\t\ttask %zu use_rdma hard hint = %zu\n\t\ttask %zu use_rdma soft hint = %zu\n\t\ttask 0 use rdma hard hint = %zu\n", header_bytes[1], data_bytes[2], task_id, task_id, send_hard_use_rdma_hint, task_id, send_soft_use_rdma_hint, recv_use_rdma_hint);

	result = PAMI_Send (context, &parameters);
	if (result != PAMI_SUCCESS)
	{
	  fprintf(stderr, "Error. Sent %zu byte header and %zu byte data from task %zu (use_rdma hard hint = %zu, use_rdma soft hint = %zu) to task 0 (use_rdma hard hint = %zu) and FAILED wth rc = %d\n",header_bytes[1], data_bytes[2], task_id, send_hard_use_rdma_hint, send_soft_use_rdma_hint, recv_use_rdma_hint, result);
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

  // ======== Cleanup ========

  result = PAMI_Context_destroy (context);
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
