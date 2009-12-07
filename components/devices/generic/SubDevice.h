/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_generic_SubDevice_h__
#define __components_devices_generic_SubDevice_h__

#include "util/queue/Queue.h"
#include "components/devices/generic/BaseGenericDevice.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/Device.h"
#include "sys/xmi.h"

////////////////////////////////////////////////////////////////////////
///  \file components/devices/generic/SubDevice.h
///  \brief Generic Device
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

class GenericSubDevSendq : public MultiQueue<2, 0> {
public:

	/// \brief Add a message to the (end of the) queue
	///
	/// Does not perform any actions on the message, the caller has
	/// already attempted early advance.
	///
	/// \param[in] msg	New message to be posted
	///
	inline void post(GenericMessage *msg) {
		pushTail(msg);
	}

	/// \brief peek at "next" message on queue
	///
	/// \return	Top message on queue
	///
	inline GenericMessage *getCurrent() {
		return (GenericMessage *)peekHead();
	}

	/// \brief pop "next" message off queue and return it
	///
	/// \return	Former top message on queue
	///
	inline GenericMessage *dequeue() {
		return (GenericMessage *)popHead();
	}

	/// \brief number of messages on the queue
	///
	/// \return	number of messages on the queue
	///
	inline int queueSize() {
		return size();
	}
protected:
}; // class GenericSubDevSendq

/// \brief standard routine for posting message
///
/// in this case, threads come from sub-device...
/// (others might have threads in the message, or ???)
/// postToGeneric() will only post threads that are not Done...
///
/// \param[in] T_Device		Sub-device class
/// \param[in] T_Thread		Sub-device thread class
/// \param[in] I_Device		Sub-device instance pointer
///
#define STD_POSTNEXT(T_Device,T_Thread,I_Device)			\
	inline bool postNext(bool devPosted) {				\
		T_Thread *t;						\
		int N, n;						\
		setStatus(XMI::Device::Initialized);			\
		(I_Device)->getThreads(&t, &N);				\
		n = __setThreads(t, N);					\
		if (!devPosted && getStatus() == XMI::Device::Done) {	\
			return true;					\
		}							\
		(I_Device)->postToGeneric(this, t, sizeof(*t), n);	\
		return false;						\
	}

class GenericSubDevice : public BaseGenericDevice {
private:
public:
	GenericSubDevice() :
	BaseGenericDevice()
	{
	}

	virtual ~GenericSubDevice() { }

	/// \brief accessor for sysdep object associated with sub-device
	inline XMI::SysDep *getSysdep() { return _sd; }

	/// \brief advance routine for unexpected (received) messages
	inline int advanceRecv(size_t context);

	// wrappers for GenericSubDevSendq...

	/// \brief Add a message to the (end of the) queue
	///
	/// Does not perform any actions on the message, the caller has
	/// already attempted early advance.
	///
	/// \param[in] msg	New message to be posted
	///
	inline void post(GenericMessage *msg) {
		_queue.pushTail(msg);
	}

	/// \brief peek at "next" message on queue
	///
	/// \return	Top message on queue
	///
	inline GenericMessage *getCurrent() {
		return (GenericMessage *)_queue.peekHead();
	}

	/// \brief pop "next" message off queue and return it
	///
	/// \return	Former top message on queue
	///
	inline GenericMessage *dequeue() {
		return (GenericMessage *)_queue.popHead();
	}

	/// \brief Remove a message from the middle of the queue
	///
	/// \param[in] msg	New message to be removed
	///
	inline void deleteElem(GenericMessage *msg) {
		_queue.deleteElem(msg);
	}

	/// \brief number of messages on the queue
	///
	/// \return	number of messages on the queue
	///
	inline int queueSize() {
		return _queue.size();
	}

	/// \brief Perform sub-device completion of a message
	///
	/// Basically, just dequeue the message from the sub-device
	/// and see if there was anything waiting to run afterwards.
	/// This routine is inlined, by the postNext() call is virtual.
	///
	/// \param[in] msg	The message that is completed
	///
	inline void __complete(XMI::Device::Generic::GenericMessage *msg) {
		/* assert msg == dequeue(); */
		dequeue();
		XMI::Device::Generic::GenericMessage *nxt = getCurrent();
		if (nxt) {
			// skips posting to sub-device... that was already done.
			// must setup threads and post threads+message to generic device.
			(void)nxt->postNext(true);
			// Note: message might have completed here, but handling
			// that is too complicated so we just let the generic
			// device do the completion when it finds it on the queue.
			// (recursion possibility is one complication)
		}
	}

