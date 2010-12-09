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

#include "util/queue/Queue.h"
#include "util/queue/MutexedQueue.h"
#include "util/queue/MultiQueue.h"

#include "components/atomic/indirect/IndirectCounter.h"
#include "components/atomic/indirect/IndirectMutex.h"

#ifdef __GNUC__

#include "components/atomic/gcc/GccCounter.h"
typedef PAMI::Counter::Gcc GenericDeviceCounter;

#else /* !__GNUC__ */

#include "components/atomic/pthread/Pthread.h"
typedef PAMI::Counter::Pthread GenericDeviceCounter;

#endif /* !__GNUC__ */

#include "util/queue/Queue.h"
#include "util/queue/MutexedQueue.h"
#include "util/queue/MultiQueue.h"
#include "util/queue/bgq/ArrayBasedQueue.h"

/// \brief Queue type to use for messages
#define GENDEVMSG_SEND_QNUM	0
#define GENDEVMSG_COMPL_QNUM	1
#define GENDEVMSG_NUM_QUEUES	2

/// \brief The type of queue used to implement generic device completion queue(s)
///
/// Must be compatible with GenericDeviceMessageQueue.
typedef PAMI::MultiQueue<GENDEVMSG_NUM_QUEUES,GENDEVMSG_COMPL_QNUM>	GenericDeviceCompletionQueue;

/// \brief The type of queue used to implement device send queue(s)
///
/// Must be compatible with GenericDeviceCompletionQueue.
typedef PAMI::MultiQueue<GENDEVMSG_NUM_QUEUES,GENDEVMSG_SEND_QNUM>	GenericDeviceMessageQueue;

/// \brief Queue type to use for generic device threads (work) queue(s)
#include "util/queue/GccThreadSafeQueue.h"
#include "components/atomic/bgq/L2Mutex.h"
typedef PAMI::GccThreadSafeQueue<PAMI::MutexedQueue<PAMI::Mutex::BGQ::IndirectL2> > GenericDeviceWorkQueue;
//typedef PAMI::ArrayBasedQueue<PAMI::Mutex::BGQ::L2InPlaceMutex> GenericDeviceWorkQueue;

#endif // __common_bgq_GenericDevicePlatform_h__
