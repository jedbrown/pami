/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_generic_Device_h__
#define __components_devices_generic_Device_h__

/// \page use_gendev How to use the Generic Device
///
/// This chapter explains the basic requirements and features of the generic device.
///
/// \section use_gendev_basic Basic requirements and features of the Generic Device
///
/// The generic device implements a pair of queues for each context.
///
/// One queue holds thread objects, which each represent a until of work.
/// Each thread on the queue will have it's work function called when
/// the generic device slice (context) is advanced. Depending on the
/// thread status and work function return code, the thread may persist
/// on the queue or be removed. Thread objects are posted using postThread().
///
/// The other queue holds message objects. These objects are queued only
/// to be checked for completion. during advance, each object on this queue
/// has it's status checked, and if Done will be dequeued and the completion
/// callback invoked. Message are posted using postMsg().
///
/// A user of the generic device may employ both message and threads,
/// or either one alone.
///
/// A typical usage is to create a message object with common data
/// and the completion callback, and then create one or more thread objects
/// with data members for the specific work assigned to the thread.
/// As each thread completes it's work it updates the message and when
/// last thread completes it will set the message status to Done.
/// The thread objects are posted to different contexts thereby achieving
/// parallelism.
///
/// All devices are given a pointer to the array of generic devices during
/// init(). By saving the pointer, any device may later post work to
/// any slice of the generic device.
///
/// \section use_gendev_syn SYNOPSIS
///
/// \#include "components/devices/generic/Device.h"
///
/// \section use_gendev_thr THREAD
///
/// All objects posted to the generic device via postThread() must
/// inherit from GenericThread.
///
/// \ref XMI::Device::ThreadStatus "GenericThread status values"
///
/// \ref XMI::Device::Generic::GenericThread "class GenericThread"
///
/// \subsubsection use_gendev_genthr_p Provides:
///
/// \ref GenericThread::getStatus "ThreadStatus getStatus()"
///
/// \ref GenericThread::setStatus "void setStatus(ThreadStatus stat)"
///
/// \ref GenericThread::setFunc "void setFunc(xmi_work_function func, void *cookie)"
///
///
///
///
///

#include "SysDep.h"
#include "WakeupManager.h"
#include "components/devices/BaseDevice.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/generic/Message.h"
#include "components/devices/FactoryInterface.h"
#include "components/atomic/Mutex.h"
#include "sys/xmi.h"

#ifndef XMI_MAX_NUM_CLIENTS
/** \todo XMI_MAX_NUM_CLIENTS needs to be setup by xmi.h */
#define XMI_MAX_NUM_CLIENTS	4
#endif // !XMI_MAX_NUM_CLIENTS

#include "GenericDevicePlatform.h"

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

