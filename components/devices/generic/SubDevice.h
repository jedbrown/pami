/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_generic_subdevice_h__
#define __components_devices_generic_subdevice_h__

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

class GenericSubDevice : public BaseGenericDevice {
private:
public:
	GenericSubDevice() :
	BaseGenericDevice(),
	_hasBlockingAdvance(false),
	_nRoles(1),
	_repl(-1)
	{
	}

	GenericSubDevice(int nRoles, int repl) :
	BaseGenericDevice(),
	_hasBlockingAdvance(false),
	_nRoles(nRoles),
	_repl(repl),
	_sd(NULL)
	{
	}

	virtual ~GenericSubDevice() { }

	inline XMI_SYSDEP_CLASS *getSysdep() { return _sd; }

	inline int advanceRecv(int channel = -1);

	/// \brief default blocking advance and prototype
	///
	/// may be overridden by parent class, along with calling setBlockingAdvance(true).
	///
	/// \param[in] thr	Thread to be advanced
	///
	virtual void advanceBlocking(GenericAdvanceThread *thr) { XMI_abort(); }

	/// \brief whether messages support blocking advance
	///
	/// \return	Boolean indicating support for blocking advance calls
	///
	inline bool hasBlockingAdvance() { return _hasBlockingAdvance; }

	/// \brief get information about message roles
	///
	/// Number of roles indicates how much distribution of work between
	/// processes may be done. Replication indicates which role (if any)
	/// is to be replicated for addition processes beyond nRoles.
	///
	/// \param[out] nRoles	Number of roles available
	/// \param[out] repl	Which role to replicate
	///
	inline void getRoles(int *nRoles, int *repl) {
		*nRoles = _nRoles;
		*repl = _repl;
	}

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
	inline void remove(GenericMessage *msg) {
		_queue.remove(msg);
	}

	/// \brief number of messages on the queue
	///
	/// \return	number of messages on the queue
	///
	inline int queueSize() {
		return _queue.size();
	}

protected:
	inline void ___init(XMI_SYSDEP_CLASS &sd) {
		_sd = &sd;
	}

	/// \brief tell whether messages support blocking advance
	///
	/// \param[in] f	Boolean indicating support for blocking advance calls
	///
	inline void setBlockingAdvance(bool f) { _hasBlockingAdvance = f; }


	/// \brief set information about message roles
	///
	/// Number of roles indicates how much distribution of work between
	/// processes may be done. Replication indicates which role (if any)
	/// is to be replicated for addition processes beyond nRoles.
	///
	/// \param[out] nRoles	Number of roles available
	/// \param[out] repl	Which role to replicate
	///
	inline void setRoles(int nRoles, int repl) {
		_nRoles = nRoles;
		_repl = repl;
	}

	GenericSubDevSendq _queue;
	bool _hasBlockingAdvance;
	int _nRoles;
	int _repl;
	XMI_SYSDEP_CLASS *_sd;
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

private:
	template <class T_Message>
	inline void __start_msg(XMI::Device::Generic::GenericMessage *msg) {
		// While threads aren't used, a thread object is needed
		// for the advance queue
		int t = static_cast<T_Message*>(msg)->__setThreads(&_threads[0], NUM_THREADS);
		msg->setStatus(XMI::Device::Initialized);
		XMI_assert_debug(t == NUM_THREADS); t = t;
	}

	inline void __post_msg(XMI::Device::Generic::GenericMessage *msg) {
		_generic->post(msg, &_threads[0], sizeof(_threads[0]), NUM_THREADS);
	}

protected:
	friend class XMI::Device::Generic::Device;

	inline void init(XMI_SYSDEP_CLASS &sd, XMI::Device::Generic::Device *device) {
		_generic = device;
		_generic->registerThreads(&_threads[0], sizeof(_threads[0]), NUM_THREADS);
		___init(sd);
	}

	inline int advanceRecv(int channel = -1) { return 0; }

private:
	// For some reason, we can't declare friends like this.
	//friend class T_Model;
	// So, we need to make this public until we figure it out.
public: // temporary

