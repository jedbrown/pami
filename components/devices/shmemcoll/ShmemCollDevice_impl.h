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
    template <class T_Atomic>
      pami_result_t ShmemCollDevice<T_Atomic>::init (size_t clientid,
          size_t contextid,
          pami_client_t     client,
          pami_context_t    context,
          PAMI::Memory::MemoryManager *mm,
          PAMI::Device::Generic::Device * progress)
      {
        TRACE_ERR((stderr, " (%zu) >> ShmemCollDevice::init ()  \n", __global.mapping.task()));

        unsigned i;
        __global.mapping.nodePeers (_num_procs);

        PAMI::Interface::Mapping::nodeaddr_t nodeaddr;
        __global.mapping.nodeAddr (nodeaddr);
        _global_task = __global.mapping.task();//nodeaddr.global;
        _local_task  = nodeaddr.local;
        TRACE_ERR((stderr, "(%zu) ShmemCollDevice::global_task:%zu local_task:%zu () << \n", __global.mapping.task(), _global_task, _local_task));

        for (i = 0; i < MATCH_DISPATCH_SIZE; i++)
        {
          _dispatch[i].function   = NULL;
          _dispatch[i].clientdata = NULL;
        }

        TRACE_ERR((stderr, "(%zu) ShmemCollDevice::init () << \n", __global.mapping.task()));
        return PAMI_SUCCESS;
      }

    template <class T_Atomic>
      bool ShmemCollDevice<T_Atomic>::isInit_impl ()
      {
        return true;
      }

    /// \see PAMI::Device::Interface::BaseDevice::peers()
    template <class T_Atomic>
      size_t ShmemCollDevice<T_Atomic>::peers_impl ()
      {
        return _num_procs;
      }

    /// \see PAMI::Device::Interface::BaseDevice::task2peer()
    template <class T_Atomic>
      size_t ShmemCollDevice<T_Atomic>::task2peer_impl (size_t task)
      {
        PAMI::Interface::Mapping::nodeaddr_t address;
        TRACE_ERR((stderr, ">> ShmemCollDevice::task2peer_impl(%zu)\n", task));
        __global.mapping.task2node (task, address);
        TRACE_ERR((stderr, "   ShmemCollDevice::task2peer_impl(%zu), address = {%zu, %zu}\n", task, address.global, address.local));

        size_t peer = 0;
        __global.mapping.node2peer (address, peer);
        TRACE_ERR((stderr, "<< ShmemCollDevice::task2peer_impl(%zu), peer = %zu\n", task, peer));

        return peer;
      }

    /// \see PAMI::Device::Interface::BaseDevice::isPeer()
    template <class T_Atomic>
      bool ShmemCollDevice<T_Atomic>::isPeer_impl (size_t task)
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
    template <class T_Atomic>
      pami_result_t ShmemCollDevice<T_Atomic>::registerMatchDispatch ( Interface::MatchFunction_t   match_func,
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


    template <class T_Atomic>
      inline pami_result_t ShmemCollDevice<T_Atomic>::getShmemWorldDesc(Shmem::ShmemCollDesc<T_Atomic>  ** desc)
      {
        unsigned desc_index;
        Shmem::ShmemCollDesc<T_Atomic>* next_free_desc = _desc_fifo.next_free_descriptor(desc_index);

        if (likely(next_free_desc != NULL))
        {
          if (likely(next_free_desc->get_my_seq_id()  == next_free_desc->get_seq_id()))
          {
            *desc = _desc_fifo.fetch_descriptor();
            TRACE_ERR((stderr, "Found descriptor pair \n"));
            return PAMI_SUCCESS;
          }
        }
        return PAMI_EAGAIN;
      }
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
