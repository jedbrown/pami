/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemDevice_impl.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemDevice_impl_h__
#define __components_devices_shmem_ShmemDevice_impl_h__

#include "Global.h"
#include "SysDep.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    template <class T_Fifo>
    xmi_result_t ShmemDevice<T_Fifo>::init (size_t clientid,
                                            size_t contextid,
                                            xmi_client_t     client,
                                            xmi_context_t    context,
                                            SysDep         * sysdep,
                                            XMI::Device::Generic::Device * progress)
    {
      TRACE_ERR((stderr, "(%zd) ShmemDevice::init ()\n", __global.mapping.task()));
      _client   = client;
      _context  = context;
      _contextid  = contextid;
      _sysdep   = sysdep;
      _progress = progress;

      unsigned i;
      __global.mapping.nodePeers (_num_procs);

      XMI::Interface::Mapping::nodeaddr_t nodeaddr;
      __global.mapping.nodeAddr (nodeaddr);
      _global_task = nodeaddr.global;
      _local_task  = nodeaddr.local;
#ifdef __bgq__
      unsigned stride = 16 / _num_procs; //hack
      _local_task = _local_task / stride;//hack
#endif

      new (_rfifo) T_Fifo ();
      _rfifo->init (*sysdep);

      // barrier ?

      // Allocate memory for and construct the queue objects,
      // one for each context on the local node
      __sendQ = (Shmem::SendQueue *) malloc ((sizeof (Shmem::SendQueue) * _total_fifos));

      for (i = 0; i < _total_fifos; i++)
        {
          new (&__sendQ[i]) Shmem::SendQueue (_progress, _contextid);
        }

      // Initialize the registered receive function array to noop().
      // The array is limited to 256 dispatch ids because of the size of the
      // dispatch id field in the packet header.
      for (i = 0; i < DISPATCH_SET_COUNT*DISPATCH_SET_SIZE; i++)
        {
          _dispatch[i].function   = noop;
          _dispatch[i].clientdata = NULL;
        }


      return XMI_SUCCESS;
    }

    template <class T_Fifo>
    bool ShmemDevice<T_Fifo>::isInit_impl ()
    {
      return true;
    }

    /// \see XMI::Device::Interface::BaseDevice::peers()
    template <class T_Fifo>
    size_t ShmemDevice<T_Fifo>::peers_impl ()
    {
      return _num_procs;
    }

    /// \see XMI::Device::Interface::BaseDevice::task2peer()
    template <class T_Fifo>
    size_t ShmemDevice<T_Fifo>::task2peer_impl (size_t task)
    {
      XMI::Interface::Mapping::nodeaddr_t address;
      TRACE_ERR((stderr, ">> ShmemDevice::task2peer_impl(%zu)\n", task));
      __global.mapping.task2node (task, address);
      TRACE_ERR((stderr, "   ShmemDevice::task2peer_impl(%zu), address = {%zu, %zu}\n", task, address.global, address.local));

      size_t peer = 0;
      __global.mapping.node2peer (address, peer);
      TRACE_ERR((stderr, "<< ShmemDevice::task2peer_impl(%zu), peer = %zu\n", task, peer));

#ifdef __bgq__
      return task; //hack
#else
      return peer;
#endif
    }

    ///
    /// \brief Regieter the receive function to dispatch when a packet arrives.
    ///
    /// \param[in] id              Dispatch set identifier
    /// \param[in] recv_func       Receive function to dispatch
    /// \param[in] recv_func_parm  Receive function client data
    ///
    /// \return Dispatch id for this registration
    ///
    template <class T_Fifo>
    xmi_result_t ShmemDevice<T_Fifo>::registerRecvFunction (size_t                      set,
                                                            Interface::RecvFunction_t   recv_func,
                                                            void                      * recv_func_parm,
                                                            uint16_t                  & id)
    {
      TRACE_ERR((stderr, ">> (%zd) ShmemDevice::registerRecvFunction(%d,%p,%p) .. DISPATCH_SET_COUNT = %d\n", __global.mapping.task(), set, recv_func, recv_func_parm, DISPATCH_SET_COUNT));

      // This device only supports up to 256 dispatch sets.
      if (set >= DISPATCH_SET_COUNT) return XMI_ERROR;

      // Find the next available id for this dispatch set.
      bool found_free_slot = false;
      size_t n = set * DISPATCH_SET_SIZE + DISPATCH_SET_COUNT;

      for (id = set * DISPATCH_SET_SIZE; id < n; id++)
        {
          TRACE_ERR((stderr, "   (%zd) ShmemDevice::registerRecvFunction(), _dispatch[%d].function= %p\n", __global.mapping.task(), id, _dispatch[id].function));

          if (_dispatch[id].function == (Interface::RecvFunction_t) noop)
            {
              found_free_slot = true;
              break;
            }
        }

      if (!found_free_slot) return XMI_ERROR;

      _dispatch[id].function   = recv_func;
      _dispatch[id].clientdata = recv_func_parm;

      TRACE_ERR((stderr, "<< (%zd) ShmemDevice::registerRecvFunction() => %d\n", __global.mapping.task(), id));
      return XMI_SUCCESS;
    };

    template <class T_Fifo>
    xmi_result_t ShmemDevice<T_Fifo>::post (size_t fnum, Shmem::SendQueue::Message * msg)
    {
      TRACE_ERR((stderr, ">> (%zd) ShmemDevice::post(%zu, %p)\n", __global.mapping.task(), fnum, msg));
      msg->setup (_progress, &__sendQ[fnum]);
      msg->postNext(true);
      TRACE_ERR((stderr, "<< (%zd) ShmemDevice::post(%zu, %p)\n", __global.mapping.task(), fnum, msg));
      return XMI_SUCCESS;
    };

    template <class T_Fifo>
    int ShmemDevice<T_Fifo>::noop (void   * metadata,
                                   void   * payload,
                                   size_t   bytes,
                                   void   * recv_func_parm,
                                   void   * cookie)
    {
      XMI_abort();
      return 0;
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