//////////////////////////////////////////////////////////////////////
///  \brief A Device implmentation of a Queuesystem
///  This class implements a queue system. The user posts to the queue
///  and the interprocess communications are called in order until all of them
///  have been executed.
//////////////////////////////////////////////////////////////////////
class Device {

public:
	/// \brief standard Device::Factory API
	class Factory : public Interface::FactoryInterface<Factory,Device,Device> {
	public:
		static inline Device *generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager & mm) {
			size_t x;
			Device *gds;
			int rc = posix_memalign((void **)&gds, 16, sizeof(*gds) * num_ctx);
			XMI_assertf(rc == 0, "posix_memalign failed for generics[%zd], errno=%d\n", num_ctx, errno);
			for (x = 0; x < num_ctx; ++x) {
				new (&gds[x]) XMI::Device::Generic::Device(client, x, num_ctx);
			}
			return gds;
		}
		static inline xmi_result_t init_impl(Device *devs, size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices) {
			return getDevice_impl(devs, client, contextId).init(ctx, client, contextId);
		}
		static inline size_t advance_impl(Device *devs, size_t client, size_t context) {
			return getDevice_impl(devs, client, context).advance();
		}
		static inline Device & getDevice_impl(Device *devs, size_t client, size_t context) {
			return devs[context];
		}
	}; // class Factory

	/// \brief  A generic device (wrapper for sub-devices)
	///
	/// The first generic device of a client
	///
	inline Device(size_t client, size_t contextId, size_t num_ctx) :
	__GenericQueue(),
	__Threads(),
	__clientId(client),
	__contextId(contextId),
	__nContexts(num_ctx)
	{
	}

	inline xmi_result_t init(xmi_context_t ctx, size_t client, size_t context) {
		__context = ctx;
		return XMI_SUCCESS;
	}

	/// \brief Advance routine for (one channel of) the generic device.
	///
	/// This advances all units of work on this context's queue, and
	/// checks the message queue for completions. It also calls the
	/// advanceRecv routine for all devices.
	///
	/// \return	number of events processed
	///
	inline size_t advance() {
		int events = 0;
		//+ Need to ensure only one of these runs per core
		//+ (even if multi-threads per core)
		//+ if (core_mutex.tryAcquire()) {

		// could check the queues here and return if empty, but it
		// probably takes just as much as the for loops would, and
		// just further delay the advance of real work when present.

		//if (!__Threads.mutex()->tryAcquire()) continue;
		GenericThread *thr, *nxtthr;
		for (thr = (GenericThread *)__Threads.peekHead(); thr; thr = nxtthr) {
			nxtthr = (GenericThread *)__Threads.nextElem(thr);
			if (thr->getStatus() == XMI::Device::Ready) {
				++events;
				xmi_result_t rc = thr->executeThread(__context);
				if (rc != XMI_EAGAIN) {
					// thr->setStatus(XMI::Device::Complete);
					__Threads.deleteElem(thr);
					continue;
				}
			} else if (thr->getStatus() == XMI::Device::OneShot) {
				++events;
				// thread is like completion callback, dequeue first.
				__Threads.deleteElem(thr);
				thr->executeThread(__context);
				continue;
			}
			// This allows a thread to be "completed" by something else...
			if (thr->getStatus() == XMI::Device::Complete) {
				__Threads.deleteElem(thr);
				continue;
			}
		}
		//__Threads.mutex()->release();

		//+ core_mutex.release();

		// Now check everything on the completion queue...
		GenericMessage *msg, *nxtmsg, *nxt;
		for (msg = (GenericMessage *)__GenericQueue.peekHead(); msg; msg = nxtmsg) {
			nxtmsg = (GenericMessage *)__GenericQueue.nextElem(msg);
			if (msg->getStatus() == Done) {
				++events;
				__GenericQueue.deleteElem(msg);
				GenericDeviceMessageQueue *qs = msg->getQS();
				qs->dequeue(); // assert return == msg
				nxt = (XMI::Device::Generic::GenericMessage *)qs->peek();
				if (nxt) nxt->postNext(true); // virtual function
				msg->executeCallback(__context);
			}
		}
		return events;
	}

	/// \brief     Advance routine for the generic device.
	///
	/// Currently not used, since subdevices have to be polled for recvs.
	///
	/// \return	Boolean indicating if device needs advancing
	///
	inline bool isAdvanceNeeded();

	/// \brief     Post a thread object on a generic device slice's queue
	///
	/// Not normally used. ProgressFuncionDev uses this to post a thread
	/// without an associated message.
	///
	/// \param[in] thr	Thread object to post for advance work
	///
	inline void postThread(GenericThread *thr) {
		__Threads.pushTail(thr);
	}

	/// \brief Post a message to the generic-device queuing system
	///
	/// \param[in] msg	Message to be queued/completed.
	///
	inline void postMsg(GenericMessage *msg) {
		__GenericQueue.pushTail(msg);
	}

	/// \brief accessor for the context-id associated with generic device slice
	/// \return	context ID
	inline size_t contextId() { return __contextId; }

	/// \brief accessor for the total number of contexts in this client
	/// \return	number of contexts/generic device slices
	inline size_t nContexts() { return __nContexts; }

	/// \brief accessor for the context associated with generic device slice
	/// \return	context handle
	inline xmi_context_t getContext() { return __context; }

private:
	/// \brief Storage for the queue for message completion
	///
	/// Queue[1] is used by the Generic::Device to enqueue messages for completion.
	/// By convention, queue[0] is used for attaching messages to a sub-device.
	///
	GenericDeviceCompletionQueue __GenericQueue;

	/// \brief Storage for the queue of threads (a.k.a. work units)
	GenericDeviceWorkQueue __Threads;

	xmi_context_t __context;	///< context handle for this generic device
	size_t __clientId;		///< client ID for context
	size_t __contextId;		///< context ID
	size_t __nContexts;		///< number of contexts in client
}; /* class Device */

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_device_h__ */
