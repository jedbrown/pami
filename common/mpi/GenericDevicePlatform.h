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

#include "components/atomic/native/NativeCounter.h"
typedef PAMI::Counter::Native GenericDeviceCounter;
//#include "components/atomic/pthread/Pthread.h"
//typedef PAMI::Counter::Pthread GenericDeviceCounter;

#include "util/queue/Queue.h"
#include "util/queue/MutexedQueue.h"
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
