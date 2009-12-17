/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/bgp/GenericDevicePlatform.h
 * \brief ???
 */

#ifndef __common_bgp_GenericDevicePlatform_h__
#define __common_bgp_GenericDevicePlatform_h__

#include "spi/kernel_interface.h"
#include "components/atomic/bgp/LockBoxMutex.h"
#include "components/atomic/bgp/LockBoxCounter.h"
typedef XMI::Mutex::LockBoxProcMutex GenericDeviceMutex;
typedef XMI::Counter::LockBoxProcCounter GenericDeviceCounter;

#include "util/queue/Queue.h"

/// \brief Queue type to use for messages
typedef XMI::MultiQueue<2,1>		GenericDeviceCompletionQueue;
typedef XMI::MultiQueue<2,0>		GenericDeviceMessageQueue;
typedef XMI::MultiQueueElem<2>		GenericDeviceMessageQueueElem;

/// \brief Queue type to use for threads (work)
typedef XMI::Queue		GenericDeviceWorkQueue;
typedef XMI::QueueElem		GenericDeviceWorkQueueElem;

#endif // __common_bgp_GenericDevicePlatform_h__