	template <class T_Message>
	inline void __post(XMI::Device::Generic::GenericMessage *msg) {
		bool first = (getCurrent() == NULL);
		if (first) {
			__start_msg<T_Message>(msg);
			static_cast<T_Message*>(msg)->__advanceThread(&_threads[0]);
			if (msg->getStatus() == XMI::Device::Done) {
				msg->executeCallback();
				return;
			}
			__post_msg(msg);
		}
		XMI::Device::Generic::GenericSubDevice::post(msg);
	}

	template <class T_Message>
	inline void __complete(XMI::Device::Generic::GenericMessage *msg) {
		/* assert msg == dequeue(); */
		dequeue();
		XMI::Device::Generic::GenericMessage *nxt = getCurrent();
		if (nxt) {
			__start_msg<T_Message>(nxt);
			// could try to advance here?
			__post_msg(nxt);
		}
	}

protected:
	XMI::Device::Generic::Device *_generic;
	T_Thread _threads[NUM_THREADS];
}; // class SimpleSubDevice

/// \brief Threaded Sub-Device
///
/// A multi-thread basic sub-device - standard boilerplate
/// N threads, no roles, no "receive" polling.
/// Allows only one message to be active at a time.
///
/// Supports only one active message at a time.
///
template <class T_Model, class T_Message, class T_Thread, int N_Threads>
class ThreadedSubDevice : public GenericSubDevice {
	static const int NUM_THREADS = N_Threads;
	#define ATOMIC_BUF_SIZE	16
public:
	ThreadedSubDevice() :
	GenericSubDevice()
	{
		// do this now so we don't have to every time we post
//		for (int x = 0; x < NUM_THREADS; ++x) {
//			//_threads[x].setPolled(true);
//		}
	}

private:
	inline void __start_msg(T_Message *msg) {
		_doneThreads.fetch_and_clear();
		int t = msg->__setThreads(&_threads[0], NUM_THREADS);
		msg->setStatus(XMI::Device::Initialized);
		_nActiveThreads = t;
	}

	inline void __post_msg(T_Message *msg) {
		_generic->post((XMI::Device::Generic::GenericMessage *)msg, &_threads[0], sizeof(_threads[0]), _nActiveThreads);
	}

protected:
	inline void init(XMI_SYSDEP_CLASS &sd, XMI::Device::Generic::Device *device) {
		_generic = device;
		_doneThreads.init(&sd);
		_doneThreads.fetch_and_clear();
		_generic->registerThreads(&_threads[0], sizeof(_threads[0]), NUM_THREADS);
		___init(sd);
	}

	inline int advanceRecv(int channel = -1) { return 0; }

private:
	// For some reason, we can't declare friends like this.
	//friend class T_Message;
	//friend class T_Model;
	// So, the inheriting class needs to this!

	inline void __post(T_Message *msg) {
		bool first = (getCurrent() == NULL);
		if (first) {
			__start_msg(msg);
			// Don't check each thread for "Done", we only care about msg
			for (int x = 0; x < _nActiveThreads; ++x) {
				msg->__advanceThread(&_threads[x]);
			}
			if (msg->getStatus() == XMI::Device::Done) {
				msg->executeCallback();
				return;
			}
			__post_msg(msg);
		}
		XMI::Device::Generic::GenericSubDevice::post((XMI::Device::Generic::GenericMessage *)msg);
	}

	inline unsigned __completeThread(T_Thread *thr) {
		// fetchIncr() returns value *before* increment,
		// and we need to return total number of threads completed,
		// so we return "+1".
		return _doneThreads.fetch_and_inc() + 1;
	}

	inline void __complete(T_Message *msg) {
		/* assert msg == dequeue(); */
		_nActiveThreads = 0;
		dequeue();
		T_Message *nxt = (T_Message *)getCurrent();
		if (nxt) {
			__start_msg(nxt);
			// could try to advance here?
			__post_msg(nxt);
		}
	}

private:
	char _atomic_buf[ATOMIC_BUF_SIZE] __attribute__((__aligned__(16)));
	T_Thread _threads[NUM_THREADS];
	int _nActiveThreads;
	XMI::Device::Generic::Device *_generic;
	GenericDeviceCounter _doneThreads;
}; // class ThreadedSubDevice

