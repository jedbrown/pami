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
#include "components/devices/generic/Device.h"

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

	inline void __create(size_t client, size_t num_ctx) {
		for (int x = 0; x < N_Queues; ++x) {
			_sendQs[x].___create(client, num_ctx);
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
	inline xmi_result_t __init(size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices) {
		if (client == 0) {
			_sd = sd;
			for (int x = 0; x < N_Queues; ++x) {
				_sendQs[x].___init(sd, client, contextId);
			}
		}
		if (contextId == 0) {
			_generics[client] = devices;
		}
		return XMI_SUCCESS;
	}

	inline XMI::SysDep *getSysdep() {
		return _sd;
	}

	inline XMI::Device::Generic::Device *getGenerics(size_t client) {
		return _generics[client];
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
		g = getGenerics(msg->getClientId());
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
		bool first = (!Use_Queue || qs->peek() == NULL);
		// If !Use_Queue, there must never be a message queued...
		// assert(Use_Queue || qs->peek() == NULL);
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
			// Also avoid the peek() check above.
		}
		if (Use_Queue) qs->enqueue(msg);
	}

protected:
	GenericSubDevice _sendQs[N_Queues];
	T_Thread _threads[N_Queues][N_Threads];
	XMI::Device::Generic::Device *_generics[XMI_MAX_NUM_CLIENTS];
	XMI::SysDep *_sd;
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
	virtual xmi_result_t init(XMI::SysDep *sd, size_t client, size_t contextId, xmi_context_t ctx) = 0;

	/// \brief CommonQueueSubDevice portion of init function
	///
	/// All classes that inherit from this must implement init(), and that
	/// must callback to __init().
	///
	/// \param[in] sd	SysDep object
	/// \param[in] device	Generic::Device to be used.
	/// \ingroup gendev_subdev_api
	///
	inline xmi_result_t __init(size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices) {
		if (client == 0) {
			_sd = sd;
			_doneThreads.init(sd);
			_doneThreads.fetch_and_clear();
			_init = 1;
		}
		if (contextId == 0) {
			_generics[client] = devices;
		}
		return ___init(sd, client, contextId);
	}

	inline void __create(size_t client, size_t num_ctx) {
		___create(client, num_ctx);
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

	inline XMI::SysDep *getSysdep() { return _sd; }

	inline XMI::Device::Generic::Device *getGenerics(size_t client) {
		return _generics[client];
	}

// protected:
//	friend class T_Model
public:
	template <class T_Message, class T_Thread>
	inline xmi_context_t __postNext(XMI::Device::Generic::GenericMessage *msg, bool devPosted) {
		XMI::Device::Generic::Device *g;
		g = getGenerics(msg->getClientId());
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
		bool first = (qs->peek() == NULL);
		if (first) {
			xmi_context_t ctx = __postNext<T_Message,T_Thread>(msg, false);
			if (ctx) {
				msg->executeCallback(ctx);
				return;
			}
		}
		qs->enqueue(msg);
	}

private:
	int _init;
	GenericDeviceCounter _doneThreads;
	unsigned _dispatch_id;
	XMI::Device::Generic::Device *_generics[XMI_MAX_NUM_CLIENTS];
	XMI::SysDep *_sd;
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

	inline void __create(size_t client, size_t num_ctx) {
		_common->__create(client, num_ctx);
	}

	inline int advance(size_t client, size_t context) { return 0; }

	inline xmi_result_t __init(size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices) {
		if (client == 0) {
			// do this now so we don't have to every time we post
//			for (int x = 0; x < NUM_THREADS; ++x)
//				//_threads[x].setPolled(true);
//			}
		}
		_common->__init(client, contextId, clt, ctx, sd, devices);
		return _common->init(sd, client, contextId, ctx);
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
