/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file devices/prod/workqueue/WorkQueue.cc
 * \brief ???
 */

#include "WorkQueue.h"
#include "SharedWorkQueue.h"
#include "MemoryWorkQueue.h"

namespace DCMF
{
  namespace Device
  {
    namespace WorkQueue
    {
      WorkQueue::~WorkQueue() { DCMF_abort(); }
      NonVirtWorkQueue::~NonVirtWorkQueue() { DCMF_abort(); }
      SharedWorkQueue::~SharedWorkQueue() { DCMF_abort(); }
      MemoryWorkQueue::~MemoryWorkQueue() { DCMF_abort(); }
    };
  };
};
