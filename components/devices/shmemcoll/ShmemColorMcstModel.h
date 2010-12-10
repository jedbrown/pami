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

#include "util/trace.h"

#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

#define NUM_SHMEM_MCST_COLORS	16
//#define NUM_LOCAL_DST_RANKS		3 //assuming that the model is run with 4 procs/node

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

      template <class T_Device, class T_Desc>
      class ShmemColorMcstModel : public Interface::MulticastModel < ShmemColorMcstModel<T_Device, T_Desc>, T_Device, sizeof(Shmem::McstMessage<T_Device, T_Desc>) >
      {

        public:
          //Shmem Multicast Model
          ShmemColorMcstModel (T_Device &device, pami_result_t &status) :
              Interface::MulticastModel < ShmemColorMcstModel<T_Device, T_Desc>, T_Device, sizeof(Shmem::McstMessage<T_Device, T_Desc>) > (device, status),
              _device(device),
              _mytask(__global.mapping.task()),
              _peer(__global.topology_local.rank2Index(_mytask)),
              _npeers(__global.topology_local.size())

          {

            pami_result_t rc;
            char key[PAMI::Memory::MMKEYSIZE];

            //create my descriptors
            strcpy(key, "/ShmemColorMcstModel-my_desc_array");
            rc = __global.heap_mm->memalign((void**) & _my_desc_array, 32, sizeof(T_Desc) * NUM_SHMEM_MCST_COLORS); //, key, (PAMI::Memory::MM_INIT_FN*)NULL,NULL);
            //PAMI_assertf(rc == PAMI_SUCCESS, "alloc failed for ShmemCollMcstModel errno=%d\n", errno);
            if(rc != PAMI_SUCCESS) 
            {
              status = PAMI_ENOMEM;
              return;
            }


            //create my collective queue
            strcpy(key, "/ShmemColorMcstModel-collectiveQ");
            rc = __global.heap_mm->memalign((void **) & __collectiveQ, 0, sizeof(*__collectiveQ), key, cq_initialize, &_device);
            //PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc __collectiveQ");
            if(rc != PAMI_SUCCESS) 
            {
              status = PAMI_ENOMEM;
              return;
            }


            //create shared descriptors
            sprintf(key, "/ShmemColorMcstModel-shared_desc_array-client-%2.2zu-context-%2.2zu", _device.clientId(), _device.contextId());
            rc = __global.mm.memalign((void**) & _shared_desc_array, 32, sizeof(T_Desc) * NUM_SHMEM_MCST_COLORS, key, desc_initialize, NULL);
            //PAMI_assertf(rc == PAMI_SUCCESS, "alloc failed for ShmemCollMcstModel errno=%d\n", errno);
            if(rc != PAMI_SUCCESS) 
            {
              status = PAMI_ENOMEM;
              return;
            }


          };

          static const size_t packet_model_state_bytes          = sizeof(Shmem::McstMessage<T_Device, T_Desc>);
          static const size_t sizeof_msg                        = sizeof(Shmem::McstMessage<T_Device, T_Desc>);
          static const size_t short_msg_cutoff                  = 512;


          inline pami_result_t postMulticast_impl(uint8_t (&state)[sizeof(Shmem::McstMessage<T_Device, T_Desc>)],
                                                  pami_multicast_t *mcast, void* devinfo)
          {
            TRACE_FN_ENTER();

            /* increment the seq_num to compare with master's */

            Shmem::McstMessageShaddr<T_Device, T_Desc>::seq_num++;

            PAMI::Topology *src_topo = (PAMI::Topology *)mcast->src_participants;
            PAMI::Topology *dst_topo = (PAMI::Topology *)mcast->dst_participants;
            size_t num_dst_ranks = 0;

            unsigned conn_id = mcast->connection_id;
            TRACE_FORMAT("conn_id:%u", conn_id);

            T_Desc *my_desc = NULL, *master_desc = NULL;

            my_desc = &_my_desc_array[conn_id];
            master_desc = &_shared_desc_array[conn_id];

            my_desc->set_mcast_params(mcast);

            if ((src_topo != NULL)  && (src_topo->index2Rank(0) == _mytask)) //am the master/root
              {
                if (dst_topo != NULL)
                  {
                    PAMI_assert (dst_topo->type() == PAMI_AXIAL_TOPOLOGY);
                    pami_coord_t *ll = NULL;
                    pami_coord_t *ur = NULL;
                    pami_coord_t *ref = NULL;
                    unsigned char *isTorus = NULL;

                    pami_result_t result = PAMI_SUCCESS;
                    result = dst_topo->axial(&ll, &ur, &ref, &isTorus);
                    PAMI_assert(result == PAMI_SUCCESS);
                    num_dst_ranks =  ur->u.n_torus.coords[LOCAL_DIM] - ll->u.n_torus.coords[LOCAL_DIM];
                    TRACE_FORMAT("num_dst_ranks:%zu", num_dst_ranks);
                    my_desc->set_master(1);
                  }
              }
            else
              {
                my_desc->set_master(0);
              }

            if ((src_topo != NULL)  && (src_topo->index2Rank(0) == _mytask)) //am the master/root
              { // I am the master
                Memregion memregion;
                //void* buf = (void*) master_desc->get_buffer();
                Shmem::McstControl* mcst_control = (Shmem::McstControl*) master_desc->get_buffer();
                void* mybuf = ((PAMI::PipeWorkQueue *)mcast->src)->bufferToConsume();
                size_t bytes_out;

                //? Have to create the entire memregion at once..does mcast->bytes include all the data bytes
                memregion.createMemregion(&bytes_out, mcast->bytes, mybuf, 0);
                PAMI_assert(bytes_out == mcast->bytes);
                void* phy_addr = (void*)memregion.getBasePhysicalAddress();
                void * global_vaddr = NULL;
                uint32_t rc = 0;
                rc = Kernel_Physical2GlobalVirtual (phy_addr, &global_vaddr);
                PAMI_assert(rc == 0);
                TRACE_FORMAT("buffer to consume:va:%p ga:%p", mybuf, global_vaddr);

                //memcpy(buf, &global_vaddr, sizeof(global_vaddr));
                mcst_control->glob_src_buffer = global_vaddr;
                //TRACE_FORMAT("copied global_vaddr:%p to %p", global_vaddr, buf);
                TRACE_FORMAT("done processes:%u", master_desc->done_peers());
                master_desc->set_consumers(num_dst_ranks);
                master_desc->reset_master_done();
                mem_barrier();
                //master_desc->set_seq_id(Shmem::McstMessageShaddr<T_Device, T_Desc>::seq_num);
                master_desc->set_state(Shmem::INIT);
              }

            Shmem::McstMessageShaddr<T_Device, T_Desc> * obj = (Shmem::McstMessageShaddr<T_Device, T_Desc> *) (&state[0]);
            new (obj) Shmem::McstMessageShaddr<T_Device, T_Desc> (&_device, my_desc, master_desc);
            //_device.post(obj);
            __collectiveQ->post(obj);
            /*PAMI::Device::Generic::GenericThread *work = new (&_shmem_work[conn_id])
            				PAMI::Device::Generic::GenericThread (Shmem::McstMessageShaddr<T_Device,T_Desc>::advanceMcstShaddr, obj);
              _device.postThread(work);*/

            TRACE_FN_EXIT();
            return PAMI_SUCCESS;

          };


        protected:

          T_Device      & _device;
          pami_context_t   _context;
          pami_task_t _mytask;
          unsigned _peer;
          unsigned _npeers;


          T_Desc 	*_my_desc_array;
          T_Desc	*_shared_desc_array;
          //pami_work_t	_shmem_work[NUM_SHMEM_MCST_COLORS];

          Shmem::SendQueue    *__collectiveQ;

          static unsigned seq_num;

          ///
          /// \brief Initialize the memory/descriptor array
          ///
          /// \see PAMI::Memory::MM_INIT_FN
          ///
          static void desc_initialize(void       * memory,
                                      size_t       bytes,
                                      const char * key,
                                      unsigned     attributes,
                                      void       * cookie)
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT("%s", key);
            T_Desc	*shared_desc_array = (T_Desc	*)memory;
            PAMI_assert(shared_desc_array != NULL);

            for (unsigned i = 0; i < NUM_SHMEM_MCST_COLORS; i++)
              {
                char skey[PAMI::Memory::MMKEYSIZE];
                sprintf(skey, "%s-%2.2u", key, i);
                TRACE_FORMAT("%s", skey);
                shared_desc_array[i].init(&__global.mm, skey);
              };

            TRACE_FN_EXIT();
          };

          ///
          /// \brief Initialize the collective queue
          ///
          /// \see PAMI::Memory::MM_INIT_FN
          ///
          static void cq_initialize(void       * memory,
                                    size_t       bytes,
                                    const char * key,
                                    unsigned     attributes,
                                    void       * cookie)
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT("%s", key);
            T_Device  *device = (T_Device  *)cookie;
            Shmem::SendQueue *collectiveQ = (Shmem::SendQueue*)memory;
            PAMI_assert(collectiveQ != NULL);
            new (collectiveQ) Shmem::SendQueue ();
            collectiveQ->init(device);
            TRACE_FN_EXIT();
          };

      };  // PAMI::Device::Shmem::ShmemColorMcstModel class



    };    // PAMI::Device::Shmem namespace
  };      // PAMI::Device namespace
};        // PAMI namespace

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif // __components_devices_shmem_ShmemPacketModel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
