///
/// \file test/time/tick.c
/// \brief Simple XMI_Wtick() test before XMI_Client_initialize()
///

#include "sys/xmi.h"
#include <stdio.h>
int main (int argc, char ** argv)
{
  fprintf (stdout, "Before XMI_Wtick()\n");
  double value = XMI_Wtick ();
  fprintf (stdout, "After XMI_Wtick, value = %g\n", value);

  return 0;
};
