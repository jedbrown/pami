/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/cau/caumessage.h
 * \brief ???
 */

#ifndef __components_devices_cau_caumessage_h__
#define __components_devices_cau_caumessage_h__

#include <pami.h>
#include "util/common.h"
#include "util/queue/Queue.h"
#include "common/lapiunix/lapifunc.h"
#include <list>

namespace PAMI
{
  namespace Device
  {
    class CAUMcastMessage
    {
    public:
      unsigned toimpl;
    };

    class CAUMsyncMessage
    {
    public:
      unsigned toimpl;
    };

    class CAUMcombineMessage
    {
    public:
      unsigned toimpl;
    };

    class CAUM2MMessage
    {
    public:
      unsigned toimpl;
    };

  };
};

#endif // __components_devices_cau_caubasemessage_h__
