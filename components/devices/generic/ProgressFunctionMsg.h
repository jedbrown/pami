/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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

#include "Client.h"

#include "components/devices/generic/Device.h"
#include "components/devices/generic/SubDevice.h"
#include "components/devices/generic/AdvanceThread.h"
#include "sys/xmi.h"

typedef struct {
	xmi_client_t client;
	size_t context;
	void *request;
	xmi_work_function func;
	void *clientdata;
	xmi_callback_t cb_done;
} XMI_ProgressFunc_t;

namespace XMI {
namespace Device {

class ProgressFunctionMdl;
class ProgressFunctionMsg;

class ProgressFunctionDev {
public:
	inline void init(XMI::SysDep &sd, XMI::Device::Generic::Device *devices, size_t client, size_t contextId) {
		if (contextId == 0) {
			_generics[client] = devices;
		}
	}

	inline int advanceRecv(size_t client, size_t context) { return 0; }

	inline xmi_context_t getContext(size_t client, size_t context) { 
		return _generics[client][context].getContext();
	}

protected:
	friend class ProgressFunctionMdl;
	friend class ProgressFunctionMsg;
	inline void __post(ProgressFunctionMsg *msg);
private:
	XMI::Device::Generic::Device *_generics[XMI_MAX_NUM_CLIENTS];
}; // class ProgressFunctionDev

}; //-- Device
}; //-- XMI

extern XMI::Device::ProgressFunctionDev _g_progfunc_dev;

namespace XMI {
namespace Device {

///
/// \brief A local function-call message
///
class ProgressFunctionMsg : public XMI::Device::Generic::GenericThread {
public:

protected:
	friend class ProgressFunctionMdl;

	ProgressFunctionMsg(XMI_ProgressFunc_t *pf) :
	XMI::Device::Generic::GenericThread()
	{
		_func = pf->func;
		_cookie = pf->clientdata;
		_cb_done = pf->cb_done;
		_contextId = pf->context;
		_clientId = XMI_GD_ClientId(pf->client);
		setStatus(XMI::Device::Ready);
	}

public:
	inline void postWorkDirect() {
		// should there be a status for "one-shot"?
		setStatus(XMI::Device::Ready);
		_g_progfunc_dev.__post(this);
	}
	inline void setFunc(xmi_work_function func) { _func = func; }
	inline void setCookie(void *cookie) { _cookie = cookie; }
	inline void setDone(xmi_callback_t done) { _cb_done = done; }
	inline void setContext(size_t ctx) { _contextId = ctx; }
	inline void setClient(size_t clt) { _clientId = clt; }
	inline size_t getContextId() { return _contextId; }
	inline size_t getClientId() { return _clientId; }
protected:
	size_t _clientId;
	size_t _contextId;
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

	inline bool postWork(void *msg, xmi_work_function func, void *cookie, xmi_callback_t done, size_t ctx);
	inline bool postWork(XMI_ProgressFunc_t *pf);

private:
}; // class ProgressFunctionMdl

}; //-- Device
}; //-- XMI

inline void XMI::Device::ProgressFunctionDev::__post(ProgressFunctionMsg *msg) {
	// 'msg' actually inherits from GenericThread...
	_generics[msg->getClientId()][msg->getContextId()].postThread(msg);
}

inline bool XMI::Device::ProgressFunctionMdl::postWork(XMI_ProgressFunc_t *pf) {
	ProgressFunctionMsg *msg = (ProgressFunctionMsg *)pf->request;

	// need a better way to get xmi_context_t...
	// problem is that this "message" has not even been constructed yet,
	// let alone posted to a generic device queue, so we have no other
	// way to derive the xmi_context_t (unless it is passed-in).
	xmi_context_t ctx = _g_progfunc_dev.getContext(XMI_GD_ClientId(pf->client), pf->context);
	int rc = pf->func(ctx, pf->clientdata);
	if (rc == 0) {
		if (pf->cb_done.function) {
			pf->cb_done.function(ctx, pf->cb_done.clientdata, XMI_SUCCESS);
		}
		return true;
	} else if (rc < 0) {
		if (pf->cb_done.function) {
			pf->cb_done.function(ctx, pf->cb_done.clientdata, (xmi_result_t)-rc);
		}
		return true;
	}
	new (msg) ProgressFunctionMsg(pf);
	_g_progfunc_dev.__post(msg);
	return true;
}

inline bool XMI::Device::ProgressFunctionMdl::postWork(void *msg, xmi_work_function func, void *cookie, xmi_callback_t done, size_t ctx) {
	ProgressFunctionMsg *work = (ProgressFunctionMsg *)msg;
	// can we get away without 'new'ing this?
	work->setFunc(func);
	work->setCookie(cookie);
	work->setDone(done);
	work->setContext(ctx);	// this tells generic device which slice to use,
				// but does not tell which client.
	work->postWorkDirect();
	return true;
}

#endif /* __components_devices_generic_progressfunction_h__ */
