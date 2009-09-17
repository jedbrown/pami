/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_generic_genericdevice_h__
#define __components_devices_generic_genericdevice_h__

#undef USE_WAKEUP_VECTORS

#include "components/devices/generic/Device.h"

#include "components/devices/generic/ProgressFunction.h"
#include "components/lockmanager/LockManagerBarrierMsg.h"
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

#ifdef __bgp__
#include "components/devices/bgp/memory/MemoryBarrierMsg.h"
#include "components/devices/bgp/memory/LLSCMsg.h"
#include "components/devices/bgp/gi/Device.h"
#include "components/devices/bgp/collective_network/CNAllreduce.h"
#include "components/devices/bgp/collective_network/CollectiveNetworkLib.h"
#include "components/devices/bgp/collective_network/CNDevice.h"
#include "components/devices/bgp/collective_network/CNAllreduceMsg.h"
//#include "components/devices/bgp/collective_network/CNAllreduceShortMsg.h"
#include "components/devices/bgp/collective_network/CNAllreducePPMsg.h"
#include "components/devices/bgp/collective_network/CNAllreduceSum2PMsg.h"
#include "components/devices/bgp/collective_network/CNBroadcastMsg.h"

extern XMI::Device::BGP::MemoryBarrierDev _g_mbarrier_dev;
extern XMI::Device::BGP::LLSCDev _g_llscbarrier_dev;
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

