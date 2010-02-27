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
class GenericSubDevice : public GenericDeviceMessageQueue, public BaseGenericDevice {
private:
public:
	GenericSubDevice() :
	GenericDeviceMessageQueue(),
	BaseGenericDevice()
	{
	}

	virtual ~GenericSubDevice() { }

	/// \brief Perform sub-device completion of a message
	///
	/// Basically, just dequeue the message from the sub-device
	/// and see if there was anything waiting to run afterwards.
	/// This routine is inlined, but the postNext() call is virtual.
	///
	/// \param[in] msg	The message that is completed
	/// \ingroup gendev_subdev_api
	///
	inline XMI::Device::Generic::GenericMessage *__complete(XMI::Device::Generic::GenericMessage *msg) {
		/* assert msg == dequeue(); */
		dequeue();
		XMI::Device::Generic::GenericMessage *nxt =
			(XMI::Device::Generic::GenericMessage *)peek();
		return nxt;
	}

	inline void ___create(size_t client, size_t num_ctx) {
	}
	/// \brief internal initialization routine for GenericSubDevice sub-class
	///
	/// \param[in] sd	SysDep for device/context/client... not used?
	/// \param[in] generics	Array of generic devices used for parallelism
	/// \param[in] client	Client ID for which init is being done
	/// \param[in] context	Context ID for which init is being done
	/// \ingroup gendev_private_api
	///
	inline xmi_result_t ___init(XMI::SysDep *sd, size_t client, size_t contextId) {
		return XMI_SUCCESS;
	}

protected:
}; /* class GenericSubDevice */

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif // __components_devices_generic_SubDevice_h__
