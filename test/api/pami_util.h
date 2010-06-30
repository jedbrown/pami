/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/pami_util.h
 * \brief ???
 */

#ifndef __test_api_pami_util_h__
#define __test_api_pami_util_h__

#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))


static double timer()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}


#include "init_util.h"
#include "coll_util.h"


#endif /* __test_api_pami_util_h__*/
