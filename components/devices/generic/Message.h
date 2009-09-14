/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_devices_generic_message_h__
#define __components_devices_generic_message_h__

#include "xmi.h"
#include "components/devices/BaseDevice.h"
#include "queueing/Message.h"
#include "components/devices/generis/AdvanceThread.h"

////////////////////////////////////////////////////////////////////////
///  \file devices/prod/generic/Device.h
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
	GenericMessage(BaseDevice &Generic_QS, XMI_Callback_t cb) :
	MultiQueueMessage<2>(Generic_QS, cb),
	_threadsWanted(0)
	{
	}

	inline void setThreadsWanted(int n) { _threadsWanted = n; }
	inline int numThreadsWanted() { return _threadsWanted; }

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
	int _threadsWanted;
}; /* class GenericMessage */

template <class T>
class GenericCDIMessage : public GenericMessage {
public:
	GenericCDIMessage(BaseDevice &Packet_QS, XMI_Callback_t cb, struct iovec *iov, size_t iov_len, size_t msg_len) :
	GenericMessage(Packet_QS, cb),
	__user_iov(iov),
	__user_iovlen(iov_len),
	__send_bytes(msg_len)
	{
	}

	inline void executeCallback(XMI_Error_t *err = NULL) {
		GenericMessage::executeCallback(err);
	}
protected:
	struct iovec *__user_iov;
	size_t __user_iovlen;
	size_t __send_bytes;
}; /* class GenericCDIMessage */

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_message_h__ */
