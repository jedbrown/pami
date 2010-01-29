///
/// \file test/multisync.c
/// \brief Simple Multisync test
///

#include "sys/xmi.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

volatile unsigned       _g_multisync_active;

void cb_multisync (void *ctxt, void * clientdata, xmi_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

static double timer()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}

void _multisync (xmi_context_t context, xmi_multisync_t *multisync)
{
  _g_multisync_active++;
  xmi_result_t result;
  result = XMI_Multisync(multisync);
  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue multisync collective. result = %d\n", result);
      exit(1);
    }
  while (_g_multisync_active)
    result = XMI_Context_advance (context, 1);

}



int main (int argc, char ** argv)
{
  xmi_client_t  client;
  xmi_context_t context;
  xmi_result_t  result = XMI_ERROR;
  char          cl_string[] = "TEST";
  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
        return 1;
      }

      { size_t _n = 1; result = XMI_Context_createv(client, NULL, 0, &context, _n); }
      if (result != XMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", result);
            return 1;
          }


      xmi_configuration_t configuration;
      configuration.name = XMI_TASK_ID;
      result = XMI_Configuration_query(client, &configuration);
      if (result != XMI_SUCCESS)
          {
            fprintf (stderr,
                     "Error. Unable query configuration (%d). result = %d\n",
                     configuration.name, result);
            return 1;
          }
      size_t task_id = configuration.value.intval;

      configuration.name = XMI_NUM_TASKS;
      result = XMI_Configuration_query(client, &configuration);
      if (result != XMI_SUCCESS)
          {
            fprintf (stderr,
                     "Error. Unable query configuration (%d). result = %d\n",
                     configuration.name, result);
            return 1;
          }
      size_t          sz    = configuration.value.intval;


      xmi_topology_t    topo;
      xmi_task_t       *tasklist = (xmi_task_t*)malloc(sz*sizeof(xmi_task_t));
      for(size_t i=0; i<sz; i++)tasklist[i]=i;
      //XMI_Topology_create_range(&topo,0,sz-1);
      XMI_Topology_create_list(&topo,tasklist,sz);

      xmi_multisync_t multisync;
      multisync.client             = 0;
      multisync.context            = (size_t)0;
      multisync.cb_done.function   = cb_multisync;
      multisync.cb_done.clientdata = (void*)&_g_multisync_active;
      multisync.connection_id      = 0;
      multisync.roles              = 0;
      multisync.participants       = &topo;

      if(!task_id)
        fprintf(stderr, "Test Multisync\n");
      _multisync(context, &multisync);

      double ti, tf, usec;
      if(!task_id)
        fprintf(stderr, "Test Multisync Correctness (sleeping 5 seconds, tolerance .5 sec)\n");
      if(!task_id)
          {
            ti=timer();
            multisync.connection_id++;
            _multisync(context, &multisync);
            tf=timer();
            usec = tf - ti;
            if(usec < 4500000.0 || usec > 5500000.0)
              fprintf(stderr, "Multisync error: usec=%f want between %f and %f!\n",
                      usec, 4500000.0, 5500000.0);
            else
              fprintf(stderr, "Multisync correct!\n");
          }
      else
          {
            sleep(5);
            multisync.connection_id++;
            _multisync(context, &multisync);
          }

      if(!task_id)
        fprintf(stderr, "Test Multisync Performance\n");
      int niter=10000;
      multisync.connection_id++;
      _multisync(context, &multisync);

      ti=timer();
      int i;
      for(i=0; i<niter; i++)
        multisync.connection_id++;
        _multisync(context, &multisync);

      tf=timer();
      usec = tf - ti;

      if(!task_id)
        fprintf(stderr,"multisync: time=%f usec\n", usec/(double)niter);


      result = XMI_Context_destroy (context);
      if (result != XMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to destroy xmi context. result = %d\n", result);
            return 1;
          }

      result = XMI_Client_finalize (client);
      if (result != XMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
            return 1;
          }

      free(tasklist);
      return 0;
};
