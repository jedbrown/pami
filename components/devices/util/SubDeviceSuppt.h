/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_generic_SubDeviceSuppt_h__
#define __components_devices_generic_SubDeviceSuppt_h__

#include "components/devices/generic/SubDevice.h"

////////////////////////////////////////////////////////////////////////
///  \file components/devices/generic/SubDeviceSuppt.h
///  \brief Generic Device Examples and Frequently-used sub-classes
///
///  Namespace:  XMI, the messaging namespace.
///
////////////////////////////////////////////////////////////////////////
namespace XMI {
namespace Device {
namespace Generic {

// This base class is for devices that have a single, global, instance
// and each context has a pseudo-device object that keeps context-specific
// info but otherwise uses the global instance to do work.
template <class T_Device, class T_RealDevice>
class SimplePseudoDevice {
public:
	static inline T_Device *__create(size_t client, size_t num_ctx, XMI::Device::Generic::Device *devices, T_RealDevice *realdev) {
		size_t x;
		T_Device *devs;
		int rc = posix_memalign((void **)&devs, 16, sizeof(*devs) * num_ctx);
		XMI_assertf(rc == 0, "posix_memalign failed for PseudoDevice[%zd], errno=%d\n", num_ctx, errno);
		realdev->__create(client, num_ctx, devices);
		for (x = 0; x < num_ctx; ++x) {
			new (&devs[x]) T_Device(client, num_ctx, devices, x);
		}
		return devs;
	}

	inline SimplePseudoDevice(size_t client, size_t num_ctx, XMI::Device::Generic::Device *devices, size_t ctx) :
	_clientid(client),
	_contextid(ctx),
	_ncontext(num_ctx),
	_generics(devices)
	{
	}

	inline void __init(SysDep *sd, size_t client, size_t num_ctx, xmi_context_t context, size_t contextid, T_RealDevice *realdev) {
		_context = context;
		if (client == 0 && contextid == 0) {
			realdev->init(sd, client, num_ctx, context, contextid);
		}
	}

	inline size_t advance(size_t client, size_t ctx) {
		SimplePseudoDevice *dev = &this[ctx];
		return static_cast<T_Device *>(dev)->advance_impl();
	}
protected:
	// do we need to save all this?
	size_t _clientid;
	size_t _contextid;
	size_t _ncontext;
	xmi_context_t _context;
	XMI::Device::Generic::Device *_generics;
}; // class SimplePseudoDevice

/// \brief Example sub-device for using multiple send queues
///
/// This is typically what 'local' point-to-point devices do, to enforce
/// ordering to a peer.
///
/// In this example, the actual Model/Message/Device would use
/// getSendQDev(peer) to get the "QS" object to pass to the Message ctor.
/// i.e. msg->getSQ() must return one of the _sendQs[] objects so that
/// the generic device can run the __complete() method on it.
///
/// An alternative is that the actual device might simply add a
/// GenericSubDevice element to each of it's FIFO objects (instead of
/// the simple sendQ). It would still need to ensure that the Message
/// contained a reference to that GenericSubDevice object in it's 'QS'.
///
template <class T_Thread,int N_Queues,int N_Threads,bool Use_Queue>
class MultiSendQSubDevice {
	static const int NUM_THREADS = N_Threads;
public:
	MultiSendQSubDevice() {
		// There must be at least one queue, since every message
		// requires a valid _QS from which to operate.
		// However, a device/message may not actually queue
		// anything to this _QS.
		COMPILE_TIME_ASSERT(N_Queues > 0);
		for (int x = 0; x < N_Queues; ++x) {
			new (&_sendQs[x]) GenericSubDevice();
			for (int y = 0; y < N_Threads; ++y) {
				new (&_threads[x][y]) T_Thread();
			}
		}
	}

	inline void __create(size_t client, size_t num_ctx,
				XMI::Device::Generic::Device *devices) {
		for (int x = 0; x < N_Queues; ++x) {
			_sendQs[x].___create(client, num_ctx, devices);
		}
	}

	// may be overridden by child class... Context calls using child class.
	inline int advance(size_t client, size_t context) { return 0; }

