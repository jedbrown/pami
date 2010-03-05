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

	/// \brief Thread status
	/// @{
	enum ThreadStatus {
		New = 0,	///< Thread has only been constructed (do not call)
		Idle,		///< Thread has no work (do not call)
		Ready,		///< Thread has work, make call to func
		OneShot,	///< Thread is run only once then dequeued
		Complete	///< Thread should be dequeued (do not call)
	};
	/// }@

namespace Generic {

/// \brief Base class for all thread objects posted to generic device
///
/// All objects posted to the generic device via postThread() must
/// inherit from this class.
///
/// This is the basic thread object. It describes a unit of work to be performed
/// during advance. Ctors allow thread to be created empty/idle or
/// with a function and ready to run.
///
class GenericThread : public GenericDeviceWorkQueueElem {
public:
	/// \brief Default constructor for thread
	/// \ingroup use_gendev
	///
	/// Constructs a thread object that is not ready to run.
	///
	GenericThread() :
	GenericDeviceWorkQueueElem(),
	_func(NULL),
	_cookie(NULL),
	_status(New)
	{
	}

	/// \brief Work constructor for thread
	///
	/// Constructs a thread object that is ready to run, 
	/// complete with work function.
	///
	/// \param[in] func	Work function to call
	/// \param[in] cookie	Opaque data for work function
	///
	GenericThread(xmi_work_function func, void *cookie) :
	GenericDeviceWorkQueueElem(),
	_func(func),
	_cookie(cookie),
	_status(Ready)
	{
	}

	/// \brief Execute work function for thread
	///
	/// Calls the work function with the context object and opaque data.
	///
	/// \param[in] context	Context in which thread is being executed
	///
	inline xmi_result_t executeThread(xmi_context_t context) {
		return _func(context, _cookie);
	}

	/// \brief Accessor for thread status
	///
	/// \return	Thread status
	///
	inline ThreadStatus getStatus() { return _status; }

	/// \brief Set thread status
	///
	/// \param[in] stat	New status for thread
	///
	inline void setStatus(ThreadStatus stat) { _status = stat; }

	/// \brief Set thread work function and opaque data
	///
	/// Status must still be set to Ready or OneShot in order for
	/// function to be called.
	///
	/// \param[in] func	Function to call from advance
	/// \param[in] cookie	Opaque data to pass to function
	///
	inline void setFunc(xmi_work_function func, void *cookie) {
		_func = func;
		_cookie = cookie;
	}
protected:
	xmi_work_function _func;
	void *_cookie;
	ThreadStatus _status;
}; // class GenericThread

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_thread_h__ */
