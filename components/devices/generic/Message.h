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
///
////////////////////////////////////////////////////////////////////////

enum MessageStatus {
	Uninitialized = 0,	///< status for uninitialized message
	Initialized,		///< status for initialized message
	Active,			///< status for active message
	Done			///< status for completed message
};

/// \brief Base Class for Messages
///
/// Messages must be able to exist on two queues at the same time.
/// This requires two actual QueueElem structs, handled by MultiQueueElem.
/// In fact, this class is templatized by number of queue elements and thus
/// is general for any number of queues.
///
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

	/// \brief get client associated with message
	/// \return	client for message posting/completion
	/// \ingroup gendev_subdev_api
	xmi_client_t getClient() { return _client; }

	/// \brief get context ID associated with message
	/// \return	Context ID for message posting/completion
	/// \ingroup gendev_subdev_api
	size_t getContextId() { return _context; }

	///  \brief Query function to determine message state
	///  \return	message status
	///
	inline MessageStatus getStatus() {return _status;}

	/// \brief Set message status
	///
	/// \param[in] status	Message status to set
	///
	inline void setStatus(MessageStatus status) {_status = status;}

	/// \brief     Returns the done status of the message
	///
	/// \return	true is message is Done
	/// \ingroup gendev_subdev_api
	///
	inline bool isDone() {return (getStatus() == Done);}

	///  \brief Sets the message completion callback
	///
	/// \param[in] cb	Callback to use for message completion
	///
	void setCallback(xmi_callback_t cb) {_cb = cb;}

	///  \brief Executes the message completion callback
	///
	/// \param[in] ctx	The context object on which completion is called
	/// \param[in] err	Optional error status (default is success)
	/// \ingroup gendev_subdev_api
	///
	void executeCallback(xmi_context_t ctx, xmi_result_t err = XMI_SUCCESS) {
		if(_cb.function) _cb.function(ctx, _cb.clientdata, err);
	}

	/// \brief accessor for sub-device linked to message
	///
	/// Returns reference to device object which contains "send queues".
	/// This may not be the actual sub-device paired with the message.
	///
	/// \return	Reference to sub-device
	/// \ingroup gendev_subdev_api
	///
	inline Device::Generic::BaseGenericDevice &getQS() { return _QS; }

	/// \brief virtual method used to activate a message that was enqueued earlier
	///
	/// Performs the setup of a message (and threads) in preparation
	/// for becoming active (on the generic device queues). Used for
	/// both first-time messages (not queued anywhere) and for when
	/// a message reaches the top of the sub-device queue (for devices
	/// that only perform one message at a time). In the latter case,
	/// 'devPosted' will be true.
	///
	/// If this returns true, then the message is complete and should be
	/// destroyed/freed (after invoking the callback). This can only return
	/// true if 'devPosted' is false.
	///
	/// \param[in] devPosted	was msg was previously posted to sub-device?
	/// \return	bool whether message is complete
	/// \ingroup gendev_subdev_api
	///
	bool __postNext(bool devPosted);

	/// \brief virtual wrapper for __postNext() method
	///
	/// Used during message complete to post the next message.
	///
	/// \param[in] devPosted	was msg was previously posted to sub-device?
	/// \return	bool whether message is complete
	/// \ingroup gendev_subdev_api
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
/// \brief Macro for declaring a routine as an advance routine for a thread
///
/// Creates a static function named 'method' that may be used for a
/// thread's advance routine (thr->setAdv('method')). Assumes there is
/// also an inlined function named __'method' which contains the actual
/// advance code for the thread(s).
///
/// \param[in] method	Basename of method used to advance thread(s)
/// \param[in] message	Class of message
/// \param[in] thread	Class of thread
///
#define ADVANCE_ROUTINE(method,message,thread)			\
static xmi_result_t method(xmi_context_t context, void *t) {	\
	thread *thr = (thread *)t;				\
	message *msg = (message *)thr->getMsg();		\
	return msg->__##method(thr);				\
}

/// \brief Message class used by the Generic Device
///
/// Generic Device allows messages to exist on two queues at the same time.
/// One queue (1) is used by the generic device, the other (0) may be used as
/// needed by the sub-device. Usage of queue 1 is in code in GenericDevice.h/Device.h
///
class GenericMessage : public MultiQueueMessage<2> {

public:
	/// \brief  Generic Message constructor
	///
	/// Note: Generic_QS must be (inherit from) GenericSubDevice.
	/// This is because Generic::Device will use the QS in order
	/// to invoke the __complete() method when the message is Done,
	/// and that is used to start any subsequent message(s) that
	/// were waiting. This method is inlined for efficiency (especially
	/// when the sub-device has no sendq or the queue is empty).
	/// This means that it cannot be overridden and the object ('this')
	/// must actually be the class that Generic::Device expects.
	///
	/// \param[in] Generic_QS	The subdevice containing the sendqueue(s)
	/// \param[in] cb		Completion callback
	/// \param[in] client		The client handle
	/// \param[in] context		The (posting) context ID
	///
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
