/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __util_queue_message_h__
#define __util_queue_message_h__

#include "xmi.h"
#include "util/queue/Queue.h"
#include "components/devices/BaseDevice.h"
////////////////////////////////////////////////////////////////////////
///  \file util/queue/Message.h
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
namespace XMI {

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
	Message(Device::BaseDevice &QS, xmi_callback_t cb) :
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
	virtual XMI_Result start() = 0;

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
	void executeCallback(xmi_result err = XMI_SUCCESS) {if(_cb.function) _cb.function(_cb.clientdata, err);}

protected:
	int _status;
	Device::BaseDevice &_QS;
	xmi_callback_t _cb;
}; /* class Message */

//////////////////////////////////////////////////////////////////////
///  \brief Base Class for Messages
//////////////////////////////////////////////////////////////////////
template<int numElems>
class MultiQueueMessage : public MultiQueueElem<numElems> {
public:
	//////////////////////////////////////////////////////////////////////
	///  \brief Constructor
	//////////////////////////////////////////////////////////////////////
	MultiQueueMessage(Device::BaseDevice &QS, xmi_callback_t cb) :
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
	void executeCallback(xmi_result err = XMI_SUCCESS) {
		if(_cb.function) _cb.function(_cb.clientdata, err);
	}

	inline Device::BaseDevice &getQS() { return _QS; }

protected:
	MessageStatus _status;
	Device::BaseDevice &_QS;
	xmi_callback_t _cb;
}; /* class MultiQueueMessage */

}; /* namespace XMI */

#endif // __util_queue_message_h__
