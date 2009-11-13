/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_devices_generic_Message_h__
#define __components_devices_generic_Message_h__

#include "sys/xmi.h"
#include "components/devices/generic/BaseGenericDevice.h"
#include "util/queue/Queue.h"
#include "components/devices/generic/AdvanceThread.h"

////////////////////////////////////////////////////////////////////////
///  \file components/devices/generic/Message.h
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

////////////////////////////////////////////////////////////////////////
///  \brief Message Class for insertion into queues
///
///  These classes implement a message class for insertion into queues
///
///  Definitions:
///  - Message:    A communication object that can be inserted into a q
///  - advance():  A method that makes the queue make progress
///  - start():    A method that starts progress
///  - reset():    A method that resets/reinitializes a message
///
///  Namespace:  DCMF, the messaging namespace.
///  Namespace:  Queueing, the queueing namespace
///
////////////////////////////////////////////////////////////////////////
enum MessageStatus {
	Uninitialized = 0,
	Initialized,
	Active,
	Done
};

//////////////////////////////////////////////////////////////////////
///  \brief Base Class for Messages
//////////////////////////////////////////////////////////////////////
class Message : public QueueElem {
public:
	//////////////////////////////////////////////////////////////////////
	///  \brief Constructor
	//////////////////////////////////////////////////////////////////////
	Message(Device::Generic::BaseGenericDevice &QS, xmi_callback_t cb) :
	QueueElem(),
	_status(0),
	_QS(QS),
	_cb(cb)
	{
	}

	//////////////////////////////////////////////////////////////////////
	///  \brief Reset a message
	///  \returns a return code to indicate reset status
	//////////////////////////////////////////////////////////////////////
	virtual int reset() = 0;

	//////////////////////////////////////////////////////////////////////
	///  \brief posts a message and begins the send
	//////////////////////////////////////////////////////////////////////
	virtual xmi_result_t start() = 0;

	//////////////////////////////////////////////////////////////////////
	///  \brief advance function
	///  \returns a return code to indicate progress was made
	//////////////////////////////////////////////////////////////////////
	virtual int advance() = 0;

	//////////////////////////////////////////////////////////////////////
	///  \brief Query function to determine message state
	///  \returns an integer indicating status
	//////////////////////////////////////////////////////////////////////
	int getStatus() {return _status;}

	//////////////////////////////////////////////////////////////////////
	///  \brief Sets the callback
	//////////////////////////////////////////////////////////////////////
	void setCallback(xmi_callback_t cb) {_cb = cb;}

	//////////////////////////////////////////////////////////////////////
	///  \brief Executes the callback
	///  \returns a return code to indicate reset status
	//////////////////////////////////////////////////////////////////////
	void executeCallback(xmi_result_t err = XMI_SUCCESS) {if(_cb.function) _cb.function(NULL, _cb.clientdata, err);}

protected:
	int _status;
	Device::Generic::BaseGenericDevice &_QS;
	xmi_callback_t _cb;
}; /* class Message */

//////////////////////////////////////////////////////////////////////
///  \brief Base Class for Messages
//////////////////////////////////////////////////////////////////////
template <int numElems>
class MultiQueueMessage : public MultiQueueElem<numElems> {
public:
	//////////////////////////////////////////////////////////////////////
	///  \brief Constructor
	//////////////////////////////////////////////////////////////////////
	MultiQueueMessage(Device::Generic::BaseGenericDevice &QS, xmi_callback_t cb) :
	MultiQueueElem<numElems>(),
	_status(Uninitialized),
	_QS(QS),
	_cb(cb)
	{
	}

	//////////////////////////////////////////////////////////////////////
	///  \brief Query function to determine message state
	///  \returns an integer indicating status
	//////////////////////////////////////////////////////////////////////
	inline MessageStatus getStatus() {return _status;}
	inline void setStatus(MessageStatus status) {_status = status;}

	//////////////////////////////////////////////////////////////////
	/// \brief     Returns the done status of the message
	//////////////////////////////////////////////////////////////////
	inline bool isDone() {return (getStatus() == Done);}

	//////////////////////////////////////////////////////////////////////
	///  \brief Sets the callback
	//////////////////////////////////////////////////////////////////////
	void setCallback(xmi_callback_t cb) {_cb = cb;}

	//////////////////////////////////////////////////////////////////////
	///  \brief Executes the callback
	///  \returns a return code to indicate reset status
	//////////////////////////////////////////////////////////////////////
	void executeCallback(xmi_result_t err = XMI_SUCCESS) {
		if(_cb.function) _cb.function(NULL, _cb.clientdata, err);
	}

	inline Device::Generic::BaseGenericDevice &getQS() { return _QS; }

protected:
	MessageStatus _status;
	Device::Generic::BaseGenericDevice &_QS;
	xmi_callback_t _cb;
}; /* class MultiQueueMessage */

// still in namespace XMI::Device...
namespace Generic {

//////////////////////////////////////////////////////////////////////
///  \brief interprocess communication message class
///  This message is posted to a Generic device
//////////////////////////////////////////////////////////////////////
class GenericMessage : public MultiQueueMessage<2> {

public:
	//////////////////////////////////////////////////////////////////
	/// \brief  Generic Message constructor
	/// \param cb: A "done" callback structure to be executed
	//////////////////////////////////////////////////////////////////
	GenericMessage(BaseGenericDevice &Generic_QS, xmi_callback_t cb,
			xmi_client_t client, size_t context) :
	MultiQueueMessage<2>(Generic_QS, cb),
	_client(client),
	_context(context)
	{
	}

	xmi_client_t getClient() { return _client; }
	size_t getContext() { return _context; }

	/// \brief Message is Done, perform all completion tasks
	///
	/// Typically the message will invoke the device complete() method
	/// (to be dequeued from the device) and then execute the users callback.
	///
	virtual void complete() = 0;

	/// \brief Advance a thread of a message
	///
	/// \param[in] thr	Thread to advance
	/// \return	Resulting status of thread (Done, etc)
	///
	virtual MessageStatus advanceThread(GenericAdvanceThread *thr) = 0;

protected:
	xmi_client_t _client;
	size_t _context;
}; /* class GenericMessage */

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_message_h__ */
