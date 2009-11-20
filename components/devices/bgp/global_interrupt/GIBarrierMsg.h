/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_bgp_global_interrupt_GIBarrierMsg_h__
#define __components_devices_bgp_global_interrupt_GIBarrierMsg_h__

#include "SysDep.h"
#include "util/common.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/generic/Device.h"
#include "components/devices/generic/SubDevice.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "spi/bgp_SPI.h"

////////////////////////////////////////////////////////////////////////
///  \file components/devices/bgp/global_interrupt/GIBarrierMsg.h
///  \brief Global Interrupt Device
///
///  The GI classes implement a BaseDevice and a Message object
///  to post into the queueing system.  The GI device is currently
///  used to implement barriers, so the GI device posts a message
///  and uses a global interrupt sysdep to signal the GI wire
///  This is used to implement
///  -
///  - Barriers
///
///  Definitions:
///  - giMessage:  A global interrupt message
///  - Device:     The global interrupt queue system
///
///  Namespace:  XMI, the messaging namespace.
///
////////////////////////////////////////////////////////////////////////

namespace XMI {
namespace Device {
namespace BGP {

class giModel;
class giMessage;
typedef XMI::Device::Generic::GenericAdvanceThread giThread;
typedef XMI::Device::Generic::SimpleSubDevice<giThread> giDevice;

}; // namespace BGP
}; // namespace Device
}; // namespace XMI

extern XMI::Device::BGP::giDevice _g_gibarrier_dev;

namespace XMI {
namespace Device {
namespace BGP {

//////////////////////////////////////////////////////////////////////
///  \brief Global interrupt message class
///  This message is posted to a GI device
//////////////////////////////////////////////////////////////////////
class giMessage : public XMI::Device::Generic::GenericMessage {
public:

	//////////////////////////////////////////////////////////////////
	/// \brief     Advance routine
	/// \returns:  The MsgStatus, Initialized, Active, or Done
	//////////////////////////////////////////////////////////////////
	inline void complete();
	inline size_t objsize_impl() { return sizeof(giMessage); }

protected:
	friend class giModel;

	//////////////////////////////////////////////////////////////////
	/// \brief  GI Message constructor
	/// \param cb: A "done" callback structure to be executed
	//////////////////////////////////////////////////////////////////
	giMessage(Generic::BaseGenericDevice &GI_QS, xmi_multisync_t *msync) :
	XMI::Device::Generic::GenericMessage(GI_QS, msync->cb_done,
				msync->client, msync->context)
	{
	}

private:
	//friend class giDevice;
	friend class XMI::Device::Generic::SimpleSubDevice<giThread>;

	static const int GI_CHANNEL = 0;

	friend class XMI::Device::Generic::GenericMessage;
	inline xmi_result_t __advanceThread(giThread *thr) {
		XMI::Device::MessageStatus stat = getStatus();

		unsigned loop = 32;
		while (stat != XMI::Device::Done && loop--) {
			switch(stat) {
			case XMI::Device::Initialized:
				GlobInt_InitBarrier(GI_CHANNEL);
				stat = XMI::Device::Active;
				// FALLTHROUGH
			case XMI::Device::Active:
				if (GlobInt_QueryDone(GI_CHANNEL) == 0) {
					break;
				}
				stat = XMI::Device::Done;
				// FALLTHROUGH
			case XMI::Device::Done:
				thr->setDone(true);
				break;
			default:
				XMI_abort();
			}
		}
		setStatus(stat);
		return stat == XMI::Device::Done ? XMI_SUCCESS : XMI_EAGAIN;
	}

	inline int __setThreads(giThread *t, int n) {
		t->setMsg(this);
		t->setAdv(advanceThread<giMessage,giThread>);
		t->setDone(false);
		_nThreads = 1;
		return _nThreads;
	}

protected:
	int _nThreads;  // needs to be atomic, if we use multiple threads

}; // class giMessage

class giModel : public XMI::Device::Interface::MultisyncModel<giModel> {
public:
	static const size_t sizeof_msg = sizeof(giMessage);

	giModel(xmi_result_t &status) :
	XMI::Device::Interface::MultisyncModel<giModel>(status)
	{
		// if we need sysdep, use _g_gibarrier_dev.getSysdep()...
	}

	inline bool postMultisync_impl(xmi_multisync_t *msync);

private:
}; // class giModel

}; // namespace BGP
}; // namespace Device
}; // namespace XMI

inline void XMI::Device::BGP::giMessage::complete() {
	((XMI::Device::BGP::giDevice &)_QS).__complete<giMessage>(this);
	executeCallback();
}

inline bool XMI::Device::BGP::giModel::postMultisync_impl(xmi_multisync_t *msync) {
	// assert(participants == ctor topology)
	giMessage *msg;

	msg = new (msync->request) giMessage(_g_gibarrier_dev, msync);
	_g_gibarrier_dev.__post<giMessage>(msg);
	return true;
}

#endif // __components_devices_bgp_gibarriermsg_h__
