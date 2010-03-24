/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/mpi/GenericDevicePlatform.h
 * \brief ???
 */

#ifndef __common_mpi_GenericDevicePlatform_h__
#define __common_mpi_GenericDevicePlatform_h__

#ifdef __GNUC__

#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"
typedef PAMI::Mutex::CounterMutex<PAMI::Counter::GccProcCounter> GenericDeviceMutex;
typedef PAMI::Counter::GccProcCounter GenericDeviceCounter;

#else /* !__GNUC__ */

#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/pthread/Pthread.h"
typedef PAMI::Mutex::CounterMutex<PAMI::Counter::Pthread> GenericDeviceMutex;
typedef PAMI::Counter::Pthread GenericDeviceCounter;

#endif /* !__GNUC__ */

#include "util/queue/Queue.h"
#include "util/queue/MultiQueue.h"

/// \brief Queue type to use for messages
#define GENDEVMSG_SEND_QNUM	0
#define GENDEVMSG_COMPL_QNUM	1
#define GENDEVMSG_NUM_QUEUES	2
typedef PAMI::MultiQueue<GENDEVMSG_NUM_QUEUES,GENDEVMSG_COMPL_QNUM>	GenericDeviceCompletionQueue;
typedef PAMI::MultiQueue<GENDEVMSG_NUM_QUEUES,GENDEVMSG_SEND_QNUM>	GenericDeviceMessageQueue;

/// \brief Queue type to use for threads (work)
typedef PAMI::Queue		GenericDeviceWorkQueue;

#endif // __common_mpi_GenericDevicePlatform_h__
