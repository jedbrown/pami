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
		OneShot,	///< Thread is run only once then dequeued
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

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_thread_h__ */
