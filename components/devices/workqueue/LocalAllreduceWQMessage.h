/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/LocalAllreduceWQMessage.h
 * \brief ???
 */

#ifndef __components_devices_workqueue_LocalAllreduceWQMessage_h__
#define __components_devices_workqueue_LocalAllreduceWQMessage_h__

#include "components/devices/MulticombineModel.h"
#include "components/devices/workqueue/SharedWorkQueue.h"
#include "components/devices/workqueue/MemoryWorkQueue.h"
#include "math/math_coremath.h"
#include "Global.h"
#include "components/devices/generic/Device.h"
#include "components/devices/generic/SubDevice.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"

namespace XMI {
namespace Device {

class LocalAllreduceWQModel;
class LocalAllreduceWQMessage;
typedef XMI::Device::Generic::GenericAdvanceThread LocalAllreduceWQThread;
typedef XMI::Device::Generic::SimpleSubDevice<LocalAllreduceWQThread> LocalAllreduceWQDevice;

}; // namespace Device
}; // namespace XMI

extern XMI::Device::LocalAllreduceWQDevice _g_l_allreducewq_dev;

namespace XMI {
namespace Device {

class LocalAllreduceWQMessage : public XMI::Device::Generic::GenericMessage {
private:
	enum roles {
		NO_ROLE = 0,
		NON_ROOT_ROLE = (1 << 0), // first role must be non-root(s)
		ROOT_ROLE = (1 << 1), // last role must be root
	};
public:

          ///
          /// \brief Local (intranode) allreduce collective message.
          ///
          /// \param[in] device       Shared Memory device
          /// \param[in] cb           Callback to invoke when the broadcast is complete
          /// \param[in] sharedmemory Location of the shared memory
          /// \param[in] peer         This core's peer id
          /// \param[in] peers        Number of ranks active on this node
          /// \param[in] sbuffer      Location of the source reduce buffer
          /// \param[in] rbuffer      Location of the result reduce buffer
          /// \param[in] count        Number of elements to reduce
          /// \param[in] func         Math function to invoke to perform the reduction
          /// \param[in] dtshift      Shift in byts of the elements for the reduction
          ///
          inline LocalAllreduceWQMessage (Generic::GenericSubDevice      & device,
					  xmi_multicombine_t *mcomb,
                                          XMI::Device::WorkQueue::SharedWorkQueue & workqueue,
                                          unsigned          peer,
                                          unsigned          npeers,
                                          coremath          func,
                                          int               dtshift) :
            XMI::Device::Generic::GenericMessage (device, mcomb->cb_done,
				XMI_GD_ClientId(mcomb->client), mcomb->context),
            _peer (peer),
            _func (func),
            _dtshift (dtshift),
            _source ((XMI::PipeWorkQueue *)mcomb->data),
            _result ((XMI::PipeWorkQueue *)mcomb->results),
            _shared (workqueue)
          {
          }

	STD_POSTNEXT(LocalAllreduceWQDevice,LocalAllreduceWQThread,&_g_l_allreducewq_dev)

protected:
	//friend class LocalAllreduceWQDevice;
	friend class XMI::Device::Generic::SimpleSubDevice<LocalAllreduceWQThread>;

	ADVANCE_ROUTINE(advanceThread,LocalAllreduceWQMessage,LocalAllreduceWQThread);
	friend class XMI::Device::Generic::GenericMessage;
	inline xmi_result_t __advanceThread(LocalAllreduceWQThread *thr) {
		if (_peer == 0) {
			_shared.Q2Q (*_source, (coremath1) XMI::Device::WorkQueue::SharedWorkQueue::shmemcpy, 0);
		} else {
			_shared.reduceInPlace (*_source, _func, _dtshift);
		}

		_shared.Q2Qr (*_result, (coremath1) XMI::Device::WorkQueue::SharedWorkQueue::shmemcpy, 0);

		// If all bytes have been copied from the shared queue into the
		// local result buffer then the peer is done.
		// NOTE! This assumes the result WQ is a "flat buffer" and thus
		// actually has a "hard stop".
		if (_result->bytesAvailableToProduce() == 0) {
			setStatus(XMI::Device::Done);
			thr->setStatus(XMI::Device::Complete);
		}

		return getStatus() == XMI::Device::Done ? XMI_SUCCESS : XMI_EAGAIN;
	}

	inline int __setThreads(LocalAllreduceWQThread *t, int n) {
		t->setMsg(this);
		t->setAdv(advanceThread);
		t->setStatus(XMI::Device::Ready);
		__advanceThread(t);
		return 1;
	}

private:

          unsigned          _peer;
          coremath          _func;
          int               _dtshift;
          XMI::PipeWorkQueue   *_source;
          XMI::PipeWorkQueue   *_result;
          XMI::Device::WorkQueue::SharedWorkQueue & _shared;
}; // class LocalAllreduceWQMessage

class LocalAllreduceWQModel : public XMI::Device::Interface::MulticombineModel<LocalAllreduceWQModel,sizeof(LocalAllreduceWQMessage)> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = 1;
	static const size_t sizeof_msg = sizeof(LocalAllreduceWQMessage);

	LocalAllreduceWQModel(xmi_result_t &status) :
	XMI::Device::Interface::MulticombineModel<LocalAllreduceWQModel,sizeof(LocalAllreduceWQMessage)>(status),
	_shared(_g_l_allreducewq_dev.getSysdep()),
	_peer(__global.topology_local.rank2Index(__global.mapping.task())),
	_npeers(__global.topology_local.size())
	{
		if (!_shared.available()) {
			status = XMI_ERROR;
			return;
		}
		_shared.setProducers(_npeers, _peer);
		_shared.setConsumers(_npeers, _peer);
		// since we hard-code topology_local, we know _peer==0 exists...
		_shared.barrier_reset(_npeers, (_peer == 0));
	}

	inline void reset_impl() {
		if (_peer == 0) {
			_shared.reset();
		}
	}

	inline xmi_result_t postMulticombine_impl(uint8_t (&state)[sizeof_msg], xmi_multicombine_t *mcomb);

private:
	XMI::Device::WorkQueue::SharedWorkQueue _shared;
	unsigned _peer;
	unsigned _npeers;
}; // class LocalAllreduceWQModel

inline xmi_result_t LocalAllreduceWQModel::postMulticombine_impl(uint8_t (&state)[sizeof_msg], xmi_multicombine_t *mcomb) {
	// assert((data_topo .U. results_topo).size() == _npeers);
	int dtshift = xmi_dt_shift[mcomb->dtype];
	coremath func = MATH_OP_FUNCS(mcomb->dtype, mcomb->optor, 2);

	LocalAllreduceWQMessage *msg =
		new (&state) LocalAllreduceWQMessage(_g_l_allreducewq_dev,
					mcomb, _shared, _peer, _npeers,
					func, dtshift);
	_g_l_allreducewq_dev.__post<LocalAllreduceWQMessage>(msg);
	return XMI_SUCCESS;
}

}; // namespace Device
}; // namespace XMI

#endif // __dcmf_workqueue_localallreducewqmsg_h__
