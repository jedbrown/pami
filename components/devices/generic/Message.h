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
	Message(Device::Generic::BaseGenericDevice &QS, xmi_callback_t cb,
					xmi_client_t client, size_t context) :
	QueueElem(),
	_status(0),
	_QS(QS),
	_client(client),
	_context(context),
	_cb(cb)
	{
	}

	xmi_client_t getClient() { return _client; }
	size_t getContextId() { return _context; }

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
	void executeCallback(xmi_context_t ctx, xmi_result_t err = XMI_SUCCESS) {
		if(_cb.function) _cb.function(ctx, _cb.clientdata, err);
	}

protected:
	int _status;
	Device::Generic::BaseGenericDevice &_QS;
	xmi_client_t _client;
	size_t _context;
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
	MultiQueueMessage(Device::Generic::BaseGenericDevice &QS, xmi_callback_t cb,
						xmi_client_t client, size_t context) :
	MultiQueueElem<numElems>(),
	_status(Uninitialized),
	_QS(QS),
	_client(client),
	_context(context),
	_cb(cb)
	{
	}

	xmi_client_t getClient() { return _client; }
	size_t getContextId() { return _context; }

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
	void executeCallback(xmi_context_t ctx, xmi_result_t err = XMI_SUCCESS) {
		if(_cb.function) _cb.function(ctx, _cb.clientdata, err);
	}

	inline Device::Generic::BaseGenericDevice &getQS() { return _QS; }

	/// \brief virtual method used to activate a message that was enqueued earlier
	///
	/// \param[in] devPosted	was msg was previously posted to sub-device?
	/// \return	bool whether message is complete
	///
	virtual bool postNext(bool devPosted) = 0;

protected:
	MessageStatus _status;
	Device::Generic::BaseGenericDevice &_QS;
	xmi_client_t _client;
	size_t _context;
	xmi_callback_t _cb;
}; /* class MultiQueueMessage */

// still in namespace XMI::Device...
namespace Generic {

// This is a bit klunky, but until templates allow methods as parameters...
#define ADVANCE_ROUTINE(method,message,thread)			\
static xmi_result_t method(xmi_context_t context, void *t) {	\
	thread *thr = (thread *)t;				\
	message *msg = (message *)thr->getMsg();		\
	return msg->__##method(thr);				\
}

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
	MultiQueueMessage<2>(Generic_QS, cb, client, context)
	{
	}

protected:
}; /* class GenericMessage */

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_message_h__ */
