/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/generic/ProgressFunctionMsg.h
 * \brief Add a general function to the progress engine loop/queue
 */

#ifndef __components_devices_generic_ProgressFunctionMsg_h__
#define __components_devices_generic_ProgressFunctionMsg_h__

#include "components/devices/generic/Device.h"
#include "components/devices/generic/SubDevice.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "sys/xmi.h"

typedef int XMI_ProgressFunc(void *clientdata);

typedef struct {
	xmi_context_t context;
	void *request;
	XMI_ProgressFunc *func;
	void *clientdata;
	xmi_callback_t cb_done;
} XMI_ProgressFunc_t;

namespace XMI {
namespace Device {

class ProgressFunctionMdl;
class ProgressFunctionMsg;

typedef XMI::Device::Generic::GenericAdvanceThread ProgressFunctionThr;
typedef XMI::Device::Generic::MultiThrdSubDevice<ProgressFunctionThr, XMI_MAX_THREAD_PER_PROC> ProgressFunctionDev;

}; //-- Device
}; //-- XMI

extern XMI::Device::ProgressFunctionDev _g_progfunc_dev;

namespace XMI {
namespace Device {

///
/// \brief A local barrier message that takes advantage of the
/// Load Linked and Store Conditional instructions
///
class ProgressFunctionMsg : public XMI::Device::Generic::GenericMessage {
public:

	/// \brief method to advance a portion of the message
	///
	/// If a message is split between multiple threads, each thread
	/// is responsible for only part of the message.
	///
	/// \param in v An opaque pointer to indicate message portion
	/// \param in l An opaque length to indicate message portion
	/// \return Message status - typically either Active or Done.
	///
	/// status Active => Done
	inline XMI::Device::MessageStatus advanceThread(XMI::Device::Generic::GenericAdvanceThread *t);

	inline void complete();
	inline size_t objsize_impl() { return sizeof(ProgressFunctionMsg); }

protected:
	friend class ProgressFunctionMdl;

	ProgressFunctionMsg(Generic::BaseGenericDevice &Generic_QS,
		XMI_ProgressFunc *func,
		void *clientdata,
		xmi_callback_t cb) :
	XMI::Device::Generic::GenericMessage(Generic_QS, cb),
	_thread(),
	_func(func),
	_clientdata(clientdata),
	_rc(XMI_SUCCESS)
	{
	}

private:
	//friend class ProgressFunctionDev;
	friend class XMI::Device::Generic::MultiThrdSubDevice<ProgressFunctionThr, XMI_MAX_THREAD_PER_PROC>;

	inline XMI::Device::MessageStatus __advanceThread(ProgressFunctionThr *thr) {
		// TBD: optimize away virt func call - add method
		// for a persistent advance?
		int rc = _func(_clientdata);
		if (rc == 0) {
			setStatus(XMI::Device::Done);
			return XMI::Device::Done;
		} else if (rc < 0) {
			_rc = (xmi_result_t)-rc;
			// GenericDevice will call complete()...
			setStatus(XMI::Device::Done);
			return XMI::Device::Done;
		}
		return XMI::Device::Active;
	}

	/// \todo for multithreading of this function, need a lot more
	inline int __setThreads(ProgressFunctionThr *t, int n) {
		// assert n >= 1 && t == NULL
		_thread.setMsg(this);
		_thread.setDone(false);
		return 1;
	}

	inline ProgressFunctionThr *__getThreads() {
		return &_thread;
	}

protected:
	ProgressFunctionThr _thread;
	XMI_ProgressFunc *_func;
	void *_clientdata;
	xmi_result_t _rc;
}; //-- ProgressFunctionMsg

/// If this ever expands into multiple types, need to make this a subclass
class ProgressFunctionMdl {

public:
	static const size_t sizeof_msg = sizeof(ProgressFunctionMsg);

	/// This model is typically never constructed, but rather just
	/// generateMessage called directly.
	ProgressFunctionMdl() {
	}
	/// In case someone constructs it the "standard" way, don't complain.
	ProgressFunctionMdl(xmi_result_t &status) {
		status = XMI_SUCCESS;
	}

	inline void reset_impl() {}

	inline bool generateMessage(XMI_ProgressFunc_t *pf);

private:
}; // class ProgressFunctionMdl

}; //-- Device
}; //-- XMI

inline void XMI::Device::ProgressFunctionMsg::complete() {
	((ProgressFunctionDev &)_QS).__complete<ProgressFunctionMsg>(this);
	executeCallback(_rc);
}

inline XMI::Device::MessageStatus XMI::Device::ProgressFunctionMsg::advanceThread(XMI::Device::Generic::GenericAdvanceThread *t) {
	return __advanceThread((ProgressFunctionThr *)t);
}

inline bool XMI::Device::ProgressFunctionMdl::generateMessage(XMI_ProgressFunc_t *pf) {

	// can't afford to have some fail and not others, so even though
	// we may not use 'msg' at all we must fail if it is too small.
	ProgressFunctionMsg *msg = (ProgressFunctionMsg *)pf->request;

	int rc = pf->func(pf->clientdata);
	if (rc == 0) {
		if (pf->cb_done.function) {
			pf->cb_done.function(NULL, pf->cb_done.clientdata, XMI_SUCCESS);
		}
		return true;
	} else if (rc < 0) {
		if (pf->cb_done.function) {
			pf->cb_done.function(NULL, pf->cb_done.clientdata, (xmi_result_t)-rc);
		}
		return true;
	}
	new (msg) ProgressFunctionMsg(_g_progfunc_dev, pf->func, pf->clientdata, pf->cb_done);
	_g_progfunc_dev.__post<ProgressFunctionMsg>(msg);
	return true;
}

#endif /* __components_devices_generic_progressfunction_h__ */