	/// \brief Initialization for the subdevice
	///
	/// \param[in] sd		SysDep object (not used?)
	/// \param[in] devices		Array of Generic::Device objects for client
	/// \param[in] contextId	Id of current context (index into devices[])
	/// \ingroup gendev_subdev_api
	///
	inline void init(XMI::SysDep *sd, size_t client, size_t nctx, xmi_context_t ctx, size_t contextId) {
		if (client == 0) {
			for (int x = 0; x < N_Queues; ++x) {
				_sendQs[x].___init(sd, client, contextId);
			}
		}
	}

	inline XMI::SysDep *getSysdep(int sendq = 0) {
		return _sendQs[sendq].getSysdep();
	}

	inline XMI::Device::Generic::Device *getGenerics(size_t client, int sendq = 0) {
		return _sendQs[sendq].getGenerics(client);
	}

	inline XMI::Device::Generic::GenericSubDevice *getQS(int sendq = 0) {
		return &_sendQs[sendq];
	}

// protected:
//	friend class T_Message
public:
	inline void __getThreads(T_Thread **t, int *n, int sendq = 0) {
		if (N_Threads > 0) {
			*t = &_threads[sendq][0];
			*n = N_Threads;
		} else {
			*t = NULL;
			*n = 0;
		}
	}

// protected:
//	friend class T_Model
public:
	template <class T_Message>
	inline xmi_context_t __postNext(XMI::Device::Generic::GenericMessage *msg, bool devPosted) {
		XMI::Device::Generic::Device *g;
		GenericSubDevice *qs = (GenericSubDevice *)msg->getQS();
		g = qs->getGenerics(msg->getClientId());
		T_Thread *t;
		int n;
		msg->setStatus(XMI::Device::Initialized);
		// setThreads() might complete some/all threads...
		n = static_cast<T_Message *>(msg)->setThreads(&t);
		size_t x = msg->getContextId();
		if (!devPosted && msg->getStatus() == XMI::Device::Done) {
			// assert(g[x].getContext() != NULL);
			return g[x].getContext();
		}
		size_t numctx = g[x].nContexts();
		g[x].postMsg(msg);
		while (n > 0) {
			if (t->getStatus() != XMI::Device::Complete) {
				if (++x >= numctx) x = 0;
				g[x].postThread(t);
			}
			++t;
			--n;
		}
		return NULL;
	}

	template <class T_Message>
	inline void __post(XMI::Device::Generic::GenericMessage *msg) {
		GenericSubDevice *qs = (GenericSubDevice *)msg->getQS();
		bool first = (!Use_Queue || qs->getCurrent() == NULL);
		// If !Use_Queue, there must never be a message queued...
		// assert(Use_Queue || qs->getCurrent() == NULL);
		if (first) {
			xmi_context_t ctx = __postNext<T_Message>(msg, false);
			if (ctx) {
				msg->executeCallback(ctx);
				return;
			}
			// If this device does not use the queue, avoid
			// enqueueing the unfinished message... assume that the
			// __postNext() call setup everything on the generic
			// device and we no longer care about it...
			// Also avoid the getCurrent() check above.
		}
		if (Use_Queue) qs->post(msg);
	}

protected:
	GenericSubDevice _sendQs[N_Queues];
	T_Thread _threads[N_Queues][N_Threads];
}; // class MultiSendQSubDevice

///
/// Implements a shared-queue for use by multiple different Thr/Msg/Dev/Mdl sets
/// which all share the same hardware (system) resource. Such a family of sets
/// would each refer to a common instance of this class object when doing their
/// init. The Device class of each set would inherit from SharedQueueSubDevice.
///
/// Supports only one active message at a time.
///
class CommonQueueSubDevice : public GenericSubDevice {
	#define ATOMIC_BUF_SIZE	16

public:

	CommonQueueSubDevice() :
	GenericSubDevice(),
	_dispatch_id(0)
	{
	}

