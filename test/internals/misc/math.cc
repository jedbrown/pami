/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/misc/math.cc
 * \brief ???
 */

#include <stdio.h>

#include "Math.h"

int main(int argc, char **argv)
{
  unsigned utmp[1];
  PAMI::Math::sum (utmp, utmp);
  PAMI::Math::sum<unsigned> (utmp, utmp, 1);
  PAMI::Math::sum (utmp, utmp, 1);

  double tmp[10];
  PAMI::Math::sum (tmp, tmp);
  PAMI::Math::sum (tmp, tmp, 10);

  double tmp64[64];
  PAMI::Math::sum (tmp64, tmp64);

  return 0;
}
