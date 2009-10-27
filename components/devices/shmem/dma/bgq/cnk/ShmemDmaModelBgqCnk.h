/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemPacketModel.h
 * \brief ???
 */

#ifndef __components_devices_shmem_shmemdmamodelbgqcnk_h__
#define __components_devices_shmem_shmemdmamodelbgqcnk_h__

#include <errno.h>

#include "sys/xmi.h"

#include "components/devices/myDmaModel.h" //for now using a separate model than M
#include "components/devices/shmem/ShmemBaseMessage.h"
#include "components/devices/shmem/dma/bgq/cnk/ShmemDmaDeviceBgqCnk.h"

#include "components/memregion/bgq/BgqMemregion.h"
#include "arch/a2qpx/Arch.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    ///
    /// \brief Packet model interface implementation for shared memory.
    ///
    /// The shared memory packet model contains the shared memory
    /// packet header which is block-copied into a shared memory message
    /// when it is generated.
    ///
    /// The model also registers the dispatch function with the shared
    /// memory device and stores the dispatch id in the packet header.
    ///
    /// \see ShmemPacketDevice
    ///
	template < class T_Device, class T_Message >
    class ShmemDmaModelBgqCnk: public myInterface::DmaModel< ShmemDmaModelBgqCnk<T_Device, T_Message>, T_Device, MemRegion::BgqMemregion, sizeof(T_Message) >
    {
      public:

	 static const size_t message_model_state_bytes    = sizeof(T_Message);
		
        ///
        /// \brief Construct a Common Device Interface shared memory packet model.
        ///
        /// \param[in] device  Shared memory device
        ///
        ShmemDmaModelBgqCnk(T_Device & device, xmi_context_t context) :
            myInterface::DmaModel < ShmemDmaModelBgqCnk<T_Device,T_Message>, T_Device, MemRegion::BgqMemregion, sizeof(T_Message) > (device, context),
            _device (device),
            _context (context)
        {};

        //static const bool deterministic = true;

        xmi_result_t init_impl (size_t             origin_rank)
        {
         // _dispatch_id = _device.registerRecvFunction (dispatch, direct_recv_func, direct_recv_func_parm);
          return XMI_SUCCESS;  
        };
		
		inline bool postDmaPut_impl (uint8_t  state[sizeof(T_Message)],
                                  xmi_event_function   local_fn,
                                  void               * cookie,
                                  size_t            target_rank,
                                  MemRegion::BgqMemregion     * local_memregion,
                                  size_t            local_offset,
                                  MemRegion::BgqMemregion     * remote_memregion,
                                  size_t            remote_offset,
                                  size_t            bytes)
		{
				size_t peer;
				XMI::Interface::Mapping::nodeaddr_t address;
				__global.mapping.task2node (target_rank, address);
				__global.mapping.node2peer (address, peer);

				void * remote_vaddr = (void *)((uint64_t)remote_memregion->getBaseGlobalVirtualAddress() + local_offset);
				void * local_vaddr  = (void *)((uint8_t *)local_memregion->getBaseVirtualAddress() + remote_offset);
				printf("remote addr:%p local_vaddr:%p\n", remote_vaddr, local_vaddr);

				if (_device.isSendQueueEmpty (peer)){ 

						memcpy (remote_vaddr, local_vaddr, bytes);
						ppc_msync();
						printf("Calling postDmaPut_impl\n");
						fflush(stdout);

						if (local_fn) local_fn (_context, cookie, XMI_SUCCESS);
						return XMI_SUCCESS;  

				}
				T_Message * obj = (T_Message *) &state[0];
				new (obj) T_Message(_context, local_fn, cookie, _dispatch_id, remote_vaddr, local_vaddr, bytes);

				//obj->enableRemoteCompletion ();
				_device.post (peer, obj);
				return XMI_SUCCESS;  
		};	

		inline bool postDmaGet_impl (uint8_t   state[sizeof(T_Message)],
                                  xmi_event_function   local_fn,
                                  void               * cookie,
                                  size_t            target_rank,
                                  MemRegion::BgqMemregion     * local_memregion,
                                  size_t            local_offset,
                                  MemRegion::BgqMemregion     * remote_memregion,
                                  size_t            remote_offset,
                                  size_t            bytes)
		{
				size_t peer;
				XMI::Interface::Mapping::nodeaddr_t address;
				__global.mapping.task2node (target_rank, address);
				__global.mapping.node2peer (address, peer);

				void * remote_vaddr = (void *)((uint64_t)remote_memregion->getBaseGlobalVirtualAddress() + local_offset);
				void * local_vaddr  = (void *)((uint8_t *)local_memregion->getBaseVirtualAddress() + remote_offset);
				printf("remote addr:%p local_vaddr:%p\n", remote_vaddr, local_vaddr);

				if (_device.isSendQueueEmpty (peer)){ 

						memcpy (local_vaddr, remote_vaddr, bytes);
						 ppc_msync();
						printf("Calling postDmaGet_impl\n");
						fflush(stdout);

						if (local_fn) local_fn (_context, cookie, XMI_SUCCESS);
						return XMI_SUCCESS;  

				}
				T_Message * obj = (T_Message *) &state[0];
				new (obj) T_Message(_context, local_fn, cookie, _dispatch_id, local_vaddr, remote_vaddr, bytes);

				//obj->enableRemoteCompletion ();
				_device.post (peer, obj);

				return XMI_SUCCESS;  
		};	

      protected:
        T_Device        &_device;
        xmi_context_t   _context;
        uint16_t        _dispatch_id; //is this required ??
    };
  };
};
#endif // __components_devices_shmem_shmemdmamodelbgqcnk_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