#endif // __bgp__

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

	//////////////////////////////////////////////////////////////////
	/// \brief  Construct the Generic Device
	///
	/// This does simple initialization.
	/// \todo How much of this should be moved to init()?
	///
	/// \param[in] sd	The SysDep object
	///
	//////////////////////////////////////////////////////////////////
	inline Device::Device(SysDep &sd) :
	__lastThreadUsed(0),
	__sysdep(sd)
	{
		for (int x = 0; x < NUM_CHANNELS; ++x) {
			new (&__Threads[x]) ThreadQueue(sd);
		}
		//
		// Each thread must have it's own "private queue" where it keeps
		// messages that it starts. This way, completions are executed only
		// on the thread that originated the message - regardless of where
		// the actual work is performed. More investigation is required to
		// to know just how much, if any, coordination of completions is needed.
		// Another benefit is this avoids locking which would be needed if
		// a single queue were used by all threads in a process. Unless it
		// becomes necessary to share completions between threads, this is
		// probably the right way to do it.
		//
		// This constructor is called by the initial thread only, so it must
		// initialize all the queues. Since only application threads will start
		// communications, helper (comm_)threads need not check their
		// __GenericQueue[] for completions - only the application calls to
		// advance(), if any, do that.
		//
		// Problems/Questions:
		//
		// Is it true that only the application threads will start communications?
		// or does a hand-off start communications, and will that result in a
		// problem here?
		//
		// Also, what about multiple application threads per hwthread? In this
		// case, and current implementation, __GenericQueue[x] would be shared
		// between several software contexts. Does __GenericQueue[] need to be
		// enlarged and indexed by software thread ID instead of hwthread?
		// With multiple app threads per hwthread, there can be only one app
		// running at a time so perhaps the queue can be shared, as long as
		// an app thread is not preempted in the middle of a queue operation.
		//
		for (int x = 0; x < MAX_THREADS_PER_PROC; ++x) {
			new (&__GenericQueue[x]) MultiQueue<2, 1>();
		}
		XMI_Result rc;
		int y = 0;

#ifdef USE_WAKEUP_VECTORS
		size_t me = __sysdep.mapping().t(); // only works on BG/P...?
		rc = __sysdep.wakeupManager().allocWakeVecs((int)NUM_PROCESSES,
			NUM_CHANNELS * MAX_REG_THREADS, (int)me,
			&__wakeupVectors);
		XMI_assert(rc == XMI_SUCCESS);
#endif /* USE_WAKEUP_VECTORS */
#ifdef __bgp__
		// This is for setting up comm_threads on BG/P.
		// These threads are used only by the Tree device,
		// even though this code is fairly abstract. Use of
		// comm_threads by other subdevices is probably risky.
		unsigned cores = 0;
		switch (sd.personality().tSize()) {
		case 1: // SMP mode
			cores = (1 << 1) | (1 << 2) | (1 << 3);
			break;
		case 2: // DUAL mode
			switch (sd.mapping().t()) {
			case 0:
				cores = (1 << 1);
				break;
			case 1:
			default:
				cores = (1 << 3);
				break;
			}
			break;
		case 4: // VN mode
			break;
		default:
			XMI_abort();
		}

		static int opcd[NUM_CORES] = {
			COMMTHRD_OPCODE_CORE0,
			COMMTHRD_OPCODE_CORE1,
			COMMTHRD_OPCODE_CORE2,
			COMMTHRD_OPCODE_CORE3,
		};
		static char _mtx[NUM_CORES][LM_MAX_SIZEOF_MUTEX] __attribute__((__aligned__(16)));

		void *ct;
		int id, op;
		int x;
		//
		// core 0 always has a process running, others might also.
		// comm_thread[1] will serve channel 0,
		// comm_thread[2] will serve channel 1,
		// comm_thread[3] will serve channel 2,
		// This means that any device using comm_threads must ensure
		// that messages intended for comm_threads are posted on the
		// proper channel(s). __maxHWThreads should ensure that
		// messages are only posted to channels backed by comm_threads.
		// This may not always be the best option?
		//
		for (x = 1; x < NUM_CORES && y < NUM_CHANNELS; ++x) {
			if (!(cores & (1 << x))) continue;
			id = Kernel_MkInterruptID(0, 24 + x);
			// Note! this must be setup as a lockbox mutex!
			rc = __sysdep.lockManager().dupLockManagerObject(_mtx[y],
				sizeof(_mtx[y]),
				XMI::LockManager::GENERIC_THRMTX_TEMPLATE);
			if (rc != XMI_SUCCESS) {
				fprintf(stderr, "failed to dup comm_thread Mutex\n");
				continue;
			}
			XMI::Mutex *mx = (XMI::Mutex *)_mtx[y];
			ct = mx->returnLock();
			op = opcd[x] |
				COMMTHRD_OPCODE_CALLFUNC |
				COMMTHRD_OPCODE_DISABLEINTONENTRY |
				COMMTHRD_OPCODE_ENABLEINTONPOOF;
			rc = (XMI_Result)Kernel_SetCommThreadConfig(id, op,
				(uint32_t *)ct,
				advanceHelper,
				(uint32_t)this,
				(uint32_t)y,
				(uint32_t)0,
				(uint32_t)0);
			if (rc) {
				fprintf(stderr, "failed to config send/recv thread (%d)\n", rc);
			} else {
				_thrIPI[y] = (1 << x);
				++y;
			}
		}
#else
#warning generic device not configured for helper threads
		rc = rc;
#endif
		__maxHWThreads = y;
		__maxSWThreads = y + 1;
		// assert(__maxSWThreads <= NUM_CHANNELS);
	}

	/// \brief Initialize the Generic Device and all subdevices.
	///
	/// When a new subdevice is added, a call to its init routine is
	/// added here.
	///
	/// \param[in] sd	The SysDep object
	///
	inline void Device::init(SysDep &sd) {
		// These are all the devices we know how to play well with...
		_g_progfunc_dev.init(sd, this);
		_g_lmbarrier_dev.init(sd, this);
		_g_wqreduce_dev.init(sd, this);
		_g_wqbcast_dev.init(sd, this);
		_g_l_allreducewq_dev.init(sd, this);
		_g_l_reducewq_dev.init(sd, this);
		_g_l_bcastwq_dev.init(sd, this);

#ifdef __bgp__
		_g_mbarrier_dev.init(sd, this);
		_g_llscbarrier_dev.init(sd, this);
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
		int t;
#ifdef __bgp__
		t = Kernel_PhysicalProcessorID();
#else
#warning need way to get thread id
		t = 0;
#endif
		return !__GenericQueue[t].isEmpty();
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
	/// \return	Number of work units processed
	///
	inline int Device::__advanceRecv() {
		int events = 0;
		// not all devices actually have "unexpected" messages, but we call anyway.
		// Presumably, empty functions will be optimized away by the compiler.
		// Has no concept of recv at all: _g_progfunc_dev
		events += _g_lmbarrier_dev.advanceRecv();
		events += _g_wqreduce_dev.advanceRecv();
		events += _g_wqbcast_dev.advanceRecv();
		events += _g_l_allreducewq_dev.advanceRecv();
		events += _g_l_reducewq_dev.advanceRecv();
		events += _g_l_bcastwq_dev.advanceRecv();

#ifdef __bgp__
		events += _g_mbarrier_dev.advanceRecv();
		events += _g_llscbarrier_dev.advanceRecv();
		events += _g_gibarrier_dev.advanceRecv();

		events += _g_cnallreduce_dev.advanceRecv();
		//events += _g_cnallreduceshort_dev.advanceRecv();
		events += _g_cnallreducepp_dev.advanceRecv();
		events += _g_cnallreduce2p_dev.advanceRecv();
		events += _g_cnbroadcast_dev.advanceRecv();
#endif // __bgp__
		return events;
	}

	/// \brief Advance (poll) Recvs on subdevices that have channels
	///
	/// When a new subdevice is added, and it supports multiple channels,
	/// a call to its advanceRecv function is added here.
	///
	/// \param[in]	channel	The channel to poll
	/// \return	Number of work units processed
	///
	inline int Device::__advanceRecv(int channel) {
		int events = 0;
		// No subdevices use recv channels, yet...
		// _g_some_dev.advanceRecv(channel);
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

		// Advance any recvs that have no "channels" (only 1 channel)...
		events += __advanceRecv();
		int t;
#ifdef __bgp__
		t = Kernel_PhysicalProcessorID();
#else
		t = 0;
#endif
		if (__GenericQueue[t].isEmpty()) {
			// still need per-channel advanceRecv... if any exist.
			return events;
		}
		for (int x = 0; x < __maxSWThreads; ++x) {
			if (!__Threads[x].mutex()->tryAcquire()) continue;
			GenericAdvanceThread *thr;
			// Advance any recvs specific to this channel...
			events += __advanceRecv(x);
#ifdef USE_WAKEUP_VECTORS
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
			for (thr = (GenericAdvanceThread *)__Threads[x].peekHead();
					thr; thr = (GenericAdvanceThread *)thr->next()) {
				GenericMessage *msg = (GenericMessage *)thr->getMsg();
				// currently, advanceThread() does not distinguish between
				// not-done and no-progress.
				++events;
				if (msg->advanceThread(thr) == Done) {
					__Threads[x].remove(thr);
				}
			}
			__Threads[x].mutex()->release();
		}
		//+ core_mutex.release();
		//+ }
		// Now check everything on the private queue...
		// Need to ensure that __GenericQueue is totally private,
		// to core and thread - to ensure the exact same conext
		// is used to invoke the completion callback.
		// (is this required?)
		// TBD: what about multiple pthreads per hwthread???
		GenericMessage *msg;
		for (msg = (GenericMessage *)__GenericQueue[t].peekHead();
				msg; msg = (GenericMessage *)msg->next(1)) {
			if (msg->getStatus() == Done) {
				++events;
				__GenericQueue[t].remove(msg);
				msg->complete();
			}
		}
		return events;
	}

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_genericdevice_h__ */
