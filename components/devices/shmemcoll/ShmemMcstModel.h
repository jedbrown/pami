/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/ShmemMcstModel.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_ShmemMcstModel_h__
#define __components_devices_shmemcoll_ShmemMcstModel_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"

#include "sys/pami.h"

#include "components/devices/MulticastModel.h"
#include "components/devices/ShmemCollInterface.h"
#include "components/devices/shmemcoll/ShmemCollDevice.h"
#include "components/devices/shmemcoll/ShmemMcstMessage.h"
#include "components/devices/shmemcoll/ShaddrMcstMessage.h"
#include "components/devices/shmemcoll/ShmemDescMessage.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

      template <class T_Device, class T_Desc>
      class ShmemMcstModel : public Interface::MulticastModel < ShmemMcstModel<T_Device,T_Desc>, T_Device, sizeof(Shmem::McstMessage<T_Device,T_Desc>) >
      {

		private:

		//Collective logic for short messages
		static int dispatch_mcst_matched_shmem(void* desc1, void* desc2 , void  *func_param )
        {
			TRACE_ERR((stderr,"descriptor matched..start the shmem protocol\n"));

			T_Desc * my_desc = (T_Desc*) desc1;
			T_Desc * matched_desc = (T_Desc*) desc2;
			Shmem::ShmemMcstModel<T_Device, T_Desc> * model  = (Shmem::ShmemMcstModel<T_Device, T_Desc> *) func_param;
            Shmem::McstMessageShmem<T_Device, T_Desc> * obj = (Shmem::McstMessageShmem<T_Device, T_Desc> *) (my_desc->get_storage());

            new (obj) Shmem::McstMessageShmem<T_Device, T_Desc> (&model->_device, my_desc, matched_desc);
            model->_device.post(obj);
			TRACE_ERR((stderr, "posted the message to generic device\n"));
            return 0;
        };

		//Collective logic for short messages
		static int dispatch_mcst_matched_shaddr(void* desc1, void* desc2 , void  *func_param )
        {
			TRACE_ERR((stderr,"descriptor matched..start the shaddr protocol\n"));

			T_Desc * my_desc = (T_Desc*) desc1;
			T_Desc * matched_desc = (T_Desc*) desc2;
			Shmem::ShmemMcstModel<T_Device, T_Desc> * model  = (Shmem::ShmemMcstModel<T_Device, T_Desc> *) func_param;
            Shmem::McstMessageShaddr<T_Device, T_Desc> * obj = (Shmem::McstMessageShaddr<T_Device, T_Desc> *) (my_desc->get_storage());

            new (obj) Shmem::McstMessageShaddr<T_Device, T_Desc> (&model->_device, my_desc, matched_desc);
            model->_device.post(obj);
			TRACE_ERR((stderr, "posted the message to generic device\n"));
            return 0;
        };

        public:

			//Shmem Multicast Model
          ShmemMcstModel (T_Device &device, pami_result_t &status) :
              Interface::MulticastModel < ShmemMcstModel<T_Device, T_Desc>, T_Device, sizeof(Shmem::McstMessage<T_Device,T_Desc>) > (device, status),
              _device(device),
			  _peer(__global.topology_local.rank2Index(__global.mapping.task())),
        	  _npeers(__global.topology_local.size())

          {
			TRACE_ERR((stderr, "registering match dispatch function\n"));
             _device.registerMatchDispatch (dispatch_mcst_matched_shmem, this, _dispatch_id_shmem);
             _device.registerMatchDispatch (dispatch_mcst_matched_shaddr, this, _dispatch_id_shaddr);
          };

          static const size_t packet_model_state_bytes          = sizeof(Shmem::McstMessage<T_Device,T_Desc>);
		  static const size_t sizeof_msg                        = sizeof(Shmem::McstMessage<T_Device,T_Desc>);
		  static const size_t short_msg_cutoff					= 128;

          inline pami_result_t postMulticast_impl(uint8_t (&state)[sizeof(Shmem::McstMessage<T_Device,T_Desc>)],
                             			          	pami_multicast_t *mcast)
          {

			//currently assume all the processes local to the node are participating
			//identify the peers by the local tasks
			TRACE_ERR((stderr, "posting multicast descriptor\n"));

			PAMI::Topology *src_topo = (PAMI::Topology *)mcast->src_participants;
			//PAMI::Topology *dst_topo = (PAMI::Topology *)mcast->dst_participants;
			/*unsigned src_local_idx, dst_local_idx;
			src_local_idx = src_topo->rank2Index(__global.mapping.task());
			dst_local_idx = dst_topo->rank2Index(__global.mapping.task());
			*/

			unsigned local_root = __global.topology_local.rank2Index(src_topo->index2Rank(0));
			//unsigned my_topo_idx = _peer; //for now my index in the group is the same as my _peer


			uint16_t	dispatch;
			if (_device.isPendingDescQueueEmpty()){

				pami_result_t res;

				if (mcast->bytes <= short_msg_cutoff){

					 res =	 _device.postMulticastShmem(mcast, local_root, _dispatch_id_shmem, (void*)state);
					dispatch = _dispatch_id_shmem;
				}
				else{
					res =	 _device.postMulticastShaddr(mcast, local_root, _dispatch_id_shaddr, (void*)state);
					dispatch = _dispatch_id_shaddr;
				}

				if (res == PAMI_SUCCESS) return PAMI_SUCCESS;

			}

			Shmem::ShmemDescMessage<T_Device,T_Desc>* desc_msg = (Shmem::ShmemDescMessage<T_Device,T_Desc>*)shmem_mcst_allocator.allocateObject ();
			new ((void*)desc_msg) Shmem::ShmemDescMessage<T_Device, T_Desc>(&_device, Shmem::ShmemMcstModel<T_Device, T_Desc>::release_desc_msg, this);

		    T_Desc & coll_desc = desc_msg->return_descriptor();
			coll_desc.set_mcast_params(mcast);
            coll_desc.set_master(local_root);
            coll_desc.set_dispatch_id(dispatch);
            coll_desc.set_storage((void*)&state);
			coll_desc.set_type(Shmem::MULTICAST);

		 	_device.postPendingDescriptor(desc_msg);

			return PAMI_SUCCESS;
          };

        static void release_desc_msg (pami_context_t context, void* cookie, pami_result_t result)
        {
            TRACE_ERR((stderr,"invoking done of the ShmemMcstModel\n"));
			//Shmem::ShmemDescMessage<T_Device,T_Desc> * obj = (Shmem::ShmemDescMessage<T_Device,T_Desc> *) cookie;
			shmem_mcst_allocator.returnObject(cookie);
            return;

         };

        protected:

          T_Device      & _device;
          uint16_t        _dispatch_id_shmem;
          uint16_t        _dispatch_id_shaddr;
          pami_context_t   _context;
   	      unsigned _peer;
          unsigned _npeers;
	      static MemoryAllocator < sizeof(Shmem::ShmemDescMessage<T_Device,T_Desc>), 16 > shmem_mcst_allocator;


      };  // PAMI::Device::Shmem::ShmemMcstModel class

	  template <class T_Device, class T_Desc>
	  MemoryAllocator < sizeof(ShmemDescMessage<T_Device,T_Desc>), 16 > ShmemMcstModel<T_Device, T_Desc>::shmem_mcst_allocator;

    };    // PAMI::Device::Shmem namespace
  };      // PAMI::Device namespace
};        // PAMI namespace
#undef TRACE_ERR
#endif // __components_devices_shmem_ShmemPacketModel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