	/// \brief returns a unique ID relative to this common sub-device
	///
	/// \return integer ID unique to this CommonQueueSubDevice
	/// \ingroup gendev_subdev_api
	///
	inline unsigned newDispID() {
		// caller must ensure number os valid for their hardware,
		// for example a network device might only have 8 bits in
		// the header for "dispatch ID".

		// might need to be atomic, in some situations?
		return ++_dispatch_id;
	}

	/// \brief init virtual function definition
	///
	/// All classes that inherit from this must implement init(), and that
	/// must callback to __init().
	///
	/// \param[in] sd	SysDep object
	/// \param[in] device	Generic::Device to be used.
	/// \ingroup gendev_subdev_api
	///
	virtual void init(XMI::SysDep *sd, size_t client, size_t nctx, xmi_context_t ctx, size_t contextId) = 0;

	/// \brief CommonQueueSubDevice portion of init function
	///
	/// All classes that inherit from this must implement init(), and that
	/// must callback to __init().
	///
	/// \param[in] sd	SysDep object
	/// \param[in] device	Generic::Device to be used.
	/// \ingroup gendev_subdev_api
	///
	inline void __init(XMI::SysDep *sd, size_t client, size_t nctx, xmi_context_t ctx, size_t contextId) {
		if (client == 0) {
			_doneThreads.init(sd);
			_doneThreads.fetch_and_clear();
			_init = 1;
		}
		___init(sd, client, contextId);
	}

	inline void __create(size_t client, size_t num_ctx, XMI::Device::Generic::Device *devices) {
		___create(client, num_ctx, devices);
	}

	/// \brief Reset for threads prior to being re-used.
	/// \ingroup gendev_subdev_internal_api
	///
	inline void __resetThreads() {
		_doneThreads.fetch_and_clear();
	}

	/// \brief CommonQueueSubDevice portion of completion for a thread
	///
	/// \param[in] t	Thread being completed
	/// \return	Total number of threads completed for message
	/// \ingroup gendev_subdev_internal_api
	///
	inline unsigned __completeThread(GenericAdvanceThread *t) {
		// fetchIncr() returns value *before* increment,
		// and we need to return total number of threads completed,
		// so we return "+1".
		return _doneThreads.fetch_and_inc() + 1;
	}

// protected:
//	friend class T_Model
public:
	template <class T_Message, class T_Thread>
	inline xmi_context_t __postNext(XMI::Device::Generic::GenericMessage *msg, bool devPosted) {
		XMI::Device::Generic::Device *g;
		GenericSubDevice *qs = (GenericSubDevice *)msg->getQS();
		g = qs->getGenerics(msg->getClientId());
		T_Thread *t;
		int n;
		msg->setStatus(XMI::Device::Initialized);
		// setThreads() might complete some/all threads...
		n = static_cast<T_Message *>(msg)->setThreads(&t);
		size_t x = msg->getContextId();
		if (!devPosted && msg->getStatus() == XMI::Device::Done) {
			// assert(g[x].getContext() != NULL);
			return g[x].getContext();
		}
		size_t numctx = g[x].nContexts();
		g[x].postMsg(msg);
		while (n > 0) {
			if (t->getStatus() != XMI::Device::Complete) {
				if (++x >= numctx) x = 0;
				g[x].postThread(t);
			}
			++t;
			--n;
		}
		return NULL;
	}

