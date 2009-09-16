/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/BinomialTree.cc
 * \brief The Binomial Schedule for Barrier, Broadcast, and [All]Reduce.
 *
 * Also used by the Rectangle schedule to implement Reduce.
 */

#include "./BinomialTree.h"


#ifndef __GNUC__
/**
 * For reasons that are not quite clear, the IBM XLC++ compiler will
 * not generate the vtable for this object. It appears to be related
 * to the fact that it is never "new"ed, but always a member of
 * another object.
 *
 * This code (which is never called) does "new" the object, causing
 * XLC++ to put the vtable in this file's resultant object file.
 */
void silly()
{
  char buffer[sizeof(CCMI::Schedule::BinomialTreeSchedule)];
  CCMI::Schedule::BinomialTreeSchedule * p = (CCMI::Schedule::BinomialTreeSchedule *) buffer;
  p = new(p) CCMI::Schedule::BinomialTreeSchedule;
}
#endif
