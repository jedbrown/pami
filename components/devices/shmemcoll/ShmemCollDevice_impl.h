/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/ShmemCollDevice_impl.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_ShmemCollDevice_impl_h__
#define __components_devices_shmemcoll_ShmemCollDevice_impl_h__

#include "Global.h"
#include "common/bgq/Memregion.h"
#include "assert.h"

#include "components/devices/MulticastModel.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MulticombineModel.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    template <class T_Desc>
    pami_result_t ShmemCollDevice<T_Desc>::init (size_t clientid,
                                            size_t contextid,
                                            pami_client_t     client,
                                            pami_context_t    context,
                                            PAMI::Memory::MemoryManager *mm,
                                            PAMI::Device::Generic::Device * progress)
    {
      TRACE_ERR((stderr, " (%zu) >> ShmemCollDevice::init ()  \n", __global.mapping.task()));
      _client   = client;
      _context  = context;
      _contextid  = contextid;
      _mm = mm;
      _progress = progress;

      unsigned i;
      __global.mapping.nodePeers (_num_procs);

      PAMI::Interface::Mapping::nodeaddr_t nodeaddr;
      __global.mapping.nodeAddr (nodeaddr);
      _global_task = __global.mapping.task();//nodeaddr.global;
      _local_task  = nodeaddr.local;
      TRACE_ERR((stderr, "(%zu) ShmemCollDevice::global_task:%zu local_task:%zu () << \n", __global.mapping.task(), _global_task, _local_task));
#ifdef __bgq__
      //unsigned stride = 16 / _num_procs; //hack
      //_local_task = _local_task / stride;//hack
#endif

      new (_my_desc_fifo) Shmem::ShmemCollDescFifo<T_Desc>(mm, clientid, contextid);
      new (_my_world_desc_fifo) Shmem::ShmemCollDescFifo<T_Desc>(mm, clientid, contextid);
      // barrier ?

		//delay
	  /*unsigned long long delay = 100000, t0, t1;
	  t0 = __global.time.timebase();
      while ((t1 = __global.time.timebase()) - t0 < delay);
		*/

		TRACE_ERR((stderr,"spin waiting\n"));
		for (unsigned i =0; i < 10000000; i++){}

	pami_result_t rc;
	rc = __global.heap_mm->memalign((void **)&__collectiveQ, 0, sizeof(*__collectiveQ));
	PAMI_assertf(rc == PAMI_SUCCESS, "alloc failed for __collectiveQ");
       new (__collectiveQ) Shmem::SendQueue (Generic::Device::Factory::getDevice (progress, 0, contextid));

	rc = __global.heap_mm->memalign((void **)&__pending_descriptorQ, 0, sizeof(*__pending_descriptorQ));
	PAMI_assertf(rc == PAMI_SUCCESS, "alloc failed for __pending_descriptorQ");
       new (__pending_descriptorQ) Shmem::SendQueue (Generic::Device::Factory::getDevice (progress, 0, contextid));

      for (i = 0; i < MATCH_DISPATCH_SIZE; i++)
        {
          _dispatch[i].function   = NULL;
          _dispatch[i].clientdata = NULL;
        }

      TRACE_ERR((stderr, "(%zu) ShmemCollDevice::init () << \n", __global.mapping.task()));
      return PAMI_SUCCESS;
    }

    template <class T_Desc>
    bool ShmemCollDevice<T_Desc>::isInit_impl ()
    {
      return true;
    }

    /// \see PAMI::Device::Interface::BaseDevice::peers()
    template <class T_Desc>
    size_t ShmemCollDevice<T_Desc>::peers_impl ()
    {
      return _num_procs;
    }

    /// \see PAMI::Device::Interface::BaseDevice::task2peer()
    template <class T_Desc>
    size_t ShmemCollDevice<T_Desc>::task2peer_impl (size_t task)
    {
      PAMI::Interface::Mapping::nodeaddr_t address;
      TRACE_ERR((stderr, ">> ShmemCollDevice::task2peer_impl(%zu)\n", task));
      __global.mapping.task2node (task, address);
      TRACE_ERR((stderr, "   ShmemCollDevice::task2peer_impl(%zu), address = {%zu, %zu}\n", task, address.global, address.local));

      size_t peer = 0;
      __global.mapping.node2peer (address, peer);
      TRACE_ERR((stderr, "<< ShmemCollDevice::task2peer_impl(%zu), peer = %zu\n", task, peer));

#ifdef __bgq__
      return task; //hack
#else
      return peer;
#endif
    }

    /// \see PAMI::Device::Interface::BaseDevice::isPeer()
    template <class T_Desc>
    bool ShmemCollDevice<T_Desc>::isPeer_impl (size_t task)
    {
      return __global.mapping.isPeer(task, _global_task);
    };

    ///
    /// \brief Regieter the receive function to dispatch when a packet arrives.
    ///
    /// \param[in] id              Dispatch set identifier
    /// \param[in] match_func       Receive function to dispatch
    /// \param[in] recv_func_parm  Receive function client data
    ///
    /// \return Dispatch id for this registration
    ///
    template <class T_Desc>
    pami_result_t ShmemCollDevice<T_Desc>::registerMatchDispatch ( Interface::MatchFunction_t   match_func,
																	void                      * recv_func_parm,
                                                             		uint16_t                  & id)
    {
      TRACE_ERR((stderr, ">> (%zu) ShmemCollDevice::registerMatchDispatch\n", __global.mapping.task()));


      // Find the next available id for this dispatch set.
      bool found_free_slot = false;

      for (id = 0; id < MATCH_DISPATCH_SIZE; id++)
        {

          if (_dispatch[id].function == NULL)
            {
              found_free_slot = true;
              break;
            }
        }

      if (!found_free_slot) return PAMI_ERROR;

      _dispatch[id].function   = match_func;
      _dispatch[id].clientdata = recv_func_parm;

      TRACE_ERR((stderr, "<< (%zu) ShmemCollDevice::registerRecvFunction() => %d\n", __global.mapping.task(), id));
      return PAMI_SUCCESS;
    };

    template <class T_Desc>
    pami_result_t ShmemCollDevice<T_Desc>::postCollective (PAMI::PipeWorkQueue *src, PAMI::PipeWorkQueue *dst, size_t bytes, pami_callback_t cb_done,
															unsigned conn_id, unsigned master, uint16_t dispatch_id, void* state)
    {
      TRACE_ERR((stderr, ">> (%zu) ShmemCollDevice::postCollective(%p, %p, %zu, %u, %u, %p)\n", __global.mapping.task(), src, dst, bytes,conn_id,master, state));

		T_Desc* coll_desc = _my_desc_fifo->fetch_descriptor();
		if (coll_desc != NULL){
			coll_desc->set_src_pwq(src);
			coll_desc->set_recv_pwq(dst);
			coll_desc->set_bytes(bytes);
			coll_desc->set_cbdone(cb_done);
			coll_desc->set_conn_id(conn_id);
			coll_desc->set_master(master);
			coll_desc->set_dispatch_id(dispatch_id);
			coll_desc->set_storage((void*)&state);
			coll_desc->set_state(Shmem::INIT);
      		return PAMI_SUCCESS;
		}
		else{
			printf("descriptor not found\n");
			return PAMI_EAGAIN;
		}

    };

    template <class T_Desc>
    pami_result_t ShmemCollDevice<T_Desc>::postMulticastShmem ( pami_multicast_t * mcast, unsigned master, uint16_t dispatch_id, void* state)
    {
      TRACE_ERR((stderr, ">> (%zu) ShmemCollDevice::postMulticastShmem(  %u, %p)\n", __global.mapping.task(), master, state));

		T_Desc* coll_desc = _my_desc_fifo->fetch_descriptor();
		if (coll_desc != NULL){

			coll_desc->set_mcast_params(mcast);
			coll_desc->set_master(master);
			coll_desc->set_dispatch_id(dispatch_id);
			coll_desc->set_storage((void*)state);

			TRACE_ERR((stderr,"master:%u local_task:%zu\n", master, _local_task));

			//short multicast and if master, copy the data inline into the descriptor
			if (master == _local_task){
				void* buf = (void*) coll_desc->get_buffer(master);
				void* mybuf = ((PAMI::PipeWorkQueue*)mcast->src)->bufferToConsume();
				memcpy(buf, mybuf, mcast->bytes);
				TRACE_ERR((stderr,"copied bytes:%zu from %p to %p data[0]:%u\n", mcast->bytes, mybuf, buf, ((unsigned*)buf)[0]));
				((PAMI::PipeWorkQueue*)mcast->src)->consumeBytes(mcast->bytes);
			}

			coll_desc->set_state(Shmem::INIT);
		return PAMI_SUCCESS;
		}
		else{
			TRACE_ERR((stderr,"descriptor not found\n"));
			return PAMI_EAGAIN;
		}

    };

    template <class T_Desc>
    pami_result_t ShmemCollDevice<T_Desc>::postMulticastShaddr ( pami_multicast_t * mcast,  unsigned master, uint16_t dispatch_id, void* state)
    {
      TRACE_ERR((stderr, ">> (%zu) ShmemCollDevice::postMulticastShaddr(%zu,  %u, %p)\n", __global.mapping.task(), mcast->bytes, master, state));

		Memregion memregion(_context);

		T_Desc* coll_desc = _my_desc_fifo->fetch_descriptor();
		if (coll_desc != NULL){

				coll_desc->set_mcast_params(mcast);
				coll_desc->set_master(master);
				coll_desc->set_dispatch_id(dispatch_id);
				coll_desc->set_storage((void*)state);

			TRACE_ERR((stderr,"master:%u local_task:%zu\n", master, _local_task));

			//short multicast and if master, copy the data inline into the descriptor
			if (master == _local_task){
				void* buf = (void*) coll_desc->get_buffer(master);
				void* mybuf = ((PAMI::PipeWorkQueue*)mcast->src)->bufferToConsume();
				size_t bytes_out;
				memregion.createMemregion(&bytes_out, mcast->bytes, mybuf, 0);
				void* phy_addr = memregion.getBasePhysicalAddress();
				void * global_vaddr = NULL;
				uint32_t rc = 0;
				rc = Kernel_Physical2GlobalVirtual (phy_addr, &global_vaddr);
				assert(rc == 0);

				memcpy(buf, &global_vaddr, sizeof(global_vaddr));
				TRACE_ERR((stderr,"copied global_vaddr:%p to %p \n", global_vaddr, buf));
				((PAMI::PipeWorkQueue*)mcast->src)->consumeBytes(mcast->bytes);
			}

			coll_desc->set_state(Shmem::INIT);
      		return PAMI_SUCCESS;
		}
		else{
			TRACE_ERR((stderr,"descriptor not found\n"));
			return PAMI_EAGAIN;
		}

    };

	//Get back to this again ??
    template <class T_Desc>
    pami_result_t ShmemCollDevice<T_Desc>::postDescriptor (T_Desc & desc)
    {
#if 0
		T_Desc* coll_desc = _my_desc_fifo->fetch_descriptor();
		if (coll_desc != NULL){
	        coll_desc->set_src_pwq(desc.get_src_pwq());
			coll_desc->set_recv_pwq(desc.get_recv_pwq());
			coll_desc->set_bytes(desc.get_bytes());
			coll_desc->set_cbdone(desc.get_cbdone());
			coll_desc->set_conn_id(desc.get_conn_id());
			coll_desc->set_master(desc.get_master());
			coll_desc->set_dispatch_id(desc.get_dispatch_id());
			coll_desc->set_storage((void*)desc.get_storage());

			TRACE_ERR((stderr,"master:%u local_task:%zu\n", desc.get_master(), _local_task));
			//short multicast and if master, copy the data inline into the descriptor
			if (desc.get_master() == _local_task){
				void* buf = (void*) coll_desc->get_buffer(desc.get_master());
				void* mybuf = desc.get_src_pwq()->bufferToConsume();
				memcpy(buf, mybuf, desc.get_bytes());
				TRACE_ERR((stderr,"copied bytes:%u from %p to %p data[0]:%u\n", desc.get_bytes(), mybuf, buf, ((unsigned*)buf)[0]));
				desc.get_src_pwq()->consumeBytes(desc.get_bytes());
			}

			coll_desc->set_state(Shmem::INIT);
      		return PAMI_SUCCESS;
		}
		else{
			TRACE_ERR((stderr,"descriptor not found\n"));
			return PAMI_EAGAIN;
		}
#endif
			return PAMI_EAGAIN;
    };

    template <class T_Desc>
    pami_result_t ShmemCollDevice<T_Desc>::getShmemWorldDesc(T_Desc** my_desc, T_Desc** master_desc, unsigned master)
	{

		TRACE_ERR((stderr,">> getShmemWorldDesc master:%u\n",master));
		unsigned desc_index;
		T_Desc* next_free_desc = _my_world_desc_fifo->next_free_descriptor(desc_index);
		//assert(next_free_desc != NULL);

		if (likely(next_free_desc != NULL)){
			TRACE_ERR((stderr,"getShmemWorldDesc: found next_free_desc:\n"));
			uint64_t	next_seq_id = next_free_desc->get_seq_id();
			TRACE_ERR((stderr,"desc_index:%u next_seq_id:%ld\n",desc_index, next_seq_id));
			T_Desc* desc = _all_world_desc_fifos[master].get_descriptor_by_idx(desc_index);

			//assert(next_seq_id == desc->get_seq_id());
			if (likely(next_seq_id == desc->get_seq_id())){
				*my_desc = _my_world_desc_fifo->fetch_descriptor();
				*master_desc = desc;
				TRACE_ERR((stderr,"Found descriptor pair \n"));
				return PAMI_SUCCESS;
			}
		}

		return PAMI_EAGAIN;
	}

    template <class T_Desc>
    pami_result_t ShmemCollDevice<T_Desc>::post (Shmem::SendQueue::Message * msg)
    {
      TRACE_ERR((stderr, ">> (%zu) ShmemCollDevice::post(%p)\n", __global.mapping.task(), msg));
      /*msg->setup (_progress, __collectiveQ);
      msg->postNext(true);
      TRACE_ERR((stderr, "<< (%zu) ShmemCollDevice::post(%p)\n", __global.mapping.task(), msg));*/
	 __collectiveQ->post(msg);
      return PAMI_SUCCESS;
    };

    template <class T_Desc>
    pami_result_t ShmemCollDevice<T_Desc>::postPendingDescriptor (Shmem::SendQueue::Message * msg)
    {
      TRACE_ERR((stderr, ">> (%zu) ShmemCollDevice::post(%p)\n", __global.mapping.task(), msg));
      msg->setup (_progress, __pending_descriptorQ);
	  msg->postNext(true);
	/*
	  if (this->isEmpty()){
		  msg->postNext(true);
	  }
	  this->enqueue (msg);
		*/
      TRACE_ERR((stderr, "<< (%zu) ShmemCollDevice::post(%p)\n", __global.mapping.task(), msg));
      return PAMI_SUCCESS;
    };

  };
};
#undef TRACE_ERR

#endif

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