/// \brief Multi Threaded Sub-Device
///
/// A multi-thread basic sub-device - standard boilerplate
/// N threads, no roles, no "receive" polling.
/// Allows more than one message to be active at a time.
///
/// Supports only one active message at a time.
///
template <class T_Thread, int N_Threads>
class MultiThrdSubDevice : public GenericSubDevice {
	static const int NUM_THREADS = N_Threads;
	#define ATOMIC_BUF_SIZE	16
public:
	MultiThrdSubDevice() :
	GenericSubDevice()
	{
		// do this now so we don't have to every time we post
//		for (int x = 0; x < NUM_THREADS; ++x) {
//			//_threads[x].setPolled(true);
//		}
	}

private:
	template <class T_Message>
	inline void __start_msg(XMI::Device::Generic::GenericMessage *msg) {
		_doneThreads.fetch_and_clear();
		int t = static_cast<T_Message*>(msg)->__setThreads(NULL, NUM_THREADS);
		msg->setStatus(XMI::Device::Initialized);
		_nActiveThreads = t;
	}

	inline void __post_msg(XMI::Device::Generic::GenericMessage *msg) {
		T_Thread *thr = msg->__getThreads();
		int x, y;
		y = _nextChan;
		for (x = 0; x < _nActiveThreads; ++x) {
			thr[x].setChannel(y++);
			if (y >= NUM_THREADS) y = 0;
		}
		_nextChan = y;
		_generic->post((XMI::Device::Generic::GenericMessage *)msg,
				thr, sizeof(T_Thread), _nActiveThreads);
	}

protected:
	friend class XMI::Device::Generic::Device;

	inline void init(XMI_SYSDEP_CLASS &sd, XMI::Device::Generic::Device *device) {
		_generic = device;
		_doneThreads.init(&sd);
		_doneThreads.fetch_and_clear();
		//_generic->registerThreads(&_threads[0], sizeof(_threads[0]), NUM_THREADS);
		___init(sd);
	}

	inline int advanceRecv(int channel = -1) { return 0; }

//private:
public:
	// For some reason, we can't declare friends like this.
	//friend class T_Message;
	//friend class T_Model;
	// So, must allow this to be public!

	template <class T_Message>
	inline void __post(XMI::Device::Generic::GenericMessage *msg) {
		// assume early advance already tried... just queue it up
		__start_msg<T_Message>(msg);
		__post_msg(msg);
		// Don't queue locally... all are active
		//XMI::Device::Generic::GenericSubDevice::post(msg);
	}

	inline unsigned __completeThread(T_Thread *thr) {
		// fetchIncr() returns value *before* increment,
		// and we need to return total number of threads completed,
		// so we return "+1".
		return _doneThreads.fetch_and_inc() + 1;
	}

	template <class T_Message>
	inline void __complete(XMI::Device::Generic::GenericMessage *msg) {
		/* assert msg == dequeue(); */
		//_nActiveThreads = 0;
		//dequeue();
//		XMI::Device::Generic::GenericMessage *nxt;
//		while (nxt = getCurrent()) {
//			__start_msg<T_Message>(nxt);
//			// could try to advance here?
//			__post_msg(nxt);
//		}
	}

private:
	int _nActiveThreads;
	int _nextChan;
	XMI::Device::Generic::Device *_generic;
	GenericDeviceCounter _doneThreads;
}; // class MultiThrdSubDevice

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

	inline unsigned newDispID() {
		// caller must ensure number os valid for their hardware,
		// for example a network device might only have 8 bits in
		// the header for "dispatch ID".

		// might need to be atomic, in some situations?
		return ++_dispatch_id;
	}

	// If we find that multiple devices (sharing this queue) are all in init()
	// at the same time, this init flag will have to become some sort of atomic op.
	// Right now, it should only be the case that a single thread is sequentially
	// calling each device's init() routine so there is no problem.
	int isInit() { return _init; }

	/// \brief init virtual function definition
	///
	/// All classes that inherit from this must implement init(), and that
	/// must callback to __init().
	///
	/// \param[in] sd	SysDep object
	/// \param[in] device	Generic::Device to be used.
	///
	virtual void init(XMI_SYSDEP_CLASS &sd, XMI::Device::Generic::Device *device) = 0;

	/// \brief CommonQueueSubDevice portion of init function
	///
	/// All classes that inherit from this must implement init(), and that
	/// must callback to __init().
	///
	/// \param[in] sd	SysDep object
	/// \param[in] device	Generic::Device to be used.
	///
	inline void __init(XMI_SYSDEP_CLASS &sd, XMI::Device::Generic::Device *device) {
		_generic = device;
		_doneThreads.init(&sd);
		_doneThreads.fetch_and_clear();
		_init = 1;
		___init(sd);
	}

	inline void post_msg(XMI::Device::Generic::GenericMessage *msg, GenericAdvanceThread *t, size_t l, int n) {
		_generic->post(msg, t, l, n);
	}

	inline void __resetThreads() {
		_doneThreads.fetch_and_clear();
	}

	inline unsigned __completeThread(GenericAdvanceThread *t) {
		// fetchIncr() returns value *before* increment,
		// and we need to return total number of threads completed,
		// so we return "+1".
		return _doneThreads.fetch_and_inc() + 1;
	}

	///
	/// \brief Complete a message on the device and return next message
	///
	/// assumes only one message active at a time. otherwise, instead of dequeue()
	/// use remove(msg) and instead of getCurrent() must search for
	/// the next "unactivated" message to be started.
	///
	/// \param[in] msg	Message being completed.
	/// \return	Next message to start, or NULL if none.
	///
	inline XMI::Device::Generic::GenericMessage *__complete(XMI::Device::Generic::GenericMessage *msg) {
		/* assert msg == dequeue(); */
		dequeue();
		return getCurrent();
	}

