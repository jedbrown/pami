/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/WorkQueue.cc
 * \brief This file ensures that the VTABLE exists for anything using these classes...
 */

#include "util/common.h"
#include "components/devices/workqueue/WorkQueue.h"
#include "components/devices/workqueue/SharedWorkQueue.h"
#include "components/devices/workqueue/MemoryWorkQueue.h"

namespace XMI
{
  namespace Device
  {
    namespace WorkQueue
    {
      WorkQueue::~WorkQueue() { XMI_abort(); }
      NonVirtWorkQueue::~NonVirtWorkQueue() { XMI_abort(); }
      SharedWorkQueue::~SharedWorkQueue() { XMI_abort(); }
      MemoryWorkQueue::~MemoryWorkQueue() { XMI_abort(); }
    };
  };
};
