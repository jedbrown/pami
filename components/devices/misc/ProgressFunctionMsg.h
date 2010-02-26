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

#include "components/devices/generic/Device.h"
#include "components/devices/generic/SubDevice.h"
#include "components/devices/generic/AdvanceThread.h"
#include "sys/xmi.h"
#include "components/devices/FactoryInterface.h"

typedef struct {
	size_t client;
	size_t context;
	void *request;
	xmi_work_function func;
	void *clientdata;
	xmi_callback_t cb_done;
} XMI_ProgressFunc_t;

namespace XMI {
namespace Device {

class ProgressFunctionMdl;

class ProgressFunctionDev {
public:
	class Factory : public Interface::FactoryInterface<Factory,ProgressFunctionDev,Generic::Device>  {
	public:
		static inline ProgressFunctionDev *generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager & mm);
		static inline xmi_result_t init_impl(ProgressFunctionDev *devs, size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices);
		static inline size_t advance_impl(ProgressFunctionDev *devs, size_t client, size_t context);
	}; // class ProgressFunctionDev::Factory
	inline ProgressFunctionDev() {}

	inline xmi_result_t init(size_t client, size_t ctx, XMI::Device::Generic::Device *devices) {
		_generics[client] = devices;
		return XMI_SUCCESS;
	}

	inline XMI::Device::Generic::Device *getGeneric(size_t client, size_t contextId) {
		return &_generics[client][contextId];
	}

	inline xmi_context_t getContext(size_t client, size_t contextId) {
		return getGeneric(client, contextId)->getContext();
	}
protected:
	XMI::Device::Generic::Device *_generics[XMI_MAX_NUM_CLIENTS];
}; // class ProgressFunctionDev

}; //-- Device
}; //-- XMI

extern XMI::Device::ProgressFunctionDev _g_progfunc_dev;

namespace XMI {
namespace Device {

inline ProgressFunctionDev *ProgressFunctionDev::Factory::generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager & mm) {
	return &_g_progfunc_dev;
}

inline xmi_result_t ProgressFunctionDev::Factory::init_impl(ProgressFunctionDev *devs, size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices) {
	return _g_progfunc_dev.init(client, contextId, devices);
}

inline size_t ProgressFunctionDev::Factory::advance_impl(ProgressFunctionDev *devs, size_t client, size_t context) { return 0; }


/// If this ever expands into multiple types, need to make this a subclass
class ProgressFunctionMdl {

public:
	static const size_t sizeof_msg = sizeof(XMI::Device::Generic::GenericThread);

	/// This model is typically never constructed, but rather just
	/// generateMessage called directly.
	ProgressFunctionMdl() {
	}
	/// In case someone constructs it the "standard" way, don't complain.
	ProgressFunctionMdl(xmi_result_t &status) {
		status = XMI_SUCCESS;
	}

	inline bool postWork(XMI_ProgressFunc_t *pf);

private:
}; // class ProgressFunctionMdl

}; //-- Device
}; //-- XMI

inline bool XMI::Device::ProgressFunctionMdl::postWork(XMI_ProgressFunc_t *pf) {
	XMI::Device::Generic::GenericThread *thr = (XMI::Device::Generic::GenericThread *)pf->request;
	XMI::Device::Generic::Device *gd;
#if 0
#warning can we really advance this progress function here?
	// need a better way to get xmi_context_t...
	// problem is that this "message" has not even been constructed yet,
	// let alone posted to a generic device queue, so we have no other
	// way to derive the xmi_context_t (unless it is passed-in).
	xmi_context_t ctx = _g_progfunc_dev.getContext(pf->client, pf->context);
	int rc = pf->func(ctx, pf->clientdata);
	if (rc != XMI_EAGAIN) {
		if (pf->cb_done.function) {
			pf->cb_done.function(ctx, pf->cb_done.clientdata, rc);
		}
		return true;
	}
#endif
	new (thr) XMI::Device::Generic::GenericThread(pf->func, pf->clientdata, pf->cb_done);
	gd = _g_progfunc_dev.getGeneric(pf->client, pf->context);
	thr->setStatus(XMI::Device::Ready);
	gd->postThread(thr);
	return true;
}

#endif /* __components_devices_generic_progressfunction_h__ */
