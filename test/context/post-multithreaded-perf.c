///
/// \file test/context/post-multithreaded-perf.c
/// \brief Multithreaded XMI_Context_post() performance test
///

#include "sys/xmi.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define ENABLE_TRACE

#ifdef ENABLE_TRACE
#define TRACE(x) fprintf x
#else
#define TRACE(x)
#endif

#define MAXTHREADS 4
//#define ITERATIONS 1000
#define ITERATIONS 1

xmi_context_t   _context[MAXTHREADS];
volatile size_t _value[MAXTHREADS];
xmi_work_t      _work[MAXTHREADS];

xmi_result_t do_work (xmi_context_t   context,
                      void          * cookie)
{
  TRACE((stderr, ">> do_work (%0x08x, %p)\n", (unsigned)context, cookie));

  size_t * value = (size_t *) cookie;
  TRACE((stderr, "   do_work (), *value = %zu -> %zu\n", *value, *value - 1));
  *value--;

  TRACE((stderr, "<< do_work ()\n"));
  return XMI_SUCCESS;
}

void * thread_main (void * arg)
{
  size_t id = (size_t) arg;
  TRACE((stderr, ">> thread_main (%zu)\n", id));

  xmi_context_t context = _context[id];

  TRACE((stderr, "   thread_main (%zu) .. 0\n", id));

  /* Lock this context */
  xmi_result_t result = XMI_Context_lock (context);

  TRACE((stderr, "   thread_main (%zu) .. 1\n", id));

  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to lock the xmi context. result = %d\n", result);
      exit(1);
    }

  TRACE((stderr, "   thread_main (%zu) .. 2, _value[id] = %zu\n", id, _value[id]));

  while (_value[id] > 0)
    {
      TRACE((stderr, "   thread_main (%zu) .. 3\n", id));
      result = XMI_Context_advance (context, 100);
#ifdef TRACE

      if (result != XMI_SUCCESS)
        {
          fprintf (stderr, "Error. Unable to advance the xmi context. result = %d\n", result);
          exit(1);
        }

#endif
    }
  TRACE((stderr, "   thread_main (%zu) .. 4\n", id));

  /* Unlock this context */
  result = XMI_Context_unlock (context);

  TRACE((stderr, "   thread_main (%zu) .. 5\n", id));

  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to unlock the xmi context. result = %d\n", result);
      exit(1);
    }

  TRACE((stderr, "<< thread_main (%zu)\n", id));
  return NULL;
}


int main (int argc, char ** argv)
{
  xmi_client_t client;
  //xmi_configuration_t * configuration = NULL;
  char                  cl_string[] = "TEST";
  xmi_result_t result = XMI_ERROR;
  size_t i;

  TRACE((stderr, ">> main ()\n"));

  result = XMI_Client_initialize (cl_string, &client);

  TRACE((stderr, "   main () .. 0\n"));

  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
      return 1;
    }

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query(client, &configuration);
  xmi_task_t task = configuration.value.intval;

  TRACE((stderr, "   main () .. 1\n"));

  /* Initialize the contexts */
  result = XMI_Context_createv (client, NULL, 0, &_context[0], MAXTHREADS);

  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to create first xmi context. result = %d\n", result);
      return 1;
    }

  TRACE((stderr, "   main () .. 2\n"));

  if (task == 0)
    {
      /* Initialize the post counters */
      for (i = 0; i < MAXTHREADS; i++)
        {
          _value[i] = ITERATIONS;
        }

      /* Create the "helper" or "endpoint" threads */
      pthread_t thread[MAXTHREADS];
      int rc = 0;
      size_t t, num_threads = 0;

  TRACE((stderr, "   main () .. 3\n"));

      for (i = 0; i < MAXTHREADS && rc == 0; i++)
        {
          rc = pthread_create (&thread[i], NULL, thread_main, (void *)(i));
          if (rc == 0) num_threads++;
        }

  TRACE((stderr, "   main () .. 4\n"));

      if (num_threads == 0)
        {
          fprintf (stderr, "Error. Unable to create any threads.\n");
        }
      else
        {
          fprintf (stdout, "XMI_Context_post() multi-threaded performance test\n");
          fprintf (stdout, "\n");
          fprintf (stdout, "  Number of 'receiver' threads:           %8zu\n", num_threads);
          fprintf (stdout, "  Number of posts to each thread:         %8zu\n", ITERATIONS);
          fprintf (stdout, "\n");

  TRACE((stderr, "   main () .. 5\n"));

          /* wait a bit to give threads time to start */
          usleep (1000);

  TRACE((stderr, "   main () .. 6\n"));

          unsigned long long t0 = XMI_Wtimebase();

          /* post all of the work */
          for (i = 0; i < ITERATIONS; i++)
            {
              for (t = 0; t < num_threads; t++)
                {
  TRACE((stderr, "   main () .. 7, %zu %zu\n", i, t));
                  result = XMI_Context_post (_context[t], &_work[t], do_work, (void *) & _value[t]);
#ifdef TRACE

                  if (result != XMI_SUCCESS)
                    {
                      fprintf (stderr, "Error. Unable to post work to context[%zu]. result = %d\n", t, result);
                      return 1;
                    }

#endif
                }
            }
  TRACE((stderr, "   main () .. 8\n"));

          /* wait until all of the work is done */
          for (t = 0; t < num_threads; t++)
            {
  TRACE((stderr, "   main () .. 9, _value[%zu] = %zu\n", t, _value[t]));
              while (_value[t] > 0);
            }

          unsigned long long t1 = XMI_Wtimebase();
  TRACE((stderr, "   main () .. 10\n"));

          unsigned long long cycles = ((t1 - t0) / ITERATIONS) / num_threads;

          fprintf (stdout, "\n");
          fprintf (stdout, "  Average number of cycles for each post: %8lld\n", cycles);
          fprintf (stdout, "\n");
        }
    }

  TRACE((stderr, "   main () .. 11\n"));

  for (i = 0; i < MAXTHREADS; i++)
    {
  TRACE((stderr, "   main () .. 12\n"));
      result = XMI_Context_destroy (_context[i]);

      if (result != XMI_SUCCESS)
        {
          fprintf (stderr, "Error. Unable to destroy context %zu. result = %d\n", i, result);
          return 1;
        }
    }
  TRACE((stderr, "   main () .. 13\n"));

  result = XMI_Client_finalize (client);
  TRACE((stderr, "   main () .. 14\n"));

  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
      return 1;
    }

  TRACE((stderr, "<< main ()\n"));

  return 0;
};
