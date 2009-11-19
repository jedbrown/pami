/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/tests/generic/barrier.h
 * \brief ccmi barrier common routines
 */

#ifndef __algorithms_tests_generic_barrier_h__
#define __algorithms_tests_generic_barrier_h__

#include "common.h"

inline void barrier_advance ()
{
  TRACE_TEST_VERBOSE((stderr, "%s:%s\n", argv0,__PRETTY_FUNCTION__));
  done = 0;
  CCMI_Barrier(&geometry,
               common_done,
               consistency);
  while(!done)
  {
    CCMI_Generic_adaptor_advance();
  }
  return;
}


void initialize(CCMI_Barrier_Protocol barrier_protocol,
                CCMI_Barrier_Protocol lbarrier_protocol)
{

  initialize_common(barrier_protocol, lbarrier_protocol);

  common_done.function  = done_callback;
  common_done.clientdata= NULL;

  if(!CCMI_Geometry_analyze(&geometry, &barrier_reg))
  {
    no_op("Not a supported geometry - NO OP\n");
    exit(0);
  }



  return;
}

#endif
