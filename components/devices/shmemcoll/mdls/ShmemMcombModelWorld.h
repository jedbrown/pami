/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/ShmemMcombModelWorld.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_mdls_ShmemMcombModelWorld_h__
#define __components_devices_shmemcoll_mdls_ShmemMcombModelWorld_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"

#include "sys/pami.h"

#include "components/devices/MulticombineModel.h"
#include "components/devices/ShmemCollInterface.h"
#include "components/devices/shmemcoll/ShmemCollDevice.h"
#include "components/devices/shmemcoll/msgs/ShortMcombMessage.h"
//#include "components/devices/shmemcoll/ShaddrMcombMessage.h"
//#include "components/devices/shmemcoll/ShmemDescMessage.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif
//#define SHORT_MCOMB_BLOCKING
namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

      template <class T_Device, class T_Desc>
        class ShmemMcombModelWorld : public Interface::MulticombineModel < ShmemMcombModelWorld<T_Device, T_Desc>, T_Device, sizeof(Shmem::ShortMcombMessage<T_Device, T_Desc>) >
      {
        public:

          //Shmem Multicombine Model
          ShmemMcombModelWorld (T_Device &device, pami_result_t &status) :
            Interface::MulticombineModel < ShmemMcombModelWorld<T_Device, T_Desc>, T_Device, sizeof(Shmem::ShortMcombMessage<T_Device, T_Desc>) > (device, status),
            _device(device),
            _localrank(__global.topology_local.rank2Index(__global.mapping.task())),
            _npeers(__global.topology_local.size())

        { };


          static const size_t packet_model_state_bytes          = sizeof(Shmem::ShortMcombMessage<T_Device, T_Desc>);
          static const size_t sizeof_msg                        = sizeof(Shmem::ShortMcombMessage<T_Device, T_Desc>);
          static const size_t short_msg_cutoff                  = 512;

          inline pami_result_t postMulticombine_impl(uint8_t (&state)[sizeof(Shmem::ShortMcombMessage<T_Device, T_Desc>)],
              pami_multicombine_t *mcomb, void* devinfo)
          {

            PAMI::Topology *src_topo = (PAMI::Topology *)mcomb->data_participants;
            PAMI::Topology *dst_topo = (PAMI::Topology *)mcomb->results_participants;
            size_t num_src_ranks = src_topo->size();
            unsigned local_root = __global.topology_local.rank2Index(dst_topo->index2Rank(0));

            T_Desc *my_desc = NULL, *master_desc = NULL;

              pami_result_t res =	 _device.getShmemWorldDesc(&my_desc, &master_desc, local_root);
              while (res != PAMI_SUCCESS)
              {
                res =	 _device.getShmemWorldDesc(&my_desc, &master_desc, local_root);
                _device.advance();
              }


              TRACE_ERR((stderr, "mcomb->count:%zd\n", mcomb->count));
              size_t bytes = mcomb->count << pami_dt_shift[mcomb->dtype];

              if (bytes <= short_msg_cutoff)
              {

                TRACE_ERR((stderr, "local_root%u my_local_rank:%u\n", local_root, _localrank));
                void* buf = (void*) master_desc->get_buffer(_localrank);
                void* mybuf = ((PAMI::PipeWorkQueue *)mcomb->data)->bufferToConsume();
                memcpy(buf, mybuf, bytes);
                TRACE_ERR((stderr, "copied bytes:%zu from %p to %p data[0]:%u\n", bytes, mybuf, buf, ((unsigned*)buf)[0]));
                ((PAMI::PipeWorkQueue *)mcomb->data)->consumeBytes(bytes);
                my_desc->set_state(Shmem::INIT);

                if (local_root == _localrank)
                  my_desc->set_consumers(num_src_ranks);

#ifdef SHORT_MCOMB_BLOCKING
                master_desc->signal_arrived(); //signal that I have copied all my addresses/data
                res = Shmem::ShortMcombMessage<T_Device, T_Desc>
                  ::short_msg_advance(master_desc, mcomb, _npeers, _localrank, __global.mapping.task());

                if (res == PAMI_SUCCESS)	//signal inline completion
                {
                  mcomb->cb_done.function(_context, mcomb->cb_done.clientdata, PAMI_SUCCESS);
                  my_desc->set_state(Shmem::DONE);
                }

                return PAMI_SUCCESS;
#else


                my_desc->set_mcomb_params(mcomb);
                Shmem::ShortMcombMessage<T_Device, T_Desc> * obj = (Shmem::ShortMcombMessage<T_Device, T_Desc> *) (&state[0]);
                new (obj) Shmem::ShortMcombMessage<T_Device, T_Desc> (_device.getContext(), my_desc, master_desc);

                PAMI::Device::Generic::Device &generic = _device.getProgressDevice();
                generic.postThread(&(obj->_work));
                master_desc->signal_arrived(); //signal that I have copied all my addresses/data
#endif
              }
              else
              {
                /*
                   if (local_root == _localrank)
                   my_desc->set_consumers(num_src_ranks);

                   my_desc->set_mcomb_params(mcomb);
                   void* src_buf = ((PAMI::PipeWorkQueue *)mcomb->data)->bufferToConsume();
                   void* dst_buf = ((PAMI::PipeWorkQueue *)mcomb->results)->bufferToProduce();
                   TRACE_ERR((stderr, "Taking shaddr path local_root%u my_local_rank:%u my_va_src_buf:%p my_va_dst_buf:%p\n",
                   local_root, _localrank, src_buf, dst_buf));

                   Memregion memreg_src;
                   Memregion memreg_dst;
                   Shmem::McombControl* mcomb_control = (Shmem::McombControl*) master_desc->get_buffer();

                   size_t bytes_out;
                   memreg_src.createMemregion(&bytes_out, bytes, src_buf, 0);
                   memreg_dst.createMemregion(&bytes_out, bytes, dst_buf, 0);

                   void* phy_addr = (void*)memreg_src.getBasePhysicalAddress();
                   void * global_vaddr = NULL;
                   uint32_t rc = 0;
                   rc = Kernel_Physical2GlobalVirtual (phy_addr, &global_vaddr);
                   assert(rc == 0);
                   mcomb_control->GlobalAddressTable.src_bufs[_localrank] = global_vaddr;
                   TRACE_ERR((stderr, "src buffer info..[%d]phy_addr:%p set my global src address:%p \n", _localrank, phy_addr, global_vaddr));
                   TRACE_ERR((stderr, "src buffer[0] via global VA:%f \n", ((double*)global_vaddr)[0]));

                   phy_addr = (void*)memreg_dst.getBasePhysicalAddress();
                   global_vaddr = NULL;
                   rc = 0;
                   rc = Kernel_Physical2GlobalVirtual (phy_addr, &global_vaddr);
                   assert(rc == 0);
                   mcomb_control->GlobalAddressTable.dst_bufs[_localrank] = global_vaddr;
                   TRACE_ERR((stderr, "dst buffer info..[%d]phy_addr:%p set my global dst address:%p \n", _localrank, phy_addr, global_vaddr));
                   TRACE_ERR((stderr, "dst buffer[0] via global VA:%f \n", ((double*)global_vaddr)[0]));

                   if (local_root == _localrank)
                   {
                   my_desc->set_consumers(num_src_ranks);

                   }

                   mcomb_control->chunks_done[_localrank] = 0;
                   mcomb_control->chunks_copied[_localrank] = 0;
                   TRACE_ERR((stderr, "[%d]setting my chunks_done:%p to 0\n", _localrank, &mcomb_control->chunks_done[_peer]));
                //master_desc->set_global_address_table(buf_src, buf_dst, mcomb->bytes,_context);
                my_desc->set_state(Shmem::INIT);
                //src->consumeBytes(bytes);

                Shmem::McombMessageShaddr<T_Device, T_Desc> * obj = (Shmem::McombMessageShaddr<T_Device, T_Desc> *) (&state[0]);
                new (obj) Shmem::McombMessageShaddr<T_Device, T_Desc> (&_device, my_desc, master_desc);
                _device.post(obj);
                master_desc->signal_arrived(); //signal that I have copied all my addresses/data
                */


            }
              return PAMI_SUCCESS;

          };


        protected:

          T_Device      & _device;
          pami_context_t   _context;
          unsigned _localrank;
          unsigned _npeers;

      };  // PAMI::Device::Shmem::ShmemMcombModelWorld class


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
