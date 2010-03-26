/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/TorusRect.cc
 * \brief The TorusRect Schedule for Barrier, Broadcast, and [All]Reduce.
 *
 */

#include "algorithms/schedule/TorusRect.h"


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
void willy()
{
}
#endif
