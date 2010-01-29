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
/** \todo XMI_MAX_NUM_CLIENTS needs to be setup by xmi.h */
#define XMI_MAX_NUM_CLIENTS	4
#endif // !XMI_MAX_NUM_CLIENTS

extern size_t XMI_GD_ClientId(xmi_client_t client);
// #define XMI_GD_ClientId(client)	(client)

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
	/// \brief  A generic device (wrapper for sub-devices)
	///
	inline Device();

	static Device *create(size_t clientid, size_t num_ctx);

	/// \brief Initialize a generic device slice
	/// \param[in] sd	SysDep object
	/// \param[in] ctx	Context object associated with slice
	/// \param[in] context	Context ID associated with slice
	/// \param[in] num_contexts	Number of contexts/slices
	/// \param[in] generics		Array of generic device slices
	/// \ingroup gendev_public_api
	///
	inline void init(xmi_context_t ctx, size_t client, size_t context, size_t num_contexts);

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
	inline int advance(size_t clientid, size_t contextid);
	inline int advance();

	/// \brief     Post a thread object on a generic device slice's queue
	///
	/// Not normally used. ProgressFuncionDev uses this to post a thread
	/// without an associated message.
	///
	/// \param[in] thr	Thread object to post for advance work
	/// \ingroup gendev_internal_api
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
