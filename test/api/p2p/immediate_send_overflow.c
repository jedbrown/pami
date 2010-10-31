/*/ */
/*/ \file test/api/p2p/immediate_send_overflow.c */
/*/ \brief Simple point-topoint PAMI_send() test */
/*/ \validates that the n+1 byte remains unchanged */
/*/ */

#include <pami.h>
#include <stdio.h>
#include <string.h>

uint8_t __recv_buffer[2048];
char recv_str[2048];               /* used to print __recv_buffer as string */
size_t __recv_size;
size_t __header_errors = 0;
size_t __data_errors = 0;
uint8_t reset_value[2] ={0, 255};  /* reset value for each byte of __recv_buffer ...all 0's or all 1's (255) */
size_t reset_elements = 2;         /* total number of reset values */
size_t r = 0;                      /* used to loop over reset values */

unsigned validate (const void * addr, size_t bytes, size_t test_n_plus_1)
{
  unsigned status = 1;
  uint8_t * byte = (uint8_t *) addr;
  uint8_t expected_value = 0;
  size_t total_bytes = 0;
  size_t i, j = 0;

  /* Verify data received as well as 0-minus-1 and n-plus-1 bytes */
  if (test_n_plus_1) {
    total_bytes = bytes+2;
  } else { /* Only verify data received */
    total_bytes = bytes;
  }

  /* Loop through recv_buffer */
  for (i=0; i<total_bytes; i++) {

    /* Determine expected value */
    if (test_n_plus_1) {
      /* Ensure 0-minus-1 and n-plus-1 bytes equal the reset value */
      if ( (i == 0) || (i == total_bytes-1) ) {
	expected_value = reset_value[r];
      } else { /* Validate received data (__recv_buffer[1:bytes]) */
	expected_value = (uint8_t)(i-1);
      }
    } else {
      expected_value = (uint8_t)i;
    }

    /* Verify current value */
    if (byte[i] != expected_value) {

      fprintf (stderr, "validate(%p,%zu) .. ERROR .. byte[%zu] != %d (&byte[%zu] = %p, value is %d)\n", addr, total_bytes, i, expected_value, i, &byte[i], byte[i]);

      status = 0;
    }

    /* Print element to string to print later if desired */
    sprintf(&recv_str[j], "%d", byte[i]);
    if (byte[i] < 10) {
      j++;
    } else if (byte[i] < 100) {
      j+=2;
    } else {
      j+=3;
    }
  }

  /* Print __recv_buffer */
  fprintf(stdout, "recv buffer[0:%zu] after send: %s\n", total_bytes-1, recv_str);

  return status;
}


