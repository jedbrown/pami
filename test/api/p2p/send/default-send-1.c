/**
 * \file test/api/p2p/send/default-send-1.c
 * \brief Ppoint-to-point PAMI_send() test using multiple contexts
 */

#include <pami.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define ENABLE_TRACE

#ifdef ENABLE_TRACE
#define TRACE(x) fprintf x
#else
#define TRACE(x)
#endif

pami_client_t g_client;
pami_context_t context[2];
size_t num_contexts = 1;
size_t initial_device = 0;
size_t device_limit = 0;
size_t create_dpids = 1;

uint8_t __junk[1024*16];

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
  TRACE((stderr, "Called dispatch function.  cookie = %p, active: %zu\n", cookie, *((volatile size_t *) cookie)));
  volatile size_t * active = (volatile size_t *) cookie;

  if (pipe_size == 0)
  {
    (*active)--;
  }
  else if (recv == NULL)
  {
    // This is an 'immediate' receive
    memcpy(__junk, pipe_addr, pipe_size);
    (*active)--;
  }
  else
  {
    // This is an 'asynchronous' receive

    recv->local_fn = recv_done;
    recv->cookie   = cookie;
    recv->type     = PAMI_TYPE_CONTIGUOUS;
    recv->addr     = __junk;
    recv->offset   = 0;
    recv->data_fn  = PAMI_DATA_COPY;
    recv->data_cookie = NULL;
    TRACE((stderr, "... dispatch function.  recv->local_fn = %p\n", recv->local_fn));
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
  TRACE((stderr, "... send_done_remote function.  active = %zu\n", *active));
}