	/// \brief Post message and threads to generic device for processing
	///
	/// Only posts threads which are not yet "Done". Simple pass-through
	/// to generic device post(), which does not care which object the
	/// method is actually call on - uses 'msg' to determine where to
	/// actually post/enqueue the objects.
	///
	/// \param[in] msg	Message object to enqueue
	/// \param[in] t	array of threads to enqueue
	/// \param[in] l	size of each thread in array
	/// \param[in] n	number of threads to enqueue
	///
	inline void postToGeneric(GenericMessage *msg, GenericAdvanceThread *t, size_t l, int n) {
		_generics->post(msg, t, l, n);
	}

	/// \brief accessor for specific generic device for context-id
	/// \return	Pointer to specific generic device object
	///
	inline XMI::Device::Generic::Device *getGeneric(size_t contextId) { return &_generics[contextId]; }

protected:
	/// \brief internal initialization routine for GenericSubDevice sub-class
	///
	/// \param[in] sd	SysDep for device/context/client... not used?
	/// \param[in] generics	Array of generic devices used for parallelism
	///
	inline void ___init(XMI::SysDep &sd, XMI::Device::Generic::Device *generics) {
		_sd = &sd;
		_generics = generics;
	}

	GenericSubDevSendq _queue;
	XMI::SysDep *_sd;
	XMI::Device::Generic::Device *_generics;
}; /* class GenericSubDevice */

/// \brief Simple Sub-Device where no threading is used.
///
/// A single-thread basic sub-device - standard boilerplate
/// One thread, no roles, no "receive" polling.
/// Thread object is 'empty', used only to queue work to Generic::Device.
///
/// Supports only one active message at a time.
///
template <class T_Thread>
class SimpleSubDevice : public GenericSubDevice {
	static const int NUM_THREADS = 1;
public:
	SimpleSubDevice() :
	GenericSubDevice()
	{
		// do this now so we don't have to every time we post
//		for (int x = 0; x < NUM_THREADS; ++x) {
//			//_threads[x].setPolled(true);
//		}
	}

	/// \brief Inform caller of where the threads array is
	///
	/// \param[out] t	Pointer to threads array
	/// \param[out] n	Pointer to number of threads in array
	///
	inline void getThreads(T_Thread **t, int *n) {
		*t = _threads;
		*n = NUM_THREADS;
	}

protected:
	friend class XMI::Device::Generic::Device;

	/// \brief Initialization for the subdevice
	///
	/// \param[in] sd		SysDep object (not used?)
	/// \param[in] devices		Array of Generic::Device objects for client
	/// \param[in] contextId	Id of current context (index into devices[])
	///
	inline void init(XMI::SysDep &sd, XMI::Device::Generic::Device *devices, size_t contextId) {
		___init(sd, devices);
	}

	/// \brief Actual advance routine for unexpected(received) messages
	///
	/// These devices do not have any unexpected messages, so routine is nil.
	///
	/// \param[in] context	Id of context which is being advanced
	///
	inline int advanceRecv(size_t context) { return 0; }

private:
	// For some reason, we can't declare friends like this.
	//friend class T_Model;
	// So, we need to make this public until we figure it out.
public: // temporary