static void test_dispatch (
    pami_context_t       context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    const void         * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    const void         * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_endpoint_t      origin,
    pami_recv_t        * recv)         /**< OUT: receive message structure */
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, "Called dispatch function.  cookie = %p (active: %zu -> %zu), header_size = %zu, pipe_size = %zu, recv = %p\n", cookie, *active, *active-1, header_size, pipe_size, recv);
  (*active)--;
  fprintf (stderr, ">>> header size:  [%zu] %s\n", header_size, (char *) header_addr);
  fprintf (stderr, ">>> payload size: [%zu] %s\n", pipe_size, (char *) pipe_addr);

  /* Validate header if header size > 0 */
  if (header_size > 0) {
    if (validate (header_addr, header_size, 0)) {
      fprintf (stderr, ">>> header validated.\n");
    } else {
      __header_errors++;
      fprintf (stderr, ">>> ERROR: Validate header FAILED!!\n");
    }
  } else {
    fprintf (stdout, ">>> Skipping header validation (header size = %zu).\n", header_size);
  }

  /* Validate payload if pipe size > 0 */
  if (pipe_size > 0) {
    if (validate (pipe_addr, pipe_size, 0)) {
      fprintf (stderr, ">>> payload validated.\n");
    } else {
      __data_errors++;
      fprintf (stderr, ">>> ERROR: Validate payload FAILED!!\n");
    }
  } else {
    fprintf (stdout, ">>> Skipping payload validation (payload size = %zu).\n", pipe_size);
  }

  fprintf (stderr, "... dispatch function.\n");

  return;
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

  volatile size_t recv_active = 1;

  pami_client_t client;
  pami_configuration_t configuration;
  pami_context_t context[2];

  char                  cl_string[] = "TEST";
  pami_result_t result = PAMI_ERROR;
  pami_result_t send_fail = PAMI_INVAL;

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

  size_t send_immediate_max [2][num_contexts];
  size_t recv_immediate_max [2][num_contexts];

  char device_str[2][50] = {"MU", "SHMem"};
  char xtalk_str[2][50] = {"no crosstalk", "crosstalk"};

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

      /* Get max send value */
      configuration.name = PAMI_DISPATCH_SEND_IMMEDIATE_MAX;
      result = PAMI_Dispatch_query(context[i],dev, &configuration,1);
      if (result != PAMI_SUCCESS) {
	fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
	return 1;
      }
  
      send_immediate_max[dev][i] = configuration.value.intval;
      fprintf (stderr, "Maximum number of bytes that can be transfered with the PAMI_Send_immediate() function from context %zu, dispatch ID %zu using %s = %zu\n", i, dev, &device_str[dev][0], send_immediate_max[dev][i]);

      /* Get max receive value */
      configuration.name = PAMI_DISPATCH_RECV_IMMEDIATE_MAX;
      result = PAMI_Dispatch_query(context[i],dev, &configuration,1);
      if (result != PAMI_SUCCESS) {
	fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
	return 1;
      }
      recv_immediate_max[dev][i] = configuration.value.intval;
      fprintf (stderr, "Maximum number of bytes that can be received, and provided to the application, in a dispatch function by context %zu, dispatch ID %zu using %s = %zu\n", i, dev, &device_str[dev][0], recv_immediate_max[dev][i]);
      
    }
  }

  char header_string[1024];
  char data_string[1024];
  for (i=0; i<1024; i++)
  {
    header_string[i] = (char)i;
    data_string[i]   = (char)i;
  }

  pami_send_immediate_t parameters;
  parameters.header.iov_base = header_string;
  parameters.data.iov_base   = data_string;

  size_t xtalk = 0;
  size_t header_bytes[16];
  size_t data_bytes[16];
  size_t max_msg_size = 0;
  size_t scenario = 0;
  size_t test = 0;
  size_t n = 0;

  /* ======== Combinations of header and payload sizes that should pass  ======== */

  if (task_id == 0)
  {

    fprintf(stderr, "====== Combinations of header and payload sizes that should pass ======\n");

    for(dev = initial_device; dev < device_limit; dev++) {      /* device loop */
      for(xtalk = 0; xtalk < num_contexts; xtalk++) {           /* xtalk loop */

	/* Skip running MU in Cross talk mode for now */
	/*	if (xtalk && !strcmp(device_str[dev], "MU")) {
	  continue;
	}
	*/
	/* Determine max message size */
	if ( send_immediate_max[dev][xtalk] < recv_immediate_max[dev][xtalk] ) {
	  max_msg_size = send_immediate_max[dev][xtalk];
	} else {
	  max_msg_size = recv_immediate_max[dev][xtalk];
	}
	  
	/* Run the following scenarios: */
	/* header = 0             payload = max                     */
	/* header = max/2         payload = max/2  */
	/* header = max           payload = 0        */

	/* Populate header and payload size arrays */
	scenario = 0;
	header_bytes[scenario++] = 0;
	header_bytes[scenario++] = max_msg_size/2;
	header_bytes[scenario++] = max_msg_size;

	scenario = 0;
	data_bytes[scenario++] = max_msg_size;
	data_bytes[scenario++] = max_msg_size/2;
	data_bytes[scenario++] = 0;

	for (test = 0; test < scenario; test++) {
	  parameters.header.iov_len = header_bytes[test];
	  parameters.data.iov_len = data_bytes[test];

	  /* Communicate with each task */
	  for (n = 1; n < num_tasks; n++) {

	    parameters.dispatch = dev;
	    result = PAMI_Endpoint_create (client, n, xtalk, &parameters.dest);
	    if (result != PAMI_SUCCESS) {
	      fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed for task_id %zu, context %zu with %d.\n", n, xtalk, result);
	      return 1;
	    }

	    fprintf (stderr, "===== PAMI_Send_immediate() functional test [%s][%s] %zu %zu (%d, 0) -> (%zu, %zu) =====\n\n", &device_str[dev][0], &xtalk_str[xtalk][0], header_bytes[test], data_bytes[test], task_id, n, xtalk);

	    fprintf (stderr, "before send immediate...\n");
	    result = PAMI_Send_immediate (context[0], &parameters);
	    fprintf (stderr, "... after send immediate.\n");

	    if ( result != PAMI_SUCCESS ) {

	      fprintf (stderr, "ERROR: PAMI_Send_immediate failed with rc = %d!!\n", result);
	      return 1;
	    } 

	    fprintf (stderr, "before advance loop ... &recv_active = %p\n", &recv_active);

	    while (recv_active != 0) {
	      result = PAMI_Context_advance (context[0], 100);
	      
	      if (result != PAMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to advance pami context 0. result = %d\n", result);
		return 1;
	      }
	    }
	      
	    if (recv_active == 0) {

	      /* Determine reset value  */
	      /* reset value = 0 for even test values, 255 for odd test values */
	      if (n == num_tasks - 1) { /* test is going to increment */
		r = (test+1) % 2; /* base reset value on next test value */
	      } else {
		r = test % 2; /* base reset value on current test value */
	      }

	      /* Reset __recv_buffer for next test */
	      for (i = 0; i < 2048; i++) {
		__recv_buffer[i] = reset_value[r];
	      }

	      recv_active = 1;
	    }

	    fprintf (stderr, "... after advance loop\n");
	  } /* end task id loop */
	} /* end scenario loop */
      } /* end xtalk loop */
    } /* end device loop */
  } /* end task = 0 */
  else { /* task > 0 */
    for(dev = initial_device; dev < device_limit; dev++) {      /* device loop */
      for(xtalk = 0; xtalk < num_contexts; xtalk++) {           /* xtalk loop */

	/* Skip running MU in Cross talk mode for now */
	/*	if (xtalk && !strcmp(device_str[dev], "MU")) {
	  continue;
	}
	*/
	parameters.dispatch = dev;
	result = PAMI_Endpoint_create (client, 0, 0, &parameters.dest);
	if (result != PAMI_SUCCESS) {
	  fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed for task_id 0, context 0 with %d.\n", result);
	  return 1;
	}

	/* Determine max message size */
	if ( send_immediate_max[dev][xtalk] < recv_immediate_max[dev][xtalk] ) {
	  max_msg_size = send_immediate_max[dev][xtalk];
	} else {
	  max_msg_size = recv_immediate_max[dev][xtalk];
	}
	  
	/* Run the following scenarios: */
	/* header = 0             payload = max                     */
	/* header = max/2         payload = max/2  */
	/* header = max           payload = 0        */

	/* Populate header and payload size arrays */
	scenario = 0;
	header_bytes[scenario++] = 0;
	header_bytes[scenario++] = max_msg_size/2;
	header_bytes[scenario++] = max_msg_size;

	scenario = 0;
	data_bytes[scenario++] = max_msg_size;
	data_bytes[scenario++] = max_msg_size/2;
	data_bytes[scenario++] = 0;

	for (test = 0; test < scenario; test++) {
	  parameters.header.iov_len = header_bytes[test];
	  parameters.data.iov_len = data_bytes[test];

	  fprintf (stderr, "before recv advance loop ... &recv_active = %p\n", &recv_active);

	  while (recv_active != 0) {
	    result = PAMI_Context_advance (context[xtalk], 100);
	    if (result != PAMI_SUCCESS) {
	      fprintf (stderr, "Error. Unable to advance pami context %zu. result = %d\n", xtalk, result);
	      return 1;
	    }
	      fprintf (stderr, "------ recv advance loop ... &recv_active = %p\n", &recv_active);
	  }
	    
	  if (recv_active == 0) {
	    
	    /* Determine reset value */
	    /* reset value = 0 for even p values, 255 for odd p values */
	    r = (test+1) % 2; /* base reset value on next p value */
	      
	    /* Reset __recv_buffer for next payload */
	    for (i = 0; i < 2048; i++) {
	      __recv_buffer[i] = reset_value[r];
	    }

	    recv_active = 1;
	  }


	  fprintf (stderr, "... after recv advance loop\n");

	  fprintf (stderr, "===== PAMI_Send_immediate() functional test [%s][%s] %zu %zu (%d, %zu) -> (0, 0) =====\n\n", &device_str[dev][0], &xtalk_str[xtalk][0], header_bytes[test], data_bytes[test], task_id, xtalk);

	  fprintf (stderr, "before send immediate...\n");
	  result = PAMI_Send_immediate (context[xtalk], &parameters);
	  fprintf (stderr, "... after send immediate.\n");

	  if ( result != PAMI_SUCCESS ) {

	    fprintf (stderr, "ERROR: PAMI_Send_immediate failed with rc = %d!!\n", result);
	    return 1;
	  } 

	  fprintf (stderr, "before send advance loop ...\n");
	  result = PAMI_Context_advance (context[xtalk], 100);
	  if (result != PAMI_SUCCESS) {
	    fprintf (stderr, "Error. Unable to advance pami context %zu. result = %d\n", xtalk, result);
	    return 1;
	  }

	  fprintf (stderr, "... after send immediate advance loop\n");
	} /* end scenario loop */
      } /* end xtalk loop */
    } /* end device loop */
  } /* end task id != 0 */

  /* ======== Combinations of header and payload sizes that should result in send FAILS  ======== */

  if (task_id == 0)
  {

    fprintf(stderr, "====== Combinations of header and payload sizes that should result in send FAILS ======\n");

    for(dev = initial_device; dev < device_limit; dev++) {      /* device loop */
      for(xtalk = 0; xtalk < num_contexts; xtalk++) {           /* xtalk loop */

	/* Skip running MU in Cross talk mode for now */
	/*	if (xtalk && !strcmp(device_str[dev], "MU")) {
	  continue;
	}
	*/
	/* Run the following scenarios: */
	/* header = 0             payload = max + 1                     */
	/* header = max/2         payload = max/2 + 1 */
	/* header = max + 1       payload = 0        */

	/* Populate header and payload size arrays */
	scenario = 0;
	header_bytes[scenario++] = 0;
	header_bytes[scenario++] = send_immediate_max[dev][xtalk]/2;
	header_bytes[scenario++] = send_immediate_max[dev][xtalk] + 1;

	scenario = 0;
	data_bytes[scenario++] = send_immediate_max[dev][xtalk] + 1;
	data_bytes[scenario++] = send_immediate_max[dev][xtalk]/2 + 1;
	data_bytes[scenario++] = 0;

	for (test = 0; test < scenario; test++) {
	  parameters.header.iov_len = header_bytes[test];
	  parameters.data.iov_len = data_bytes[test];

	  /* Communicate with each task */
	  for (n = 1; n < num_tasks; n++) {

	    parameters.dispatch = dev;
	    result = PAMI_Endpoint_create (client, n, xtalk, &parameters.dest);
	    if (result != PAMI_SUCCESS) {
	      fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed for task_id %zu, context %zu with %d.\n", n, xtalk, result);
	      return 1;
	    }

	    fprintf (stderr, "===== PAMI_Send_immediate() functional test [%s][%s] %zu %zu (%d, 0) -> (%zu, %zu) =====\n\n", &device_str[dev][0], &xtalk_str[xtalk][0], header_bytes[test], data_bytes[test], task_id, n, xtalk);

	    fprintf (stderr, "before send immediate...\n");
	    result = PAMI_Send_immediate (context[0], &parameters);
	    fprintf (stderr, "... after send immediate.\n");

	    if ( result != send_fail ) {
	      fprintf (stderr, "ERROR: PAMI_Send_immediate passed with rc = %d ...expected it to fail with rc = %d!!\n", result, send_fail);
	      return 1;
	    } 
	  } /* end task id loop */
	} /* end scenario loop */
      } /* end xtalk loop */
    } /* end device loop */
  } /* end task = 0 */