	/// \brief Internal posting of message to sub-device
	///
	/// Since postNext() will try to advance the message, it may
	/// also complete it. This is tested and appropriate action taken.
	///
	/// \param[in] msg	Message to start and/or enqueue
	/// \ingroup gendev_subdev_internal_api
	///
	template <class T_Message, class T_Thread>
	inline void __post(XMI::Device::Generic::GenericMessage *msg) {
		GenericSubDevice *qs = (GenericSubDevice *)msg->getQS();
		bool first = (qs->getCurrent() == NULL);
		if (first) {
			xmi_context_t ctx = __postNext<T_Message,T_Thread>(msg, false);
			if (ctx) {
				msg->executeCallback(ctx);
				return;
			}
		}
		qs->post(msg);
	}

private:
	int _init;
	GenericDeviceCounter _doneThreads;
	unsigned _dispatch_id;
}; // class CommonQueueSubDevice

/// \brief class for a Model/Device/Message/Thread tuple that shares hardware with others
///
/// Each Model/Device/Message/Thread tuple will inherit from SharedQueueSubDevice and
/// reference the same instance of CommonQueueSubDevice in the constructor.
/// One instance of CommonQueueSubDevice represents the hardware, which is shared, that
/// can only accomodate one message (of any type) active at a time.
///
/// We would like to have the _threads[] storage kept in the CommonQueueSubDevice,
/// but each Model/Device/Message/Thread tuple could have a different sized Thread class
/// and so we must have the _threads[] array here, where we know the exact Thread type.
///
template <class T_CommonDevice, class T_Thread, int N_Threads>
class SharedQueueSubDevice : public BaseGenericDevice {
	static const int NUM_THREADS = N_Threads;
public:
	// Note, 'common' must have been constructed but otherwised untouched.
	// The first SharedQueueSubDevice to encounter it will initialize it.
	SharedQueueSubDevice(T_CommonDevice *common) :
	BaseGenericDevice(),
	_common(common)
	{
	}

	/// \brief returns a unique ID relative to this common sub-device
	///
	/// \return integer ID unique to the CommonQueueSubDevice for this sub-device
	///
	inline unsigned newDispID() {
		return _common->newDispID();
	}

	/// \brief accessor for the common device for this sub-device
	/// \return	CommonQueueSubDevice
	/// \ingroup gendev_subdev_internal_api
	inline T_CommonDevice *common() { return _common; }

	/// \brief Inform caller of where the threads array is
	///
	/// In this case, threads are not in the CommonQueueSubDevice but
	/// instead in each SharedQueueSubDevice.
	///
	/// \param[out] t	Pointer to threads array
	/// \param[out] n	Pointer to number of threads in array
	/// \ingroup gendev_subdev_internal_api
	///
	inline void getThreads(T_Thread **t, int *n) {
		*t = _threads;
		*n = NUM_THREADS;
	}

	inline XMI::Device::Generic::Device *getGenerics(size_t client) {
		return _common->getGenerics(client);
	}

	inline void __create(size_t client, size_t num_ctx, XMI::Device::Generic::Device *devices) {
		_common->__create(client, num_ctx, devices);
	}

	inline int advance(size_t client, size_t context) { return 0; }

	inline void init(XMI::SysDep *sd, size_t client, size_t nctx, xmi_context_t ctx, size_t contextId) {
		if (client == 0) {
			// do this now so we don't have to every time we post
//			for (int x = 0; x < NUM_THREADS; ++x)
//				//_threads[x].setPolled(true);
//			}
		}
		_common->init(sd, client, nctx, ctx, contextId);
	}

	inline void __getThreads(T_Thread **t, int *n, int sendq = 0) {
		*t = &_threads[sendq];
		*n = N_Threads;
	}

private:
	// For some reason, we can't declare friends like this.
	//friend class T_Message;
	//friend class T_Model;
	// So, need to make it public for now...
public:	// temporary?

	/// \brief Internal posting of message to sub-device
	///
	/// Since postNext() will try to advance the message, it may
	/// also complete it. This is tested and appropriate action taken.
	///
	/// \param[in] msg	Message to start and/or enqueue
	/// \ingroup gendev_subdev_internal_api
	///
	template <class T_Message>
	inline void __post(XMI::Device::Generic::GenericMessage *msg) {
		_common->__post<T_Message,T_Thread>(msg);
	}

	/// \brief SharedQueueSubDevice portion of completion for a thread
	///
	/// Must at least call CommonQueueSubDevice __completeThread().
	///
	/// \param[in] t	Thread being completed
	/// \return	Total number of threads completed for message
	/// \ingroup gendev_subdev_internal_api
	///
	inline unsigned __completeThread(T_Thread *thr) {
		return _common->__completeThread(thr);
	}

private:
	T_CommonDevice *_common;
	T_Thread _threads[NUM_THREADS];
	int _nActiveThreads;
}; // class SharedQueueSubDevice

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif // __components_devices_generic_SubDeviceSuppt_h__
