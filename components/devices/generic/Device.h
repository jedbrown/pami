/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_generic_device_h__
#define __components_devices_generic_device_h__

// The section of the Generic::Device implements the interfaces
// used by the sub-devices. See GenericDevice.h for interfaces
// used by messaging (advance).

#include "components/devices/BaseDevice.h"
#include "components/sysdep/SysDep.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/generic/Message.h"
#include "components/atomic/Mutex.h"
#include "components/atomic/bgp/LockBoxMutex.h"
#include "components/atomic/bgp/LockBoxCounter.h"
#include "sys/xmi.h"

#ifdef __bgp__
#include "spi/kernel_interface.h"
#include "components/sysdep/bgp/BgpSysDep.h"
typedef XMI::Mutex::LockBoxProcMutex<XMI_SYSDEP_CLASS> GenericDeviceMutex;
typedef XMI::Counter::LockBoxProcCounter<XMI_SYSDEP_CLASS> GenericDeviceCounter;
#endif /* __bgp__ */


// For BG/P, NUM_CORES is the max number of threads.
#define NUM_PROCESSES	NUM_CORES	// at most
#define NUM_CHANNELS	NUM_CORES
#define MAX_THREADS_PER_PROC	NUM_CORES // * NUM_THREADS...
#define MAX_REG_THREADS	16	// >= total max threads in all subdevices

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

	ThreadQueue(XMI_SYSDEP_CLASS &sd) : Queue()
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

public:
	//////////////////////////////////////////////////////////////////
	/// \brief  A device
	//////////////////////////////////////////////////////////////////
	inline Device(XMI_SYSDEP_CLASS &sd);

	inline void init(XMI_SYSDEP_CLASS &sd);

	inline bool isAdvanceNeeded();

	// Helper thread runs this...
	// NOT run from application call to XMI_Messager_advance()!
	static void advanceHelper(uint32_t arg1,	// Generic::Device *
				uint32_t arg2,		// thread index
				uint32_t arg3, uint32_t arg4);


	//////////////////////////////////////////////////////////////////
	/// \brief     Advance routine for the device.  This pulls the
	///            message from the front of the queue and calls the
	///            advance routine on the message.
	/// \returns:  Return code of the advance routine (number of
	///            events processed)
	//////////////////////////////////////////////////////////////////
	inline int advance();

	//////////////////////////////////////////////////////////////////
	/// \brief       Post a message to the device queuing system
	/// \param msg:  Message to be queued/advanced.
	//////////////////////////////////////////////////////////////////
	inline void post(GenericMessage *msg, GenericAdvanceThread *thr,
							size_t len, int num) {
		// early advance was done by the "real" device post()
		{ int t;
#ifdef __bgp__
		t = Kernel_PhysicalProcessorID();
#else
		t = 0;
#endif
		__GenericQueue[t].pushTail(msg);
		}
		// round-robin threads to available "channels"...
		// does this need to be made thread-safe?
		// note: might be called from a completion callback.
		// only assigns work to HW threads, unless there are no
		// HW threads in which case all work is on a single (SW) thread.
		int t = msg->numThreadsWanted();
		if (t > __maxHWThreads) t = __maxHWThreads;
		for (int x = 0; x < num; ++x) {
			if (!thr->isDone()) {
				int n = thr->getChannel();
#ifdef USE_WAKEUP_VECTORS
				if (thr->isPolled()) {
					__Threads[n].pushTail(thr);
				}
#else /* !USE_WAKEUP_VECTORS */
				__Threads[n].pushTail(thr);
#endif /* !USE_WAKEUP_VECTORS */
				if (t > 0) {
#ifdef __bgp__
					// Just kick off threads... they compete for work units.
					// Presumably, each will get one...
					// TBD: need to keep track of in-use threads...
					// for now we assume all are available...
					(void)Kernel_DeliverCommSignal(0, _thrIPI[n]);
#else
#warning generic device cannot activate thread to do work
#endif
					--t;
				}
			}
			thr = (GenericAdvanceThread *)((char *)thr + len);
		}
	}

	inline void registerThreads(GenericAdvanceThread *thr, size_t len, int num) {
		int x, y;
		int n = __lastThreadUsed;
		for (x = 0, y = __numRegThreads; x < num; ++x, ++y) {
#ifdef USE_WAKEUP_VECTORS
			XMI_assert_debug(y < MAX_REG_THREADS);
			__thrRegistry[y] = thr;
			void *vec = __sysdep.wakeupManager().getWakeVec(__myWakeupVectors, n * MAX_REG_THREADS + y);
			thr->setWakeVec(vec);
#endif /* USE_WAKEUP_VECTORS */
			thr->setChannel(n);
			thr = (GenericAdvanceThread *)((char *)thr + len);
			if (++n >= __maxHWThreads) n = 0;
		}
		__lastThreadUsed = n;
		__numRegThreads = y;
	}

private:
	/// \brief Advance a reception channel
	///
	/// Check a channel for received messages (or within the context of a channel)
	///
	/// \param[in] channel	The channel to check, or context to check within
	/// \return	Number of work units performed (typically, 0 or 1)
	///
	inline int __advanceRecv(int channel);
	inline int __advanceRecv();

	//////////////////////////////////////////////////////////////////
	/// \brief Storage for the queue of messages
	//////////////////////////////////////////////////////////////////
	MultiQueue<2, 1> __GenericQueue[MAX_THREADS_PER_PROC];

	//////////////////////////////////////////////////////////////////
	/// \brief Storage for the queue of threads (a.k.a. channels)
	//////////////////////////////////////////////////////////////////
	ThreadQueue __Threads[NUM_CHANNELS];
	int __lastThreadUsed;
#ifdef __bgp__
	unsigned _thrIPI[NUM_CHANNELS];
#endif
	int __maxHWThreads;
	int __maxSWThreads;

#ifdef USE_WAKEUP_VECTORS
	void *__wakeupVectors;
	void *__myWakeupVectors;
	GenericAdvanceThread *__thrRegistry[MAX_REG_THREADS];
#endif /* USE_WAKEUP_VECTORS */
	int __numRegThreads;

	//////////////////////////////////////////////////////////////////
	/// \brief Lockmanager object for local barrier calls
	//////////////////////////////////////////////////////////////////
	XMI_SYSDEP_CLASS &__sysdep;

}; /* class Device */

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_device_h__ */
