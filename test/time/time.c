///
/// \file tests/time/time.c
/// \brief Simple XMI_Wtime() test before XMI_Client_initialize()
///

#include "sys/xmi.h"

int main (int argc, char ** argv)
{
  fprintf (stdout, "Before XMI_Wtime()\n");
  double t0 = XMI_Wtime ();
  fprintf (stdout, "After first XMI_Wtime, value = %g\n", t0);
  double t1 = XMI_Wtime ();
  fprintf (stdout, "After second XMI_Wtime, value = %g\n", t1);

  fprintf (stdout, "Elapsed time, value = %g\n", t1-t0);

  return 0;
};




