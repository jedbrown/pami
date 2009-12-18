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
#include "util/queue/MultiQueue.h"

/// \brief Queue type to use for messages
#define GENDEVMSG_SEND_QNUM	0
#define GENDEVMSG_COMPL_QNUM	1
#define GENDEVMSG_NUM_QUEUES	2
typedef XMI::MultiQueue<GENDEVMSG_NUM_QUEUES,GENDEVMSG_COMPL_QNUM>	GenericDeviceCompletionQueue;
typedef XMI::MultiQueue<GENDEVMSG_NUM_QUEUES,GENDEVMSG_SEND_QNUM>	GenericDeviceMessageQueue;
typedef XMI::MultiQueueElem<GENDEVMSG_NUM_QUEUES>			GenericDeviceMessageQueueElem;

/// \brief Queue type to use for threads (work)
typedef XMI::Queue		GenericDeviceWorkQueue;
typedef XMI::QueueElem		GenericDeviceWorkQueueElem;

#endif // __common_bgp_GenericDevicePlatform_h__