#if 0
  /* Commenting out for now to avoid output confusion */
  else { /* task > 0 */
    for(dev = initial_device; dev < device_limit; dev++) {      /* device loop */
      for(xtalk = 0; xtalk < num_contexts; xtalk++) {           /* xtalk loop */

	/* Skip running MU in Cross talk mode for now */
/*		if (xtalk && !strcmp(device_str[dev], "MU")) { */
/*	  continue; */
/*	} */
  
	parameters.dispatch = dev;
	result = PAMI_Endpoint_create (client, 0, 0, &parameters.dest);
	if (result != PAMI_SUCCESS) {
	  fprintf (stderr, "ERROR:  PAMI_Endpoint_create failed for task_id 0, context 0 with %d.\n", result);
	  return 1;
	}

	/* Run the following scenarios: */
	/* header = 0             payload = max + 1                     */
	/* header = max/2         payload = max/2 + 1 */
	/* header = max + 1       payload = 0        */

	/* Populate header and payload size arrays */
	scenario = 0;
	header_bytes[scenario++] = 0;
	header_bytes[scenario++] = send_immediate_max[dev][xtalk]/2;
	header_bytes[scenario++] = send_immediate_max[dev][xtalk] + 1;

	scenario = 0;
	data_bytes[scenario++] = send_immediate_max[dev][xtalk] + 1;
	data_bytes[scenario++] = send_immediate_max[dev][xtalk]/2 + 1;
	data_bytes[scenario++] = 0;

	for (test = 0; test < scenario; test++) {
	  parameters.header.iov_len = header_bytes[test];
	  parameters.data.iov_len = data_bytes[test];



	  fprintf (stderr, "===== PAMI_Send_immediate() functional test [%s][%s] %zu %zu (%d, %zu) -> (0, 0) =====\n\n", &device_str[dev][0], &xtalk_str[xtalk][0], header_bytes[test], data_bytes[test], task_id, xtalk);

	  fprintf (stderr, "before send immediate...\n");
	  result = PAMI_Send_immediate (context[xtalk], &parameters);
	  fprintf (stderr, "... after send immediate.\n");

	  if ( result != send_fail ) {
	      fprintf (stderr, "ERROR: PAMI_Send_immediate passed with rc = %d ...expected it to fail with rc = %d!!\n", result, send_fail);
	      return 1;
	  } 
	} /* end scenario loop */
      } /* end xtalk loop */
    } /* end device loop */
  } /* end task id != 0 */
#endif
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

  if ( __header_errors || __data_errors )
  {
    fprintf (stdout, "Error. immediate_send FAILED with %zu header errors and %zu data errors on task %d.\n", __header_errors, __data_errors, task_id);
    return 1;
  }
  else
  {
    fprintf (stdout, "Success (%d)\n", task_id);
    return 0;
  }
}
