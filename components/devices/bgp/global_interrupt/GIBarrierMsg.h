/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_bgp_global_interrupt_GIBarrierMsg_h__
#define __components_devices_bgp_global_interrupt_GIBarrierMsg_h__

#include "SysDep.h"
#include "util/common.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/FactoryInterface.h"
#include "components/devices/util/SubDeviceSuppt.h"
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
class giDevice : public XMI::Device::Generic::MultiSendQSubDevice<giThread,1,true> {
public:
	class Factory : public Interface::FactoryInterface<Factory,giDevice,Generic::Device> {
	public:
		static inline giDevice *generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager &mm);
		static inline xmi_result_t init_impl(giDevice *devs, size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices);
		static inline size_t advance_impl(giDevice *devs, size_t client, size_t contextId);
		static inline giDevice & getDevice_impl(giDevice *devs, size_t client, size_t contextId);
	}; // class Factory
}; // class giDevice

}; // namespace BGP
}; // namespace Device
}; // namespace XMI

extern XMI::Device::BGP::giDevice _g_gibarrier_dev;

namespace XMI {
namespace Device {
namespace BGP {

inline giDevice *giDevice::Factory::generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager &mm) {
	return &_g_gibarrier_dev;
}

inline xmi_result_t giDevice::Factory::init_impl(giDevice *devs, size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices) {
	return _g_gibarrier_dev.__init(client, contextId, clt, ctx, sd, devices);
}

inline size_t giDevice::Factory::advance_impl(giDevice *devs, size_t client, size_t contextId) {
	return 0;
}

inline giDevice & giDevice::Factory::getDevice_impl(giDevice *devs, size_t client, size_t contextId) {
	return _g_gibarrier_dev;
}

//////////////////////////////////////////////////////////////////////
///  \brief Global interrupt message class
///  This message is posted to a GI device
//////////////////////////////////////////////////////////////////////
class giMessage : public XMI::Device::Generic::GenericMessage {
public:

protected:
	friend class giModel;

	//////////////////////////////////////////////////////////////////
	/// \brief  GI Message constructor
	/// \param cb: A "done" callback structure to be executed
	//////////////////////////////////////////////////////////////////
	giMessage(GenericDeviceMessageQueue *GI_QS, xmi_multisync_t *msync) :
	XMI::Device::Generic::GenericMessage(GI_QS, msync->cb_done,
				msync->client, msync->context)
	{
	}

protected:
	static const int GI_CHANNEL = 0;

	DECL_ADVANCE_ROUTINE(advanceThread,giMessage,giThread);
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
				thr->setStatus(XMI::Device::Complete);
				break;
			default:
				XMI_abortf("Unexpected message status of %d (loop %d)\n", stat, loop);
			}
		}
		setStatus(stat);
		return stat == XMI::Device::Done ? XMI_SUCCESS : XMI_EAGAIN;
	}

public:
	// virtual function
	xmi_context_t postNext(bool devPosted) {
		return _g_gibarrier_dev.__postNext<giMessage>(this, devPosted);
	}

	inline int setThreads(giThread **th) {
		// This is only called if we are the top of the queue.
		// We get our threads object(s) from our device.
		giThread *t;
		int n;
		_g_gibarrier_dev.__getThreads(&t, &n);
		t->setMsg(this);
		t->setAdv(advanceThread);
		t->setStatus(XMI::Device::Ready);
		__advanceThread(t);
		*th = t;
		return 1;
	}

protected:
}; // class giMessage

class giModel : public XMI::Device::Interface::MultisyncModel<giModel,giDevice,sizeof(giMessage)> {
public:
	static const size_t sizeof_msg = sizeof(giMessage);

	giModel(giDevice &device, xmi_result_t &status) :
	XMI::Device::Interface::MultisyncModel<giModel,giDevice,sizeof(giMessage)>(device,status)
	{
		// assert(device == _g_gibarrier_dev);
		// if we need sysdep, use _g_gibarrier_dev.getSysdep()...
	}

	inline xmi_result_t postMultisync_impl(uint8_t (&state)[sizeof_msg], xmi_multisync_t *msync);

private:
}; // class giModel

}; // namespace BGP
}; // namespace Device
}; // namespace XMI

inline xmi_result_t XMI::Device::BGP::giModel::postMultisync_impl(uint8_t (&state)[sizeof_msg], xmi_multisync_t *msync) {
	// assert(participants == ctor topology)
	giMessage *msg;

	msg = new (&state) giMessage(_g_gibarrier_dev.getQS(), msync);
	_g_gibarrier_dev.__post<giMessage>(msg);
	return XMI_SUCCESS;
}

#endif // __components_devices_bgp_gibarriermsg_h__
