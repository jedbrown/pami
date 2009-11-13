/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_generic_GenericDevice_h__
#define __components_devices_generic_GenericDevice_h__

// This file implements the interfaces used by messaging (advance).
// See generic/Device.h for interfaces used by sub-devices.

#undef USE_WAKEUP_VECTORS

#include "components/devices/generic/Device.h"

// All sub-devices are instantiated in generic/Device.cc, but are
// used here. There must be a call to the init() for each sub-device,
// and a call to advance?_recv() for every sub-device that requires
// recv polling - convention is to call *all* sub-device advanceRecv()
// routines, the compiler will optimize away any that are simply "return 0".
// (Note, advanceRecv() methods are inlined) See Device::__advanceRecv(),
// and also Device::init(). Note, there are two __advanceRecv(), one which
// takes a "channel" arg - this one is used for sub-devices that have multiple
// recv channels and must poll only one channel at a time. It is up to the
// sub-device to understand how "channel" relates to it's recv resources.
// Typically, an association is made during init().
// [ Not used yet - This may require changes in order to make it work ]
//
#include "components/devices/generic/ProgressFunctionMsg.h"
#include "components/devices/generic/AtomicBarrierMsg.h"
#include "components/devices/workqueue/WQRingReduceMsg.h"
#include "components/devices/workqueue/WQRingBcastMsg.h"
#include "components/devices/workqueue/LocalAllreduceWQMessage.h"
#include "components/devices/workqueue/LocalReduceWQMessage.h"
#include "components/devices/workqueue/LocalBcastWQMessage.h"

extern XMI::Device::ProgressFunctionDev _g_progfunc_dev;
extern XMI::Device::AtomicBarrierDev _g_lmbarrier_dev;
extern XMI::Device::WQRingReduceDev _g_wqreduce_dev;
extern XMI::Device::WQRingBcastDev _g_wqbcast_dev;
extern XMI::Device::LocalAllreduceWQDevice _g_l_allreducewq_dev;
extern XMI::Device::LocalBcastWQDevice _g_l_bcastwq_dev;
extern XMI::Device::LocalReduceWQDevice _g_l_reducewq_dev;

#if defined(__bgp__) and !defined(__bgq__)
// These are needed in order to use the Collective Network sub-devices.
#include "components/devices/bgp/collective_network/CNAllreduce.h"
#include "components/devices/bgp/collective_network/CollectiveNetworkLib.h"
#include "components/devices/bgp/collective_network/CNDevice.h"

#ifdef NOT_YET
#include "components/devices/bgp/memory/MemoryBarrierMsg.h"
#include "components/devices/bgp/memory/LLSCMsg.h"
#endif
#include "components/devices/bgp/global_interrupt/GIBarrierMsg.h"
#include "components/devices/bgp/collective_network/CNAllreduceMsg.h"
//#include "components/devices/bgp/collective_network/CNAllreduceShortMsg.h"
#include "components/devices/bgp/collective_network/CNAllreducePPMsg.h"
#include "components/devices/bgp/collective_network/CNAllreduceSum2PMsg.h"
#include "components/devices/bgp/collective_network/CNBroadcastMsg.h"

