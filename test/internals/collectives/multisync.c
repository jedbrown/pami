///
/// \file test/internals/collectives/multisync.c
/// \brief Simple Multisync test
///

#include <pami.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

volatile unsigned       _g_multisync_active;

void cb_multisync (void *ctxt, void * clientdata, pami_result_t err)
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

void _multisync (pami_context_t context, pami_multisync_t *multisync)
{
  _g_multisync_active++;
  pami_result_t result;
  result = PAMI_Multisync(multisync);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to issue multisync collective. result = %d\n", result);
      exit(1);
    }
  while (_g_multisync_active)
    result = PAMI_Context_advance (context, 1);

}



int main (int argc, char ** argv)
{
  pami_client_t  client;
  pami_context_t context;
  pami_result_t  result = PAMI_ERROR;
  char          cl_string[] = "TEST";
  result = PAMI_Client_create (cl_string, &client, NULL, 0);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", result);
        return 1;
      }

      {  result = PAMI_Context_createv(client, NULL, 0, &context, 1); }
      if (result != PAMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to create pami context. result = %d\n", result);
            return 1;
          }


      pami_configuration_t configuration;
      configuration.name = PAMI_CLIENT_TASK_ID;
      result = PAMI_Client_query(client, &configuration,1);
      if (result != PAMI_SUCCESS)
          {
            fprintf (stderr,
                     "Error. Unable query configuration (%d). result = %d\n",
                     configuration.name, result);
            return 1;
          }
      size_t task_id = configuration.value.intval;

      configuration.name = PAMI_CLIENT_NUM_TASKS;
      result = PAMI_Client_query(client, &configuration,1);
      if (result != PAMI_SUCCESS)
          {
            fprintf (stderr,
                     "Error. Unable query configuration (%d). result = %d\n",
                     configuration.name, result);
            return 1;
          }
      size_t          sz    = configuration.value.intval;


      pami_topology_t    topo;
      pami_task_t       *tasklist = (pami_task_t*)malloc(sz*sizeof(pami_task_t));
      size_t i;
      for(i=0; i<sz; i++)tasklist[i]=i;
      //PAMI_Topology_create_range(&topo,0,sz-1);
      PAMI_Topology_create_list(&topo,tasklist,sz);

      pami_multisync_t multisync;
      multisync.client             = (size_t)client;
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
      for(i=0; i<niter; i++)
        multisync.connection_id++;
        _multisync(context, &multisync);

      tf=timer();
      usec = tf - ti;

      if(!task_id)
        fprintf(stderr,"multisync: time=%f usec\n", usec/(double)niter);


      result = PAMI_Context_destroyv(&context, 1);
      if (result != PAMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
            return 1;
          }

      result = PAMI_Client_destroy(&client);
      if (result != PAMI_SUCCESS)
          {
            fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
            return 1;
          }

      free(tasklist);
      return 0;
};
