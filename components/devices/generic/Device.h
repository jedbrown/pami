/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_generic_Device_h__
#define __components_devices_generic_Device_h__

// The section of the Generic::Device implements the interfaces
// used by the sub-devices. See GenericDevice.h for interfaces
// used by messaging (advance).

#include "SysDep.h"
#include "components/devices/BaseDevice.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/generic/Message.h"
#include "components/atomic/Mutex.h"
#include "sys/xmi.h"

// NOTE, these typedefs choose which type of atomic to use for a specific device.
// since the fine details of all this have not been designed yet, these will
// remain here until such time.
//#warning The atomic class to use for a platform should be a template parameter. These typedefs belong in the context class.
#if defined(__bgp__) and !defined(__bgq__)
#include "spi/kernel_interface.h"
#include "components/atomic/bgp/LockBoxMutex.h"
#include "components/atomic/bgp/LockBoxCounter.h"
typedef XMI::Mutex::LockBoxProcMutex GenericDeviceMutex;
typedef XMI::Counter::LockBoxProcCounter GenericDeviceCounter;

#else
// Other platform optimizations to follow...

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

#endif /* !__bgp__ */

////////////////////////////////////////////////////////////////////////
///  \file components/devices/generic/Device.h
///  \brief Generic Device
///
///  The Generic classes implement a BaseDevice and a Message object
///  to post into the queueing system.  The GI device is currently
///  used to implement barriers, so the Generic device posts a message
///  and uses a interprocess communication sysdep to signal the Generic wire
///  This is used to implement
///  -
///  - Barriers
///
///  Definitions:
///  - GenericMessage:  An Generic message
///  - Device:      Queue System for messages
///
///  Namespace:  XMI, the messaging namespace.
///
////////////////////////////////////////////////////////////////////////
namespace XMI {
namespace Device {
namespace Generic {

class ThreadQueue : public Queue {
public:
	ThreadQueue() : Queue() { }

	ThreadQueue(XMI::SysDep &sd) : Queue()
	{
		// need status/result here...
		_mutex.init(&sd);
	}

	GenericDeviceMutex *mutex() { return &_mutex; }

protected:
	GenericDeviceMutex _mutex;
}; /* class ThreadQueue */

//////////////////////////////////////////////////////////////////////
///  \brief A Device implmentation of a Queuesystem
///  This class implements a queue system. The user posts to the queue
///  and the interprocess communications are called in order until all of them
///  have been executed.
//////////////////////////////////////////////////////////////////////
class Device {

	inline void __platform_generic_init(XMI::SysDep &sd, Device *generic);
	inline int __platform_generic_advanceRecv(size_t context);

public:
	//////////////////////////////////////////////////////////////////
	/// \brief  A device
	//////////////////////////////////////////////////////////////////
	inline Device();

	inline void init(XMI::SysDep &sd, xmi_context_t ctx, size_t context, size_t num_contexts, Device *generics);

	inline bool isAdvanceNeeded();

	//////////////////////////////////////////////////////////////////
	/// \brief     Advance routine for the generic device.
	///
	/// \return	number of events processed
	//////////////////////////////////////////////////////////////////
	inline int advance();

	inline void postThread(GenericThread *thr) {
		__Threads.pushTail(thr);
	}
	//////////////////////////////////////////////////////////////////
	/// \brief       Post a message to the device queuing system
	/// \param msg:  Message to be queued/advanced.
	//////////////////////////////////////////////////////////////////
	inline void post(GenericMessage *msg, GenericAdvanceThread *thr,
							size_t len, int num) {
		// early advance was done by the "real" device post()

		// get access to client-global contexts array,
		// in order to stage work on separate contexts.
		// the context specified in the msg is used only for
		// completion, even the first thread of work is posted
		// to a different context.

		size_t t = msg->getContext();
		size_t n = __nContexts;
		Generic::Device *g0 = __generics;
		g0[t].__GenericQueue.pushTail(msg);

		// round-robin threads to available "channels"...
		// does this need to be made thread-safe?
		// we expect to make enqueue routines atomic, lockless.
		// note: might be called from a completion callback.

		// t = msg->getClient()->__lastThreadUsed;
		for (int x = 0; x < num; ++x) {
			if (!thr->isDone()) {
				if (++t >= n) {
					t = 0;
				}
#ifdef USE_WAKEUP_VECTORS
				if (thr->isPolled()) {
					g0[t].__Threads.pushTail(thr);
				}
#else /* !USE_WAKEUP_VECTORS */
				g0[t].__Threads.pushTail(thr);
#endif /* !USE_WAKEUP_VECTORS */
			}
			thr = (GenericAdvanceThread *)((char *)thr + len);
		}
		// msg->getClient()->__lastThreadUsed = t;
	}

	inline size_t contextId() { return __contextId; }
	inline size_t nContexts() { return __nContexts; }

private:
	/// \brief Advance a reception channel
	///
	/// Check a channel for received messages (or within the context of a channel)
	///
	/// \param[in] channel	The channel to check, or context to check within
	/// \return	Number of work units performed (typically, 0 or 1)
	///
	inline int __advanceRecv();

	//////////////////////////////////////////////////////////////////
	/// \brief Storage for the queue of messages
	//////////////////////////////////////////////////////////////////
	MultiQueue<2, 1> __GenericQueue;

	//////////////////////////////////////////////////////////////////
	/// \brief Storage for the queue of threads (a.k.a. channels)
	//////////////////////////////////////////////////////////////////
	ThreadQueue __Threads;

	xmi_context_t __context;
	size_t __contextId;
	size_t __nContexts;
	Generic::Device *__generics;

#ifdef USE_WAKEUP_VECTORS
	void *__wakeupVectors;
	void *__myWakeupVectors;
	GenericAdvanceThread *__thrRegistry[MAX_REG_THREADS];
	int __numRegThreads;

	//////////////////////////////////////////////////////////////////
	/// \brief SysDep object - only used to get WakeupManager
	//////////////////////////////////////////////////////////////////
	XMI::SysDep &__sysdep;
#endif /* USE_WAKEUP_VECTORS */

}; /* class Device */

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_device_h__ */
