/** \file test/api/p2p/default-send.c
    \brief Simple point-topoint PAMI_send() test 
*/

#include <pami.h>
#include <stdio.h>
#include <string.h>

uint8_t __recv_buffer[2048];
size_t __recv_size;
size_t __header_errors = 0;
size_t __data_errors;

unsigned validate (const void * addr, size_t bytes)
{
  unsigned status = 1;
  uint8_t * byte = (uint8_t *) addr;
  size_t i;
  for (i=0; i<bytes; i++)
  {
    if (byte[i] != (uint8_t)i)
    {
      fprintf (stderr, "validate(%p,%zu) .. ERROR .. byte[%zu] != %d (&byte[%zu] = %p, value is %d)\n", addr, bytes, i, (uint8_t)i, i, &byte[i], byte[i]);
      status = 0;
    }
  }

  return status;
}


static void recv_done (pami_context_t   context,
                       void          * cookie,
                       pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called recv_done function.  active(%p): %zu -> %zu, __recv_size = %zu\n", active, *active, *active-1, __recv_size);

  if (!validate(__recv_buffer, __recv_size))
  {
    __data_errors++;
    fprintf (stderr, ">>> ERROR: Validate payload FAILED!\n");
  } else {
    fprintf (stdout, ">>> payload validated.\n");
  }

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
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called dispatch function.  cookie = %p, active: %zu, header_size = %zu, pipe_size = %zu\n", cookie, *active, header_size, pipe_size);
  /*(*active)--; */
  /*fprintf (stderr, "... dispatch function.  active = %zu\n", *active); */

  if (header_size > 0) {
    if (!validate(header_addr, header_size)) {
      __header_errors++;
      fprintf (stderr, ">>> ERROR: Validate header FAILED!\n");
    } else {
      fprintf (stdout, ">>> header validated.\n");
    }
  } else {
    fprintf (stdout, ">>> Skipping header validation (header size = %zu).\n", header_size);
  }

  if (pipe_size == 0)
  {
    (*active)--;
    fprintf (stdout, ">>> Skipping payload validation (payload size = %zu).\n", pipe_size);
  }
  else
  {
    __recv_size = pipe_size;

    recv->local_fn = recv_done;
    recv->cookie   = cookie;
    recv->type     = PAMI_BYTE;
    recv->addr     = __recv_buffer;
    recv->offset   = 0;
    /*fprintf (stderr, "... dispatch function.  recv->local_fn = %p\n", recv->local_fn); */
  }

  return;
}

static void send_done_local (pami_context_t   context,
                             void          * cookie,
                             pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called send_done_local function.  active(%p): %zu -> %zu\n", active, *active, *active-1);
  (*active)--;
}

static void send_done_remote (pami_context_t   context,
                              void          * cookie,
                              pami_result_t    result)
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called send_done_remote function.  active(%p): %zu -> %zu\n", active, *active, *active-1);
  (*active)--;
  /*fprintf (stderr, "... send_done_remote function.  active = %zu\n", *active); */
}