private:
	int _init;
	XMI::Device::Generic::Device *_generic;
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

	inline unsigned newDispID() {
		return _common->newDispID();
	}

	inline T_CommonDevice *common() { return _common; }
private:
	template <class T_Message>
	inline void __start_msg(XMI::Device::Generic::GenericMessage *msg) {
		int n;
		_common->__resetThreads();
		msg->setStatus(XMI::Device::Initialized);
		n = static_cast<T_Message*>(msg)->__setThreads(&_threads[0], NUM_THREADS);
		_nActiveThreads = n;
	}

	inline void __post_msg(XMI::Device::Generic::GenericMessage *msg) {
		_common->post_msg(msg, &_threads[0], sizeof(_threads[0]), _nActiveThreads);
	}

protected:
	friend class XMI::Device::Generic::Device;

	inline void init(XMI_SYSDEP_CLASS &sd, XMI::Device::Generic::Device *device) {
		// do this now so we don't have to every time we post
//		for (int x = 0; x < NUM_THREADS; ++x) {
//			//_threads[x].setPolled(true);
//		}
		device->registerThreads(&_threads[0], sizeof(_threads[0]), NUM_THREADS);
		if (!_common->isInit()) {
			_common->init(sd, device);
		}
	}

	inline int advanceRecv(int channel = -1) { return 0; }

private:
	// For some reason, we can't declare friends like this.
	//friend class T_Message;
	//friend class T_Model;
	// So, need to make it public for now...
public:	// temporary?

	template <class T_Message>
	inline void __post(XMI::Device::Generic::GenericMessage *msg) {
		bool first = (_common->getCurrent() == NULL);
		if (first) {
			__start_msg<T_Message>(msg);
			// Don't check each thread for "Done", we only care about msg
			for (int x = 0; x < _nActiveThreads; ++x) {
				static_cast<T_Message*>(msg)->__advanceThread(&_threads[x]);
			}
			if (msg->getStatus() == XMI::Device::Done) {
				msg->executeCallback();
				return;
			}
			__post_msg(msg);
		}
		_common->post(msg);
	}

	inline unsigned __completeThread(T_Thread *thr) {
		return _common->__completeThread(thr);
	}

	template <class T_Message>
	inline void __complete(T_Message *msg) {
		_nActiveThreads = 0;
		T_Message *nxt = (T_Message *)_common->__complete(msg);
		if (nxt) {
			__start_msg<T_Message>(nxt);
			// could try to advance here?
			__post_msg(nxt);
		}
	}

private:
	T_CommonDevice *_common;
	T_Thread _threads[NUM_THREADS];
	int _nActiveThreads;
}; // class SharedQueueSubDevice

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace Device */

#endif /* __components_devices_generic_subdevice_h__ */
