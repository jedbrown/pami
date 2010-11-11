/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/ShmemColorMcstModel.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_ShmemColorMcstModel_h__
#define __components_devices_shmemcoll_ShmemColorMcstModel_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"

#include "sys/pami.h"

#include "components/devices/MulticastModel.h"
#include "components/devices/ShmemCollInterface.h"
//#include "components/devices/shmemcoll/ShmemCollDevice.h"
#include "components/devices/shmemcoll/ShmemMcstMessage.h"
#include "components/devices/shmemcoll/ShaddrMcstMessage.h"
#include "components/devices/shmemcoll/ShmemDescMessage.h"

#undef TRACE_ERR

#ifndef TRACE_ERR
#define TRACE_ERR(x)  fprintf x
#endif

#define NUM_SHMEM_MCST_COLORS	10 
#define NUM_LOCAL_DST_RANKS		3 //assuming that the model is run with 4 procs/node

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

	template <class T_Device, class T_Desc>
	class ShmemColorMcstModel : public Interface::MulticastModel < ShmemColorMcstModel<T_Device,T_Desc>, T_Device, sizeof(Shmem::McstMessage<T_Device,T_Desc>) >
      {

	public:
		//Shmem Multicast Model
		ShmemColorMcstModel (T_Device &device, pami_result_t &status) :
		Interface::MulticastModel < ShmemColorMcstModel<T_Device, T_Desc>, T_Device, sizeof(Shmem::McstMessage<T_Device,T_Desc>) > (device, status),
		_device(device),
		_peer(__global.topology_local.rank2Index(__global.mapping.task())),
		_npeers(__global.topology_local.size())

		{

		//create my descriptors
		int rc;
		rc = posix_memalign((void**)&_my_desc_array, 32, sizeof(T_Desc)*NUM_SHMEM_MCST_COLORS);
		PAMI_assertf(rc == 0, "posix_memalign failed for ShmemCollMcstModel errno=%d\n", errno);

		//create shared descriptors
		__global.mm.memalign((void**)&_shared_desc_array, 32, sizeof(T_Desc)*NUM_SHMEM_MCST_COLORS);
		assert(_shared_desc_array != NULL);

		if (_peer == 0) //only one rank inits the shared buffers
		{
			for(unsigned i =0; i < NUM_SHMEM_MCST_COLORS; i++)
				_shared_desc_array[i].init(&__global.mm);				

		}

		__collectiveQ = (Shmem::SendQueue *) malloc ((sizeof (Shmem::SendQueue)));
       new (__collectiveQ) Shmem::SendQueue (device);

		void* dummy;
		__global.mm.memalign((void**)&dummy, 16, sizeof(size_t)*NUM_SHMEM_MCST_COLORS);
		printf("before\n");
		 local_barriered_shmemzero((void *)dummy, sizeof(size_t)*NUM_SHMEM_MCST_COLORS, _npeers, _peer == 0);
		printf("after\n");

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

	PAMI::Topology *src_topo = (PAMI::Topology *)mcast->src_participants;
	PAMI::Topology *dst_topo = (PAMI::Topology *)mcast->dst_participants;
	size_t num_dst_ranks =0;
	/*if (dst_topo != NULL)
		num_dst_ranks = dst_topo->size();*/
	PAMI_assert (dst_topology->type() == PAMI_AXIAL_TOPOLOGY);

	mi_coord_t *ll=NULL;
    pami_coord_t *ur=NULL;
    pami_coord_t *ref=NULL;
    unsigned char *isTorus=NULL;

    pami_result_t result = PAMI_SUCCESS;
    result = dst_topology->axial(&ll, &ur, &ref, &isTorus);
    PAMI_assert(result == PAMI_SUCCESS);
	num_dst_ranks =  ll->u.n_torus.coords[LOCAL_DIM] - ur->u.n_torus.coords[LOCAL_DIM] + 1;
	
	TRACE_ERR((stderr, "size of destination topology:%zu\n", num_dst_ranks));

	unsigned conn_id = mcast->connection_id;
	TRACE_ERR((stderr, "conn_id:%u\n", conn_id));

	//unsigned local_root = __global.topology_local.rank2Index(src_topo->index2Rank(0));
	//unsigned my_topo_idx = _peer; //for now my index in the group is the same as my _peer

	T_Desc *my_desc=NULL, *master_desc=NULL;

	my_desc = &_my_desc_array[conn_id]; 
	master_desc = &_shared_desc_array[conn_id];

	my_desc->set_mcast_params(mcast);

	//? Not setting this..is master index used elsewhere
	//my_desc->set_master(local_root); 
	if (src_topo != NULL)
		my_desc->set_master(1);
	else
		my_desc->set_master(0);

	if (src_topo != NULL){ // I am the master
			Memregion memregion;
			void* buf = (void*) master_desc->get_buffer();
			void* mybuf = ((PAMI::PipeWorkQueue *)mcast->src)->bufferToConsume();
			size_t bytes_out;
			
			//? Have to create the entire memregion at once..does mcast->bytes include all the data bytes
			memregion.createMemregion(&bytes_out, mcast->bytes, mybuf, 0);
			void* phy_addr = (void*)memregion.getBasePhysicalAddress();
			void * global_vaddr = NULL;
			uint32_t rc = 0;
			rc = Kernel_Physical2GlobalVirtual (phy_addr, &global_vaddr);
			assert(rc == 0);

			memcpy(buf, &global_vaddr, sizeof(global_vaddr));
			TRACE_ERR((stderr,"copied global_vaddr:%p to %p \n", global_vaddr, buf));
			TRACE_ERR((stderr,"done processes:%u \n", master_desc->done_peers()));
			master_desc->set_consumers(num_dst_ranks);
			master_desc->set_state(Shmem::INIT);
	}

	Shmem::McstMessageShaddr<T_Device, T_Desc> * obj = (Shmem::McstMessageShaddr<T_Device, T_Desc> *) (&state[0]);
	new (obj) Shmem::McstMessageShaddr<T_Device, T_Desc> (&_device, my_desc, master_desc);
	//_device.post(obj);
	__collectiveQ->post(obj);
	/*PAMI::Device::Generic::GenericThread *work = new (&_shmem_work[conn_id]) 
					PAMI::Device::Generic::GenericThread (Shmem::McstMessageShaddr<T_Device,T_Desc>::advanceMcstShaddr, obj);
    _device.postThread(work);*/

	return PAMI_SUCCESS;

  };


protected:

  T_Device      & _device;
  pami_context_t   _context;
  unsigned _peer;
  unsigned _npeers;


  T_Desc 	*_my_desc_array;
  T_Desc	*_shared_desc_array;		
  pami_work_t	_shmem_work[NUM_SHMEM_MCST_COLORS];

  Shmem::SendQueue    *__collectiveQ;


      };  // PAMI::Device::Shmem::ShmemColorMcstModel class


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
