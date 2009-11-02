/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_devices_generic_BaseGenericDevice_h__
#define __components_devices_generic_BaseGenericDevice_h__

#include <util/common.h>

////////////////////////////////////////////////////////////////////////
/// \file components/devices/generic/BaseGenericDevice.h
/// \brief An opaque placeholder for Devices, akin to (void *) for pointers.
///
///  Namespace: XMI::Device::Generic::BaseGenericDevice
///
////////////////////////////////////////////////////////////////////////
namespace XMI {
namespace Device {
namespace Generic {
	//////////////////////////////////////////////////////////////////////
	///  \brief Base Class for Devices
	//////////////////////////////////////////////////////////////////////
	class BaseGenericDevice {
	public:
		//////////////////////////////////////////////////////////////////////
		///  \brief Constructor
		//////////////////////////////////////////////////////////////////////
		BaseGenericDevice() :
                    _device_id((unsigned)-1)
                    {}

		//////////////////////////////////////////////////////////////////
		/// \brief Destructor
		/// \todo fixme ... old destructor doesn't work with virtual classes.
		//////////////////////////////////////////////////////////////////
		virtual ~BaseGenericDevice() { }

		/// NOTE: This is required to make "C" programs link successfully with virtual destructors
		inline void operator delete(void *p) { }

		//////////////////////////////////////////////////////////////////////
		///  \brief Used by the protocol to get _device_id.
		//////////////////////////////////////////////////////////////////////
		inline  unsigned getDeviceID() const { return _device_id; };
		virtual unsigned advance_channel(unsigned channel) { XMI_abort(); return 0; };
	protected:
		//////////////////////////////////////////////////////////////////////
		///  \brief The multichannel work required each device to have an ID
		///
		/// This is used as a table index to get information
		/// on the number and types of channels hosted by that
		/// device
		//////////////////////////////////////////////////////////////////////
		unsigned _device_id;

	}; // class BaseGenericDevice
}; // namespace Generic
}; // namespace Device
}; // namespace XMI

#endif // __components_devices_generic_basedevice_h__
