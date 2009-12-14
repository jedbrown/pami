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

/// \defgroup gendev_internal_api Internal API for Generic::Device
///
/// The internal API is the set of methods that are/may be called by
/// sub-devices.
/// SubDevice -> Generic::Device
/// SubDeviceTemplate -> Generic::Device
///

/// \defgroup gendev_private_api Private API for Generic::Device
///
/// The private API is the set of methods that are/may be called by
/// by/from various parts of the Generic::Device.
/// Generic::Device -> Generic::Device
///

#include "SysDep.h"
#include "WakeupManager.h"
#include "components/devices/BaseDevice.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/generic/Message.h"
#include "components/atomic/Mutex.h"
#include "sys/xmi.h"

#ifndef XMI_MAX_NUM_CLIENTS
#warning XMI_MAX_NUM_CLIENTS needs to be setup by xmi.h
#define XMI_MAX_NUM_CLIENTS	4
#endif // !XMI_MAX_NUM_CLIENTS

extern size_t XMI_GD_ClientId(xmi_client_t client);
// #define XMI_GD_ClientId(client)	(client)

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

/// \brief Object for implementing a generic device queue for thread objects (work units)
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

	/// \brief Initialize platform-specific sub-devices
	/// \param[in] first_global	Flag indicating if this is the first call, globally
	/// \param[in] first_client	Flag indicating if this is the first call on client
	/// \param[in] sd		SysDep object
	/// \ingroup gendev_private_api
	inline void __platform_generic_init(bool first_global, bool first_client,
								XMI::SysDep &sd);

	/// \brief advance unexpected(received) messages for platform devices
	/// \return	Number of "events"
	/// \ingroup gendev_private_api
	inline int __platform_generic_advanceRecv();

public:
	/// \brief  A generic device (wrapper for sub-devices)
	///
	inline Device();

	/// \brief Initialize a generic device slice
	/// \param[in] sd	SysDep object
	/// \param[in] ctx	Context object associated with slice
	/// \param[in] context	Context ID associated with slice
	/// \param[in] num_contexts	Number of contexts/slices
	/// \param[in] generics		Array of generic device slices
	/// \ingroup gendev_public_api
	///
	inline void init(XMI::SysDep &sd, xmi_context_t ctx, size_t client, size_t context, size_t num_contexts, Device *generics);

	/// \brief     Advance routine for the generic device.
	///
	/// Currently not used, since subdevices have to be polled for recvs.
	///
	/// \return	Boolean indicating if device needs advancing
	/// \ingroup gendev_public_api
	///
	inline bool isAdvanceNeeded();

	/// \brief     Advance routine for the generic device.
	/// \return	number of events processed
	/// \ingroup gendev_public_api
	///
	inline int advance();

	/// \brief     Post a thread object on a generic device slice's queue
	///
	/// Not normally used. ProgressFuncionDev uses this to post a thread
	/// without an associated message.
	///
	/// \param[in] thr	Thread object to post
	/// \ingroup gendev_internal_api
	///
	inline void postThread(GenericThread *thr) {
		__Threads.pushTail(thr);
	}
	/// \brief Post a message to the generic-device queuing system
	///
	/// Only threads that are !Complete are enqueued. The message is
	/// not checked for Done, assuming that was previously checked
	/// if needed. In some cases, a message is intentionally posted
	/// even though it is done. This is to avoid recursion within
	/// a callback that is posting the next message from the sub-device.
	///
	/// \param[in] msg	Message to be queued/advanced.
	/// \param[in] thr	Array of thread objects to post
	/// \param[in] len	sizeof each thread
	/// \param[in] num	number of threads to post
	///
	/// \ingroup gendev_internal_api
	///
	inline void post(GenericMessage *msg, GenericAdvanceThread *thr,
							size_t len, int num) {
		// early advance was done by the "real" device post()

		// get access to client-global contexts array,
		// in order to stage work on separate contexts.
		// the context specified in the msg is used only for
		// completion, even the first thread of work is posted
		// to a different context.

		size_t c = msg->getClientId();
		size_t t = msg->getContextId();
		size_t n = __nContexts;
		Generic::Device *g0 = __generics[c];
		g0[t].__GenericQueue.pushTail(msg);

		// round-robin threads to available "channels"...
		// does this need to be made thread-safe?
		// we expect to make enqueue routines atomic, lockless.
		// note: might be called from a completion callback.

		// t = msg->getClient()->__lastThreadUsed;
		for (int x = 0; x < num; ++x) {
			if (thr->getStatus() != Complete) {
				if (++t >= n) {
					t = 0;
				}
				g0[t].__Threads.pushTail(thr);
			}
			thr = (GenericAdvanceThread *)((char *)thr + len);
		}
		// msg->getClient()->__lastThreadUsed = t;
	}

	/// \brief accessor for the context-id associated with generic device slice
	/// \return	context ID
	/// \ingroup gendev_internal_api
	inline size_t contextId() { return __contextId; }

	/// \brief accessor for the total number of contexts in this client
	/// \return	number of contexts/generic device slices
	/// \ingroup gendev_internal_api
	inline size_t nContexts() { return __nContexts; }

	/// \brief accessor for the context associated with generic device slice
	/// \return	context handle
	/// \ingroup gendev_internal_api
	inline xmi_context_t getContext() { return __context; }

private:
	/// \brief Advance a reception channel
	///
	/// Check a channel for received messages (or within the context of a channel)
	///
	/// \return	Number of work units performed (typically, 0 or 1)
	/// \ingroup gendev_private_api
	///
	inline int __advanceRecv();

	/// \brief Storage for the queue for message completion
	///
	/// Queue[1] is used by the Generic::Device to enqueue messages for completion.
	/// By convention, queue[0] is used for attaching messages to a sub-device.
	///
	MultiQueue<2, 1> __GenericQueue;

	/// \brief Storage for the queue of threads (a.k.a. work units)
	ThreadQueue __Threads;

	xmi_context_t __context;
	size_t __clientId;
	size_t __contextId;
	size_t __nContexts;
	Generic::Device *__generics[XMI_MAX_NUM_CLIENTS];

}; /* class Device */

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_device_h__ */
