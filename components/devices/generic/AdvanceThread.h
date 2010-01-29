/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_devices_generic_AdvanceThread_h__
#define __components_devices_generic_AdvanceThread_h__

#include "GenericDevicePlatform.h"
#include "components/devices/BaseDevice.h"
#include "components/devices/generic/Message.h"
#include "SysDep.h"

////////////////////////////////////////////////////////////////////////
///  \file components/devices/generic/AdvanceThread.h
///  \brief Generic Device Threads implementation
///
////////////////////////////////////////////////////////////////////////
namespace XMI {
namespace Device {

	enum ThreadStatus {
		New = 0,	///< Thread has only been constructed
		Idle,		///< Thread has no work (do not call)
		Ready,		///< Thread has work, make call to func
		Complete	///< Thread  should be dequeued
	};

namespace Generic {

class GenericThread : public GenericDeviceWorkQueueElem {
public:
	GenericThread() :
	GenericDeviceWorkQueueElem(),
	_func(NULL),
	_cookie(NULL),
	_cb_done((xmi_callback_t){NULL,NULL}),
	_status(New)
	{
	}

	GenericThread(xmi_work_function func, void *cookie) :
	GenericDeviceWorkQueueElem(),
	_func(func),
	_cookie(cookie),
	_cb_done((xmi_callback_t){NULL,NULL}),
	_status(New)
	{
	}

	GenericThread(xmi_work_function func, void *cookie, xmi_callback_t cb_done) :
	GenericDeviceWorkQueueElem(),
	_func(func),
	_cookie(cookie),
	_cb_done(cb_done),
	_status(New)
	{
	}

	inline xmi_result_t executeThread(xmi_context_t context) {
		return _func(context, _cookie);
	}

	inline void executeCallback(xmi_context_t context, xmi_result_t rc = XMI_SUCCESS) {
		if (_cb_done.function) {
			_cb_done.function(context, _cb_done.clientdata, rc);
		}
	}

	inline ThreadStatus getStatus() { return _status; }
	inline void setStatus(ThreadStatus stat) { _status = stat; }
protected:
	xmi_work_function _func;
	void *_cookie;
	xmi_callback_t _cb_done;
	ThreadStatus _status;
}; // class GenericThread

//////////////////////////////////////////////////////////////////////
///  \brief A Generic Device implmentation of a thread.
///  This class implements a base thread object.
///  Normally, this class is not used directly by a sub-device
/// as the device's thread, but all actual working threads inherit
/// from this class.
//////////////////////////////////////////////////////////////////////
class GenericAdvanceThread : public GenericThread {

public:
	GenericAdvanceThread() :
	GenericThread(),
	_msg(NULL),
	_dev_wake(NULL)
	{
	}

	inline void setMsg(GenericMessage *msg) { _msg = msg; }
	inline void setAdv(xmi_work_function advThr) { _func = advThr; _cookie = this; }
	inline GenericMessage *getMsg() { return _msg; }

	inline void setWakeVec(void *v) { _dev_wake = v; }
	inline void *getWakeVec() { return _dev_wake; }

protected:
	GenericMessage *_msg;
	void *_dev_wake;
}; /* class GenericAdvanceThread */

//////////////////////////////////////////////////////////////////////
///  \brief A Generic Device implmentation of a thread.
///  This class implements a useable, simple, thread object.
///  Other, more complex, thread classes are implemented in specific
///  sub-devices.  For example, see bgp/collective_network/CollectiveNetworkLib.h
///  and class BaseGenericCNThread.
//////////////////////////////////////////////////////////////////////
class SimpleAdvanceThread : public GenericAdvanceThread {
public:
	SimpleAdvanceThread() :
	GenericAdvanceThread(),
	_bytesLeft(0)
	{
	}
public:
	size_t _bytesLeft;
}; /* class SimpleAdvanceThread */

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_thread_h__ */
