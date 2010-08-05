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

#ifndef TRACE_ERR
#define TRACE_ERR(x)
#endif

// Testcase delay/sleep function (n seconds)
#define delayTest(n) sleep(n)

// Redefine delay/sleep for bgq mambo workarounds
#ifdef __pami_target_bgq__
#ifdef ENABLE_MAMBO_WORKAROUNDS
extern unsigned __isMambo();
// sleep() doesn't appear to work in mambo right now.  A hackey simulation...
#undef delayTest 
#define delayTest(n) _mamboSleep(n, __LINE__)
unsigned _mamboSleep(unsigned seconds, unsigned from)
{
  if (__isMambo()) /// \todo mambo hack
  {
    double dseconds = ((double)seconds)/1000; //mambo seconds are loooong.
    double start = PAMI_Wtime (), d=0;
    while (PAMI_Wtime() < (start+dseconds))
    {
      int i=0;
      for (; i<200000; ++i) ++d;
      TRACE_ERR((stderr, "%s:%d sleep - %.0f, start %f, %f < %f\n",__PRETTY_FUNCTION__,from,d,start,PAMI_Wtime(),start+dseconds));
    }
    return 0;
  }
  else return sleep(seconds);
}
#endif
#endif

#endif /* __test_api_pami_util_h__*/
