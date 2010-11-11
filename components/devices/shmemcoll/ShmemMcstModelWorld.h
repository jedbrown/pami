/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/ShmemMcstModelWorld.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_ShmemMcstModelWorld_h__
#define __components_devices_shmemcoll_ShmemMcstModelWorld_h__

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

#undef TRACE_ERR

#ifndef TRACE_ERR
#define TRACE_ERR(x)  fprintf x
#endif

#define SHORT_MCST_OPT

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

	template <class T_Device, class T_Desc>
	class ShmemMcstModelWorld : public Interface::MulticastModel < ShmemMcstModelWorld<T_Device,T_Desc>, T_Device, sizeof(Shmem::McstMessage<T_Device,T_Desc>) >
      {

	public:
		//Shmem Multicast Model
		ShmemMcstModelWorld (T_Device &device, pami_result_t &status) :
		Interface::MulticastModel < ShmemMcstModelWorld<T_Device, T_Desc>, T_Device, sizeof(Shmem::McstMessage<T_Device,T_Desc>) > (device, status),
		_device(device),
		_peer(__global.topology_local.rank2Index(__global.mapping.task())),
		_npeers(__global.topology_local.size())

		{
			TRACE_ERR((stderr, "registering match dispatch function\n"));
		};

          static const size_t packet_model_state_bytes          = sizeof(Shmem::McstMessage<T_Device,T_Desc>);
		  static const size_t sizeof_msg                        = sizeof(Shmem::McstMessage<T_Device,T_Desc>);
		  static const size_t short_msg_cutoff                  = 512;


  inline pami_result_t postMulticast_impl(uint8_t (&state)[sizeof(Shmem::McstMessage<T_Device,T_Desc>)],
                     			          	pami_multicast_t *mcast, void* devinfo)
  {

	//currently assume all the processes local to the node are participating
	//identify the peers by the local tasks
	TRACE_ERR((stderr, "posting multicast descriptor\n"));
	fprintf(stderr, "posting multicast descriptor from the model\n");

	PAMI::Topology *src_topo = (PAMI::Topology *)mcast->src_participants;
	PAMI::Topology *dst_topo = (PAMI::Topology *)mcast->dst_participants;
	size_t num_dst_ranks = dst_topo->size();
	TRACE_ERR((stderr, "size of destination topology:%zu\n", num_dst_ranks));

	unsigned local_root = __global.topology_local.rank2Index(src_topo->index2Rank(0));
	//unsigned my_topo_idx = _peer; //for now my index in the group is the same as my _peer

	T_Desc *my_desc=NULL, *master_desc=NULL;

	if (_device.isPendingDescQueueEmpty()){
		pami_result_t res =	 _device.getShmemWorldDesc(&my_desc, &master_desc, local_root);
		 while (res != PAMI_SUCCESS)
        {
             res =   _device.getShmemWorldDesc(&my_desc, &master_desc, local_root);
             _device.advance();
        }

		if (res == PAMI_SUCCESS)
		{
			TRACE_ERR((stderr,"local_root%u my_local_rank:%u\n", local_root, _peer));

			if (mcast->bytes <= Shmem::McstMessage<T_Device, T_Desc>::short_msg_cutoff)
			{
				if (local_root == _peer){
					void* buf = (void*) my_desc->get_buffer(local_root);
					void* mybuf = ((PAMI::PipeWorkQueue *)mcast->src)->bufferToConsume();
					memcpy(buf, mybuf, mcast->bytes);
					TRACE_ERR((stderr,"copied bytes:%zu from %p to %p data[0]:%u\n", mcast->bytes, mybuf, buf, ((unsigned*)buf)[0]));
					((PAMI::PipeWorkQueue *)mcast->src)->consumeBytes(mcast->bytes);
					 my_desc->set_consumers(num_dst_ranks);
				}
				my_desc->set_state(Shmem::INIT);
#ifdef SHORT_MCST_OPT
				if (local_root == _peer)
					 master_desc->signal_flag();

				res = Shmem::McstMessageShmem<T_Device, T_Desc>::short_msg_advance(master_desc, mcast, local_root);
			  	if (res == PAMI_SUCCESS)    //signal inline completion
                {
                     mcast->cb_done.function(_context, mcast->cb_done.clientdata, PAMI_SUCCESS);
                     my_desc->set_state(Shmem::DONE);
                }
                return PAMI_SUCCESS;

#else
				my_desc->set_mcast_params(mcast);
				my_desc->set_master(local_root);
				Shmem::McstMessageShmem<T_Device, T_Desc> * obj = (Shmem::McstMessageShmem<T_Device, T_Desc> *) (&state[0]);
				new (obj) Shmem::McstMessageShmem<T_Device, T_Desc> (&_device, my_desc, master_desc);
    			_device.post(obj);
#endif
			}
			else
			{
					my_desc->set_mcast_params(mcast);
					my_desc->set_master(local_root);
					if (local_root == _peer){
						Memregion memregion;
						void* buf = (void*) my_desc->get_buffer(local_root);
						void* mybuf = ((PAMI::PipeWorkQueue *)mcast->src)->bufferToConsume();
						size_t bytes_out;
						memregion.createMemregion(&bytes_out, mcast->bytes, mybuf, 0);
						void* phy_addr = (void*)memregion.getBasePhysicalAddress();
						void * global_vaddr = NULL;
						uint32_t rc = 0;
						rc = Kernel_Physical2GlobalVirtual (phy_addr, &global_vaddr);
						assert(rc == 0);

						memcpy(buf, &global_vaddr, sizeof(global_vaddr));
						TRACE_ERR((stderr,"copied global_vaddr:%p to %p \n", global_vaddr, buf));
						my_desc->set_consumers(num_dst_ranks);
						my_desc->set_state(Shmem::INIT);
					}

				Shmem::McstMessageShaddr<T_Device, T_Desc> * obj = (Shmem::McstMessageShaddr<T_Device, T_Desc> *) (&state[0]);
				new (obj) Shmem::McstMessageShaddr<T_Device, T_Desc> (&_device, my_desc, master_desc);
   				_device.post(obj);

			}

			return PAMI_SUCCESS;
		}
	}

	TRACE_ERR((stderr,"Creating descriptor message for retrying the collective\n"));
	Shmem::ShmemDescMessageWorld<T_Device,T_Desc>* desc_msg = (Shmem::ShmemDescMessageWorld<T_Device,T_Desc>*)shmem_mcst_world_allocator.allocateObject ();
	new ((void*)desc_msg) Shmem::ShmemDescMessageWorld<T_Device, T_Desc>(&_device, Shmem::ShmemMcstModelWorld<T_Device, T_Desc>::release_desc_msg, this,_peer);

	T_Desc & coll_desc = desc_msg->return_descriptor();

	coll_desc.set_mcast_params(mcast);
	coll_desc.set_master(local_root);
	coll_desc.set_storage((void*)&state);
	coll_desc.set_type(Shmem::MULTICAST);

	_device.post(desc_msg);
	TRACE_ERR((stderr,"Posted the descriptor message for retrying the collective\n"));

	return PAMI_SUCCESS;

  };

static void release_desc_msg (pami_context_t context, void* cookie, pami_result_t result)
{
    TRACE_ERR((stderr,"invoking done of the ShmemMcstModelWorld\n"));
	//Shmem::ShmemDescMessage<T_Device,T_Desc> * obj = (Shmem::ShmemDescMessage<T_Device,T_Desc> *) cookie;
	shmem_mcst_world_allocator.returnObject(cookie);
    return;

 };

protected:

  T_Device      & _device;
  pami_context_t   _context;
  unsigned _peer;
  unsigned _npeers;
	      static MemoryAllocator < sizeof(Shmem::ShmemDescMessage<T_Device,T_Desc>), 16 > shmem_mcst_world_allocator;


      };  // PAMI::Device::Shmem::ShmemMcstModelWorld class

	  template <class T_Device, class T_Desc>
	  MemoryAllocator < sizeof(ShmemDescMessage<T_Device,T_Desc>), 16 > ShmemMcstModelWorld<T_Device, T_Desc>::shmem_mcst_world_allocator;

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
