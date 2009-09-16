/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/generic/AtomicBarrierMsg.h
 * \brief Barrier that uses whatever Atomic Barrier object  is specified
 */

#ifndef  __components_devices_generic_atomicbarrier_h__
#define  __components_devices_generic_atomicbarrier_h__

#include "components/devices/generic/Device.h"
#include "components/devices/generic/SubDevice.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/MultisyncModel.h"
#include "components/atomic/Barrier.h"
#include "xmi.h"

#ifdef __bgp__
#include "components/atomic/bgp/LockBoxBarrier.h"
#endif

namespace XMI {
namespace Device {

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

	AtomicBarrierMsg(BaseDevice &Generic_QS,
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
class AtomicBarrierMdl {
public:
	AtomicBarrierMdl(XMI::SysDep *sysdep, xmi_result_t &status)
	{
		// "default" barrier: all local processes...
		_barrier.init(_g_topology_local->size());
		// participants = _g_topology_local
	}

	inline bool postMultisync_impl(AtomicBarrierMsg<T_Barrier> *msg);

private:
	T_Barrier _barrier;
}; // class AtomicBarrierMdl

}; //-- Device
}; //-- XMI

template <class T_Barrier>
inline void XMI::Device::AtomicBarrierMsg<T_Barrier>::complete() {
	((AtomicBarrierDev &)_QS).__complete(this);
	executeCallback();
}

template <class T_Barrier>
inline XMI::Device::MessageStatus XMI::Device::AtomicBarrierMsg<T_Barrier>::advanceThread(XMI::Device::Generic::GenericAdvanceThread *t) {
	return __advanceThread((AtomicBarrierThr *)t);
}

template <class T_Barrier>
inline bool XMI::Device::AtomicBarrierMdl<T_Barrier>::postMultisync_impl(AtomicBarrierMsg<T_Barrier> *msg) {

	_barrier.pollInit();
	// See if we can complete the barrier immediately...
	for (int x = 0; x < 32; ++x) {
		if (_barrier.poll() == XMI::Atomic::Interface::Done) {
			_executeCallback();
			return true;
		}
	}
	// must "continue" current barrier, not start new one!
	new (msg) AtomicBarrierMsg<T_Barrier>(_g_lmbarrier_dev, &_barrier, _getCallback());
	_g_lmbarrier_dev.__post(msg);
	return true;
}

#endif //  __components_devices_generic_atomicbarrier_h__
