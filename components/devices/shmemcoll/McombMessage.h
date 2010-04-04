/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/McombMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmem_McombMessage_h__
#define __components_devices_shmem_McombMessage_h__

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
      class McombMessage : public SendQueue::Message
      {

		
        protected:
	   inline McombMessage (T_Device * device, T_Desc * desc, T_Desc * matched_desc, 
 						pami_work_function  work_func, void * work_cookie ) :
       SendQueue::Message (work_func, work_cookie, McombMessage::__done, this, device->getContextOffset()),
      _device (device),
 	  _my_desc(desc),
 	  _master_desc(matched_desc),
   	  _master(desc->get_master()),
      _local_rank(__global.topology_local.rank2Index(__global.mapping.task())),
      _npeers(__global.topology_local.size()),
      _task(__global.mapping.task())

   {
     TRACE_ERR((stderr, "<> McombMessage::McombMessage()\n"));
   };


  static void __done (pami_context_t context, void* cookie, pami_result_t result)
   {
     TRACE_ERR((stderr,"invoking done of the message\n"));

     McombMessage * msg = (McombMessage *) cookie;

     pami_multicombine_t & mcomb_params = msg->_my_desc->get_mcomb_params();
     mcomb_params.cb_done.function(context, mcomb_params.cb_done.clientdata, PAMI_SUCCESS);

     msg->_my_desc->set_state(Shmem::DONE);
     return;

   };


   T_Device      * _device;
   T_Desc		* _my_desc;
   T_Desc		* _master_desc;	
   unsigned      _master, _local_rank, _npeers;
   pami_task_t  _task;

      };  // PAMI::Device::McombMessage class

    };
  };    // PAMI::Device namespace
};      // PAMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_ShmemMcombMessage_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
