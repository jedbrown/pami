/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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
#include "components/devices/util/SubDeviceSuppt.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/FactoryInterface.h"
#include "components/atomic/Barrier.h"
#include "sys/xmi.h"

namespace XMI {
namespace Device {

template <class T_Barrier> class AtomicBarrierMsg;
template <class T_Barrier> class AtomicBarrierMdl;
typedef XMI::Device::Generic::GenericAdvanceThread AtomicBarrierThr;
class AtomicBarrierDev : public XMI::Device::Generic::MultiSendQSubDevice<AtomicBarrierThr,1,true> {
public:
	class Factory : public Interface::FactoryInterface<Factory,AtomicBarrierDev,Generic::Device> {
	public:
		static inline AtomicBarrierDev *generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager & mm);
		static inline xmi_result_t init_impl(AtomicBarrierDev *devs, size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices);
		static inline size_t advance_impl(AtomicBarrierDev *devs, size_t client, size_t context);
		static inline AtomicBarrierDev & getDevice_impl(AtomicBarrierDev *devs, size_t client, size_t context);
	}; // class Factory
}; // class AtomicBarrierDev

}; //-- Device
}; //-- XMI

extern XMI::Device::AtomicBarrierDev _g_lmbarrier_dev;

namespace XMI {
namespace Device {

inline AtomicBarrierDev *AtomicBarrierDev::Factory::generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager &mm) {
	_g_lmbarrier_dev.__create(client, num_ctx);
	return &_g_lmbarrier_dev;
}

inline xmi_result_t AtomicBarrierDev::Factory::init_impl(AtomicBarrierDev *devs, size_t client, size_t contextid, xmi_client_t clt, xmi_context_t context, SysDep *sd, XMI::Device::Generic::Device *devices) {
	return _g_lmbarrier_dev.__init(client, contextid, clt, context, sd, devices);
}

inline size_t AtomicBarrierDev::Factory::advance_impl(AtomicBarrierDev *devs, size_t clientid, size_t contextid) {
	return 0;
}

inline AtomicBarrierDev & AtomicBarrierDev::Factory::getDevice_impl(AtomicBarrierDev *devs, size_t clientid, size_t contextid) {
	return _g_lmbarrier_dev;
}

///
/// \brief A local barrier message that takes advantage of the
/// Load Linked and Store Conditional instructions
///
template <class T_Barrier>
class AtomicBarrierMsg : public XMI::Device::Generic::GenericMessage {
public:

protected:
	friend class AtomicBarrierMdl<T_Barrier>;

	AtomicBarrierMsg(Generic::GenericSubDevice *Generic_QS,
		T_Barrier *barrier,
		xmi_multisync_t *msync) :
	XMI::Device::Generic::GenericMessage(Generic_QS, msync->cb_done,
					msync->client, msync->context),
	_barrier(barrier)
	{
		// assert(role == DEFAULT_ROLE);
	}

protected:

	DECL_ADVANCE_ROUTINE(advanceThread,AtomicBarrierMsg<T_Barrier>,AtomicBarrierThr);
	inline xmi_result_t __advanceThread(AtomicBarrierThr *thr) {
		for (int x = 0; x < 32; ++x) {
			if (_barrier->poll() == XMI::Atomic::Interface::Done) {
				setStatus(XMI::Device::Done);
				thr->setStatus(XMI::Device::Complete);
				return XMI_SUCCESS;
			}
		}
		return XMI_EAGAIN;
	}

public:
	// virtual function
	xmi_context_t postNext(bool devPosted) {
		return _g_lmbarrier_dev.__postNext<AtomicBarrierMsg>(this, devPosted);
	}

	inline int setThreads(AtomicBarrierThr **th) {
		AtomicBarrierThr *t;
		int n;
		_g_lmbarrier_dev.__getThreads(&t, &n);
		t->setMsg(this);
		t->setAdv(advanceThread);
		t->setStatus(XMI::Device::Ready);
		__advanceThread(t);
		*th = t;
		return 1;
	}

protected:
	T_Barrier *_barrier;
}; //-- AtomicBarrierMsg

template <class T_Barrier>
class AtomicBarrierMdl : public XMI::Device::Interface::MultisyncModel<AtomicBarrierMdl<T_Barrier>,
                                            AtomicBarrierDev,sizeof(AtomicBarrierMsg<T_Barrier>) > {
public:
	static const size_t sizeof_msg = sizeof(AtomicBarrierMsg<T_Barrier>);

	AtomicBarrierMdl(AtomicBarrierDev &device, xmi_result_t &status) :
          XMI::Device::Interface::MultisyncModel<AtomicBarrierMdl<T_Barrier>,
                        AtomicBarrierDev,sizeof(AtomicBarrierMsg<T_Barrier>) >(device, status)
	{
		// assert(device == _g_lmbarrier_dev);
		// "default" barrier: all local processes...
		size_t peers = __global.topology_local.size();
		size_t peer0 = __global.topology_local.index2Rank(0);
		size_t me = __global.mapping.task();
		_barrier.init(_g_lmbarrier_dev.getSysdep(), peers, (peer0 == me));
	}

	inline xmi_result_t postMultisync_impl(uint8_t (&state)[sizeof_msg],
                                               xmi_multisync_t *msync);

private:
	T_Barrier _barrier;
}; // class AtomicBarrierMdl

}; //-- Device
}; //-- XMI

template <class T_Barrier>
inline xmi_result_t XMI::Device::AtomicBarrierMdl<T_Barrier>::postMultisync_impl(uint8_t (&state)[sizeof_msg],
                                                                                 xmi_multisync_t *msync) {
	_barrier.pollInit();
	// See if we can complete the barrier immediately...
	for (int x = 0; x < 32; ++x) {
		if (_barrier.poll() == XMI::Atomic::Interface::Done) {
			if (msync->cb_done.function) {
				xmi_context_t ctx = _g_lmbarrier_dev.getGenerics(msync->client)[msync->context].getContext();
				msync->cb_done.function(ctx, msync->cb_done.clientdata, XMI_SUCCESS);
			}
			return XMI_SUCCESS;
		}
	}
	// must "continue" current barrier, not start new one!
	AtomicBarrierMsg<T_Barrier> *msg;
	msg = new (&state) AtomicBarrierMsg<T_Barrier>(_g_lmbarrier_dev.getQS(), &_barrier, msync);
	_g_lmbarrier_dev.__post<AtomicBarrierMsg<T_Barrier> >(msg);
	return XMI_SUCCESS;
}

#endif //  __components_devices_generic_atomicbarrier_h__
