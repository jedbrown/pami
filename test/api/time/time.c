/**
 *\file test/api/time/time.c
 *\brief Simple PAMI_Wtime() test before PAMI_Client_create()
 */

#include <pami.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
int main (int argc, char ** argv)
{
  pami_result_t rc;
  pami_client_t client;
  rc = PAMI_Client_create("TEST", &client, NULL, 0);
  assert(rc == PAMI_SUCCESS);

  fprintf (stdout, "Before PAMI_Wtime()\n");
  double t0 = PAMI_Wtime (client);
  sleep(4);
  fprintf (stdout, "After first PAMI_Wtime, value = %g\n", t0);
  double t1 = PAMI_Wtime (client);
  fprintf (stdout, "After second PAMI_Wtime, value = %g\n", t1);

  fprintf (stdout, "Elapsed time, value = %g\n", t1-t0);

  return 0;
};
