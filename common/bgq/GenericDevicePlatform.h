/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/bgq/GenericDevicePlatform.h
 * \brief ???
 */

#ifndef __common_bgq_GenericDevicePlatform_h__
#define __common_bgq_GenericDevicePlatform_h__

#ifdef __GNUC__

#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"
typedef XMI::Mutex::CounterMutex<XMI::Counter::GccProcCounter> GenericDeviceMutex;
typedef XMI::Counter::GccProcCounter GenericDeviceCounter;

#else /* !__GNUC__ */

#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/pthread/Pthread.h"
typedef XMI::Mutex::CounterMutex<XMI::Counter::Pthread> GenericDeviceMutex;
typedef XMI::Counter::Pthread GenericDeviceCounter;

#endif /* !__GNUC__ */

#include "util/queue/Queue.h"

/// \brief Queue type to use for messages
typedef XMI::MultiQueue<2,1>		GenericDeviceCompletionQueue;
typedef XMI::MultiQueue<2,0>		GenericDeviceMessageQueue;
typedef XMI::MultiQueueElem<2>		GenericDeviceMessageQueueElem;

/// \brief Queue type to use for threads (work)
typedef XMI::Queue		GenericDeviceWorkQueue;
typedef XMI::QueueElem		GenericDeviceWorkQueueElem;

#endif // __common_bgq_GenericDevicePlatform_h__