int main (int argc, char ** argv)
{

  /* Determine which Device is being used */
  char * device;
  size_t initial_device = 0;
  size_t device_limit = 0;
  device = getenv ("PAMI_DEVICE");

  if (device != NULL) {
    if (!strcmp(device, "M")) {
      fprintf (stderr, "Only the MU device is initialized.\n");
      initial_device = 0;
      device_limit = 1;
    } else if (!strcmp(device, "S")) {
      fprintf (stderr, "Only the SHMem device is initialized.\n");
      initial_device = 1;
      device_limit = 2;
    } else if ( !strcmp(device, "B")){
      fprintf (stderr, "Both the MU and SHMem devices are initialized.\n");
      initial_device = 0;
      device_limit = 2;
    } else {
      fprintf (stderr, "ERROR:  PAMI_DEVICE = %s is unsupported. Valid values are:  M (MU only), S (SHMem only), B (both MU & SHMem)\n", device);
      return 1;
    }
  } else {
      fprintf (stderr, "Both the MU and SHMem devices are initialized.\n");
      initial_device = 0;
      device_limit = 2;
  }

  volatile size_t send_active = 1;
  volatile size_t recv_active = 1;

  __data_errors = 0;

  pami_client_t client;
  pami_configuration_t configuration;
  pami_context_t context[2];

  char                  cl_string[] = "TEST";
  pami_result_t result = PAMI_ERROR;

  result = PAMI_Client_create (cl_string, &client, NULL, 0);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }

  configuration.name = PAMI_CLIENT_NUM_CONTEXTS;
  result = PAMI_Client_query(client, &configuration,1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t max_contexts = configuration.value.intval;
  if (max_contexts > 0) {
    fprintf (stderr, "Max number of contexts = %zu\n", max_contexts);
  } else {
    fprintf (stderr, "ERROR:  Max number of contexts (%zu) <= 0. Exiting\n", max_contexts);
    PAMI_Client_destroy(client);
    return 1;
  }

  size_t num_contexts = 1;
  if (max_contexts > 1) {
    num_contexts = 2; /* allows for cross talk */
  }

  result = PAMI_Context_createv(client, NULL, 0, context, num_contexts);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami context. result = %d\n", result);
    return 1;
  }

  configuration.name = PAMI_CLIENT_TASK_ID;
  result = PAMI_Client_query(client, &configuration,1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  pami_task_t task_id = configuration.value.intval;
  fprintf (stderr, "My task id = %d\n", task_id);

  configuration.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query(client, &configuration,1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  fprintf (stderr, "Number of tasks = %zu\n", num_tasks);
  if (num_tasks < 2) {
    fprintf(stderr, "Error. This test requires >= 2 tasks. Number of tasks in this job: %zu\n", num_tasks);
    return 1;
  }

  /*size_t dispatch = 0; */
  pami_dispatch_callback_function fn;
  fn.p2p = test_dispatch;
  pami_send_hint_t options={0};
  size_t i, dev = 0;

  for (i = 0; i < num_contexts; i++) {
    /* For each context: */
    /* Set up dispatch ID 0 for MU (use_shmem = 2) */
    /* set up dispatch ID 1 for SHMem (use_shmem = 1) */

    for (dev = initial_device; dev < device_limit; dev++) {
      fprintf (stderr, "Before PAMI_Dispatch_set() .. &recv_active = %p, recv_active = %zu\n", &recv_active, recv_active);
      options.use_shmem = 2 - dev;
      result = PAMI_Dispatch_set (context[i],
				  dev,
				  fn,
				  (void *)&recv_active,
				  options);
      if (result != PAMI_SUCCESS) {
	fprintf (stderr, "Error. Unable to register pami dispatch %zu on context %zu. result = %d\n", dev, i, result);
	return 1;
      }
    }
  }

  uint8_t header[1024];
  uint8_t data[1024];
  for (i=0; i<1024; i++)
  {
    header[i] = (uint8_t)i;
    data[i]   = (uint8_t)i;
  }

  size_t h, hsize = 0;
  size_t header_bytes[16];
  header_bytes[hsize++] = 0;
  header_bytes[hsize++] = 16;
  header_bytes[hsize++] = 32;

  size_t p, psize = 0;
  size_t data_bytes[16];
  /*data_bytes[psize++] = 0; */
  /*data_bytes[psize++] = 16; */
  /*data_bytes[psize++] = 32; */
  /*data_bytes[psize++] = 64; */
  data_bytes[psize++] = 128;
  data_bytes[psize++] = 256;
  data_bytes[psize++] = 512;
  data_bytes[psize++] = 1024;

  pami_send_t parameters;
  parameters.send.header.iov_base = header;
  parameters.send.data.iov_base   = data;
  parameters.events.cookie        = (void *) &send_active;
  parameters.events.local_fn      = send_done_local;

  size_t xtalk = 0;
  size_t remote_cb = 0;
  size_t n = 0;

  char device_str[2][50] = {"MU", "SHMem"};
  char xtalk_str[2][50] = {"no crosstalk", "crosstalk"};
  char callback_str[2][50] = {"no callback", "callback"};

  if (task_id == 0)
  {
    for(dev = initial_device; dev < device_limit; dev++) {      /* device loop */

      parameters.send.dispatch = dev;

      for(xtalk = 0; xtalk < num_contexts; xtalk++) {           /* xtalk loop */

	/* Skip running MU in Cross talk mode for now */
	/*	if (xtalk && !strcmp(device_str[dev], "MU")) {
	  continue;
	}
	*/
	for (remote_cb = 0; remote_cb < 2; remote_cb++) { /* remote callback loop */
	  if (remote_cb) {
	    parameters.events.remote_fn     = send_done_remote;
	  } else {
	    parameters.events.remote_fn     = NULL;
	  }

	  for (h=0; h<hsize; h++) {
	    parameters.send.header.iov_len = header_bytes[h];
	    for (p=0; p<psize; p++) {
	      parameters.send.data.iov_len = data_bytes[p];

	      /* Communicate with each task */
	      for (n = 1; n < num_tasks; n++) {

		result = PAMI_Endpoint_create (client, n, xtalk, &parameters.send.dest);
		if (result != PAMI_SUCCESS) {
		  fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed for task_id %zu, context %zu with %d.\n", n, xtalk, result);
		  return 1;
		}

		fprintf (stderr, "===== PAMI_Send() functional test [%s][%s][%s] %zu %zu (%d, 0) -> (%zu, %zu) =====\n\n", &device_str[dev][0], &xtalk_str[xtalk][0], &callback_str[remote_cb][0], header_bytes[h], data_bytes[p], task_id, n, xtalk);

		if (remote_cb) {
		  send_active++;
		}

		fprintf (stderr, "before send ...\n");
		result = PAMI_Send (context[0], &parameters);
		fprintf (stderr, "... after send.\n");

		if (result != PAMI_SUCCESS) {
		  fprintf (stderr, "ERROR:   PAMI_Send failed with rc = %d\n", result);
		  return 1;
		}



		fprintf (stderr, "before send-recv advance loop ... &send_active = %p, &recv_active = %p\n", &send_active, &recv_active);

		while (send_active || recv_active) {
		  result = PAMI_Context_advance (context[0], 100);

		  if (result != PAMI_SUCCESS) {
		    fprintf (stderr, "Error. Unable to advance pami context 0. result = %d\n", result);
		    return 1;
		  }
		}

		send_active = 1;
		recv_active = 1;

		fprintf (stderr, "... after send-recv advance loop\n");
	      } /* end task id loop */
	    } /* end payload loop */
	  } /* end header loop */
	} /* end remote callback loop */
      } /* end xtalk loop */
    } /* end device loop */
  } /* end task = 0 */
  else {
    for(dev = initial_device; dev < device_limit; dev++) {      /* device loop */

      parameters.send.dispatch = dev;

      for(xtalk = 0; xtalk < num_contexts; xtalk++) {           /* xtalk loop */

	/* Skip running MU in Cross talk mode for now */
	/*	if (xtalk && !strcmp(device_str[dev], "MU")) {
	  continue;
	}
	*/
	result = PAMI_Endpoint_create (client, 0, 0, &parameters.send.dest);
	if (result != PAMI_SUCCESS) {
	  fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed for task_id 0, context 0 with %d.\n", result);
	  return 1;
	}

	for (remote_cb = 0; remote_cb < 2; remote_cb++) { /* remote callback loop */

	  if (remote_cb) {
	    parameters.events.remote_fn     = send_done_remote;
	  } else {
	    parameters.events.remote_fn     = NULL;
	  }

	  for (h=0; h<hsize; h++) {
	    parameters.send.header.iov_len = header_bytes[h];
	    for (p=0; p<psize; p++) {
	      parameters.send.data.iov_len = data_bytes[p];

	      fprintf (stderr, "before recv advance loop ... &recv_active = %p\n", &recv_active);

	      while (recv_active != 0) {
		result = PAMI_Context_advance (context[xtalk], 100);
		if (result != PAMI_SUCCESS) {
		  fprintf (stderr, "Error. Unable to advance pami context %zu. result = %d\n", xtalk, result);
		  return 1;
		}
		fprintf (stderr, "------ recv advance loop ... &recv_active = %p\n", &recv_active);
	      }

	      recv_active = 1;
	      fprintf (stderr, "... after recv advance loop\n");

	      fprintf (stderr, "===== PAMI_Send() functional test [%s][%s][%s] %zu %zu (%d, %zu) -> (0, 0) =====\n\n", &device_str[dev][0], &xtalk_str[xtalk][0], &callback_str[remote_cb][0], header_bytes[h], data_bytes[p], task_id, xtalk);

	      if (remote_cb) {
		send_active++;
	      }

	      fprintf (stderr, "before send ...\n");
	      result = PAMI_Send (context[xtalk], &parameters);
	      fprintf (stderr, "... after send.\n");

	      if (result != PAMI_SUCCESS) {
		fprintf (stderr, "ERROR:   PAMI_Send failed with rc = %d\n", result);
		return 1;

}

	      fprintf (stderr, "before send advance loop ... &send_active = %p\n", &send_active);

	      while (send_active) {
		result = PAMI_Context_advance (context[xtalk], 100);
		if (result != PAMI_SUCCESS) {
		  fprintf (stderr, "Error. Unable to advance pami context %zu. result = %d\n", xtalk, result);
		  return 1;
		}
		fprintf (stderr, "------ send advance loop ... &send_active = %p\n", &send_active);
	      }

	      send_active = 1;

	      fprintf (stderr, "... after send advance loop\n");
	    } /* end payload loop */
	  } /* end header loop */
	} /* end remote callback loop */
      } /* end xtalk loop */
    } /* end device loop */
  } /* end task id != 0 */


  /* ====== CLEANUP ====== */

  result = PAMI_Context_destroyv(context, num_contexts);
  if (result != PAMI_SUCCESS) {
    fprintf (stderr, "Error. Unable to destroy context, result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy(&client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami client. result = %d\n", result);
    return 1;
  }

  if ( (__header_errors > 0) || (__data_errors > 0) ) {
    fprintf (stdout, "Error. default-send-nplus1 FAILED with %zu header errors and %zu data errors on task %d!!\n", __header_errors, __data_errors, task_id);
    return 1;
  }
  else
  {
    fprintf (stdout, "Success (%d)\n", task_id);
    return 0;
  }
}
