/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/generic/AtomicBarrierMsg.h
 * \brief Barrier that uses whatever Atomic Barrier object  is specified
 */

/// \page atomic_barrier_usage
///
/// Typical usage to instantiate a "generic" barrier based on an atomic object:
///
/// #include "components/atomic/bgp/LockBoxBarrier.h"
/// // Change this line to switch to different barrier implementations...
/// typedef XMI::Barrier::BGP::LockBoxNodeProcBarrier MY_BARRIER;
///
/// #include "components/devices/generic/AtomicBarrierMsg.h"
/// typedef XMI::Device::AtomicBarrierMdl<MY_BARRIER> MY_BARRIER_MODEL;
/// typedef XMI::Device::AtomicBarrierMsg<MY_BARRIER> MY_BARRIER_MESSAGE;
///
/// xmi_result_t status;
/// MY_BARRIER_MODEL _barrier(status);
/// XMI_assert(status == XMI_SUCCESS);
///
/// xmi_multisync_t _msync;
/// MY_BARRIER_MESSAGE _msg;
/// _msync.request = &_msg;
/// _msync.cb_done = ...;
/// _msync.roles = ...;
/// _msync.participants = ...;
/// _barrier.postMultisync(&_msync);
///

#ifndef __components_devices_generic_AtomicBarrierMsg_h__
#define __components_devices_generic_AtomicBarrierMsg_h__

#include "Global.h"
#include "components/devices/generic/Device.h"
#include "components/devices/generic/SubDevice.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/MultisyncModel.h"
#include "components/atomic/Barrier.h"
#include "sys/xmi.h"

namespace XMI {
namespace Device {

template <class T_Barrier> class AtomicBarrierMsg;
template <class T_Barrier> class AtomicBarrierMdl;
typedef XMI::Device::Generic::GenericAdvanceThread AtomicBarrierThr;
typedef XMI::Device::Generic::SimpleSubDevice<AtomicBarrierThr> AtomicBarrierDev;

}; //-- Device
}; //-- XMI

extern XMI::Device::AtomicBarrierDev _g_lmbarrier_dev;

namespace XMI {
namespace Device {
///
/// \brief A local barrier message that takes advantage of the
/// Load Linked and Store Conditional instructions
///
template <class T_Barrier>
class AtomicBarrierMsg : public XMI::Device::Generic::GenericMessage {
public:

	/// \brief method to advance a portion of the message
	///
	/// If a message is split between multiple threads, each thread
	/// is responsible for only part of the message.
	///
	/// \param in v An opaque pointer to indicate message portion
	/// \param in l An opaque length to indicate message portion
	/// \return Message status - typically either Active or Done.
	///
	/// status Active => Done
	inline XMI::Device::MessageStatus advanceThread(XMI::Device::Generic::GenericAdvanceThread *t);

	inline void complete();
	inline size_t objsize_impl() { return sizeof(AtomicBarrierMsg<T_Barrier>); }

protected:
	friend class AtomicBarrierMdl<T_Barrier>;

	AtomicBarrierMsg(Generic::BaseGenericDevice &Generic_QS,
		T_Barrier *barrier,
		xmi_callback_t cb) :
	XMI::Device::Generic::GenericMessage(Generic_QS, cb),
	_barrier(barrier)
	{
		// assert(role == DEFAULT_ROLE);
	}

private:
	//friend class AtomicBarrierDev;
	friend class XMI::Device::Generic::SimpleSubDevice<AtomicBarrierThr>;

	inline XMI::Device::MessageStatus __advanceThread(AtomicBarrierThr *thr) {
		// TBD: optimize away virt func call - add method
		// for a persistent advance?
		for (int x = 0; x < 32; ++x) {
			if (_barrier->poll() == XMI::Atomic::Interface::Done) {
				setStatus(XMI::Device::Done);
				return XMI::Device::Done;
			}
		}
		return XMI::Device::Active;
	}

	inline int __setThreads(AtomicBarrierThr *t, int n) {
		t->setMsg(this);
		t->setDone(false);
		return 1;
	}

protected:
	T_Barrier *_barrier;
}; //-- AtomicBarrierMsg

template <class T_Barrier>
class AtomicBarrierMdl : public XMI::Device::Interface::MultisyncModel<AtomicBarrierMdl<T_Barrier> > {
public:
	static const size_t sizeof_msg = sizeof(AtomicBarrierMsg<T_Barrier>);

	AtomicBarrierMdl(xmi_result_t &status) :
	XMI::Device::Interface::MultisyncModel<AtomicBarrierMdl<T_Barrier> >(status)
	{
		// "default" barrier: all local processes...
		size_t peers = __global.topology_local.size();
		size_t peer0 = __global.topology_local.index2Rank(0);
		size_t me = __global.mapping.task();
		_barrier.init(_g_lmbarrier_dev.getSysdep(), peers, (peer0 == me));
	}

	inline bool postMultisync_impl(xmi_multisync_t *msync);

private:
	T_Barrier _barrier;
}; // class AtomicBarrierMdl

}; //-- Device
}; //-- XMI

template <class T_Barrier>
inline void XMI::Device::AtomicBarrierMsg<T_Barrier>::complete() {
	((AtomicBarrierDev &)_QS).__complete<AtomicBarrierMsg<T_Barrier> >(this);
	executeCallback();
}

template <class T_Barrier>
inline XMI::Device::MessageStatus XMI::Device::AtomicBarrierMsg<T_Barrier>::advanceThread(XMI::Device::Generic::GenericAdvanceThread *t) {
	return __advanceThread((AtomicBarrierThr *)t);
}

template <class T_Barrier>
inline bool XMI::Device::AtomicBarrierMdl<T_Barrier>::postMultisync_impl(xmi_multisync_t *msync) {
	_barrier.pollInit();
	// See if we can complete the barrier immediately...
	for (int x = 0; x < 32; ++x) {
		if (_barrier.poll() == XMI::Atomic::Interface::Done) {
			if (msync->cb_done.function) {
				msync->cb_done.function(NULL, msync->cb_done.clientdata, XMI_SUCCESS);
			}
			return true;
		}
	}
	// must "continue" current barrier, not start new one!
	AtomicBarrierMsg<T_Barrier> *msg;
	msg = new (msync->request) AtomicBarrierMsg<T_Barrier>(_g_lmbarrier_dev, &_barrier, msync->cb_done);
	_g_lmbarrier_dev.__post<AtomicBarrierMsg<T_Barrier> >(msg);
	return true;
}

#endif //  __components_devices_generic_atomicbarrier_h__
