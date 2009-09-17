/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/LocalAllreduceWQMessage.h
 * \brief ???
 */

#ifndef __dcmf_workqueue_localallreducewqmsg_h__
#define __dcmf_workqueue_localallreducewqmsg_h__

#include "prod/cdi/ReduceModel.h"
#include "workqueue/SharedWorkQueue.h"
#include "workqueue/MemoryWorkQueue.h"
#include "math_coremath.h"
#include "SysDep.h"
#include "generic/Device.h"
#include "generic/SubDevice.h"
#include "generic/Message.h"
#include "generic/AdvanceThread.h"

extern XMI::Topology *_g_topology_local;

namespace DCMF {
namespace CDI {

class LocalAllreduceWQModel;
class LocalAllreduceWQMessage;
typedef DCMF::Queueing::Generic::GenericAdvanceThread LocalAllreduceWQThread;
typedef DCMF::Queueing::Generic::SimpleSubDevice<LocalAllreduceWQModel,LocalAllreduceWQMessage,LocalAllreduceWQThread> LocalAllreduceWQDevice;

}; // namespace CDI
}; // namespace DCMF

extern DCMF::CDI::LocalAllreduceWQDevice _g_l_allreducewq_dev;

namespace DCMF {
namespace CDI {

class LocalAllreduceWQMessage : public DCMF::Queueing::Generic::GenericMessage {
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
          inline LocalAllreduceWQMessage (BaseDevice      & device,
                                          XMI_Callback_t   cb,
                                          DCMF::Device::WorkQueue::SharedWorkQueue & workqueue,
                                          unsigned          peer,
                                          unsigned          npeers,
                                          XMI::PipeWorkQueue *swq,
                                          XMI::PipeWorkQueue *rwq,
                                          unsigned          count,
                                          coremath          func,
                                          int               dtshift) :
            DCMF::Queueing::Generic::GenericMessage (device, cb),
            _peer (peer),
            _func (func),
            _dtshift (dtshift),
            _source (swq),
            _result (rwq),
            _shared (workqueue)
          {
          }

	inline void complete();

protected:
	//friend class LocalAllreduceWQDevice;
	friend class DCMF::Queueing::Generic::SimpleSubDevice<LocalAllreduceWQModel,LocalAllreduceWQMessage,LocalAllreduceWQThread>;

          ///
          /// \brief Advance the allreduce shared memory message
          ///
          inline DCMF::Queueing::MessageStatus advanceThread(LocalAllreduceWQThread *thr);

	inline DCMF::Queueing::MessageStatus __advanceThread(LocalAllreduceWQThread *thr) {
		if (_peer == 0) {
			_shared.Q2Q (*_source, (coremath1) DCMF::Device::WorkQueue::SharedWorkQueue::shmemcpy, 0);
		} else {
			_shared.reduceInPlace (*_source, _func, _dtshift);
		}

		_shared.Q2Qr (*_result, (coremath1) DCMF::Device::WorkQueue::SharedWorkQueue::shmemcpy, 0);

		// If all bytes have been copied from the shared queue into the
		// local result buffer then the peer is done.
		// NOTE! This assumes the result WQ is a "flat buffer" and thus
		// actually has a "hard stop".
		if (_result->bytesAvailableToProduce() == 0) setStatus(DCMF::Queueing::Done);

		return getStatus();
	}

	inline int __setThreads(LocalAllreduceWQThread *t, int n) {
		t->setMsg(this);
		t->setDone(false);
		return 1;
	}

private:

          unsigned          _peer;
          coremath          _func;
          int               _dtshift;
          XMI::PipeWorkQueue   *_source;
          XMI::PipeWorkQueue   *_result;
          DCMF::Device::WorkQueue::SharedWorkQueue & _shared;
}; // class LocalAllreduceWQMessage

class LocalAllreduceWQModel : public Reduce::Model<LocalAllreduceWQModel,LocalAllreduceWQDevice,LocalAllreduceWQMessage> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = 1;

	LocalAllreduceWQModel(DCMF::SysDep *sysdep, XMI_Result &status) :
	Reduce::Model<LocalAllreduceWQModel,LocalAllreduceWQDevice,LocalAllreduceWQMessage>(_g_l_allreducewq_dev, status),
	_shared(sysdep),
	_peer(_g_topology_local->rank2Index(sysdep->mapping().rank())),
	_npeers(_g_topology_local->size())
	{
		if (!_shared.available()) {
			status = XMI_ERROR;
			return;
		}
		_shared.setProducers(_npeers, _peer);
		_shared.setConsumers(_npeers, _peer);
		reset_impl();
	}

	inline void reset_impl() {
		if (_peer == 0) {
			_shared.reset();
		}
	}

	inline bool generateMessage_impl(XMI_Multicombine_t *mcomb);

private:
	DCMF::Device::WorkQueue::SharedWorkQueue _shared;
	unsigned _peer;
	unsigned _npeers;

	static inline void compile_time_assert () {
		COMPILE_TIME_ASSERT(sizeof(XMI_Request_t) >= sizeof(LocalAllreduceWQMessage));
	}
}; // class LocalAllreduceWQModel

void LocalAllreduceWQMessage::complete() {
	((LocalAllreduceWQDevice &)_QS).__complete(this);
	executeCallback();
}

inline DCMF::Queueing::MessageStatus LocalAllreduceWQMessage::advanceThread(LocalAllreduceWQThread *thr) {
	return __advanceThread(thr);
}

inline bool LocalAllreduceWQModel::generateMessage_impl(XMI_Multicombine_t *mcomb) {
	if (mcomb->req_size < sizeof(LocalAllreduceWQMessage)) {
		return false;
	}
	// assert((data_topo .U. results_topo).size() == _npeers);
	int dtshift = dcmf_dt_shift[mcomb->dtype];
	coremath func = MATH_OP_FUNCS(mcomb->dtype, mcomb->optor, 2);

	LocalAllreduceWQMessage *msg =
		new (mcomb->request) LocalAllreduceWQMessage(_g_l_allreducewq_dev,
					mcomb->cb_done, _shared, _peer, _npeers,
					(XMI::PipeWorkQueue *)mcomb->data,
					(XMI::PipeWorkQueue *)mcomb->results,
					mcomb->count, func, dtshift);
	_g_l_allreducewq_dev.__post(msg);
	return true;
}

}; // namespace CDI
}; // namespace DCMF

#endif // __dcmf_workqueue_localallreducewqmsg_h__