	/// \brief Internal posting of message to sub-device
	///
	/// Since postNext() will try to advance the message, it may
	/// also complete it. This is tested and appropriate action taken.
	///
	/// \param[in] msg	Message to start and/or enqueue
	///
	template <class T_Message>
	inline void __post(XMI::Device::Generic::GenericMessage *msg) {
		// assert(isLocked(msg->getContext()));
		bool first = (getCurrent() == NULL);
		if (first) {
			if (static_cast<T_Message*>(msg)->postNext(false)) {
				msg->executeCallback(getGeneric(msg->getContextId())->getContext());
				return;
			}
		}
		XMI::Device::Generic::GenericSubDevice::post(msg);
	}

protected:
	T_Thread _threads[NUM_THREADS];
}; // class SimpleSubDevice

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
	_init(0),
	_dispatch_id(0)
	{
	}

	/// \brief returns a unique ID relative to this common sub-device
	///
	/// \return integer ID unique to this CommonQueueSubDevice
	///
	inline unsigned newDispID() {
		// caller must ensure number os valid for their hardware,
		// for example a network device might only have 8 bits in
		// the header for "dispatch ID".

		// might need to be atomic, in some situations?
		return ++_dispatch_id;
	}

	/// \brief test whether this common sub-device has been initialized previously
	///
	/// If we find that multiple devices (sharing this queue) are all in init()
	/// at the same time, this init flag will have to become some sort of atomic op.
	/// Right now, it should only be the case that a single thread is sequentially
	/// calling each device's init() routine so there is no problem.
	///
	/// \return	Number of init() calls previously made
	///
	int isInit() { return _init; }

	/// \brief init virtual function definition
	///
	/// All classes that inherit from this must implement init(), and that
	/// must callback to __init().
	///
	/// \param[in] sd	SysDep object
	/// \param[in] device	Generic::Device to be used.
	///
	virtual void init(XMI::SysDep &sd, XMI::Device::Generic::Device *devices, size_t contextId) = 0;

	/// \brief CommonQueueSubDevice portion of init function
	///
	/// All classes that inherit from this must implement init(), and that
	/// must callback to __init().
	///
	/// \param[in] sd	SysDep object
	/// \param[in] device	Generic::Device to be used.
	///
	inline void __init(XMI::SysDep &sd, XMI::Device::Generic::Device *devices, size_t contextId) {
		_doneThreads.init(&sd);
		_doneThreads.fetch_and_clear();
		_init = 1;
		___init(sd, devices);
	}

	/// \brief Reset for threads prior to being re-used.
	///
	inline void __resetThreads() {
		_doneThreads.fetch_and_clear();
	}

	/// \brief CommonQueueSubDevice portion of completion for a thread
	///
	/// \param[in] t	Thread being completed
	/// \return	Total number of threads completed for message
	///
	inline unsigned __completeThread(GenericAdvanceThread *t) {
		// fetchIncr() returns value *before* increment,
		// and we need to return total number of threads completed,
		// so we return "+1".
		return _doneThreads.fetch_and_inc() + 1;
	}

	/// \brief Internal posting of message to sub-device
	///
	/// Since postNext() will try to advance the message, it may
	/// also complete it. This is tested and appropriate action taken.
	///
	/// \param[in] msg	Message to start and/or enqueue
	///
	template <class T_Message>
	inline void __post(XMI::Device::Generic::GenericMessage *msg) {
		// assert(isLocked(msg->getContext()));
		bool first = (getCurrent() == NULL);
		if (first) {
			if (static_cast<T_Message*>(msg)->postNext(false)) {
				msg->executeCallback(getGeneric(msg->getContextId())->getContext());
				return;
			}
		}
		XMI::Device::Generic::GenericSubDevice::post(msg);
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
	inline T_CommonDevice *common() { return _common; }

	/// \brief Inform caller of where the threads array is
	///
	/// In this case, threads are not in the CommonQueueSubDevice but
	/// instead in each SharedQueueSubDevice.
	///
	/// \param[out] t	Pointer to threads array
	/// \param[out] n	Pointer to number of threads in array
	///
	inline void getThreads(T_Thread **t, int *n) {
		*t = _threads;
		*n = NUM_THREADS;
	}

	/// \brief Post message and threads to generic device for processing
	///
	/// Only posts threads which are not yet "Done". Simple pass-through
	/// to generic device post(), which does not care which object the
	/// method is actually call on - uses 'msg' to determine where to
	/// actually post/enqueue the objects.
	///
	/// \param[in] msg	Message object to enqueue
	/// \param[in] t	array of threads to enqueue
	/// \param[in] l	size of each thread in array
	/// \param[in] n	number of threads to enqueue
	///
	inline void postToGeneric(GenericMessage *msg, GenericAdvanceThread *t, size_t l, int n) {
		_common->postToGeneric(msg, t, l, n);
	}

protected:
	friend class XMI::Device::Generic::Device;

	inline void init(XMI::SysDep &sd, XMI::Device::Generic::Device *devices, size_t contextId) {
		// do this now so we don't have to every time we post
//		for (int x = 0; x < NUM_THREADS; ++x) {
//			//_threads[x].setPolled(true);
//		}
		if (!_common->isInit()) {
			_common->init(sd, devices, contextId);
		}
	}

	inline int advanceRecv(size_t context) { return 0; }

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
	///
	template <class T_Message>
	inline void __post(XMI::Device::Generic::GenericMessage *msg) {
		_common->__post<T_Message>(msg);
	}

	/// \brief SharedQueueSubDevice portion of completion for a thread
	///
	/// Must at least call CommonQueueSubDevice __completeThread().
	///
	/// \param[in] t	Thread being completed
	/// \return	Total number of threads completed for message
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

#endif /* __components_devices_generic_subdevice_h__ */
