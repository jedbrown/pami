/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bsr/bsrmessage.h
 * \brief ???
 */

#ifndef __components_devices_bsr_bsrmessage_h__
#define __components_devices_bsr_bsrmessage_h__

#include <pami.h>
#include "util/common.h"
#include "common/lapiunix/lapifunc.h"
#include "TypeDefs.h"
#include "components/memory/MemoryAllocator.h"

namespace PAMI
{
  namespace Device
  {
    class BSRMsyncMessage : public Generic::GenericThread
    {
    public:
      BSRMsyncMessage(pami_work_function workfn, void* cookie, pami_callback_t cb_done, void* model):
        GenericThread(workfn, cookie),
        _cb_done(cb_done),
        _multisyncmodel(model)
        {
        }

      pami_callback_t    _cb_done;
      void              *_multisyncmodel;
    };

    class BSRMcastMessage
    {
    };

    class BSRMcombineMessage
    {
    };


  };
};

#endif // __components_devices_bsr_bsrbasemessage_h__