unsigned do_test ()
{
  volatile size_t send_active = 1;
  volatile size_t recv_active = 1;

  size_t i = 0;

  pami_result_t result = PAMI_ERROR;

  /* Lock the context(s) */
  for (i = 0; i < num_contexts; i++) {
    pami_result_t result = PAMI_Context_lock (context[i]);
    if (result != PAMI_SUCCESS)
      {
	fprintf (stderr, "Error. Unable to lock pami context %zu. result = %d\n", i, result);
	return 1;
      }
  }

  pami_configuration_t configuration;

  configuration.name = PAMI_CLIENT_TASK_ID;
  result = PAMI_Client_query(g_client, &configuration,1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  /*TRACE((stderr, "My task id = %zu\n", task_id)); */

  configuration.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query(g_client, &configuration,1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  /*TRACE((stderr, "Number of tasks = %zu\n", num_tasks)); */
  if (num_tasks < 2) {
    fprintf(stderr, "Error. This test requires >= 2 tasks. Number of tasks in this job: %zu\n", num_tasks);
    return 1;
  }

  /*size_t dispatch = 0; */
  pami_dispatch_callback_function fn;
  fn.p2p = test_dispatch;
  pami_dispatch_hint_t options={0};
  size_t dev = 0;

  if (create_dpids) {
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
      } /* end dp id/device loop */
    } /* end context loop */

    create_dpids = 0;

  } /* end dispatch id creation */

  pami_send_t parameters;
  parameters.send.header.iov_base = NULL;
  parameters.send.header.iov_len  = 0;
  parameters.send.data.iov_base   = NULL;
  parameters.send.data.iov_len    = 0;
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

      for(xtalk = 0; xtalk < num_contexts; xtalk++) {                /* xtalk loop */

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

	  /* Communicate with each task */
	  for (n = 1; n < num_tasks; n++) {	  

	    result = PAMI_Endpoint_create (g_client, n, xtalk, &parameters.send.dest);
	    if (result != PAMI_SUCCESS) {
	      fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed for task_id %zu, context %zu with %d.\n", n, xtalk, result);
	      return 1;
	    }

	    fprintf (stderr, "===== PAMI_Send() functional test [%s][%s][%s] 0 0 (%zu, 0) -> (%zu, %zu) =====\n\n", &device_str[dev][0], &xtalk_str[xtalk][0], &callback_str[remote_cb][0], task_id, n, xtalk);

	    if (remote_cb) {
	      send_active++;
	    }

	    TRACE((stderr, "before send ...\n"));
	    result = PAMI_Send (context[0], &parameters);
	    TRACE((stderr, "... after send.\n"));

	    if (result != PAMI_SUCCESS) {
	      fprintf (stderr, "ERROR:   PAMI_Send failed with rc = %d\n", result);
	      return 1;
	    }

	    TRACE((stderr, "before send-recv advance loop ...\n"));
	    while (send_active || recv_active) {
	      result = PAMI_Context_advance (context[0], 100);
	      if (result != PAMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to advance pami context 0. result = %d\n", result);
		return 1;
	      }
	    }

	    send_active = 1;
	    recv_active = 1;

	    TRACE((stderr, "... after send-recv advance loop\n"));
	  } /* end task loop */
	} /* end remote callback loop */
      } /* end xtalk loop */
    } /* end device loop */
  } /* end task = 0 loop */
  else
  {

    for(dev = initial_device; dev < device_limit; dev++) {      /* device loop */

      parameters.send.dispatch = dev;

      for(xtalk = 0; xtalk < num_contexts; xtalk++) {                /* xtalk loop */

	/* Skip running MU in Cross talk mode for now */
	/*	if (xtalk && !strcmp(device_str[dev], "MU")) {
	  continue;
	}
	*/
	result = PAMI_Endpoint_create (g_client, 0, 0, &parameters.send.dest);
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

	  TRACE((stderr, "before recv advance loop ...\n"));
	  while (recv_active != 0) {
	    result = PAMI_Context_advance (context[xtalk], 100);
	    if (result != PAMI_SUCCESS) {
	      fprintf (stderr, "Error. Unable to advance pami context %zu. result = %d\n", xtalk, result);
	      return 1;
	    }
	  }

	  recv_active = 1;
	  TRACE((stderr, "... after recv advance loop\n"));

	  fprintf (stderr, "===== PAMI_Send() functional test [%s][%s][%s] 0 0 (%zu, %zu) -> (0, 0) =====\n\n", &device_str[dev][0], &xtalk_str[xtalk][0], &callback_str[remote_cb][0], task_id, xtalk);

	  if (remote_cb) {
	    send_active++;
	  }

	  TRACE((stderr, "before send ...\n"));
	  result = PAMI_Send (context[xtalk], &parameters);
	  TRACE((stderr, "... after send.\n"));

	  if (result != PAMI_SUCCESS) {
	    fprintf (stderr, "ERROR:   PAMI_Send failed with rc = %d\n", result);
	    return 1;
	  }

	  TRACE((stderr, "before send advance loop ...\n"));
	  while (send_active) {
	    result = PAMI_Context_advance (context[xtalk], 100);
	    if (result != PAMI_SUCCESS) {
	      fprintf (stderr, "Error. Unable to advance pami context %zu. result = %d\n", xtalk, result);
	      return 1;
	    }
	  }

	  send_active = 1;

	  TRACE((stderr, "... after send advance loop\n"));
	} /* end remote callback loop */
      } /* end xtalk loop */
    } /* end device loop */
  } /* end task != 0 */

  /* Unlock the context */
  for( i = 0; i < num_contexts; i++) {
    result = PAMI_Context_unlock (context[i]);
    if (result != PAMI_SUCCESS)
      {
	fprintf (stderr, "Error. Unable to unlock the pami context %zu. result = %d\n", i, result);
	return 1;
      }
  }

  return 0;
};


int main (int argc, char ** argv)
{

  size_t status = 0;

  /* Determine which Device is being used */
  char * device;
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

  pami_configuration_t configuration;
  char                  cl_string[] = "TEST";
  pami_result_t result = PAMI_ERROR;

  result = PAMI_Client_create (cl_string, &g_client, NULL, 0);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
    return 1;
  }


  configuration.name = PAMI_CLIENT_NUM_CONTEXTS;
  result = PAMI_Client_query(g_client, &configuration,1);
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
    return 1;
  }

  if (max_contexts > 1) {
    num_contexts = 2;
  }

  result = PAMI_Context_createv (g_client, NULL, 0, &context[0], num_contexts);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami context(s). result = %d\n", result);
    return 1;
  }

  TRACE((stderr, "Before do_test.\n"));
  status = do_test ();
  TRACE((stderr, "After do_test.\n"));

  /* ====== CLEANUP ====== */

  result = PAMI_Context_destroyv(context, num_contexts);
  if (result != PAMI_SUCCESS) {
    fprintf (stderr, "Error. Unable to destroy context, result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy (g_client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami client. result = %d\n", result);
    return 1;
  }

  if (status) {
    fprintf (stderr, "ERROR: default-send-1 FAILED with rc = %zu!!\n", status);
  } else {
    fprintf (stderr, "Success.\n");
  }
  return status;
}
