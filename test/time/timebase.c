///
/// \file test/time/timebase.c
/// \brief Simple XMI_Wtimebase() test before XMI_Client_initialize()
///

#include "sys/xmi.h"
#include <stdio.h>
int main (int argc, char ** argv)
{
  fprintf (stdout, "Before XMI_Wtimebase()\n");
  unsigned long long t0 = XMI_Wtimebase ();
  fprintf (stdout, "After first XMI_Wtimebase, value = %lld\n", t0);
  unsigned long long t1 = XMI_Wtimebase ();
  fprintf (stdout, "After second XMI_Wtimebase, value = %lld\n", t1);

  fprintf (stdout, "Elapsed timebase, value = %lld\n", t1-t0);

  return 0;
};
