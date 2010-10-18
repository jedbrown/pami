/*/ */
/*/ \file test/api/time/timebase.c */
/*/ \brief Simple PAMI_Wtimebase() test before PAMI_Client_create() */
/*/ */

#include <pami.h>
#include <stdio.h>
int main (int argc, char ** argv)
{
  fprintf (stdout, "Before PAMI_Wtimebase()\n");
  unsigned long long t0 = PAMI_Wtimebase ();
  fprintf (stdout, "After first PAMI_Wtimebase, value = %lld\n", t0);
  unsigned long long t1 = PAMI_Wtimebase ();
  fprintf (stdout, "After second PAMI_Wtimebase, value = %lld\n", t1);

  fprintf (stdout, "Elapsed timebase, value = %lld\n", t1-t0);

  return 0;
};