#ifdef NOT_YET
extern XMI::Device::BGP::MemoryBarrierDev _g_mbarrier_dev;
extern XMI::Device::BGP::LLSCDev _g_llscbarrier_dev;
#endif
extern XMI::Device::BGP::giDevice _g_gibarrier_dev;
extern XMI::Device::BGP::CNAllreduceDevice _g_cnallreduce_dev;
//extern XMI::Device::BGP::CNAllreduceShortDevice _g_cnallreduceshort_dev;
extern XMI::Device::BGP::CNAllreducePPDevice _g_cnallreducepp_dev;
extern XMI::Device::BGP::CNAllreduce2PDevice _g_cnallreduce2p_dev;
extern XMI::Device::BGP::CNBroadcastDevice _g_cnbroadcast_dev;

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
	__contextId((size_t)-1),
	__nContexts(0)
	{
	}

	/// \brief Initialize the Generic Device and all subdevices.
	///
	/// When a new subdevice is added, a call to its init routine is
	/// added here.
	///
	/// \param[in] sd	The SysDep object
	///
	inline void Device::init(XMI_SYSDEP_CLASS &sd, size_t context, size_t num_contexts) {
		__contextId = context;
		__nContexts = num_contexts;
#ifdef USE_WAKEUP_VECTORS
		xmi_result_t rc = XMI_ERROR;
		size_t me = __global.mapping.t(); // only works on BG/P...?
		rc = __sysdep.wakeupManager().allocWakeVecs((int)XMI_MAX_PROC_PER_NODE,
			XMI_MAX_THREAD_PER_PROC * MAX_REG_THREADS, (int)me,
			&__wakeupVectors);
		XMI_assert(rc == XMI_SUCCESS);
#endif /* USE_WAKEUP_VECTORS */
		// These are all the devices we know how to play well with...

		// todo: need to work out how to handle devices that need
		// a new instance for each client, or context. For now,
		// these devices will check __contextId and __nContexts, or
		// some static init variable, to avoid repeated initialization.
		// Subdevice instances could be part of the generic device
		// (per context) or created only when 'context' is zero
		// (per client). Then need to work out how to pass around
		// those instances.

		_g_progfunc_dev.init(sd, this);
		_g_lmbarrier_dev.init(sd, this);
		_g_wqreduce_dev.init(sd, this);
		_g_wqbcast_dev.init(sd, this);
		_g_l_allreducewq_dev.init(sd, this);
		_g_l_reducewq_dev.init(sd, this);
		_g_l_bcastwq_dev.init(sd, this);

#ifdef __bgp__
#ifdef NOT_YET
		_g_mbarrier_dev.init(sd, this);
		_g_llscbarrier_dev.init(sd, this);
#endif
		_g_gibarrier_dev.init(sd, this);

		_g_cnallreduce_dev.init(sd, this);
		//_g_cnallreduceshort_dev.init(sd, this);
		_g_cnallreducepp_dev.init(sd, this);
		_g_cnallreduce2p_dev.init(sd, this);
		_g_cnbroadcast_dev.init(sd, this);
#endif // __bgp__
	}

	/// \brief Quick check whether full advance is needed.
	///
	/// Currently not used, since subdevices have to be polled for recvs.
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

	/// \brief Advance (poll) Recvs on subdevices that are non channel-oriented
	///
	/// When a new subdevice is added, and it supports only one channel,
	/// a call to its advanceRecv function is added here. Convention is
	/// to add a call here even if the device has no recvs (unexpected msgs),
	/// and the compiler will optimize it away. Typically, every subdevice
	/// will have one call to advanceRecv, either here or in the multi-channel
	/// routine.
	///
	/// \param[in]	ctx	The channel to poll
	/// \return	Number of work units processed
	///
	inline int Device::__advanceRecv() {
		int events = 0;
		// not all devices actually have "unexpected" messages, but we call anyway.
		// Presumably, empty functions will be optimized away by the compiler.
		// Has no concept of recv at all: _g_progfunc_dev
		events += _g_lmbarrier_dev.advanceRecv(__contextId);
		events += _g_wqreduce_dev.advanceRecv(__contextId);
		events += _g_wqbcast_dev.advanceRecv(__contextId);
		events += _g_l_allreducewq_dev.advanceRecv(__contextId);
		events += _g_l_reducewq_dev.advanceRecv(__contextId);
		events += _g_l_bcastwq_dev.advanceRecv(__contextId);

#ifdef __bgp__
#ifdef NOT_YET
		events += _g_mbarrier_dev.advanceRecv(__contextId);
		events += _g_llscbarrier_dev.advanceRecv(__contextId);
#endif
		events += _g_gibarrier_dev.advanceRecv(__contextId);

		events += _g_cnallreduce_dev.advanceRecv(__contextId);
		//events += _g_cnallreduceshort_dev.advanceRecv(__contextId);
		events += _g_cnallreducepp_dev.advanceRecv(__contextId);
		events += _g_cnallreduce2p_dev.advanceRecv(__contextId);
		events += _g_cnbroadcast_dev.advanceRecv(__contextId);
#endif // __bgp__
		return events;
	}

	//////////////////////////////////////////////////////////////////
	/// \brief     Advance routine for the device.  This pulls the
	///            message from the front of the queue and calls the
	///            advance routine on the message.
	/// \returns:  Return code of the advance routine (number of
	///            events processed)
	//////////////////////////////////////////////////////////////////
	inline int Device::advance() {
		int events = 0;
		//+ Need to ensure only one of these runs per core
		//+ (even if multi-threads per core)
		//+ if (core_mutex.tryAcquire()) {

		// Advance any recvs on this "channel" (only 1 channel)...
		events += __advanceRecv();
		//if (!__Threads.mutex()->tryAcquire()) continue;
		GenericAdvanceThread *thr;
#ifdef USE_WAKEUP_VECTORS
#error WakeupManager TBD
		// poll the wake-up arena for any "wake ups".
		for (int y = 0; y < __numRegThreads; ++y) {
#if 0
			thr = __thrRegistry[y];
			if (thr->isDone()) {
				//__sysdep.wakeupManager().resetWakeup(vec);
				continue;
			}
			void *vec = thr->getWakeVec();
#endif
			void *vec = __sysdep.wakeupManager().getWakeVec(__wakeupVectors, y)) {
			if (__sysdep.wakeupManager().pollWakeup(vec)) {
				__sysdep.wakeupManager().callWakeup(__wakeupVectors, vec);
				GenericMessage *msg = (GenericMessage *)thr->getMsg();
				// currently, advanceThread() does not distinguish between
				// not-done and no-progress.
				++events;
				msg->advanceThread(thr);
			}
		}
#endif /* USE_WAKEUP_VECTORS */
		for (thr = (GenericAdvanceThread *)__Threads.peekHead();
					thr; thr = (GenericAdvanceThread *)thr->next()) {
			GenericMessage *msg = (GenericMessage *)thr->getMsg();
			// currently, advanceThread() does not distinguish between
			// not-done and no-progress.
			++events;
			if (msg->advanceThread(thr) == Done) {
				__Threads.deleteElem(thr);
			}
		}
		//__Threads.mutex()->release();

		//+ core_mutex.release();

		// Now check everything on the completion queue...
		GenericMessage *msg;
		for (msg = (GenericMessage *)__GenericQueue.peekHead();
				msg; msg = (GenericMessage *)msg->next(1)) {
			if (msg->getStatus() == Done) {
				++events;
				__GenericQueue.deleteElem(msg);
				msg->complete();
			}
		}
		return events;
	}

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_genericdevice_h__ */
