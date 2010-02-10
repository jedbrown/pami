/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_generic_GenericDevice_h__
#define __components_devices_generic_GenericDevice_h__

// This file implements the interfaces used by messaging (advance).
// See generic/Device.h for interfaces used by sub-devices.

/// \defgroup gendev_public_api Public API for Generic::Device
///
/// The public API is the set of functions called by XMI::Context
/// for initialization and progress.
/// XMI::Context -> Generic::Device
///

#include "components/devices/generic/Device.h"

#include "components/devices/util/SubDeviceSuppt.h"

#if defined(__bgp__) and !defined(__bgq__)

#ifndef __defined__tsc__
static inline unsigned long __tsc() {
	long tsc;
	asm volatile ("mfspr %0,%1" : "=r" (tsc) : "i" (SPRN_TBRL));
	return tsc;
}
#define __defined__tsc__
#endif /* ! __defined__tsc__ */

#endif // __bgp__ and !__bgq__

////////////////////////////////////////////////////////////////////////
///  \file components/devices/generic/GenericDevice.h
///  \brief Generic Device - actual implementation
///
///  The Generic classes implement a QueueSystem and a Message object
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

	/// \brief The first generic device of a client
	///
	inline Device::Device() :
	__GenericQueue(),
	__Threads(),
	__clientId((size_t)-1),
	__contextId((size_t)-1)
	{
	}

	/// \brief Initialize the Generic Device and all subdevices.
	///
	/// When a new subdevice is added, a call to its init routine is
	/// added here.
	///
	/// \param[in] sd		The SysDep object
	/// \param[in] context		The specific context being initialized
	/// \param[in] num_contexts	Total number of contexts in current client
	///
	/// \ingroup gendev_public_api
	///
	inline void Device::init(xmi_context_t ctx, size_t client, size_t context, size_t num_contexts) {
		Device *dev = &this[context];
		dev->__context = ctx;
		dev->__clientId = client;
		dev->__contextId = context;
		dev->__nContexts = num_contexts;
	}

	/// \brief Quick check whether full advance is needed.
	///
	/// Currently not used, since subdevices have to be polled for recvs.
	///
	/// \return	Boolean indicating if device needs advancing
	/// \ingroup gendev_public_api
	///
	inline bool Device::isAdvanceNeeded() {
#if 1
		return true;
#else
		// There can be no threads to advance unless there is at least one
		// message, on the __GenericQueue[].
		return !__GenericQueue.isEmpty();
#endif
	}

	/// \brief Advance routine for (one channel of) the generic device.
	///
	/// This advances all units of work on this context's queue, and
	/// checks the message queue for completions. It also calls the
	/// advanceRecv routine for all devices.
	///
	/// \return	number of events processed
	///
	/// \ingroup gendev_public_api
	///
	inline int Device::advance(size_t clientid, size_t contextid) {
		// This device has one instance per context per client.
		// assert(clientid == __clientId);
		Device *gd = &this[contextid];
		return gd->advance();
	}

	inline int Device::advance() {
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
					thr->executeCallback(__context, rc);
					continue;
				}
			}
			// This allows a thread to be "completed" by something else...
			if (thr->getStatus() == XMI::Device::Complete) {
				__Threads.deleteElem(thr);
				thr->executeCallback(__context);
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
				nxt = static_cast<GenericSubDevice *>(msg->getQS())->__complete(msg);
				if (nxt) nxt->postNext(true); // virtual function
				msg->executeCallback(__context);
			}
		}
		return events;
	}

	// This is called before contexts are created... Device init must wait.
	inline Device *Device::create(size_t clientid, size_t num_ctx) {
		size_t x;
		Device *gds;
		int rc = posix_memalign((void **)&gds, 16, sizeof(*gds) * num_ctx);
		XMI_assertf(rc == 0, "posix_memalign failed for generics[%zd], errno=%d\n", num_ctx, errno);
		for (x = 0; x < num_ctx; ++x) {
			new (&gds[x]) XMI::Device::Generic::Device();
		}
		return gds;
	}

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_genericdevice_h__ */
