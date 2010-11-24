/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/McstMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_McstMessage_h__
#define __components_devices_shmemcoll_McstMessage_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"
#include "sys/pami.h"
#include "components/devices/shmem/ShmemMessage.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

      template <class T_Device, class T_Desc>
      class McstMessage : public SendQueue::Message
		{
		protected:

  		static void __done (pami_context_t context, void* cookie, pami_result_t result)
		  {
			TRACE_ERR((stderr,"invoking done of the message\n"));
			McstMessage * msg = (McstMessage *) cookie;
			pami_multicast_t & mcast_params = msg->_my_desc->get_mcast_params();

			mcast_params.cb_done.function(context, mcast_params.cb_done.clientdata, PAMI_SUCCESS);
			msg->_my_desc->set_state(Shmem::DONE);
			return;

		  };


		inline McstMessage (T_Device * device, T_Desc * desc, T_Desc * master_desc,
                                pami_work_function  work_func, void * work_cookie) :
		//SendQueue::Message (work_func, work_cookie, McstMessage::__done, this, device->getContextOffset()),
		SendQueue::Message (work_func, work_cookie, McstMessage::__done, this, 0),
		_device (device),
		_my_desc(desc),
		_master_desc(master_desc),
		_bytes_consumed(0)
		{
            TRACE_ERR((stderr, "<> McstMessage::McstMessage()\n"));
		};

	public:
		static const size_t short_msg_cutoff                  = 512;
		static unsigned seq_num;

		protected:
		 T_Device      * _device;
		 T_Desc		* _my_desc;
		 T_Desc		* _master_desc;
		 size_t	_bytes_consumed;

      };  // PAMI::Device::McstMessage class
		
	  template <class T_Device, class T_Desc>
    	unsigned   McstMessage<T_Device,T_Desc> :: seq_num = 0;



    };
  };    // PAMI::Device namespace
};      // PAMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_ShmemMcstMessage_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
