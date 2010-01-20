/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_generic_SubDevice_h__
#define __components_devices_generic_SubDevice_h__

#include "GenericDevicePlatform.h"
#include "components/devices/generic/BaseGenericDevice.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/Device.h"
#include "sys/xmi.h"

/// \defgroup gendev_subdev_api Internal API for Generic::Device sub-devices
///
/// The internal API is the set of functions called by Generic::Device
/// on the various sub-devices.
/// Generic::Device -> SubDevice
///

/// \defgroup gendev_subdev_internal_api Internal API for sub-device templates
///
/// The internal API is the set of functions called by sub-devices
/// on the various sub-device-templates.
/// SubDevice -> SubDeviceTemplate
///

/// \defgroup gendev_subdev_private_api Private API for sub-device templates
///
/// The private API is the set of functions called by/from sub-device templates.
/// SubDeviceTemplate -> SubDeviceTemplate
///

////////////////////////////////////////////////////////////////////////
///  \file components/devices/generic/SubDevice.h
///  \brief Generic Device
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

/// \brief Queue object used for messages on sub-devices
///
/// Instantiates queue[0] of a two-queue system. Queue[0] by convention
/// is the queue used for attaching messages to a sub-device queue.
/// Queue[1] is used by the Generic::Device to enqueue messages for completion.
///
class GenericSubDevSendq : public GenericDeviceMessageQueue {
public:

	/// \brief Add a message to the (end of the) queue
	///
	/// Does not perform any actions on the message, the caller has
	/// already attempted early advance.
	///
	/// \param[in] msg	New message to be posted
	/// \ingroup gendev_subdev_api
	///
	inline void post(GenericMessage *msg) {
		pushTail(msg);
	}

	/// \brief peek at "next" message on queue
	///
	/// \return	Top message on queue
	/// \ingroup gendev_subdev_api
	///
	inline GenericMessage *getCurrent() {
		return (GenericMessage *)peekHead();
	}

	/// \brief pop "next" message off queue and return it
	///
	/// \return	Former top message on queue
	/// \ingroup gendev_subdev_api
	///
	inline GenericMessage *dequeue() {
		return (GenericMessage *)popHead();
	}

	/// \brief number of messages on the queue
	///
	/// \return	number of messages on the queue
	/// \ingroup gendev_subdev_api
	///
	inline int queueSize() {
		return size();
	}
protected:
}; // class GenericSubDevSendq

/// \brief Base class from which all sub-devices should inherit
///
/// This class works for single-instantiation for multiple-clients,
/// but does not require the sub-device to instantiate that way.
///
/// The data member _generics[] and its accessors provide the capability.
/// In case of only one client, the additional elements of _generics[] are
/// not used.  A better way might be to push _generics[] into __global,
/// but it will still be accessed as an array. We might need to explore
/// ways that one-per-client and one-per-context sub-devices can avoid
/// the array handling, if needed.
///
/// NOTE: one-per-context sub-devices are not capable of using
/// multiple contexts in a single communication (e.g. for parallelism).
///
class GenericSubDevice : public BaseGenericDevice {
private:
public:
	GenericSubDevice() :
	BaseGenericDevice()
	{
	}

	virtual ~GenericSubDevice() { }

	/// \brief accessor for sysdep object associated with sub-device
	/// \ingroup gendev_subdev_api
	inline XMI::SysDep *getSysdep() { return _sd; }

	/// \brief advance routine for unexpected (received) messages
	inline int advanceRecv(size_t client, size_t context);

	// wrappers for GenericSubDevSendq...

	/// \brief Add a message to the (end of the) queue
	///
	/// Does not perform any actions on the message, the caller has
	/// already attempted early advance.
	///
	/// \param[in] msg	New message to be posted
	/// \ingroup gendev_subdev_api
	///
	inline void post(GenericMessage *msg) {
		_queue.pushTail(msg);
	}

	/// \brief peek at "next" message on queue
	///
	/// \return	Top message on queue
	/// \ingroup gendev_subdev_api
	///
	inline GenericMessage *getCurrent() {
		return (GenericMessage *)_queue.peekHead();
	}

	/// \brief pop "next" message off queue and return it
	///
	/// \return	Former top message on queue
	/// \ingroup gendev_subdev_api
	///
	inline GenericMessage *dequeue() {
		return (GenericMessage *)_queue.popHead();
	}

	/// \brief Remove a message from the middle of the queue
	///
	/// \param[in] msg	New message to be removed
	/// \ingroup gendev_subdev_api
	///
	inline void deleteElem(GenericMessage *msg) {
		_queue.deleteElem(msg);
	}

	/// \brief number of messages on the queue
	///
	/// \return	number of messages on the queue
	/// \ingroup gendev_subdev_api
	///
	inline int queueSize() {
		return _queue.size();
	}

	/// \brief Perform sub-device completion of a message
	///
	/// Basically, just dequeue the message from the sub-device
	/// and see if there was anything waiting to run afterwards.
	/// This routine is inlined, but the postNext() call is virtual.
	///
	/// \param[in] msg	The message that is completed
	/// \ingroup gendev_subdev_api
	///
	inline void __complete(XMI::Device::Generic::GenericMessage *msg) {
		/* assert msg == dequeue(); */
		dequeue();
		XMI::Device::Generic::GenericMessage *nxt = getCurrent();
		if (nxt) {
			// skips posting to sub-device... that was already done.
			// must setup threads and post threads+message to generic device.
			(void)nxt->postNext(true);
			// Note: message might have completed here, but handling
			// that is too complicated so we just let the generic
			// device do the completion when it finds it on the queue.
			// (recursion possibility is one complication)
		}
	}

	/// \brief Post message and threads to generic device for processing
	///
	/// Only posts threads which are not yet "Done". Simple pass-through
	/// to generic device post(), which does not care which object the
	/// method is actually called on - uses 'msg' to determine where to
	/// actually post/enqueue the objects.
	///
	/// \param[in] msg	Message object to enqueue
	/// \param[in] t	array of threads to enqueue
	/// \param[in] l	size of each thread in array
	/// \param[in] n	number of threads to enqueue
	/// \ingroup gendev_subdev_api
	///
	inline void postToGeneric(GenericMessage *msg, GenericAdvanceThread *t, size_t l, int n) {
		size_t c = msg->getClientId();
		_generics[c]->post(msg, t, l, n);
	}

	/// \brief accessor for specific generic device for context-id
	/// \param[in] contextId	Context ID to get generic device slice
	/// \return	Pointer to specific generic device object
	/// \ingroup gendev_subdev_api
	///
	inline XMI::Device::Generic::Device *getGeneric(size_t client, size_t contextId) {
		return &_generics[client][contextId];
	}

protected:
	/// \brief internal initialization routine for GenericSubDevice sub-class
	///
	/// \param[in] sd	SysDep for device/context/client... not used?
	/// \param[in] generics	Array of generic devices used for parallelism
	/// \param[in] client	Client ID for which init is being done
	/// \param[in] context	Context ID for which init is being done
	/// \ingroup gendev_private_api
	///
	inline void ___init(XMI::SysDep &sd, XMI::Device::Generic::Device *generics,
						size_t client, size_t context) {
		if (client == 0) {
			_sd = &sd;
		}
		if (context == 0) {
			_generics[client] = generics;
		}
	}

	GenericSubDevSendq _queue;
	XMI::SysDep *_sd;
	XMI::Device::Generic::Device *_generics[XMI_MAX_NUM_CLIENTS];
}; /* class GenericSubDevice */

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif // __components_devices_generic_SubDevice_h__
