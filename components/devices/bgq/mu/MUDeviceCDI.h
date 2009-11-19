/**
 * \file components/devices/bgq/mu/MUDeviceCDI.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu_MUDeviceCDI_h__
#define __components_devices_bgq_mu_MUDeviceCDI_h__

///
/// \brief CDI is currently unimplemented in the MU Device.
///

namespace DCMF {
  namespace Queueing{
    namespace MU {

#if 0  //Implemented in MUDevice.cc
      inline int MUDevice::init_impl(SysDep &sysdep)
	{
	}
#endif

      inline int MUDevice::getPktSize_impl()
	{
	}

      inline int MUDevice::isInit_impl()
	{
	}

      inline int MUDevice::advance_impl ()
	{
	  int events = 0;

	  if (_pollQueues)
	    events += pollQueuedMessages ();

	  events += _resMgr.advanceReceptionFifos();
	  events += _resMgr.advanceInjectionFifos();

	  //Counter advance for rzv messages
	  return events;
	}


      inline int MUDevice::post_impl(MUMessage &msg_in)
	{
	}

      inline int MUDevice::registerRecvFunction_impl
	(RecvFunction_t  recv_func,
	 void           *recv_func_parm)
	{
	}

      inline int MUDevice::generateMessage_impl(MUMessage       &msg,
						DCMF_Callback_t   cb,
						unsigned          dispatch_id,
						unsigned          origin_rank,
						unsigned          dest_rank,
						DCMF_Iovec_t     *iov,
						unsigned          iov_len,
						size_t            msg_len)
	{

	}

      inline int    MUDevice::readData_impl (int index, void* buf, size_t length)
	{
	}

      inline void MUDevice::recvPkt_finalize(int index)
	{
	}

      inline char *MUDevice::recvPktBuf(int index)
	{
	}


      inline int MUDevice::getMessageSize_impl()
	{

	}
    };
  };
};

#endif
