/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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
#include "util/queue/MultiQueue.h"

/// \brief Queue type to use for messages
#define GENDEVMSG_SEND_QNUM	0
#define GENDEVMSG_COMPL_QNUM	1
#define GENDEVMSG_NUM_QUEUES	2

/// \brief The type of queue used to implement generic device completion queue(s)
///
/// Must be compatible with GenericDeviceMessageQueue.
typedef XMI::MultiQueue<GENDEVMSG_NUM_QUEUES,GENDEVMSG_COMPL_QNUM>	GenericDeviceCompletionQueue;

/// \brief The type of queue used to implement device send queue(s)
///
/// Must be compatible with GenericDeviceCompletionQueue.
typedef XMI::MultiQueue<GENDEVMSG_NUM_QUEUES,GENDEVMSG_SEND_QNUM>	GenericDeviceMessageQueue;

/// \brief Queue type to use for generic device threads (work) queue(s)
typedef XMI::AtomicQueue<GenericDeviceMutex>	GenericDeviceWorkQueue;

#endif // __common_bgq_GenericDevicePlatform_h__
